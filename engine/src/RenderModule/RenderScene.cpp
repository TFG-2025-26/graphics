#include "RenderScene.h"

// ---------- OGRE ----------
#include <OgreSceneManager.h>
#include <OgreViewport.h>
#include <OgreRenderWindow.h>
#include <OgreEntity.h>
#include <OgreRoot.h>
#pragma warning(disable:4305)
// ---- FLUX_RENDER ----
#include "RenderObject.h"
#include "RenderManager.h"
#include "RenderSceneManager.h"

//---FLUX_UTILS---
#include "FluxError.h"
#include "checkML.h"

#include "Vector3.h"

flux_render::RenderScene::RenderScene(const std::string& sceneID, Ogre::SceneManager* sceneManager, Ogre::SceneNode* sceneNode)
{
	_sceneID = sceneID;
	_sceneManager = sceneManager;
	_sceneNode = sceneNode;
}

flux_render::RenderScene::~RenderScene()
{
	for (auto& e : _entities) {
		delete e.second;
	}

	_entities.clear();
}

void flux_render::RenderScene::createSceneObject(const std::string& entityID)
{
	auto it = _entities.find(entityID);

	if (it == _entities.cend()) {
		std::string newName = entityID + "_" + _sceneNode->getName();
		RenderObject* object = new RenderObject(newName, _sceneManager, _sceneNode);
		_entities.insert({ entityID, object });
	}
	else
	{
		throw std::exception("Entidad con mismo ID ya creada");
		//throwFluxError(, "Entidad con mismo ID ya creada");
	}
}

void flux_render::RenderScene::addChildToObject(const std::string& childEntityID, const std::string& parentEntityID)
{
	auto pit = _entities.find(parentEntityID);

	if (pit != _entities.cend()) {
		RenderObject* parentObject = pit->second;

		auto cit = _entities.find(childEntityID);

		if (cit == _entities.cend()) { // si no existe el hijo
			RenderObject* childObject = new RenderObject(childEntityID, _sceneManager, parentObject->getSceneNode());
			_entities.insert({ childEntityID, childObject });
		}
		else { // si existe el hijo
			RenderObject* childObject = cit->second;
			Ogre::SceneNode* childNode = childObject->getSceneNode();

			if (childNode->getParentSceneNode() != nullptr) childNode->getParentSceneNode()->removeChild(childNode);
			parentObject->getSceneNode()->addChild(childNode);
		}
	}
	else
	{
		throw std::exception("Entidad padre no encontrada");
		//throwFluxError(, "Entidad padre no encontrada");
	}
}

flux_render::RenderObject* flux_render::RenderScene::getRenderObject(const std::string& entityID) const
{
	auto it = _entities.find(entityID);

	if (it != _entities.cend()) return it->second;
	else
	{
		throw std::exception("Entidad no encontrada");
		//throwFluxError(, "Entidad no encontrada");
	}
}

void flux_render::RenderScene::deleteSceneObject(const std::string& entityID)
{
	auto it = _entities.find(entityID);

	if (it != _entities.cend()) {
		RenderObject* obj = it->second;
		Ogre::SceneNode* node = obj->getSceneNode();

		if (node) {
			auto children = node->getChildren();
			for (auto& child : children) {
				Ogre::SceneNode* childNode = static_cast<Ogre::SceneNode*>(child);
				node->removeChild(childNode);
			}
		}

		delete obj;
		_entities.erase(it);
	}
	else {
		// throwFluxError(false, "...");
	}
}

void flux_render::RenderScene::deleteSceneObjectRecursive(const std::string& entityID)
{
	auto it = _entities.find(entityID);

	if (it != _entities.cend()) {
		RenderObject* parentObj = it->second;
		Ogre::SceneNode* parentNode = parentObj->getSceneNode();

		std::vector<std::string> childrenID = getChildrenNames(parentNode);

		for (auto& e : childrenID) {
			deleteSceneObjectRecursive(e);
		}

		delete parentObj;

		_entities.erase(it);
	}
	else
	{
		//throwFluxError(, "Entidad " +entityID +"no encontrada para ser eliminada recursivamente");
	}
}

