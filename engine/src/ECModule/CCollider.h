#pragma once

#ifndef COLLIDER_H_
#define COLLIDER_H_

#include "Component.h"
#include <functional>
#include "PhysicsManager.h"

namespace flux_script {
    class ComponentArguments;
}

namespace flux_ec {

    class CCollider : public Component {
    public:
        using CollisionCallback = std::function<void(CCollider* other)>;

        FLUX_API CCollider() = default;
        FLUX_API virtual ~CCollider() {}

        bool init(flux_script::ComponentArguments* args);
        void update(float dt);

        FLUX_API void setOnCollisionEnter(CollisionCallback callback);
        FLUX_API void setOnCollisionStay(CollisionCallback callback);
        FLUX_API void setOnCollisionExit(CollisionCallback callback);

        FLUX_API  void triggerCollisionEnter(CCollider* other);
        FLUX_API  void triggerCollisionStay(CCollider* other);
        FLUX_API  void triggerCollisionExit(CCollider* other);

        // Métodos nuevos para configurar grupos y máscaras
        FLUX_API  void setCollisionGroup(int group);
        FLUX_API  void setCollisionMask(int mask);

        FLUX_API  int getCollisionGroup() const;
        FLUX_API   int getCollisionMask() const;

        static ID getID() { return "COLLIDER"; }
        static ID getNameOb() { return "COLLIDER"; }
        uint8_t getType() const override { return COLLIDER; }
    private:
        CollisionCallback onCollisionEnter;
        CollisionCallback onCollisionStay;
        CollisionCallback onCollisionExit;

        // Nuevos atributos
        int _collisionGroup;
        int _collisionMask; // por defecto colisiona con todo
    };

}

#endif // COLLIDER_H_