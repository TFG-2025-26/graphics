#pragma once

#ifndef RIGIDBODY_H_
#define RIGIDBODY_H_

#include <memory>
#include "Component.h"

#include "RigidBodyWrapper.h"
#include <iostream>

namespace flux_script {
    class ComponentArguments;
}

struct btDefaultMotionState;
class btBulletDynamicsCommon;

class btRigidBody;
class btRigidBodyConstructionInfo;
class btVector3;
class btQuaternion;
class btTransform;
class btMotionState;
class btCollisionShape;

namespace flux_utils {
    class Vector3;
    class Vector4;
    enum shapeType;
    enum rigidBodyType;
}

namespace flux_physics {
    class PhysicsManager;
}

namespace flux_ec {

    class Entity;
    class CTransform;
    class CCollider;

    class CRigidBody : public Component {

    public:
        FLUX_API CRigidBody() = default;
        // Destructor que elimina el cuerpo rígido del mundo físico
        FLUX_API ~CRigidBody();

        // Inicialización y Configuración

        bool init(flux_script::ComponentArguments* args) override; // Inicializa el cuerpo rígido con parámetros específicos.
        
        FLUX_API void setMass(float mass);// Establece la masa del cuerpo rígido.
        FLUX_API void setFriction(float friction); // Establece la fricción del cuerpo rígido.
        FLUX_API void setDamping(float linearDamping, float angularDamping); // Establece la amortiguación.
        FLUX_API void setRestitution(float restitution); // Establece la restitución(elasticidad) del cuerpo rígido.
        FLUX_API flux_utils::rigidBodyType getMotionType(); // Establece el tipo de movimiento(estático, dinámico, cinemático).
        FLUX_API void setTrigger(bool isTrigger); // Establece si el cuerpo rígido es un trigger.
        FLUX_API void setAngularFactor(flux_utils::Vector3 v);
        FLUX_API void setLinearFactor(flux_utils::Vector3 v);
        void setCollisionFilter(int group, int mask); // Establece la máscara de colisión.

        // Aplicación de Fuerzas e Impulsos

        FLUX_API void applyForce(const flux_utils::Vector3& force); // Aplica una fuerza al cuerpo rígido.
        FLUX_API void applyTorque(const flux_utils::Vector3& torque); // Aplica un torque al cuerpo rígido.
        FLUX_API void applyImpulse(const flux_utils::Vector3& impulse); // Aplica un impulso al cuerpo rígido.

        // Gestión de Velocidades

        FLUX_API flux_utils::Vector3 getLinearVelocity() const; // Obtiene la velocidad lineal del cuerpo rígido.
        FLUX_API void setLinearVelocity(const flux_utils::Vector3& velocity); // Establece la velocidad lineal del cuerpo rígido.
        FLUX_API flux_utils::Vector3 getAngularVelocity() const; // Establece la velocidad angular del cuerpo rígido.
        FLUX_API void setAngularVelocity(const flux_utils::Vector3& velocity); // Obtiene la velocidad angular del cuerpo rígido.
        FLUX_API void setGravity(const flux_utils::Vector3& gravity); // Obtiene la velocidad angular del cuerpo rígido.

        // Gestión de Transformaciones

        FLUX_API void setPosition(const flux_utils::Vector3& position); // Establece la posición del cuerpo rígido.
        FLUX_API flux_utils::Vector3 getPosition() const; // Obtiene la posición del cuerpo rígido.
        FLUX_API void setOrientation(const flux_utils::Vector4& orientation); // Establece la orientación del cuerpo rígido.
        FLUX_API flux_utils::Vector4 getOrientation() const; // Obtiene la orientación del cuerpo rígido.

        // Propiedades Físicas

        FLUX_API float GetMass() const; // Obtiene la masa del cuerpo rígido.
        FLUX_API float GetFriction() const; // Obtiene la fricción del cuerpo rígido.
        FLUX_API float GetRestitution() const; // Obtiene la restitución del cuerpo rígido.
        
        FLUX_API void RemoveRB() const; // Obtiene la restitución del cuerpo rígido.

        // Interacción con el Motor de Física

        void update(float dt) override;
        flux_physics::RigidBodyWrapper* getWrapper();

        static ID getID() { return "RIGIDBODY"; }
        uint8_t getType() const override { return RIGIDBODY; }
    private:

        /// @brief Masa asociada al RigidBody
        float _mass = 0;

        /// @brief Efecto de rebote, indica cuanta energia se mantiene despues de la colision
        float _restitution = 0;

        /// @brief Friccion asociada al RigidBody
        float _friction = 0;

        /// @brief Booleano para indicar si se comporta como un trigger o un collider
        bool _isTrigger = false;

        /// @brief Variable que se encarga de contener los parametros de la figura asociada al rigidBody
        flux_physics::shapeParameters _params;

        /// @brief Tipo de RigidBody (STATIC, DYNAMIC, KINEMATIC)
        flux_utils::rigidBodyType _type;

        /// @brief Nombre de la capa de colision asignada
        //std::string _layer = "DEFAULT";

        /// @brief Referencia al wrapper de rigid body del modulo de fisicas
        flux_physics::RigidBodyWrapper* _rb = nullptr;

        /// @brief Referencia al componente transform
        flux_ec::CTransform* _transform = nullptr;
        flux_ec::CCollider* _collider = nullptr;

        /// @brief Convierte el componente transform propio al transform del motor de fisicas
        void fluxTransformToPhysicsTransform();

        /// @brief Convierte el transform del motor de fisicas al componente transform propio
        void physicsTransformToFluxTransform();


    };

}
#endif // RIGIDBODY_H_