void flux_render::RenderScene::deleteSceneObjectReparent(const std::string& entityID)
{
	auto it = _entities.find(entityID);

	if (it != _entities.cend()) {
		RenderObject* parentObj = it->second;
		Ogre::SceneNode* parentNode = parentObj->getSceneNode();

		std::vector<Ogre::SceneNode*> childrenNodes = getChildrenNodes(parentNode);

		for (auto* childNode : childrenNodes) {
			parentNode->removeChild(childNode);
			_sceneNode->addChild(childNode);
		}

		delete parentObj;
		_entities.erase(it);
	}
	else
	{
		//throwFluxError(, "Entidad " +entityID +"no encontrada para ser eliminada");
	}
}

bool flux_render::RenderScene::createCamera(const std::string& entityID, 
	const int8_t& nearDist, const int16_t& farDist)
{
	try {
		auto it = _entities.find(entityID);

		if (it != _entities.cend()) {
			Ogre::Camera* cam;
			//Ogre::Camera* cam = _sceneManager->createCamera(entityID);
			if (_sceneManager->hasCamera(entityID))
			{
				cam = _sceneManager->getCamera(entityID);
			}
			else
			{
				cam = _sceneManager->createCamera(entityID);
				cam->setNearClipDistance(nearDist);
				cam->setFarClipDistance(farDist);
				it->second->getSceneNode()->attachObject(cam);
			}
			// Aseguramos que la cámara esté activa en el viewport 0
			Ogre::RenderWindow* window = RenderManager::instance()->getRenderWindow();

			// Si no hay viewports, añadimos uno
			if (window->getNumViewports() == 0) {
				Ogre::Viewport* vp = window->addViewport(cam);
				vp->setBackgroundColour(Ogre::ColourValue(0.6, 0.7, 0.8));
			}
			else {
				// Si ya hay, simplemente cambiamos la cámara del primer viewport
				window->getViewport(0)->setCamera(cam);
			}

			// vp->setCamera(cam); -> revisar cámara para escenas a futuro			
		}
		return true;
	}
	catch (std::exception e) {
		throwFluxError(false, "No se pudo crear la camara");
	}
}

bool flux_render::RenderScene::createLight(const std::string& entityID,
	const flux_utils::Vector3& diffuseColor, const uint8_t& lightType) 
{
	try {
		auto it = _entities.find(entityID);

		if (it != _entities.cend()) {
			if (_sceneManager->hasLight(entityID))
			{
				_sceneManager->destroyLight(entityID);
			}

			Ogre::Light* light = _sceneManager->createLight(entityID);

			switch (lightType) {
			case 0: // "POINT"
				light->setType(Ogre::Light::LT_POINT);
				break;
			case 1: // "DIRECTIONAL"
				light->setType(Ogre::Light::LT_DIRECTIONAL);
				break;
			case 2: // "SPOTLIGHT"
				light->setType(Ogre::Light::LT_SPOTLIGHT);
				break;
			default:
				light->setType(Ogre::Light::LT_POINT);
				break;
			}

			light->setDiffuseColour(diffuseColor.getX(), diffuseColor.getY(), diffuseColor.getZ());

			it->second->getSceneNode()->attachObject(light);
		}
		return true;
	}
	catch (std::exception e) {
		throwFluxError(false, "Error al crear la luz de la entidad: " + entityID);
	}
}

bool flux_render::RenderScene::createMesh(const std::string& entityID,
	const std::string& meshName, const std::string& materialName)
{
	try {
		auto it = _entities.find(entityID);

		if (it != _entities.cend()) {
			Ogre::Entity* entity = _sceneNode->getCreator()->createEntity(meshName);

			entity->setMaterialName(materialName);

			it->second->setEntity(entity);
		}
		return true;
	}
	catch (std::exception e) {
		throwFluxError(false, "No se pudo crear el mesh: " + meshName);
	}
}

