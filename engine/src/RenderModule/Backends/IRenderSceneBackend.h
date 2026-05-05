#pragma once

#ifndef I_RENDER_SCENE_BACKEND_H_
#define I_RENDER_SCENE_BACKEND_H_

#include <cstdint>
#include <string>

namespace flux_utils {
    class Vector3;
    class Vector4;
}

namespace flux_render {
    class IRenderSceneBackend {
    public:
        virtual ~IRenderSceneBackend() = default;

        virtual bool init() = 0;
        virtual void shutdown() = 0;

        virtual bool createScene(const std::string& sceneID) = 0;
        virtual bool destroyScene(const std::string& sceneID) = 0;
        virtual bool setCurrentScene(const std::string& sceneID) = 0;

        virtual bool createSceneObject(
            const std::string& sceneID,
            const std::string& entityID) = 0;

        virtual bool destroySceneObject(
            const std::string& sceneID,
            const std::string& entityID) = 0;

        virtual bool setObjectTransform(
            const std::string& sceneID,
            const std::string& entityID,
            const flux_utils::Vector3& position,
            const flux_utils::Vector4& rotation,
            const flux_utils::Vector3& scale) = 0;

        virtual bool attachMesh(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& meshName,
            const std::string& materialName) = 0;

        virtual bool createLight(
            const std::string& sceneID,
            const std::string& entityID,
            const flux_utils::Vector3& diffuseColor,
            uint8_t lightType) = 0;

        virtual bool createCamera(
            const std::string& sceneID,
            const std::string& entityID,
            int8_t nearDist,
            int16_t farDist) = 0;

        virtual bool addAnimation(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName) = 0;

        virtual bool removeAnimation(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName) = 0;

        virtual bool setAnimationEnabled(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName,
            bool enabled) = 0;

        virtual bool setAnimationLoop(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName,
            bool loop) = 0;

        virtual bool updateAnimation(
            const std::string& sceneID,
            const std::string& entityID,
            const std::string& animationName,
            float dt) = 0;

        virtual bool isCurrentScene(const std::string& sceneID) const = 0;
    };
}

#endif // I_RENDER_SCENE_BACKEND_H_
