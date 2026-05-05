#include "CTransform.h"

// --- FLUX_EC ---
#include "Entity.h"

// -- FLUX_UTILS --
#include "FluxError.h"
#include "Vector3.h"
#include "Vector4.h"

// ----- FLUX_RENDER -----
#include "Backends/IRenderSceneBackend.h"
#include "RenderManager.h"
#include "RenderSceneManager.h"
#include "RenderScene.h"
#include "RenderObject.h"
#include "CMeshRenderer.h"

// ---- STD ----
#include <string>

#include "ComponentArguments.h"

flux_ec::CTransform::~CTransform()
{
	if (_owner != nullptr) {
		const std::string entityName = _owner->getName();
		const std::string sceneID = _owner->getSceneID();

		const bool hasMeshRenderer = _owner->hasComponent(MESH);

		if (!hasMeshRenderer) {
			auto* renderManager = flux_render::RenderManager::instance();

			if (renderManager != nullptr) {
				auto* sceneBackend = renderManager->getSceneBackend();

				if (sceneBackend != nullptr) {
					sceneBackend->destroySceneObject(sceneID, entityName);
				}
			}
		}
	}

	delete _pos;
	_pos = nullptr;

	delete _rot;
	_rot = nullptr;

	delete _scale;
	_scale = nullptr;
}

bool flux_ec::CTransform::init(flux_script::ComponentArguments* args)
{
	_pos = new flux_utils::Vector3(args->getValueToVector3("Position"));
	_rot = new flux_utils::Vector4(args->getValueToVector4("Rotation"));
	_scale = new flux_utils::Vector3(args->getValueToVector3("Scale"));

	if (*_rot == flux_utils::Vector4(0, 0, 0, 0))
		*_rot = flux_utils::Vector4(0, 0, 0, 1);
	

	std::string entityName = getOwner()->getName();
	std::string sceneID = getOwner()->getSceneID();

	flux_render::RenderManager* renderMngr = flux_render::RenderManager::instance();

	auto* sceneBackend = flux_render::RenderManager::instance()->getSceneBackend();

	if (sceneBackend == nullptr) {
		throwFluxError(false, "No existe SceneBackend para crear el objeto '" + entityName + "'");
		return false;
	}

	if (!sceneBackend->createSceneObject(sceneID, entityName)) {
		throwFluxError(false, "No se pudo crear el objeto de render '" + entityName + "'");
		return false;
	}

	sceneBackend->setObjectTransform(sceneID, entityName, *_pos, *_rot, *_scale);

	return true;
}

void flux_ec::CTransform::update(float dt)
{

}

flux_utils::Vector3 flux_ec::CTransform::getPos() const
{
	return *_pos;
}

flux_utils::Vector4 flux_ec::CTransform::getRot() const
{
	return *_rot;
}

flux_utils::Vector3 flux_ec::CTransform::getScale() const
{
	return *_scale;
}

void flux_ec::CTransform::setPos(const flux_utils::Vector3& pos)
{
	if (_pos == nullptr) {
		return;
	}

	*_pos = pos;
	syncRenderTransform();
}

void flux_ec::CTransform::setRot(const flux_utils::Vector4& rot)
{
	if (_rot == nullptr) {
		return;
	}

	*_rot = rot;
	syncRenderTransform();
}

void flux_ec::CTransform::setScale(const flux_utils::Vector3& scale)
{
	if (_scale == nullptr) {
		return;
	}

	*_scale = scale;
	syncRenderTransform();
}

void flux_ec::CTransform::lookAt(const flux_utils::Vector3& pos)
{
	_renderObject->lookAt(pos);

	*_rot = _renderObject->getOrientation();
}

void flux_ec::CTransform::syncRenderTransform()
{
	if (_owner == nullptr || _pos == nullptr || _rot == nullptr || _scale == nullptr) {
		return;
	}

	auto* renderManager = flux_render::RenderManager::instance();
	if (renderManager == nullptr) {
		return;
	}

	auto* sceneBackend = renderManager->getSceneBackend();
	if (sceneBackend == nullptr) {
		return;
	}

	sceneBackend->setObjectTransform(
		_owner->getSceneID(),
		_owner->getName(),
		*_pos,
		*_rot,
		*_scale
	);
}