#include "PhysicsManager.h"
#include "CCollider.h"
#include "Vector4.h"
#include "RigidBodyWrapper.h"
#include "CTransform.h"
#include "PhysicsUtils.h"
#include "Entity.h"
#include <btBulletDynamicsCommon.h>
#include <iostream>


namespace flux_physics {

    RigidBodyWrapper::RigidBodyWrapper(flux_ec::CTransform* transform, float mass, shapeParameters& params, flux_utils::rigidBodyType type,
        bool trigger, flux_ec::CCollider* collider)
    {
        _collider = collider;

        _transform = new btTransform(fluxToBulletQuaternion(transform->getRot()), fluxToBulletVector(transform->getPos()));

        addShape(params);

        _motionState = new btDefaultMotionState(*_transform);
        btVector3 localInertia(0, 0, 0);
        if (mass > 0.0f)
            _collisionShape->calculateLocalInertia(mass, localInertia);

        _rigidBody = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(mass, _motionState, _collisionShape, localInertia));

        if (trigger)
            _rigidBody->setCollisionFlags(_rigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);

        _rigidBody->setUserPointer(this);
        setMotionType(type);

        // Usa los grupos desde el collider si existe
        if (_collider) {
            _collisionGroup = _collider->getCollisionGroup();
            _collisionMask = _collider->getCollisionMask();
        }

        _rigidBody->activate();
        _rigidBody->setGravity(btVector3(0,-9.8,0 ));

