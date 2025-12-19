#include "RenderLight.h"

// --------- OGRE ---------
#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>
#include <OgreMovableObject.h>

// -- FLUX_UTILS --
#include "Vector3.h"

flux_render::RenderLight::RenderLight(const std::string& id, Ogre::SceneManager* mngr, Ogre::SceneNode* parent)
	:RenderObject(id,mngr,parent)
{
	_light = mngr->createLight();
	_node->attachObject(_light);
}

void flux_render::RenderLight::setType(const std::string& type) {
	if (type == "directional") {
		_light->setType(Ogre::Light::LT_DIRECTIONAL);
	}
	else if (type == "point") {
		_light->setType(Ogre::Light::LT_POINT);
	}
	else if (type == "spotlight") {
		_light->setType(Ogre::Light::LT_SPOTLIGHT);
	}

}

void flux_render::RenderLight::setDiffuseColour(flux_utils::Vector3 colour) {
	_light->setDiffuseColour(colour.getX(),colour.getY(),colour.getZ());
}

void flux_render::RenderLight::setDirection(flux_utils::Vector3 direction) {
	_node->setDirection({direction.getX(),direction.getY(),direction.getZ()});
}