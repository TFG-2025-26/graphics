#include "CMeshRenderer.h"

#include "Entity.h"

// ---- FLUX_RENDER ----
#include "RenderManager.h"
#include "RenderSceneManager.h"
#include "RenderScene.h"

#include "ComponentArguments.h"

// ---- FLUX_UTILS ----
#include "FluxError.h"

flux_ec::CMeshRenderer::~CMeshRenderer()
{
	if (getOwner() != nullptr && _meshName != "") {
		flux_render::RenderManager::instance()->getSceneManager()->
			getCurrentScene()->deleteSceneObject(getOwner()->getName());
	}
}

bool flux_ec::CMeshRenderer::init(flux_script::ComponentArguments* args)
{
	_meshName = args->getValueToString("Mesh");
	_materialName = args->getValueToString("Material");

	if (_materialName.empty()) _materialName = "MissingTexture";

	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();

	if (sceneBackend == nullptr) {
		throwFluxError(false, "No existe SceneBackend para MeshRenderer");
		return false;
	}

	if (!sceneBackend->attachMesh(sceneID, entityName, _meshName, _materialName)) {
		throwFluxError(false, "Fallo al inicializar el componente MeshRenderer");
		return false;
	}
}

void flux_ec::CMeshRenderer::update(float dt)
{

}
