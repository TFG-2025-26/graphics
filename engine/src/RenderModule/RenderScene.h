#pragma once

#ifndef RENDER_SCENE_H_
#define RENDER_SCENE_H_

#include <string>
#include <unordered_map>

namespace Ogre {
    class SceneManager;
    class SceneNode;
}

namespace flux_utils {
    class Vector3;
}

namespace flux_render {
    class RenderObject;

    class RenderScene {
    public:
        RenderScene(const std::string& sceneID, 
            Ogre::SceneManager* sceneManager, Ogre::SceneNode* node);
        virtual ~RenderScene();

        void createSceneObject(const std::string& entityID);
        void addChildToObject(const std::string& childEntityID,
            const std::string& parentEntityID);

        RenderObject* getRenderObject(const std::string& entityID) const;

        void deleteSceneObject(const std::string& entityID);
        void deleteSceneObjectRecursive(const std::string& entityID);
        void deleteSceneObjectReparent(const std::string& entityID);

        bool createCamera(const std::string& entityID, 
            const int8_t& nearDist, const int16_t& farDist);
        bool createLight(const std::string& entityID,
            const flux_utils::Vector3& diffuseColor,
            const uint8_t& lightType);
        bool createMesh(const std::string& entityID,
            const std::string& meshName, 
            const std::string& materialName);
        bool createAnimation(const std::string& entityID,
            const std::string& animationName);
        void deleteAnimation(const std::string& entityID,
            const std::string& animationName);
        void setAnimationEnabled(const std::string& entityID,
            const std::string& animationName, bool enabled);
        void setAnimationLoop(const std::string& entityID,
            const std::string& animationName, bool loop);
        void updateAnimations(const std::string& entityID,
            float dt);

        std::vector<Ogre::SceneNode*> getChildrenNodes(Ogre::SceneNode* node);
        std::vector<std::string> getChildrenNames(Ogre::SceneNode* node);

        Ogre::SceneManager* getSceneManager() const;
        Ogre::SceneNode* getSceneNode() const;
        void setOgreNodeVisible(bool visible);
        void clearEntities(const std::string& entityID);
    private:
        std::string _sceneID;
        Ogre::SceneManager* _sceneManager;
        Ogre::SceneNode* _sceneNode;

        std::unordered_map<std::string, RenderObject*> _entities;
    };
}

#endif // RENDER_SCENE_H_