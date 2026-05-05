#include "CAnimator.h"

// --- FLUX_EC ---
#include "Entity.h"

// ---- FLUX_RENDER ----
#include "RenderManager.h"
#include "RenderSceneManager.h"
#include "RenderScene.h"

// ---- FLUX_UTILS ----
#include "FluxError.h"

#include "ComponentArguments.h"

bool flux_ec::CAnimator::init(flux_script::ComponentArguments* args) {
	std::list<std::string> anim = args->getValueToListString("Animations");
	std::list<bool> enable = args->getValueToListBool("Enabled");
	std::list<bool> loop = args->getValueToListBool("Loop");


	if (anim.size() != enable.size() || anim.size() != loop.size()) {
		throwFluxError(false, "Fallo al inicializar el componente Animator");
	}
	auto it1 = anim.begin();
	auto it2 = enable.begin();
	auto it3 = loop.begin();

	for (it1; it1 != anim.end(); it1++) {
		if (!addAnimation((*it1)))
		{
			throwFluxError(false, "Fallo al inicializar el componente Animator");
		}

		setAnimationEnabled((*it1), (*it2));
		setAnimationLoop((*it1), (*it3));

		it2++;
		it3++;
	}
	return true;
}

void flux_ec::CAnimator::update(float dt) {
	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();

	if (sceneBackend == nullptr) {
		return;
	}

	for (const auto& animationName : _animations) {
		sceneBackend->updateAnimation(sceneID, entityName, animationName, dt);
	}
}

bool flux_ec::CAnimator::addAnimation(const std::string& animationName) 
{
	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();

	if (sceneBackend == nullptr) {
		throwFluxError(false, "No existe SceneBackend para añadir a la animacion " + animationName);
		return false;
	}

	if (!sceneBackend->addAnimation(sceneID, entityName, animationName)) {
		throwFluxError(false, "No se pudo añadir la animación " + animationName +
			" a la entidad " + entityName);
		return false;
	}

	_animations.insert(animationName);

	return true;
}

void flux_ec::CAnimator::deleteAnimation(const std::string& animationName) 
{
	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();

	if (sceneBackend == nullptr) {
		// throwFluxError(false, "No existe SceneBackend para borrar la animación " + animationName);
		return;
	}

	if (!sceneBackend->removeAnimation(sceneID, entityName, animationName)) {
		// throwFluxError(false, "No se pudo borrar la animación " + animationName + " de la entidad " + entityName);
		return;
	}

	_animations.erase(animationName);
}

void flux_ec::CAnimator::setAnimationEnabled(const std::string& animationName, bool enabled) 
{
	if (_animations.find(animationName) == _animations.end()) {
		return;
	}

	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();

	if (sceneBackend == nullptr) {
		// throwFluxError(false, "No existe SceneBackend para activar/desactivar la animación " + animationName);
		return;
	}

	if (!sceneBackend->setAnimationEnabled(sceneID, entityName, animationName, enabled)) {
		// throwFluxError(false, "No se pudo cambiar el estado de la animación " + animationName)
	}
}

void flux_ec::CAnimator::setAnimationLoop(const std::string& animationName, bool loop) {

	if (_animations.find(animationName) == _animations.end()) {
		return;
	}

	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();

	if (sceneBackend == nullptr) {
		// throwFluxError(false, "No existe SceneBackend para cambiar el loop de la animación " + animationName);
		return;
	}

	if (!sceneBackend->setAnimationLoop(sceneID, entityName, animationName, loop)) {
		// throwFluxError(false, "No se pudo cambiar el loop de la animación " + animationName);
	}
}