#pragma once

#ifndef OGRE_SCENE_BACKEND_H_
#define OGRE_SCENE_BACKEND_H_

#include "IRenderSceneBackend.h"

class OgreBackend;

namespace flux_render {
    class RenderSceneManager;
    class RenderScene;
    class UIManager;

    class OgreSceneBackend : public IRenderSceneBackend {
    public:
        OgreSceneBackend(OgreBackend* renderBackend, UIManager* uiManager);
        ~OgreSceneBackend() override = default;

        bool init() override;
        void shutdown() override;

        bool createScene(const std::string& sceneID) override;
        bool destroyScene(const std::string& sceneID) override;
        bool setCurrentScene(const std::string& sceneID) override;

        bool createSceneObject(
            const std::string& sceneID,
            const std::string& entityID) override;

        bool destroySceneObject(
            const std::string& sceneID,
            const std::string& entityID) override;

        bool setObjectTransform(
            const std::string& sceneID,
            const std::string& entityID,
            const flux_utils::Vector3& position,
            const flux_utils::Vector4& rotation,
            const flux_utils::Vector3& scale) override;

        bool attachMesh(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& meshName,
            const std::string& materialName) override;

        bool createLight(
            const std::string& sceneID,
            const std::string& entityID,
            const flux_utils::Vector3& diffuseColor,
            uint8_t lightType) override;

        bool createCamera(
            const std::string& sceneID,
            const std::string& entityID,
            int8_t nearDist,
            int16_t farDist) override;

        bool addAnimation(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName) override;

        bool removeAnimation(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName) override;

        bool setAnimationEnabled(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName,
            bool enabled) override;

        bool setAnimationLoop(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName,
            bool loop) override;

        bool updateAnimation(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName,
            float dt) override;

        bool isCurrentScene(const std::string& sceneID) const override;

        RenderSceneManager* getSceneManager() const;

    private:
        RenderScene* getScene(const std::string& sceneID) const;

        OgreBackend* _renderBackend = nullptr;
        UIManager* _uiManager = nullptr;
        RenderSceneManager* _sceneManager = nullptr;

    protected:
        std::string resolveMeshName(const std::string& logicalMeshName) const override;
    };
}

#endif // OGRE_SCENE_BACKEND_H_