bool flux_render::RenderScene::createAnimation(const std::string& entityID,
	const std::string& animationName)
{
	//Buscamos entidad
	auto it = _entities.find(entityID);
	//booleano que detemina si se ha creado la animacion correctamente o no
	bool isCreated = false;

	if (it != _entities.cend()) {
		isCreated = it->second->addAnimation(animationName);
		return isCreated;
	}
	else {
		throwFluxError(false, "No se ha encontrado la entidad " + entityID +
			" para añadirle la animacion " + animationName);
	}
}

void flux_render::RenderScene::deleteAnimation(const std::string& entityID,
	const std::string& animationName)
{
	auto it = _entities.find(entityID);

	if (it != _entities.cend()) {
		it->second->deleteAnimation(animationName);
	}
	else
	{
		//throwFluxError(, "No se encontro la animacion "+ animationName +"para ser eliminada");
	}
}

void flux_render::RenderScene::setAnimationEnabled(const std::string& entityID,
	const std::string& animationName, bool enabled)
{
	auto it = _entities.find(entityID);

	if (it != _entities.cend()) {
		it->second->setAnimationEnabled(animationName,enabled);
	}
	else {
		writeFluxError("No se encuentra la animacion " + animationName +
			" en la entidad " + entityID + " para ser activada");
	}
}

void flux_render::RenderScene::setAnimationLoop(const std::string& entityID,
	const std::string& animationName, bool loop)
{
	auto it = _entities.find(entityID);

	if (it != _entities.cend()) {
		it->second->setAnimationLoop(animationName, loop);
	}
	else {
		writeFluxError("No se encuentra la animacion " + animationName +
			" en la entidad " + entityID + " para ser loopeada");
	}
}

void flux_render::RenderScene::updateAnimations(const std::string& entityID,
	float dt)
{
	auto it = _entities.find(entityID);

	if (it != _entities.cend()) {
		it->second->updateAnimations(dt);
	}
	//throwFluxError(, "No se encontro la animacion "+ animationName +"para ser actualizada");
}

void flux_render::RenderScene::updateAnimation(const std::string& entityID, const std::string& animationName, float dt)
{
	auto it = _entities.find(entityID);

	if (it != _entities.end() && it->second != nullptr) {
		it->second->updateAnimation(animationName, dt);
	}
}

std::vector<Ogre::SceneNode*> flux_render::RenderScene::getChildrenNodes(Ogre::SceneNode* node)
{
	std::vector<Ogre::SceneNode*> result;
	auto& ogreChildren = node->getChildren();

	for (auto& n : ogreChildren) {
		Ogre::SceneNode* childNode = static_cast<Ogre::SceneNode*>(n);
		result.push_back(childNode);
	}

	return result;
}

// O(n^2) -> hace falta hacer un mapa inverso para volver a O(1)
std::vector<std::string> flux_render::RenderScene::getChildrenNames(Ogre::SceneNode* node)
{
	std::vector<std::string> result;

	auto& ogreChildren = node->getChildren();
	for (auto* n : ogreChildren) {
		Ogre::SceneNode* childNode = static_cast<Ogre::SceneNode*>(n);

		for (auto& kv : _entities) {
			if (kv.second->getSceneNode() == childNode) {
				result.push_back(kv.first);
				break;
			}
		}
	}

	return result;
}

Ogre::SceneManager* flux_render::RenderScene::getSceneManager() const
{
	return _sceneManager;
}

Ogre::SceneNode* flux_render::RenderScene::getSceneNode() const
{
	return _sceneNode;
}

void flux_render::RenderScene::setOgreNodeVisible(bool visible) {
	_sceneNode->setVisible(visible);
}

void flux_render::RenderScene::clearEntities(const std::string& entityID)
{
	_entities.erase(entityID);
}
