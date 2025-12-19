#pragma once

#ifndef SCENE_MANAGER_H_
#define SCENE_MANAGER_H_

#include "Manager.h"
#include "Singleton.h"

#include <unordered_map>
#include <string>
#include <list>


namespace flux_ec {
    class Entity;
}

namespace flux_render {
    class RenderManager;
    class RenderSceneManager;
}


namespace flux_utils {
    using entitiesMap = std::unordered_map<std::string, flux_ec::Entity*>;
    class SceneManager : public Manager, public Singleton<SceneManager>
    {
    public:
        friend class Singleton<SceneManager>;

        SceneManager();
        virtual ~SceneManager();

        bool init() override;
        void update(float dt) override;
        bool shutdown() override;

        bool createScene(const std::string& sceneID,
            bool setAsCurrent = false);
        FLUX_API bool destroyScene(const std::string& sceneID);
        FLUX_API bool clearScene(const std::string& sceneID);
        bool setActiveScene(const std::string& sceneID);
        FLUX_API bool sceneExists(const std::string& sceneID);
        FLUX_API std::string getActiveScene();
        FLUX_API std::unordered_map<std::string, flux_ec::Entity*> getEntities();
        FLUX_API void addEntity(const std::string& sceneID, 
            const std::string& entityID, flux_ec::Entity* e);
        FLUX_API bool removeEntity(const std::string& sceneID, 
            const std::string& entityID);
        FLUX_API bool searchNameEntity(const std::string& sceneID,std::string entName);

        bool addPrefabs(const std::string& prefabsName, flux_ec::Entity* ent);
        flux_ec::Entity* getPrefab(const std::string& prefabsName)const;

        FLUX_API flux_ec::Entity* getEntity(const std::string& sceneID,
            const std::string& entityID) const;
        FLUX_API void queueSceneChange(const std::string& newScene);

        FLUX_API void processPendingSceneChange();

    private:
        std::unordered_map<std::string, entitiesMap> _scenesMap;
        std::unordered_map<std::string, flux_ec::Entity*> _prefabsMap;

        std::string _activeScene;

        std::list<std::pair<std::string,std::string>> borrar;
        std::string _sceneToLoadPending = "";

    };
}

#endif // SCENE_MANAGER_H_