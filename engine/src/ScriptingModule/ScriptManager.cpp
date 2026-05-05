#include "ScriptManager.h"

// ----- LUA -----
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

// ------- LUA_BRIDGE -------
#include <LuaBridge/LuaBridge.h>

#include <CCamera.h>
#include <ComponentFactory.h>

#include "ComponentArguments.h"
#include "LuaManager.h"

#include "FluxError.h"
#include "checkML.h"
#include "SceneManager.h"

#include "RenderManager.h"
#include "RenderSceneManager.h"
#include "RenderScene.h"
#include "UIManager.h"

#include "Backends/IRenderSceneBackend.h"

flux_script::ScriptManager::ScriptManager()
{
	auto mngr = LuaManager::instance();
}

flux_script::ScriptManager::~ScriptManager()
{
	LuaManager::instance()->close();
}

bool flux_script::ScriptManager::init()
{
	bool ok = LuaManager::instance()->init();
    if (!ok) {
        throwFluxError(false, "No se pudo inicializar el ScriptManager");
    }
	return ok;
}

void flux_script::ScriptManager::update(float dt)
{
	(void)dt;
}

bool flux_script::ScriptManager::shutdown()
{
	bool ok = LuaManager::instance()->shutdown();
    if (!ok)
    {
        throwFluxError(false, "No se pudo cerrar el ScriptManager");
    }
    return ok;
}

bool flux_script::ScriptManager::loadScene(const std::string& sceneName)
{
    bool ok = flux_utils::SceneManager::instance()->setActiveScene(sceneName);
    if (!ok) {
        throwFluxError(false, "No se pudo activar la escena '" + sceneName + "'");
        return false;
    }

    auto* renderManager = flux_render::RenderManager::instance();
    auto* sceneBackend = renderManager->getSceneBackend();

    if (sceneBackend == nullptr) {
        throwFluxError(false, "No existe SceneBackend para activar la escena '" + sceneName + "'");
        return false;
    }

    if (!sceneBackend->setCurrentScene(sceneName)) {
        throwFluxError(false, "No se pudo activar la escena de render ' " + sceneName + "'");
        return false;
    }

    auto* uiManager = renderManager->getUIManager();
    if (uiManager != nullptr) {
        uiManager->setSceneActive(sceneName);
    }

    // Buscar cmaras pendientes y crearlas
    auto entities = flux_utils::SceneManager::instance()->getEntities();
    for (auto& [name, ent] : entities) {
        if (ent->hasComponent(CAMERA)) {
            auto comp = ent->getComponent(CAMERA);
            auto* cameraComp = static_cast<flux_ec::CCamera*>(comp);
            if (cameraComp->isPendingCreation()) cameraComp->createCamera();
        }
    }

    return true;
}

bool flux_script::ScriptManager::readScene(const std::string& sceneName)
{
	bool ok = LuaManager::instance()->loadScript(sceneName);
    if (!ok) {
        throwFluxError(false, "No se pudo cargar el script en el ScriptManager correctamente");
    }

	lua_State* l = LuaManager::instance()->getLuaState();
    if (!l) {
        throwFluxError(false, "No se pudo obtener el estado de Lua");
    }

	return parseScene(sceneName, l);
}

bool flux_script::ScriptManager::readScript(const std::string& scriptName)
{
	bool ok = LuaManager::instance()->loadScript(scriptName);
    if (!ok) {
        throwFluxError(false, "No se pudo leer el script en el ScriptManager");
    }
	return ok;
}

