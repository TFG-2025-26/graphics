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

	flux_render::RenderManager* rMngr = flux_render::RenderManager::instance();
	flux_render::RenderSceneManager* sceneMngr = rMngr->getSceneManager();

	flux_render::RenderScene* currentScene = sceneMngr->getScene(sceneID);

	currentScene->updateAnimations(entityName, dt);
}

bool flux_ec::CAnimator::addAnimation(const std::string& animationName) {
	_animations.insert(animationName);
	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	flux_render::RenderManager* rMngr = flux_render::RenderManager::instance();
	flux_render::RenderSceneManager* sceneMngr = rMngr->getSceneManager();

	flux_render::RenderScene* currentScene = sceneMngr->getScene(sceneID);

	return currentScene->createAnimation(entityName, animationName);
}

void flux_ec::CAnimator::deleteAnimation(const std::string& animationName) {
	_animations.erase(animationName);
	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	flux_render::RenderManager* rMngr = flux_render::RenderManager::instance();
	flux_render::RenderSceneManager* sceneMngr = rMngr->getSceneManager();

	flux_render::RenderScene* currentScene = sceneMngr->getScene(sceneID);

	currentScene->deleteAnimation(entityName,animationName);
}

void flux_ec::CAnimator::setAnimationEnabled(const std::string& animationName, bool enabled) {
	_animations.erase(animationName);
	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	flux_render::RenderManager* rMngr = flux_render::RenderManager::instance();
	flux_render::RenderSceneManager* sceneMngr = rMngr->getSceneManager();

	flux_render::RenderScene* currentScene = sceneMngr->getScene(sceneID);

	currentScene->setAnimationEnabled(entityName, animationName, enabled);
}

void flux_ec::CAnimator::setAnimationLoop(const std::string& animationName, bool loop) {
	_animations.erase(animationName);
	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	flux_render::RenderManager* rMngr = flux_render::RenderManager::instance();
	flux_render::RenderSceneManager* sceneMngr = rMngr->getSceneManager();

	flux_render::RenderScene* currentScene = sceneMngr->getScene(sceneID);

	currentScene->setAnimationLoop(entityName, animationName, loop);
}