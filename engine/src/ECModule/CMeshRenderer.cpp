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

	flux_render::RenderManager* rMngr = flux_render::RenderManager::instance();
	flux_render::RenderSceneManager* sceneMngr = rMngr->getSceneManager();

	flux_render::RenderScene* currentScene = sceneMngr->getScene(sceneID);

	if (!currentScene->createMesh(entityName, _meshName, _materialName)) {
		throwFluxError(false, "Fallo al inicializar el componente MeshRenderer");
	}
}

void flux_ec::CMeshRenderer::update(float dt)
{

}
