#include "SceneManager.h"
#include "Entity.h"
#include "ScriptManager.h"
#include "RenderManager.h"
#include "RenderSceneManager.h"


flux_utils::SceneManager::SceneManager() : _activeScene("")
{

}

flux_utils::SceneManager::~SceneManager()
{
    // _sceneMap.clear();
}

bool flux_utils::SceneManager::init()
{
    return true;
}

bool flux_utils::SceneManager::shutdown()
{
    // Borramos todas las escenas y sus entidades
    auto it = _scenesMap.begin();
    while (it != _scenesMap.end()) {
        for (auto& e : it->second) {
            delete e.second;
        }
        it = _scenesMap.erase(it);
    }
    _scenesMap.clear();

    return true;
}

void flux_utils::SceneManager::update(float dt)
{
    if (!_activeScene.empty()) {
        auto it = _scenesMap.find(_activeScene);
        if (it != _scenesMap.cend()) {
            for (auto &e : it->second) {
                e.second->update(dt);
            }
        }
    }

    if (!borrar.empty()) {
        auto it = borrar.begin();
        for (it; it != borrar.end(); it++) {
            auto it2 = _scenesMap.find(it->first);

            if (it2 != _scenesMap.cend()) {
                auto it3 = it2->second.find(it->second);
                if (it3 != it2->second.cend()) {
                    delete it3->second;
                    it2->second.erase(it3);
                }
                // else throwFluxError(...);
            }
        }

        borrar.clear();
    }
}

bool flux_utils::SceneManager::createScene(const std::string& sceneID,
    bool setAsCurrent)
{
    auto it = _scenesMap.find(sceneID);

    if (it == _scenesMap.cend()) {
        entitiesMap newEntitiesMap;
        _scenesMap.insert({ sceneID, newEntitiesMap });
        
        if (_activeScene.empty() || setAsCurrent) _activeScene = sceneID;
    }
    // else throwFluxError(...);

    return true;
}

bool flux_utils::SceneManager::destroyScene(const std::string& sceneID)
{
    auto it = _scenesMap.find(sceneID);

    if (it != _scenesMap.cend()) {
        for (auto& e : it->second) {
            delete e.second;
        }

        _scenesMap.erase(it);
    }
    // else throwFluxError(...);

    return true;
}


bool flux_utils::SceneManager::setActiveScene(const std::string& sceneId)
{
    auto it = _scenesMap.find(sceneId);

    if (it == _scenesMap.cend()) return false;
    _activeScene = sceneId;

    return true;
}

bool flux_utils::SceneManager::sceneExists(const std::string& sceneId)
{
    auto it = _scenesMap.find(sceneId);
    return it != _scenesMap.cend();
}

std::string flux_utils::SceneManager::getActiveScene() {
    return _activeScene;
}

std::unordered_map<std::string, flux_ec::Entity*> flux_utils::SceneManager::getEntities()
{
    return _scenesMap[_activeScene];
}

void flux_utils::SceneManager::addEntity(const std::string& sceneID, 
    const std::string& entityID, flux_ec::Entity* e)
{
    auto it = _scenesMap.find(sceneID);

    if (it != _scenesMap.cend()) {
        auto it2 = it->second.find(entityID);
        if (it2 == it->second.cend()) it->second.insert({ entityID, e });
        // else throwFluxError(...);
    }
}

bool flux_utils::SceneManager::removeEntity(const std::string& sceneID, 
    const std::string& entityID)
{

    borrar.push_back({ sceneID,entityID });

    return true;
}

bool flux_utils::SceneManager::addPrefabs(const std::string& prefabsName, flux_ec::Entity* ent)
{
    auto it = _prefabsMap.find(prefabsName);
    if (it == _prefabsMap.cend())
    {
        _prefabsMap.insert({ prefabsName,ent });
    }
    else
        return false;
    return true;
}

flux_ec::Entity* flux_utils::SceneManager::getPrefab(const std::string& prefabsName)const
{
    auto it = _prefabsMap.find(prefabsName);
    if (it == _prefabsMap.cend())
    {
        return nullptr;
    }
    return it->second;
}


flux_ec::Entity* flux_utils::SceneManager::getEntity(const std::string& sceneID, 
    const std::string& entityID) const
{
    auto it = _scenesMap.find(sceneID);
    if (it != _scenesMap.cend()) {
        auto it2 = it->second.find(entityID);
        if (it2 != it->second.cend()) return it2->second;
    }

    return nullptr;
}

void flux_utils::SceneManager::queueSceneChange(const std::string& newScene) {
    _sceneToLoadPending = newScene;
}

void flux_utils::SceneManager::processPendingSceneChange() {
    if (_sceneToLoadPending != "") {

        std::string oldScene = _activeScene;
        _activeScene = _sceneToLoadPending;
        flux_script::ScriptManager::instance()->readScene(_sceneToLoadPending);
        flux_script::ScriptManager::instance()->loadScene(_sceneToLoadPending);
        destroyScene(oldScene);
        
        auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();
        if (sceneBackend != nullptr) {
            sceneBackend->destroyScene(oldScene);
        }

        _sceneToLoadPending = "";
    }
}



bool flux_utils::SceneManager::searchNameEntity(const std::string& sceneID,std::string entName) {


    auto it = _scenesMap.find(sceneID);
    if (it != _scenesMap.cend()) {
        auto it2 = it->second.find(entName);

        return it2 != it->second.cend();
    }
    
    return false;
}
