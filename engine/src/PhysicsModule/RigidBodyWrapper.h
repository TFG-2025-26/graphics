#pragma once

#ifndef RIGIDBODY_WRAPPER_H_
#define RIGIDBODY_WRAPPER_H_

#include<memory>

#include "Vector3.h"


struct btDefaultMotionState;
class btBulletDynamicsCommon;

class btRigidBody;
class btRigidBodyConstructionInfo;
class btVector3;
class btQuaternion;
class btTransform;
class btMotionState;
class btCollisionShape;

class PhysicsManager;

namespace flux_ec {
    class CTransform;
    class CCollider;
    class Entity;
}

namespace flux_utils {
    class Vector4;
    enum rigidBodyType;
    enum shapeType;
}
namespace flux_physics {

    struct shapeParameters {
    public:
        // Tipo de forma
        flux_utils::shapeType type;
        //Radio (en caso de esfera y capsula)
        float radius = 0;
        /// @brief Longitudes de anchura, altura y profundidad (caso caja, altura en capsula y cilindro)
        flux_utils::Vector3 length = flux_utils::Vector3(0, 0, 0);
        /// @brief Offset de posicion con respecto al centro del RigidBody
        flux_utils::Vector3 positionOffset = flux_utils::Vector3(0, 0, 0);
    };

    class RigidBodyWrapper {


    public:

        // Constructor que inicializa el cuerpo r�gido
        RigidBodyWrapper(flux_ec::CTransform* transform, float mass, shapeParameters& params, flux_utils::rigidBodyType type, bool trigger,
            flux_ec::CCollider* collider);
        // Destructor que limpia los recursos
        ~RigidBodyWrapper();

        // Aplica una fuerza al cuerpo r�gido
        void applyCentralForce(const flux_utils::Vector3& force);

        // Aplica un torque al cuerpo r�gido
        void applyTorque(const flux_utils::Vector3& torque);

        // Aplica un impulso al cuerpo r�gido
        void applyCentralImpulse(const flux_utils::Vector3& impulse);

        // Establece la masa del cuerpo r�gido
        void setMass(float mass);

        // Establece la fricci�n del cuerpo r�gido
        void setFriction(float friction);

        // Establece el factor de amortiguaci�n lineal y angular
        void setDamping(float linearDamping, float angularDamping);

        //Establece la restituci�n(elasticidad) del cuerpo r�gido.
        void setRestitution(float res);

        // Obtiene la posicion del cuerpo r�gido
        flux_utils::Vector3 getPosition() const;

        // Establece la posicion del cuerpo r�gido
        void setPosition(const flux_utils::Vector3& pos);

        // Obtiene la orientaci�n del cuerpo r�gido
        flux_utils::Vector4 getRotation() const;

        // Establece la orientaci�n del cuerpo r�gido
        void setRotation(const flux_utils::Vector4& orientation);

        // Obtiene la velocidad lineal del cuerpo r�gido
        flux_utils::Vector3 getLinearVelocity() const;

        // Establece la velocidad lineal del cuerpo r�gido
        void setLinearVelocity(const flux_utils::Vector3& velocity);

        // Obtiene la velocidad angular del cuerpo r�gido
        flux_utils::Vector3 getAngularVelocity() const;

        // Establece la velocidad angular del cuerpo r�gido
        void setAngularVelocity(const flux_utils::Vector3& velocity);

        // Establece la transformaci�n del cuerpo r�gido
        void setTransform(flux_utils::Vector3& origin, flux_utils::Vector4& rotation);

        void addShape(const shapeParameters& params);

        void setMotionType(flux_utils::rigidBodyType type);
        flux_utils::rigidBodyType getMotionType();

        void setTrigger(bool trigger);

        bool getTrigger();

        btRigidBody* getRigidBody();

        void update(float dt);

        void setGrav(const flux_utils::Vector3& gravity); // Obtiene la velocidad angular del cuerpo rígido.


        void setCollisionFilter(int group, int mask);

        int getCollisionGroup();

        int getCollisionMask();

        void setAngularFactor(flux_utils::Vector3 v);

        void setLinearFactor(flux_utils::Vector3 v);

        void removeRigidbody();

        flux_ec::CCollider* getCollider() const {
            return _collider;
        }


    private:
        btRigidBody* _rigidBody = nullptr;
        btTransform* _transform = nullptr;
        btCollisionShape* _collisionShape = nullptr;
        btDefaultMotionState* _motionState = nullptr;
        flux_utils::rigidBodyType _motionType;

        btVector3 fluxToBulletVector(flux_utils::Vector3 vector) const;
        flux_utils::Vector3 bulletToFluxVector(btVector3 vector) const;
        btQuaternion fluxToBulletQuaternion(flux_utils::Vector4 quaternion) const;
        flux_utils::Vector4 bulletToFluxQuaternion(btQuaternion quaternion) const;

        int _collisionGroup; // COLLISION_DEFAULT
        int _collisionMask; // Colisiona con todo por defecto

        // Añadir referencia al collider del usuario
        flux_ec::CCollider* _collider = nullptr;  // nueva referencia al componente collider
    };
}
#endif // RIGIDBODY_WAPPER_H_