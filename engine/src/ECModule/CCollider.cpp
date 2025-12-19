#include "CCollider.h"
#include <iostream>

// ----- FLUX_UTILS -----
#include "FluxError.h"

#include "ComponentArguments.h"

namespace flux_ec {
    bool CCollider::init(flux_script::ComponentArguments* args) {

        _collisionGroup = args->getValueToInt("Group");
        if (!_collisionGroup)
        {

            _collisionGroup = flux_physics::COLLISION_DEFAULT;
        }
        else if ( !(_collisionGroup < 1 || (_collisionGroup & (_collisionGroup - 1)) == 0)) {
            // n es potencia de 2
            throwFluxError(false, "Fallo al inicializar el componente collider, el grupo de colision tiene que ser potencia de 2");
        }


        _collisionMask = args->getValueToInt("Mask");
        if (!_collisionMask)
        {

            _collisionMask = -1; // por defecto colisiona con todo
        }
        else if (_collisionMask!= -1 && _collisionMask < 1) {

            throwFluxError(false, "Fallo al inicializar el componente collider, la capa de colision tiene o -1 o mayor que 1");
        }


        return true;
    }

    void CCollider::update(float dt) {
    }

    void CCollider::setOnCollisionEnter(CollisionCallback callback) {
        onCollisionEnter = callback;
    }

    void CCollider::setOnCollisionStay(CollisionCallback callback) {
        onCollisionStay = callback;

    }

    void CCollider::setOnCollisionExit(CollisionCallback callback) {
        onCollisionExit = callback;
    }

    void CCollider::triggerCollisionEnter(CCollider* other) {
        if (onCollisionEnter)
            onCollisionEnter(other);
        //std::cout << "Hola" << std::endl;
    }

    void CCollider::triggerCollisionStay(CCollider* other) {
        if (onCollisionStay)
            onCollisionStay(other);
    }

    void CCollider::triggerCollisionExit(CCollider* other) {
        if (onCollisionExit)
            onCollisionExit(other);
    }
    void CCollider::setCollisionGroup(int group) {
        _collisionGroup = group;
    }

    void CCollider::setCollisionMask(int mask) {
        _collisionMask = mask;
    }

    int CCollider::getCollisionGroup() const {
        return _collisionGroup;
    }

    int CCollider::getCollisionMask() const {
        return _collisionMask;
    }

}