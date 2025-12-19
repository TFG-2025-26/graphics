#include "PhysicsManager.h"
#include "CCollider.h"
#include <btBulletDynamicsCommon.h>
#include "Vector3.h"

#include "CRigidBody.h"
#include "Entity.h"
#include "SceneManager.h"
#include <iostream>
#include <unordered_map>
#include "FluxError.h"


namespace flux_physics {

    PhysicsManager::PhysicsManager()
        : broadphase(nullptr), collisionConfiguration(nullptr), dispatcher(nullptr), solver(nullptr), dynamicsWorld(nullptr) {

    }

    PhysicsManager::~PhysicsManager() {
    }

    bool PhysicsManager::shutdown()
    {
        if (isInitialized) {
            if (dynamicsWorld) {
                delete dynamicsWorld;
                dynamicsWorld = nullptr;  // Evitar referencias inv�lidas
            }

            if (solver) {
                delete solver;
                solver = nullptr;
            }

            if (dispatcher) {
                delete dispatcher;
                dispatcher = nullptr;
            }

            if (collisionConfiguration) {
                delete collisionConfiguration;
                collisionConfiguration = nullptr;
            }

            if (broadphase) {
                delete broadphase;
                broadphase = nullptr;
            }

            isInitialized = false;
            return true;
        }
        else {
            throwFluxError(false, "No se puede cerrar un modulo que no esta inicializado.");
        }
    }
    bool PhysicsManager::init() {

        // Configuraci�n de colisi�n por defecto
        collisionConfiguration = new btDefaultCollisionConfiguration();

        // El dispatcher maneja el despacho de colisiones
        dispatcher = new btCollisionDispatcher(collisionConfiguration);

        // El solver resuelve las restricciones f�sicas
        solver = new btSequentialImpulseConstraintSolver();

        broadphase = new btDbvtBroadphase();

        // Creamos el mundo din�mico que gestionar� la simulaci�n f�sica
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);

        // Establecemos la gravedad en el mundo f�sico
        dynamicsWorld->setGravity(btVector3(btScalar(0.0f), btScalar(-9.8f), btScalar(0.0f)));

        accumulatedTime = 0.0f;

        isInitialized = true;

