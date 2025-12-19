#include "RenderObject.h"

// --------- OGRE ---------
#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreMovableObject.h>

// -- FLUX_UTILS --
#include "Vector3.h"
#include "Vector4.h"
#include "FluxError.h"

flux_render::RenderObject::RenderObject(const std::string& id, 
	Ogre::SceneManager* mngr, Ogre::SceneNode* parent) {
	_id = id;
	_mngr = mngr;

	if (parent != nullptr) _node = parent->createChildSceneNode(id);
	else _node = mngr->getRootSceneNode()->createChildSceneNode(id);
}

flux_render::RenderObject::~RenderObject()
{
	if (_node && _entity) {
		_node->detachObject(_entity);
		_entity = nullptr;
	}

	_animations.clear();

	if (_node && _mngr) {
		_mngr->destroySceneNode(_node);
		_node = nullptr;
	}
}

void flux_render::RenderObject::setPosition(const flux_utils::Vector3& pos)
{
	if (_node != nullptr) _node->setPosition(pos.getX(), pos.getY(), pos.getZ());
}

void flux_render::RenderObject::setOrientation(const flux_utils::Vector4& rot)
{
	if (_node != nullptr) _node->setOrientation(rot.getW(), rot.getX(), rot.getY(), rot.getZ());
}

void flux_render::RenderObject::setScale(const flux_utils::Vector3& scale)
{
	if (_node != nullptr) _node->setScale(scale.getX(), scale.getY(), scale.getZ());
}

flux_utils::Vector3 flux_render::RenderObject::getPosition() const
{
	Ogre::Vector3 pos = _node->getPosition();
	return flux_utils::Vector3(pos.x, pos.y, pos.z);
}

flux_utils::Vector4 flux_render::RenderObject::getOrientation() const
{
	Ogre::Quaternion rot = _node->getOrientation();
	return flux_utils::Vector4(rot.x, rot.y, rot.z, rot.w);
}

flux_utils::Vector3 flux_render::RenderObject::getScale() const
{
	Ogre::Vector3 scale = _node->getScale();
	return flux_utils::Vector3(scale.x, scale.y, scale.z);
}

Ogre::SceneNode* flux_render::RenderObject::getSceneNode() const
{
	return _node;
}

void flux_render::RenderObject::lookAt(const flux_utils::Vector3& lookAt)
{
	if (_node != nullptr) _node->lookAt({ lookAt.getX(),lookAt.getY(),lookAt.getZ() }, Ogre::Node::TS_WORLD);
}

void flux_render::RenderObject::setDirection(const flux_utils::Vector3& direction)
{
	if (_node != nullptr) _node->setDirection({direction.getX(),direction.getY(),direction.getZ()});
}

bool flux_render::RenderObject::addAnimation(const std::string& animationName) {
	//Comprobamos si la propia entidad tiene animaciones (normalmente están dentro de un .mesh)
	if (_entity == nullptr) {
		throwFluxError(false, "La entidad es nula. Se necesita un componente Mesh para poder inicializar Animator.");
	}

	//Comprobamos si el mesh asociado a la entidad contiene animaciones. 
	//En caso de que no las contenga, la inicializacion falla
	Ogre::AnimationStateSet* animationSet = _entity->getAllAnimationStates();
	if (animationSet == nullptr) {
		throwFluxError(false, "El mesh de la entidad" + _id + " no tiene animaciones asociadas.");
	}

	//Buscamos en nuestro map de animaciones
	auto it = _animations.find(animationName);

	//En caso de sea una animacion nueva
	if (it == _animations.cend()) {
		//Comporbamos si dicha animacion tambien se encuentra en el propio mesh. 
		//Si no esta, solo escribimos un aviso
		if (animationSet->hasAnimationState(animationName)) {
			Ogre::AnimationState* newAnimation = _entity->getAnimationState(animationName);
			_animations.insert({ animationName,newAnimation });
		}
		else {
			throwFluxError(false, "No se ha podido encontrar la animacion " + animationName + " en el mesh");
		}
	}

	return true;
}

bool flux_render::RenderObject::deleteAnimation(const std::string& animationName) {
	try {
		auto it = _animations.find(animationName);

		if (it != _animations.cend()) {
			delete it->second;
			_animations.erase(animationName);
		}
		return true;
	}
	catch (std::exception e) {
		throwFluxError(false, "No se pudo eliminar la animacion: " + animationName);
	}
}

bool flux_render::RenderObject::setAnimationEnabled(const std::string& animationName, bool enabled) {
	try {
		auto it = _animations.find(animationName);

		if (it != _animations.cend()) {
			it->second->setEnabled(enabled);
		}

		return true;
	}
	catch (std::exception e) {
		throwFluxError(false, "No se pudo activar la animacion: " + animationName);
	}
}

bool flux_render::RenderObject::setAnimationLoop(const std::string& animationName, bool loop) {
	try {
		auto it = _animations.find(animationName);

		if (it != _animations.cend()) {
			it->second->setLoop(loop);
		}
		return true;
	}
	catch (std::exception e) {
		throwFluxError(false, "No se pudo loopear la animacion: " + animationName);
	}
}

void  flux_render::RenderObject::updateAnimations(float dt) {
	for (auto& a : _animations) {
		a.second->addTime(dt);
	}
}

void flux_render::RenderObject::setEntity(Ogre::Entity* entity) {
	_entity = entity;
	_node->attachObject(_entity);
}