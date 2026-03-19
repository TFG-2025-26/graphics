#pragma once

#ifndef I_RENDER_SCENE_BACKEND_H_
#define I_RENDER_SCENE_BACKEND_H_

#include <string>

namespace flux_render {
	class IRenderSceneBackend {
	public:
		virtual ~IRenderSceneBackend() = default;

		virtual bool init() = 0;
		virtual void shutdown() = 0;

		virtual bool createScene(const std::string& sceneID) = 0;
		virtual bool destroyScene(const std::string& sceneID) = 0;
		virtual bool setCurrentScene(const std::string& sceneID) = 0;

		virtual bool createSceneObject(
			const std::string& sceneID,
			const std::string& entityID) = 0;

		virtual bool destroySceneObject(
			const std::string& sceneID,
			const std::string& entityID) = 0;
	};
}

#endif // I_RENDER_SCENE_BACKEND_H_