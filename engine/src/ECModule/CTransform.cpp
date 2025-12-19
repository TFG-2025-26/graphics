#include "CTransform.h"

// --- FLUX_EC ---
#include "Entity.h"

// -- FLUX_UTILS --
#include "Vector3.h"
#include "Vector4.h"

// ----- FLUX_RENDER -----
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
	delete _pos;
	delete _rot;
	delete _scale;

	if (!dynamic_cast<CMeshRenderer*>(_owner->getComponent(MESH)))
	{
		flux_render::RenderManager::instance()->getSceneManager()->
			getCurrentScene()->deleteSceneObject(getOwner()->getName());
	}
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
	flux_render::RenderScene* renderScene = renderMngr->getSceneManager()->getScene(sceneID);

	renderScene->createSceneObject(entityName);
	_renderObject = renderScene->getRenderObject(entityName);

	_renderObject->setPosition(*_pos);
	_renderObject->setOrientation(*_rot);
	_renderObject->setScale(*_scale);

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
	*_pos = pos;

	_renderObject->setPosition(pos);
}

void flux_ec::CTransform::setRot(const flux_utils::Vector4& rot)
{
	*_rot = rot;

	_renderObject->setOrientation(rot);
}

void flux_ec::CTransform::setScale(const flux_utils::Vector3& scale)
{
	*_scale = scale;

	_renderObject->setScale(scale);
}

void flux_ec::CTransform::lookAt(const flux_utils::Vector3& pos)
{
	_renderObject->lookAt(pos);

	*_rot = _renderObject->getOrientation();
}
