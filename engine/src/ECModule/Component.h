#pragma once

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "defs.h"

#include <memory>
#include <string>

using ID = std::string;

enum EngineID : uint8_t { TRANSFORM, CAMERA, MESH, ANIMATOR, LIGHT, COLLIDER, RIGIDBODY, AUDIO_SOURCE, UI, ENGINE_MAX };

namespace flux_script {
    class ComponentArguments;
}

namespace flux_ec {
    class Entity;
    class Component {
    public:
        FLUX_API Component() = default;
        FLUX_API virtual ~Component() = default;

        FLUX_API virtual void awake() {}
        FLUX_API virtual bool init(flux_script::ComponentArguments* args) = 0;
        FLUX_API virtual void update(float dt) = 0;

        FLUX_API virtual void fixedUpdate() {}
        FLUX_API virtual void lateUpdate() {}

        FLUX_API virtual uint8_t getType() const = 0;
        FLUX_API Entity* getOwner() const;
        FLUX_API void setOwner(Entity* owner);

        FLUX_API bool getActive();
        FLUX_API void setActive(bool b);

    protected:
        FLUX_API explicit Component(Entity* owner);

        Entity* _owner;  // Puntero a la entidad que posee el componente
        bool _isActive;  // Booleano para activar y desactivar el componente
        std::string _sceneID; // Identificador de escena
    };
}

#endif COMPONENT_H_