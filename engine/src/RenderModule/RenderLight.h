#pragma once

#ifndef RENDER_LIGHT_H_
#define RENDER_LIGHT_H_

#include <string>
#include "RenderObject.h"

namespace Ogre {
	class SceneManager;
	class SceneNode;
	class Light;
}

namespace flux_utils {
	class Vector3;
}

namespace flux_render {

	class RenderLight: public RenderObject {
	public:
		RenderLight(const std::string& id, Ogre::SceneManager* mngr, Ogre::SceneNode* parent = nullptr);
		void setType(const std::string& type);
		void setDiffuseColour(flux_utils::Vector3 colour);
		void setDirection(flux_utils::Vector3 direction);
	private:
		Ogre::Light* _light;
	};
}
#endif