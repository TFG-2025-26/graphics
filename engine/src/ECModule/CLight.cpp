#include "CLight.h"

// --- FLUX_EC ---
#include "Entity.h"

// -- FLUX_UTILS --
#include "Vector3.h"
#include "FluxError.h"

// ---- FLUX_RENDER ----
#include "RenderManager.h"
#include "RenderSceneManager.h"
#include "RenderScene.h"

// ---- STD ----
#include <string>

#include "ComponentArguments.h"

flux_ec::CLight::~CLight()
{
	delete _diffuseColor;
}

bool flux_ec::CLight::init(flux_script::ComponentArguments* args)
{
	_diffuseColor = new flux_utils::Vector3(args->getValueToVector3("Color"));
	_lightType = LightType::DIRECTIONAL; // cambiar luego

	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	flux_render::RenderManager* rMngr = flux_render::RenderManager::instance();
	flux_render::RenderSceneManager* sceneMngr = rMngr->getSceneManager();

	flux_render::RenderScene* currentScene = sceneMngr->getScene(sceneID);

	if (!currentScene->createLight(entityName, *_diffuseColor, _lightType)) {
		throwFluxError(false, "Fallo al inicializar el componente Light");
	}

	return true;
}

void flux_ec::CLight::update(float dt)
{
}

flux_utils::Vector3 flux_ec::CLight::getDiffuseColor() const
{
	return *_diffuseColor;
}

void flux_ec::CLight::setDiffuseColor(const flux_utils::Vector3& color)
{
	*_diffuseColor = color;
}