FLUX_API bool flux_script::ScriptManager::readPrefabs(const std::string& scriptName)
{
    bool ok = LuaManager::instance()->loadScript(scriptName);
    if (!ok) {
        throwFluxError(false, "No se pudo leer el archivo Prefab");
    }

    lua_State* l = LuaManager::instance()->getLuaState();
    if (!l) {
        throwFluxError(false, "No se pudo obtener el estado de Lua");
    }

    return parsePrefabs(l);
}

 bool flux_script::ScriptManager::parsePrefabs(lua_State* l)
{
    lua_getglobal(l, "Prefabs");

    if (!lua_istable(l, -1))
    {
        lua_pop(l, 1);
        return false;
    }
    int n = (int)lua_rawlen(l, -1);
    for (int i = 1; i <= n; ++i) {
        lua_rawgeti(l, -1, i); // pushea Entities[i]
        if (lua_istable(l, -1)) {
            // 1) Leer Id (int)
            lua_getfield(l, -1, "Id");
            int entId = 0;
            if (lua_isnumber(l, -1)) {
                entId = (int)lua_tointeger(l, -1);
            }
            lua_pop(l, 1);

            // 2) Leer Name (string)
            lua_getfield(l, -1, "Name");
            std::string entName;
            if (lua_isstring(l, -1)) {
                entName = lua_tostring(l, -1);
            }
            lua_pop(l, 1);

            // Crear la entidad en tu motor
            flux_ec::Entity* newEnt = new flux_ec::Entity(entId, entName);

            // 3) Leer la subtabla "Components"
            lua_getfield(l, -1, "Components");
            if (lua_istable(l, -1)) {
                int numComponents = (int)lua_rawlen(l, -1);
                for (int c = 1; c <= numComponents; ++c) {
                    lua_rawgeti(l, -1, c); // pushea la tabla del componente

                    if (lua_istable(l, -1)) {
                        // 3.1) Leer "Name" (tipo de componente: "Transform", "Camera", ...)
                        lua_getfield(l, -1, "Name");
                        std::string compName;
                        if (lua_isstring(l, -1)) {
                            compName = lua_tostring(l, -1);
                        }
                        lua_pop(l, 1);

                        // 3.2) Leer subtabla "Arguments"
                        lua_getfield(l, -1, "Arguments");
                        // parsear key->value en un map
                        std::unordered_map<std::string, std::string> argsMap;
                        if (lua_istable(l, -1)) {
                            lua_pushnil(l);
                            while (lua_next(l, -2) != 0) {
                                // key en -2, value en -1
                                std::string key = lua_tostring(l, -2);
                                std::string value = lua_tostring(l, -1);
                                argsMap[key] = value;
                                lua_pop(l, 1);
                            }
                        }
                        lua_pop(l, 1); // pop de la subtabla "Arguments"

                        // 3.3) Construir un ComponentArguments
                        flux_script::ComponentArguments compArgs;
                        for (auto& kv : argsMap) {
                            compArgs.setArg(kv.first, kv.second);
                        }

                        // 3.5) Crear el componente con ComponentFactory
                        flux_ec::ComponentFactory* factory = flux_ec::ComponentFactory::getInstance();
                        flux_ec::Component* newComp = factory->createComponentByName(compName);
                        if (newComp) {
                            // Asignarle la entidad para su constructor
                            newComp->setOwner(newEnt); // O si usas constructor con (Entity*)
                            // 3.6) Llamar a init(...) con compArgs
                            newComp->init(&compArgs);
                            // 3.7) A�adirlo a la entidad
                            newEnt->addComponent(newComp);
                        }
                    }

                    lua_pop(l, 1); // pop de la tabla del componente
                }
            }
            lua_pop(l, 1); // pop de "Components"

            flux_utils::SceneManager::instance()->addPrefabs(entName, newEnt);
        }
    }
    return true;
}

