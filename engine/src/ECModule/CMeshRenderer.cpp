#include "CMeshRenderer.h"

#include "Entity.h"

// ---- FLUX_RENDER ----
#include "Backends/IRenderSceneBackend.h"
#include "RenderManager.h"
#include "RenderSceneManager.h"
#include "RenderScene.h"

#include "ComponentArguments.h"

// ---- FLUX_UTILS ----
#include "FluxError.h"

flux_ec::CMeshRenderer::~CMeshRenderer()
{
	// El RenderObject lo destruye CTransform
	// Más adelante podemos añadir detachMesh() si queremos permitir quitar solo la malla
}

bool flux_ec::CMeshRenderer::init(flux_script::ComponentArguments* args)
{
	_meshName = args->getValueToString("Mesh");
	_meshName = stripKnownMeshExtension(_meshName);
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

	return true;
}

void flux_ec::CMeshRenderer::update(float dt)
{

}

std::string flux_ec::CMeshRenderer::stripKnownMeshExtension(const std::string& meshName)
{
	std::string result = meshName;

	const std::string extensions[] = {
		".mesh",
		".fluxmesh",
		".obj",
		".gltf",
		".glb"
	};

	for (const auto& ext : extensions) {
		if (result.size() >= ext.size() &&
			result.compare(result.size() - ext.size(), ext.size(), ext) == 0) {
			result = result.substr(0, result.size() - ext.size());
			break;
		}
	}

	return result;
}