        return true;
    }

    void PhysicsManager::update(float dt) {

        if (!dynamicsWorld) return;
        dynamicsWorld->debugDrawWorld();

        int i = dynamicsWorld->getNumCollisionObjects();

        accumulatedTime += dt;
        while (accumulatedTime >= fixedTimeStep) {
            fixedUpdate(fixedTimeStep);
            accumulatedTime -= fixedTimeStep;
        }
    }

    void PhysicsManager::fixedUpdate(float fixedDt) {
        dynamicsWorld->stepSimulation(1.f / 60.f);

        for (auto& pair : previousCollisions) {
            if (previousCollisions.find(pair) == previousCollisions.end()) {
                // ¡Aquí debes validar!
                if (pair.first && pair.second && pair.first != nullptr && pair.second != nullptr) {

                    pair.first->getCollider()->triggerCollisionExit(pair.second->getCollider());
                    pair.second->getCollider()->triggerCollisionExit(pair.first->getCollider());
                }
            }
        }

        //delete marked rigidbodys
        if (!deleteEntities.empty())
        {
            for (auto& e : deleteEntities) {

                for (auto it = previousCollisions.begin(); it != previousCollisions.end(); ) {
                    if (it->first == e || it->second == e) {
                        it = previousCollisions.erase(it);  // erase devuelve el siguiente iterador
                    }
                    else {
                        ++it;
                    }
                }

                removeRBformWorld(e->getRigidBody());
            }
            deleteEntities.clear();
        }

  


        std::set<std::pair<flux_physics::RigidBodyWrapper*, flux_physics::RigidBodyWrapper*>> currentCollisions;

        int numManifolds = dispatcher->getNumManifolds();
        for (int i = 0; i < numManifolds; ++i) {
            btPersistentManifold* contactManifold = dispatcher->getManifoldByIndexInternal(i);

            const btCollisionObject* objA = contactManifold->getBody0();
            const btCollisionObject* objB = contactManifold->getBody1();

            flux_physics::RigidBodyWrapper* rbWrapperA = static_cast<flux_physics::RigidBodyWrapper*>(objA->getUserPointer());
            flux_physics::RigidBodyWrapper* rbWrapperB = static_cast<flux_physics::RigidBodyWrapper*>(objB->getUserPointer());

            if (rbWrapperA && rbWrapperA->getCollider() && rbWrapperB && rbWrapperB->getCollider()) {
                int numContacts = contactManifold->getNumContacts();

                bool collisionHappened = false;
                for (int j = 0; j < numContacts; ++j) {
                    btManifoldPoint& pt = contactManifold->getContactPoint(j);
                    if (pt.getDistance() < 0.f) {
                        collisionHappened = true;
                        break;
                    }
                }

                if (collisionHappened) {
                    if (rbWrapperA && rbWrapperB && rbWrapperA != nullptr && rbWrapperB != nullptr) {
                        currentCollisions.insert({ rbWrapperA, rbWrapperB });
                        if (previousCollisions.find({ rbWrapperA, rbWrapperB }) == previousCollisions.end()) {
                            // Nueva colisión
                            rbWrapperA->getCollider()->triggerCollisionEnter(rbWrapperB->getCollider());
                            rbWrapperB->getCollider()->triggerCollisionEnter(rbWrapperA->getCollider());
                        }
                        else {
                            // Colisión persistente
                            rbWrapperA->getCollider()->triggerCollisionStay(rbWrapperB->getCollider());
                            rbWrapperB->getCollider()->triggerCollisionStay(rbWrapperA->getCollider());
                        }
                    }
                    else {
                    }
                }
            }
        }

    }


    btDiscreteDynamicsWorld* PhysicsManager::getDynamicsWorld() const {
        return dynamicsWorld;
    }

    void PhysicsManager::addRigidBody(btRigidBody* body, int group, int mask) {
        dynamicsWorld->addRigidBody(body, group, mask);
        auto a = dynamicsWorld->getCollisionWorld();
    }

    void PhysicsManager::updateCollisionGroup(btRigidBody* body, int newGroup, int newMask) {
        // Lo eliminamos primero del mundo
        dynamicsWorld->removeRigidBody(body);

        // Lo volvemos a añadir con los nuevos grupos/máscaras
        dynamicsWorld->addRigidBody(body, newGroup, newMask);
    }
    void PhysicsManager::deleteRigidBody(RigidBodyWrapper* body) {

        deleteEntities.push_back(body);
    }

    void PhysicsManager::emptyWorld()
    {
        for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; --i) {
            btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];

            btRigidBody* body = btRigidBody::upcast(obj);
            if (body && body->getMotionState()) {
                delete body->getMotionState(); // eliminar motionState si es dinámico
            }

            dynamicsWorld->removeCollisionObject(obj);
            delete obj; // eliminar el objeto (RigidBody o CollisionObject)
        }
    }

    bool PhysicsManager::isInWorld(btRigidBody* body) const
    {
        int numObjects = dynamicsWorld->getNumCollisionObjects();
        for (int i = 0; i < numObjects; ++i)
        {
            if (dynamicsWorld->getCollisionObjectArray()[i] == body)
                return true;
        }
        return false;
    }


    void PhysicsManager::removeRBformWorld(btRigidBody* bt)
    {
        if (isInWorld(bt))
            dynamicsWorld->removeRigidBody(bt);

    }

    void PhysicsManager::addForce(btRigidBody* body, btVector3 force) {
        body->btRigidBody::applyForce(force, body->getWorldTransform().getOrigin());
    }

    void PhysicsManager::clearForces(btRigidBody* body, btVector3 force) {
        body->btRigidBody::clearForces();
    }

    void PhysicsManager::setGravity(flux_utils::Vector3 g) {
        dynamicsWorld->setGravity({ g.getX(), g.getY(), g.getZ() });
    }

}