bool flux_script::ScriptManager::parseScene(const std::string& sceneName, 
    lua_State* L)
{
    flux_utils::SceneManager::instance()->createScene(sceneName, false);

    auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();
    if (sceneBackend == nullptr) {
        throwFluxError(false, "No existe SceneBackend para crear la escena '" + sceneName + "'");
        return false;
    }

    if (!sceneBackend->createScene(sceneName)) {
        throwFluxError(false, "No se pudo crear la escena de render '" + sceneName + "'");
        return false;
    }

    lua_getglobal(L, "Entities");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }

    int n = (int)lua_rawlen(L, -1);
    for (int i = 1; i <= n; ++i) {  // desde 1 hasta n
        lua_rawgeti(L, -1, i); // pushea Entities[i]

        if (lua_istable(L, -1)) {
            // 1) Leer Id (int)
            lua_getfield(L, -1, "Id");
            int entId = 0;
            if (lua_isnumber(L, -1)) {
                entId = (int)lua_tointeger(L, -1);
            }
            lua_pop(L, 1);

            // 2) Leer Name (string)
            lua_getfield(L, -1, "Name");
            std::string entName;
            if (lua_isstring(L, -1)) {
                entName = lua_tostring(L, -1);
                if (flux_utils::SceneManager::instance()->searchNameEntity(sceneName,entName)) {
                    throwFluxError(false, "El nombre de entidad " + entName + "esta repetido");
                }
            }
            lua_pop(L, 1);

            // Crear la entidad en tu motor
            flux_ec::Entity* newEnt = new flux_ec::Entity(entId, entName);

            // 3) Leer la subtabla "Components"
            lua_getfield(L, -1, "Components");
            if (lua_istable(L, -1)) {
                int numComponents = (int)lua_rawlen(L, -1);
                for (int c = 1; c <= numComponents; ++c) {
                    lua_rawgeti(L, -1, c); // pushea la tabla del componente

                    if (lua_istable(L, -1)) {
                        // 3.1) Leer "Name" (tipo de componente: "Transform", "Camera", ...)
                        lua_getfield(L, -1, "Name");
                        std::string compName;
                        if (lua_isstring(L, -1)) {
                            compName = lua_tostring(L, -1);
                        }
                        lua_pop(L, 1);

                        // 3.2) Leer subtabla "Arguments"
                        lua_getfield(L, -1, "Arguments");
                        // parsear key->value en un map
                        std::unordered_map<std::string, std::string> argsMap;
                        if (lua_istable(L, -1)) {
                            lua_pushnil(L);
                            while (lua_next(L, -2) != 0) {
                                // key en -2, value en -1
                                std::string key = lua_tostring(L, -2);
                                std::string value = lua_tostring(L, -1);
                                argsMap[key] = value;
                                lua_pop(L, 1);
                            }
                        }
                        lua_pop(L, 1); // pop de la subtabla "Arguments"

                        // 3.3) Construir un ComponentArguments
                        flux_script::ComponentArguments compArgs;
                        for (auto& kv : argsMap) {
                            compArgs.setArg(kv.first, kv.second);
                        }

                        // 3.5) Crear el componente con ComponentFactory
                        flux_ec::ComponentFactory* factory = flux_ec::ComponentFactory::getInstance();
                        flux_ec::Component* newComp = factory->createComponentByName(compName);

                        //Si el componente esta registrado en la factoria
                        if (newComp) {

                            //Si ya tiene el componente, sale de la incializaci�n
                            if (newEnt->hasComponent(newComp->getType())) {
                                delete newEnt;
                                delete newComp;
                                throwFluxError(false, "La entidad " + entName + " ya tiene el componente" + compName);
                            }

                            // Asignarle la entidad para su constructor
                            newComp->setOwner(newEnt); // O si usas constructor con (Entity*)
                            // 3.6) Asignar la escena donde crear
                            newEnt->setSceneID(sceneName);
                            // 3.7) Llamar a init(...) con compArgs
                            if (!newComp->init(&compArgs)) {
                                //Liberamos la memoria dinamica creada en esta parte del metodo en caso de error para evitar fugas de memoria
                                //La otra memoria o componentes creados anteriormente y que se han seteado en la escena se liberan en SceneManager::shutdown()
                                delete newEnt;
                                delete newComp;
                                //Abandonamos el metodo de lectura con la excepcion
                                throwFluxError(false, "No se pudo inicializar el componente " + compName + " correctamente.");
                            }

                            // 3.8) A�adirlo a la entidad
                            newEnt->addComponent(newComp);
                        }
                        //Si el componente no esta registrado en la factoria, entonces borramos al entidad creada
                        else {
                            delete newEnt;
                            throwFluxError(false, "Nombre de componente " + compName + " invalido");
                        }
                    }

                    lua_pop(L, 1); // pop de la tabla del componente
                }
            }
            lua_pop(L, 1); // pop de "Components"

            // 4) Finalmente, guardar la entidad en tu contenedor
            flux_utils::SceneManager::instance()->addEntity(sceneName, entName, newEnt);
            // O directamente en un vector<flux_ec::Entity*> en tu motor
            // (en tu main, o un EntityManager, etc.)
            // e.g. g_entities.push_back(newEnt);
        }
        lua_pop(L, 1); // pop de Entities[i]
    }

    lua_pop(L, 1); // pop de "Entities"
    return true;
}


bool flux_script::ScriptManager::parseEntities(lua_State* l)
{
	return false;
}
