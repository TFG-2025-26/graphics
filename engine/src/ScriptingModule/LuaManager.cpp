#include "LuaManager.h"

extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

#include <LuaBridge/LuaBridge.h>
#include "FluxError.h"

using namespace flux_script;

LuaManager::LuaManager() : _l(nullptr)
{
}

LuaManager::~LuaManager()
{
	if (_l) {
		lua_close(_l);
		_l = nullptr;
	}
}

bool LuaManager::init()
{
	if (!isInitialized) {
		_l = luaL_newstate();
		if (_l == nullptr) {
			throwFluxError(false, "Error al crear un nuevo estado de LUA.");
			return false; // Aunque el throw interrumpe la ejecución, se añade para calmar la advertencia.
		}
		luaL_openlibs(_l);
		// registerClasses();
		isInitialized = true;
		return true;
	}
	// Si ya está inicializado, puedes decidir retornar false o true, según tu lógica.
	return true;
}

bool LuaManager::shutdown()
{
	if (isInitialized) {
		if (_l) {
			lua_close(_l);
			_l = nullptr;
			isInitialized = false;
			return true;
		}
		else {
			throwFluxError(false, "Error al cerrar el estado de LUA.");
			return false; // Añadido para garantizar un retorno.
		}
	}
	else {
		throwFluxError(false, "No se puede cerrar un módulo que no está inicializado.");
		return false; // Añadido para garantizar un retorno.
	}
}


void LuaManager::update(float dt)
{
	(void)dt;
}

template <class T>
void flux_script::LuaManager::registerClasses(const std::string& luaName)
{
	luabridge::getGlobalNamespace(_l)
		.beginClass<T>(luaName.c_str())
		// .addFunction("loQueSea", &MiClase::loQueSea)
		.endClass();

	// lua_register(_l, "miFuncionGlobal", &MiFuncionNativa);
}

bool flux_script::LuaManager::loadScript(const std::string& filename)
{
	std::string path = "assets/scenes/" + filename + ".lua";
	if (luaL_dofile(_l, path.c_str()) != LUA_OK) {
		const char* error = lua_tostring(_l, -1);
		return false;
	}

	return true;
}

lua_State* flux_script::LuaManager::getLuaState()
{
	return _l;
}
