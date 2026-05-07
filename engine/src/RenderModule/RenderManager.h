#pragma once

#ifndef RENDER_MANAGER_H_
#define RENDER_MANAGER_H_

// --- FLUX_UTILS ---
#include "Manager.h"
#include "Singleton.h"

// ----- STD -----
#include <memory>
#include <string>
#include <vector>

// --- FLUX_RENDER ---
#include "Backends/IRenderBackend.h"
#include "Backends/IRenderSceneBackend.h"

// --- FLUX_PHYSICS ---
#include "DebugDrawer.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define OGRE_DEBUG_NS_BEGIN namespace DEBUG_BUILD_REQUIRED {
#define OGRE_DEBUG_NS_END }
namespace Ogre { namespace DEBUG_BUILD_REQUIRED {} using namespace DEBUG_BUILD_REQUIRED; }
#else
#define OGRE_DEBUG_NS_BEGIN
#define OGRE_DEBUG_NS_END
#endif

namespace Ogre {
	class FileSystemLayer;
	class RenderWindow;
	class SceneManager;
	OGRE_DEBUG_NS_BEGIN
	class Root;
	OGRE_DEBUG_NS_END
	namespace RTShader {
		class ShaderGenerator;
	}
}

class SDL_Window;

class SGTechniqueResolverListener;

typedef SDL_Window NativeWindowType;

class btDiscreteDynamicsWorld;

/// <summary>
/// Unión entre la ventana de renderizado y la ventana de la
/// plataforma específica.
/// </summary>
struct NativeWindowPair
{
	Ogre::RenderWindow* render = nullptr;
	NativeWindowType* native = nullptr;
};

class OgreBackend;

namespace flux_render {
	class UIManager;
	class RenderSceneManager;
	/// <summary>
	/// Clase base responsable de establecer un contexto común para las aplicaciones.
	/// La subclase implementa adicionalmente eventos específicos de callbacks.
	/// </summary>
	class RenderManager : public flux_utils::Manager, 
		public flux_utils::Singleton<RenderManager>
	{
	public:
		friend flux_utils::Singleton<RenderManager>;
		virtual ~RenderManager();

		/// <summary>
		/// Devuelve la ventana de renderizado.
		/// </summary>
		/// <returns>La ventana de renderizado</returns>
		Ogre::RenderWindow* getRenderWindow() const;

		Ogre::Root* getRoot() const;

		/// <summary>
		/// Inicialización del sistema de renderizado y recursos.
		/// </summary>
		bool init() override;

		/// <summary>
		/// Actualiza el renderizado de un frame de OGRE y de SDL.
		/// </summary>
		void update(float dt) override;

		/// <summary>
		/// Previo a cerrar la applicación, guarda la configuración.
		/// </summary>
		bool shutdown() override;

		FLUX_API void setWindowName(const std::string& windowName);
		// FLUX_API void setWindowIcon(std::string& windowIcon);

		FLUX_API void setResolutions(const std::vector<std::pair<int, int>>& resolutions);
		FLUX_API void changeResolution();
		FLUX_API void fullScreen();

		FLUX_API void nextResolution();
		FLUX_API void previousResolution();

		void changeWindowSize(uint32_t width, uint32_t height);
		void setSync(bool enabled);

		FLUX_API void enablePhysicsDebugDraw(btDiscreteDynamicsWorld* world, bool enable);
		FLUX_API UIManager* getUIManager() const;

		FLUX_API void setBackendAPI(BackendAPI api);
		FLUX_API BackendAPI getBackendAPI() const;

		IRenderBackend* getBackend() const;
		OgreBackend* getOgreBackend() const;
		SDL_Window* getNativeWindow() const;

		RenderSceneManager* getSceneManager() const;
		IRenderSceneBackend* getSceneBackend() const;
	private:
		RenderManager(const std::string& appName = "FLUX_ENGINE");

		void updateAspectRatio();

		bool createNativeWindow();
		bool createBackend();

		SDL_Window* _nativeWindow = nullptr;
		std::unique_ptr<IRenderBackend> _backend = nullptr;
		std::unique_ptr<IRenderSceneBackend> _sceneBackend = nullptr;
		BackendAPI _selectedAPI = BackendAPI::Ogre;

		bool _vsync = true;
	protected:
		Ogre::Root* _root;        // raíz de OGRE
		NativeWindowPair _window; // ventana principal

		Ogre::FileSystemLayer* _FSLayer; // capa de abstracción de sistema de ficheros
		bool _firstRun;
		std::string _appName; // nombre de la ventana
		std::string _solutionPath; // ubicación de la solución

		std::string _RTShaderLibPath;
		Ogre::RTShader::ShaderGenerator* _shaderGenerator; // instancia de generador de shaders
		SGTechniqueResolverListener* _materialMgrListener; // gestor de listener de materiales del generador de shaders

		RenderSceneManager* _sceneMngr; // gestor de escenas

		std::vector<std::pair<int, int>> _resolutions;
		int _currRes = 0;
		bool _isFullScreen = false;
		uint32_t _fullW = 0, _fullH = 0;
		uint32_t _currW = 800, _currH = 600;

		UIManager* _uiManager;
	};
}

#endif // RENDER_MANAGER_H_