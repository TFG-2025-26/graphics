#pragma once

#ifndef OGRE_SCENE_BACKEND_H_
#define OGRE_SCENE_BACKEND_H_

#include "IRenderSceneBackend.h"

namespace flux_render {
	class RenderSceneManager;
	class UIManager;
}

class OgreBackend;

namespace flux_render {
	class OgreSceneBackend : public IRenderSceneBackend {
	public:
		OgreSceneBackend(OgreBackend* renderBackend, UIManager* uiManager);
		~OgreSceneBackend() override = default;

		bool init() override;
		void shutdown() override;

		bool createScene(const std::string& sceneID) override;
		bool destroyScene(const std::string& sceneID) override;
		bool setCurrentScene(const std::string& sceneID) override;

		bool createSceneObject(
			const std::string& sceneID,
			const std::string& entityID) override;

		bool destroySceneObject(
			const std::string& sceneID,
			const std::string& entityID) override;

		RenderSceneManager* getSceneManager() const;

	private:
		OgreBackend* _renderBackend = nullptr;
		UIManager* _uiManager = nullptr;
		RenderSceneManager* _sceneManager = nullptr;
	};
}


#endif // OGRE_SCENE_BACKEND_H_