#pragma once

#ifndef EXPORT_H_
#define EXPORT_H_

#ifdef FLUX_EXPORTS
#define FLUX_API __declspec(dllexport)
#else
#define FLUX_API __declspec(dllimport)
#endif

#include <Windows.h>

#include <list>

typedef void (*SceneFuncNoArgs)();
typedef void (*SceneFuncDt)(float);
typedef bool (*SceneFuncNoArgsBool)();

extern HMODULE game;
extern SceneFuncNoArgsBool loadScene;

extern SceneFuncNoArgs registerGameComponents;

namespace flux_physics {
	class PhysicsManager;
}

namespace flux_render {
	class RenderManager;
}

namespace flux_input {
	class InputManager;
}

namespace flux_audio {
	class AudioManager;
}

namespace flux_script {
	class ScriptManager;
}

namespace flux_utils {
	class SceneManager;
	class Manager;
}

namespace flux_export {
	class Export {
	public:
		FLUX_API void registerEngineComponents();

		FLUX_API bool initEngine();
		FLUX_API bool runEngine();
		FLUX_API bool stopEngine();
		FLUX_API bool callRunEngine();

	private:
		HMODULE game = nullptr;
		SceneFuncNoArgsBool loadScene = nullptr;

		SceneFuncNoArgs registerGameComponents = nullptr;

		std::list<flux_utils::Manager*> managers;

		flux_physics::PhysicsManager* physicsMng = nullptr;
		flux_render::RenderManager* rdrMngr = nullptr;
		flux_input::InputManager* inputMngr = nullptr;
		flux_audio::AudioManager* audioMng = nullptr;
		flux_script::ScriptManager* scriptMngr = nullptr;
		flux_utils::SceneManager* sceneMngr = nullptr;
	};
}


#endif // EXPORT_H_