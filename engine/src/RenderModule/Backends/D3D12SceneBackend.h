#pragma once

#ifndef D3D12_SCENE_BACKEND_H_
#define D3D12_SCENE_BACKEND_H_

#include "IRenderSceneBackend.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace flux_render {
    class D3D12Backend;

    class D3D12SceneBackend : public IRenderSceneBackend {
    public:
        explicit D3D12SceneBackend(D3D12Backend* renderBackend);
        ~D3D12SceneBackend() override = default;

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

        bool hasScene(const std::string& sceneID) const;
        bool hasSceneObject(const std::string& sceneID, const std::string& entityID) const;
        const std::string& getCurrentSceneID() const;

    private:
        D3D12Backend* _renderBackend = nullptr;
        std::unordered_map<std::string, std::unordered_set<std::string>> _scenes;
        std::string _currentSceneID;
        bool _initialized = false;
    };
}

#endif // D3D12_SCENE_BACKEND_H_
