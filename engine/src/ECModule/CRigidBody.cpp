#include "CRigidbody.h"

#include "CTransform.h"
#include "PhysicsManager.h"
#include "Entity.h"
#include "CCollider.h"
#include "PhysicsUtils.h"

// ---- FLUX_UTILS ----
#include "FluxError.h"
#include "Vector3.h"
#include "Vector4.h"

#include "ComponentArguments.h"



flux_ec::CRigidBody::~CRigidBody() {

    if (_rb != nullptr) {
        delete _rb;
        _rb = nullptr;
    }
    if (_transform != nullptr)
        _transform = nullptr;
    
    if (_collider != nullptr)
        _collider = nullptr;
}

bool flux_ec::CRigidBody::init(flux_script::ComponentArguments* args) {

    _mass = args->getValueToFloat("Mass");

    _params.type = args->getValueToShapeType("ShapeType");

    if (_params.type == flux_utils::shapeType::SPHERE || (_params.type == flux_utils::shapeType::CAPSULE))
        _params.radius = args->getValueToFloat("Radius");
    else _params.radius = 0;

    if (_params.type == flux_utils::shapeType::BOX || _params.type == flux_utils::shapeType::CAPSULE || _params.type == flux_utils::shapeType::CYLINDER)
        _params.length = args->getValueToVector3("Length");
    else _params.length = { 0,0,0 };

    _params.positionOffset = args->getValueToVector3("Offset");

    _type = args->getValueToRBType("RBType");


    std::string entityName = getOwner()->getName();

    if (!_owner->hasComponent(TRANSFORM)) {
        throwFluxError(false, "Para inicializar un componente RigidBody se necesita un componente Transform");
    }
    _transform = dynamic_cast <CTransform*>(_owner->getComponent(TRANSFORM));

    if (!_owner->hasComponent(COLLIDER)) {
        throwFluxError(false, "Para inicializar un componente RigidBody se necesita un componente Collider");
    }
    _collider = dynamic_cast<CCollider*>(_owner->getComponent(COLLIDER));

    _rb = new flux_physics::RigidBodyWrapper(_transform, _mass, _params, _type, _isTrigger, _collider);

    if (!_rb) {
        throwFluxError(false, "RigidBodyWrapper (_rb) es nulo!");
    }
    return true;
}

// Inicialización y Configuración-----------------------------------------------------------------

void flux_ec::CRigidBody::setMass(float mass) {
    _mass = mass;
    _rb->setMass(mass);
}

void flux_ec::CRigidBody::setFriction(float friction) {
    _friction = friction;
    _rb->setFriction(friction);
}

void flux_ec::CRigidBody::setDamping(float linearDamping, float angularDamping) {
    _rb->setDamping(linearDamping, angularDamping);
}
void flux_ec::CRigidBody::setRestitution(float restitution) {
    _restitution = restitution;
    _rb->setRestitution(restitution);
}

flux_utils::rigidBodyType flux_ec::CRigidBody::getMotionType()
{
    return _type;
}


void flux_ec::CRigidBody::setTrigger(bool isTrigger) {
    _rb->setTrigger(isTrigger);
}
 
void flux_ec::CRigidBody::setAngularFactor(flux_utils::Vector3 v)
{
    
    _rb->setAngularFactor(flux_utils::Vector3(v));
}
void flux_ec::CRigidBody::setLinearFactor(flux_utils::Vector3 v)
{
    _rb->setLinearFactor(flux_utils::Vector3(v));

}
void flux_ec::CRigidBody::setCollisionFilter(int group, int mask)
{
    _rb->setCollisionFilter(group, mask);
}
//void SetCollisionMask(int mask) {

//}
//void SetCollisionGroup(int group) {

//}

// Aplicación de Fuerzas e Impulsos---------------------------------------------------------------

void flux_ec::CRigidBody::applyForce(const flux_utils::Vector3& force) {
    _rb->applyCentralForce(force);
}

void flux_ec::CRigidBody::applyTorque(const flux_utils::Vector3& torque) {
    _rb->applyTorque(torque);
}

void flux_ec::CRigidBody::applyImpulse(const flux_utils::Vector3& impulse) {
    _rb->applyCentralImpulse(impulse);
}

// Gestión de Velocidades-------------------------------------------------------------------------

flux_utils::Vector3 flux_ec::CRigidBody::getLinearVelocity() const {
    return _rb->getLinearVelocity();
}

void flux_ec::CRigidBody::setLinearVelocity(const flux_utils::Vector3& vel) {
    _rb->setLinearVelocity(vel);
}

flux_utils::Vector3 flux_ec::CRigidBody::getAngularVelocity() const {
    return _rb->getAngularVelocity();
}

void flux_ec::CRigidBody::setAngularVelocity(const flux_utils::Vector3& vel) {
    _rb->setAngularVelocity(vel);
}

FLUX_API void flux_ec::CRigidBody::setGravity(const flux_utils::Vector3& gravity)
{
    _rb->setGrav(gravity);
}

// Gestión de Transformaciones--------------------------------------------------------------------

void flux_ec::CRigidBody::setPosition(const flux_utils::Vector3& position) {
    _transform->setPos(position);
    _rb->setPosition(position);

}

flux_utils::Vector3 flux_ec::CRigidBody::getPosition() const {
    return _transform->getPos();
}

void  flux_ec::CRigidBody::setOrientation(const flux_utils::Vector4& orientation) {
    _transform->setRot(orientation);
    _rb->setRotation(orientation);
}

flux_utils::Vector4 flux_ec::CRigidBody::getOrientation() const {
    return _transform->getRot();
}

/*CTransform* CRigidBody::getTransform() const {////////////////////
    return _transform;
}*/

// Propiedades Físicas-----------------------------------------------------------------------------

float flux_ec::CRigidBody::GetMass() const {
    return _mass;
}

float flux_ec::CRigidBody::GetFriction() const {
    return _friction;
}

float flux_ec::CRigidBody::GetRestitution() const {
    return _restitution;
}

void flux_ec::CRigidBody::RemoveRB() const{
    _rb->removeRigidbody();
}

// Interacción con el Motor de Física----------------------------------------------------------

void flux_ec::CRigidBody::update(float dt)
{
    _rb->update(dt);
    physicsTransformToFluxTransform();
}

flux_physics::RigidBodyWrapper* flux_ec::CRigidBody::getWrapper() {
    return _rb;
}


// Métodos para coordinar el componente y el wrapper ------------------------------------------

void flux_ec::CRigidBody::fluxTransformToPhysicsTransform()
{
    //Se modifican las componentes del transform del wrapper conforme a los valores del del motor
    _rb->setPosition(_transform->getPos());
    _rb->setRotation(_transform->getRot());
    //_rb->setScale(_transform->getScale());////////////////////////
}

void flux_ec::CRigidBody::physicsTransformToFluxTransform()
{
    if (_transform == nullptr || _rb == nullptr) {
        return;
    }

    _transform->setPos(_rb->getPosition());
    _transform->setRot(_rb->getRotation());
}