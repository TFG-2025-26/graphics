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

	auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();

	if (sceneBackend == nullptr) {
		throwFluxError(false, "No existe SceneBackend para inicializar la camara de la entidad " +
			entityName);
		return false;
	}

	if (!sceneBackend->isCurrentScene(sceneID)) {
		_pendingCreation = true;
		return true;
	}

	return createCamera();
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

	auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();

	if (sceneBackend == nullptr) {
		throwFluxError(false, "No existe SceneBackend para Camera");
		return false;
	}

	if (!sceneBackend->createCamera(sceneID, entityName, _nearDist, _farDist)) {
		throwFluxError(false, "Fallo al crear la camara de la entidad " + entityName);
		return false;
	}

	_pendingCreation = false;
	return true;
}

bool flux_ec::CCamera::isPendingCreation() const
{
	return _pendingCreation;
}
