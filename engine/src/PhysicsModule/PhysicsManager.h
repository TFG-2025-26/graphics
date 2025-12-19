#pragma once
#ifndef PHYSICS_MANAGER_H_
#define PHYSICS_MANAGER_H_

// --- FLUX_UTILS ---
#include "Manager.h"
#include "Singleton.h"
#include "Vector3.h"
#include "Vector4.h"
#include "RigidBodyWrapper.h"
#include <set>
#include <vector>

class btDefaultCollisionConfiguration;//
class btCollisionDispatcher;//
class btSequentialImpulseConstraintSolver;//
class btBroadphaseInterface;//
class btDiscreteDynamicsWorld;//
class btRigidBody;//
class btVector3;//
//class btConstraintSolver;
//class btDynamicsWorld;
//class btCollisionObject;
//class btCollisionConfiguration;
//class btTransform;
//class btPersistentManifold;

namespace flux_utils {
    class Vector3;
    class Vector4;
}

namespace flux_ec {
    class Entity;
    class CRigidBody;
    class CCollider;
}

namespace flux_physics {

    enum MotionType {
        STATIC,
        DYNAMIC,
        KINEMATIC
    };

    // Máscaras de colisión
    enum CollisionGroup {
        COLLISION_DEFAULT = 1 << 0,
        COLLISION_PLAYER = 1 << 1,
        COLLISION_ENEMY = 1 << 2,
        COLLISION_TRIGGER = 1 << 3,
        COLLISION_PROJECTILE = 1 << 4
    };



    class PhysicsManager : public flux_utils::Manager,
        public flux_utils::Singleton<PhysicsManager>
    {

    public:
        friend flux_utils::Singleton<PhysicsManager>;

        virtual ~PhysicsManager();

        /// <summary>
        /// Inicializaci�n del sistema de renderizado y recursos.
        /// </summary>
        bool init() override;

        /// <summary>
        /// Actualiza el renderizado de un frame de OGRE y de SDL.
        /// </summary>
        void update(float dt) override;

        /// <summary>
        /// Previo a cerrar la applicaci�n, guarda la configuraci�n.
        /// </summary>
        bool shutdown() override;

        void fixedUpdate(float fixedDt);

        btDiscreteDynamicsWorld* getDynamicsWorld() const;

        void addRigidBody(btRigidBody* body, int group, int mask);

        void updateCollisionGroup(btRigidBody* body, int newGroup, int newMask);

        FLUX_API void deleteRigidBody(RigidBodyWrapper* body);
        FLUX_API void emptyWorld();
        void removeRBformWorld( btRigidBody* rb);

        // Configuraci�n de F�sica
        void setGravity(flux_utils::Vector3 g);




    private:

        PhysicsManager();

        btBroadphaseInterface* broadphase = nullptr; // Interfaz de detecci�n de colisiones a gran escala
        btDefaultCollisionConfiguration* collisionConfiguration = nullptr; // Configuraci�n de colisi�n
        btCollisionDispatcher* dispatcher = nullptr; // Despachador de colisiones
        btSequentialImpulseConstraintSolver* solver = nullptr; // Solver de restricciones
        btDiscreteDynamicsWorld* dynamicsWorld = nullptr; // Mundo din�mico de Bullet

        void addForce(btRigidBody* body, btVector3 force);

        void clearForces(btRigidBody* body, btVector3 force);

        bool isInWorld(btRigidBody* body) const;

        const float fixedTimeStep = 1.0f / 60.0f; // Tiempo fijo por actualización
        float accumulatedTime; // Tiempo acumulado

        std::set<std::pair<RigidBodyWrapper*, RigidBodyWrapper*>> previousCollisions;
        std::vector<RigidBodyWrapper*> deleteEntities;

    };
}

#endif // PHYSICS_MANAGER_H_