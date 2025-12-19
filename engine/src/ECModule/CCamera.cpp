#include "CCamera.h"

// --- FLUX_EC ---
#include "Entity.h"

// ---- FLUX_RENDER ----
#include "RenderManager.h"
#include "RenderSceneManager.h"
#include "RenderScene.h"

// ---- FLUX_UTILS ----
#include "FluxError.h"

#include "ComponentArguments.h"

bool flux_ec::CCamera::init(flux_script::ComponentArguments* args)
{
	_nearDist = args->getValueToInt("NearClip");
	_farDist = args->getValueToInt("FarClip");

	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	flux_render::RenderManager* rMngr = flux_render::RenderManager::instance();
	flux_render::RenderSceneManager* sceneMngr = rMngr->getSceneManager();

	flux_render::RenderScene* currentScene = sceneMngr->getScene(sceneID);
	flux_render::RenderScene* activeScene = sceneMngr->getCurrentScene();

	if (currentScene != activeScene) {
		_pendingCreation = true;
		return true;
	}

	// return createCamera();
}

void flux_ec::CCamera::update(float dt)
{
}

int8_t flux_ec::CCamera::getNearClip() const
{
	return _nearDist;
}

int16_t flux_ec::CCamera::getFarClip() const
{
	return _farDist;
}

void flux_ec::CCamera::setNearClip(int8_t nearDist)
{
	_nearDist = nearDist;
}

void flux_ec::CCamera::setFarClip(int16_t farDist)
{
	_farDist = farDist;
}

bool flux_ec::CCamera::createCamera()
{
	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	auto* rMngr = flux_render::RenderManager::instance();
	auto* sceneMngr = rMngr->getSceneManager();

	flux_render::RenderScene* scene = sceneMngr->getScene(sceneID);
	if (!scene->createCamera(entityName, _nearDist, _farDist))
		throwFluxError(false, "Fallo al crear la cámara de la entidad" + entityName);

	_pendingCreation = false;
	return true;
}

bool flux_ec::CCamera::isPendingCreation() const
{
	return _pendingCreation;
}