        flux_physics::PhysicsManager::instance()->addRigidBody(_rigidBody, _collisionGroup, _collisionMask);

    }



    RigidBodyWrapper::~RigidBodyWrapper() {

        flux_physics::PhysicsManager::instance()->removeRBformWorld(_rigidBody);
        delete _rigidBody;
        delete _transform;
        delete _motionState;
        delete _collisionShape;
    }

    void RigidBodyWrapper::applyCentralForce(const flux_utils::Vector3& force) {
        _rigidBody->applyCentralForce(fluxToBulletVector(force));
    }

    void RigidBodyWrapper::applyTorque(const flux_utils::Vector3& torque) {
        _rigidBody->applyTorqueImpulse(fluxToBulletVector(torque));
    }

    void RigidBodyWrapper::applyCentralImpulse(const flux_utils::Vector3& impulse) {
        _rigidBody->applyCentralImpulse(fluxToBulletVector(impulse));
      
    }

    void RigidBodyWrapper::setMass(float mass) {
        btVector3 inertia(0, 0, 0);
        if (mass != 0.0f) {
            _collisionShape->calculateLocalInertia(mass, inertia);
        }
        _rigidBody->setMassProps(mass, inertia);
    }

    void RigidBodyWrapper::setFriction(float friction) {
        _rigidBody->setFriction(friction);
    }

    void RigidBodyWrapper::setRestitution(float res) {
        _rigidBody->setRestitution(res);
    }

    void RigidBodyWrapper::setDamping(float linearDamping, float angularDamping) {
        _rigidBody->setDamping(linearDamping, angularDamping);
    }

    // Obtiene la posicion del cuerpo r�gido
    flux_utils::Vector3 RigidBodyWrapper::getPosition() const {
        return bulletToFluxVector(_transform->getOrigin());
    }

    // Establece la posicion del cuerpo r�gido
    void RigidBodyWrapper::setPosition(const flux_utils::Vector3& pos) {
        btTransform newTransform = *_transform;
        newTransform.setOrigin(fluxToBulletVector(pos));
        _rigidBody->setWorldTransform(newTransform);
        _rigidBody->getMotionState()->setWorldTransform(newTransform);
        if (_motionType == flux_utils::KINEMATIC)
            _rigidBody->setInterpolationWorldTransform(newTransform);
    }

    flux_utils::Vector4 RigidBodyWrapper::getRotation() const {
        btTransform transform = _rigidBody->getWorldTransform();
        return bulletToFluxQuaternion(transform.getRotation());
    }

    void RigidBodyWrapper::setRotation(const flux_utils::Vector4& rot) {
        btTransform transform = _rigidBody->getWorldTransform();
        transform.setRotation(fluxToBulletQuaternion(rot));
        _rigidBody->setWorldTransform(transform);
        _rigidBody->getMotionState()->setWorldTransform(transform);
        if(_motionType == flux_utils::KINEMATIC)
            _rigidBody->setInterpolationWorldTransform(transform);
    }

    flux_utils::Vector3 RigidBodyWrapper::getLinearVelocity() const {
        btVector3 v = _rigidBody->getLinearVelocity();
        flux_utils::Vector3 ve = bulletToFluxVector(v);
        return ve;
    }

    void RigidBodyWrapper::setLinearVelocity(const flux_utils::Vector3& velocity) {
        _rigidBody->setLinearVelocity(fluxToBulletVector(velocity));
    }

    flux_utils::Vector3 RigidBodyWrapper::getAngularVelocity() const {
        return bulletToFluxVector(_rigidBody->getAngularVelocity());
    }

    void RigidBodyWrapper::setAngularVelocity(const flux_utils::Vector3& velocity) {
        _rigidBody->setAngularVelocity(fluxToBulletVector(velocity));

    }


    void RigidBodyWrapper::setTransform(flux_utils::Vector3& origin, flux_utils::Vector4& rotation) {
        _rigidBody->getMotionState()->setWorldTransform(btTransform(fluxToBulletQuaternion(rotation), fluxToBulletVector(origin)));
    }

    btVector3 RigidBodyWrapper::fluxToBulletVector(flux_utils::Vector3 vector) const
    {
        return btVector3(vector.getX(), vector.getY(), vector.getZ());
    }

    flux_utils::Vector3 RigidBodyWrapper::bulletToFluxVector(btVector3 vector) const
    {
        return flux_utils::Vector3(vector.getX(), vector.getY(), vector.getZ());
    }

    btQuaternion RigidBodyWrapper::fluxToBulletQuaternion(flux_utils::Vector4 quaternion) const
    {
        return btQuaternion(quaternion.getX(), quaternion.getY(), quaternion.getZ(), quaternion.getW());
    }

    flux_utils::Vector4 RigidBodyWrapper::bulletToFluxQuaternion(btQuaternion quaternion) const
    {
        return flux_utils::Vector4(quaternion.getX(), quaternion.getY(), quaternion.getZ(), quaternion.getW());
    }
    void RigidBodyWrapper::addShape(const shapeParameters& params)
    {
        switch (params.type) {
        case flux_utils::shapeType::BOX:
            _collisionShape = new btBoxShape(btVector3(params.length.getX(), params.length.getY(), params.length.getZ()));
            break;
        case flux_utils::shapeType::CAPSULE:
            _collisionShape = new btCapsuleShape(params.radius, params.length.getY());
            break;
        case flux_utils::shapeType::CYLINDER:
            _collisionShape = new btCylinderShape(btVector3(params.length.getX(), params.length.getY(), params.length.getZ()));
            break;
        case flux_utils::shapeType::SPHERE:
            _collisionShape = new btSphereShape(params.radius);
            break;
        default:
            _collisionShape = new btBoxShape(btVector3(params.length.getX(), params.length.getY(), params.length.getZ()));
        }
    }

    void RigidBodyWrapper::setMotionType(flux_utils::rigidBodyType type)
    {
        if (type == flux_utils::rigidBodyType::DYNAMIC)
        {
            _rigidBody->setCollisionFlags(0); // limpia flags anteriores
            _rigidBody->setActivationState(DISABLE_DEACTIVATION);
            _rigidBody->setCollisionFlags(_rigidBody->getCollisionFlags() | btCollisionObject::CF_DYNAMIC_OBJECT);
        }
        if (type == flux_utils::rigidBodyType::STATIC)
        {
            _rigidBody->setCollisionFlags(0); // limpia flags anteriores
            _rigidBody->setAngularFactor(btVector3(0, 0, 0));
            _rigidBody->setLinearFactor(btVector3(0, 0, 0));
            _rigidBody->setCollisionFlags(_rigidBody->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
            _rigidBody->setMassProps(0, btVector3(0, 0, 0));
            _rigidBody->setActivationState(DISABLE_DEACTIVATION);
        }
        if (type == flux_utils::rigidBodyType::KINEMATIC) {
            _rigidBody->setCollisionFlags(0); // limpia flags anteriores

            _rigidBody->setCollisionFlags(_rigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
            _rigidBody->setActivationState(DISABLE_DEACTIVATION);
            _rigidBody->setMassProps(0, btVector3(0, 0, 0));
        }
        _motionType = type;

    }

    flux_utils::rigidBodyType RigidBodyWrapper::getMotionType()
    {
        return _motionType;
    }

    void RigidBodyWrapper::setTrigger(bool trigger)
    {
        if (trigger) _rigidBody->setCollisionFlags(_rigidBody->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
        else
            _rigidBody->setCollisionFlags(_rigidBody->getCollisionFlags() & ~btCollisionObject::CF_NO_CONTACT_RESPONSE);
    }
    bool RigidBodyWrapper::getTrigger()
    {
        return _rigidBody->getCollisionFlags();
    }

    btRigidBody* RigidBodyWrapper::getRigidBody()
    {
        return _rigidBody;
    }

    void RigidBodyWrapper::update(float dt)
    {
        if (_rigidBody) {
            btTransform newTransform;
            _rigidBody->getMotionState()->getWorldTransform(newTransform);
            *_transform = newTransform;  // Actualizamos el puntero _transform
        }
    }

    void RigidBodyWrapper::setGrav(const flux_utils::Vector3& gravity)
    {
        _rigidBody->setGravity(fluxToBulletVector(gravity));
    }

    void RigidBodyWrapper::setCollisionFilter(int group, int mask)
    {
        _collisionGroup = group;
        _collisionMask = mask;

        flux_physics::PhysicsManager::instance()->updateCollisionGroup(_rigidBody, group, mask);


    }
    int RigidBodyWrapper::getCollisionGroup()
    {
        return _collisionGroup;
    }   
    int RigidBodyWrapper::getCollisionMask()
    {
       return _collisionMask;
    }

    void RigidBodyWrapper::setAngularFactor(flux_utils::Vector3 v)
    {
        _rigidBody->setAngularFactor(fluxToBulletVector(v)); // Permitir rotación

    }

    void RigidBodyWrapper::setLinearFactor(flux_utils::Vector3 v)
    {
        _rigidBody->setLinearFactor(fluxToBulletVector(v)); // Permitir rotación

    }

    void RigidBodyWrapper::removeRigidbody()
    {
        flux_physics::PhysicsManager::instance()->deleteRigidBody(this);

    }

}