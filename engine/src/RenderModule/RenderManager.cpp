#include "RenderManager.h"

// ---------- OGRE ----------
#include <OgreRoot.h>
#include <OgreGpuProgramManager.h>
#include <OgreConfigFile.h>
#include <OgreRenderWindow.h>
#include <OgreViewport.h>
#include <OgreDataStream.h>
#include <OgreFileSystemLayer.h>
#include <OgrePrerequisites.h>
#include <OgreShaderGenerator.h>
#include <OgreMaterialManager.h>
#include <OgreOverlaySystem.h>

// ----- SDL -----
#include <SDL.h>
#include <SDL_video.h>
#include <SDL_syswm.h>

#include "Backends/OgreBackend.h"
#include "Backends/D3D12Backend.h"
#include "RenderSceneManager.h"
#include "RenderScene.h"
#include "FluxError.h"

//---------- debug-----------
#include "DebugDrawer.h"
#include <btBulletDynamicsCommon.h>
#include <iostream>

#include "UIManager.h"


static DebugDrawer* _debugDrawer = nullptr;

flux_render::RenderManager::RenderManager(const std::string& appName)
{
	_appName = appName;
	// _FSLayer = new Ogre::FileSystemLayer(_appName);
	_root = nullptr;
	_firstRun = true;

	_shaderGenerator = nullptr;
	_materialMgrListener = nullptr;

	_sceneMngr = nullptr;

	_uiManager = nullptr;
}

void flux_render::RenderManager::nextResolution()
{
	if (!_isFullScreen && !_resolutions.empty()) {
		_currRes = (_currRes + 1) % _resolutions.size();
		changeResolution();
	}
}

void flux_render::RenderManager::previousResolution()
{
	if (!_isFullScreen && !_resolutions.empty()) {
		--_currRes;
		if (_currRes < 0) _currRes = static_cast<int>(_resolutions.size()) - 1;
		changeResolution();
	}
}

void flux_render::RenderManager::updateAspectRatio()
{
	if (!_window.render || !_sceneMngr) return;

	Ogre::Viewport* vp = _window.render->getViewport(0);
	if (!vp) return;

	Ogre::Camera* cam = vp->getCamera();
	if (!cam) return;

	Ogre::Real aspectRatio = static_cast<Ogre::Real>(vp->getActualWidth()) / (vp->getActualHeight());
	cam->setAspectRatio(aspectRatio);
}

bool flux_render::RenderManager::createNativeWindow()
{
	if (_nativeWindow != nullptr) return true;

	_nativeWindow = SDL_CreateWindow(
		_appName.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		static_cast<int>(_currW),
		static_cast<int>(_currH),
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	if (_nativeWindow == nullptr) {
		return false;
	}

	return true;
}

bool flux_render::RenderManager::createBackend()
{
	switch (_selectedAPI) {
		case BackendAPI::Ogre:
			_backend = std::make_unique<OgreBackend>(_appName);
			return true;
		case BackendAPI::D3D12:
			_backend = std::make_unique<D3D12Backend>(_appName);
			return true;
		default:
			return false;
	}
}

void flux_render::RenderManager::enablePhysicsDebugDraw(btDiscreteDynamicsWorld* world, bool enable) {
	if (enable && world) {
		if (!_debugDrawer) {
			_debugDrawer = new DebugDrawer(_sceneMngr->getOgreSceneManager());
			_debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
			world->setDebugDrawer(static_cast<btIDebugDraw*>(_debugDrawer->getBtIDebugDraw()));
			std::cout<<"DebugDrawer creado/-------------------------------------------------------------------------------//"<<std::endl;
		}
	}
	else {
		if (_debugDrawer) {
			delete _debugDrawer;
			_debugDrawer = nullptr;
		}
	}
}

flux_render::RenderManager::~RenderManager()
{

}

Ogre::RenderWindow* flux_render::RenderManager::getRenderWindow() const
{
	return _window.render;
}

Ogre::Root* flux_render::RenderManager::getRoot() const
{
	return _root;
}

bool flux_render::RenderManager::init()
{
	if (!createNativeWindow()) return false;
	if (!createBackend()) {
		SDL_DestroyWindow(_nativeWindow);
		_nativeWindow = nullptr;
		return false;
	}

	RenderBackendDesc desc;
	desc.nativeWindow = _nativeWindow;
	desc.appName = _appName;
	desc.width = _currW;
	desc.height = _currH;
	desc.vsync = _vsync;

	if (!_backend->init(desc)) {
		_backend.reset();
		SDL_DestroyWindow(_nativeWindow);
		_nativeWindow = nullptr;
		return false;
	}

	// ----- PUENTE TEMPORAL A OGRE -----
	if (_selectedAPI == BackendAPI::Ogre) {
		OgreBackend* ogreBackend = getOgreBackend();
		if (ogreBackend == nullptr) {
			_backend->shutdown();
			_backend.reset();
			SDL_DestroyWindow(_nativeWindow);
			_nativeWindow = nullptr;
			return false;
		}

		// Estos ya NO son owners: solo aliases temporales
		_root = ogreBackend->getRoot();
		_window.native = _nativeWindow;
		_window.render = ogreBackend->getRenderWindow();

		_sceneMngr = new RenderSceneManager(_root);

		_uiManager = new UIManager();
		_uiManager->init();
		_uiManager->setSceneManager(_sceneMngr->getOgreSceneManager());
		_sceneMngr->setUIManager(_uiManager);

		RenderScene* scene = _sceneMngr->createScene("SampleScene");
		if (!_sceneMngr->setCurrentScene("SampleScene")) {
			throwFluxError(false, "Error al establecer una escena");
		}

		ogreBackend->addSceneManagerToRTShaderSystem(
			_sceneMngr->getOgreSceneManager()
		);
	}

	isInitialized = true;
	return true;
}

void flux_render::RenderManager::update(float dt)
{
	if (_backend == nullptr) return;
	if (!_backend->beginFrame()) return;

	if (_uiManager != nullptr) {
		_uiManager->update(dt);
	}

	if (_debugDrawer) {
		_debugDrawer->clear();
	}

	_backend->endFrame();
}

bool flux_render::RenderManager::shutdown()
{
	// 0) Destruye el renderizado en modo debug
	if (_debugDrawer != nullptr) {
		delete _debugDrawer;
		_debugDrawer = nullptr;
	}

	// 1) Destruye primero los sistemas del motor que usan Ogre
	if (_sceneMngr != nullptr) {
		delete _sceneMngr;
		_sceneMngr = nullptr;
	}

	if (_uiManager != nullptr) {
		delete _uiManager;
		_uiManager = nullptr;
	}

	// 2) Apagar backend
	if (_backend != nullptr) {
		_backend->waitIdle();
		_backend->shutdown();
		_backend.reset();
	}

	// 3) Invalida aliases temporales a Ogre
	_root = nullptr;
	_window.render = nullptr;

	// 4) Destruye ventana nativa
	if (_nativeWindow != nullptr) {
		SDL_DestroyWindow(_nativeWindow);
		_nativeWindow = nullptr;
	}

	_window.native = nullptr;

	return true;
}

void flux_render::RenderManager::setWindowName(const std::string& windowName)
{
	_appName = windowName;
	if (_nativeWindow != nullptr) {
		SDL_SetWindowTitle(_window.native, windowName.c_str());
	}
}

void flux_render::RenderManager::setResolutions(const std::vector<std::pair<int, int>>& resolutions)
{
	std::map<int, std::pair<int, int>> ordered;

	for (const auto& res : resolutions) {
		if ((uint32_t)res.first < _fullW && (uint32_t)res.second < _fullH) {
			int key = res.first + static_cast<int>(std::sqrt(res.first * res.first + res.second * res.second));
			ordered[key] = res;
		}
	}

	_resolutions.clear();
	for (const auto& it : ordered) {
		_resolutions.push_back(it.second);
	}

	if (_resolutions.empty()) _resolutions.push_back({ 800, 600 });

	_currRes = 0;
}

void flux_render::RenderManager::changeResolution()
{
	if (!_isFullScreen && !_resolutions.empty()) {
		_currW = _resolutions[_currRes].first;
		_currH = _resolutions[_currRes].second;
		changeWindowSize(_currW, _currH);
	}

	std::cout << _currW << "x" << _currH << std::endl;
}

void flux_render::RenderManager::fullScreen()
{
	uint32_t w, h;
	if (_isFullScreen) {
		w = _currW;
		h = _currH;
	}
	else {
		SDL_DisplayMode displayMode;
		SDL_GetCurrentDisplayMode(0, &displayMode);
		_fullW = displayMode.w;
		_fullH = displayMode.h;

		w = _fullW;
		h = _fullH;
	}

	_isFullScreen = !_isFullScreen;
	changeWindowSize(w, h);
}

void flux_render::RenderManager::changeWindowSize(uint32_t width, uint32_t height)
{
	_currW = width;
	_currH = height;

	if (_nativeWindow != nullptr) {
		SDL_SetWindowSize(_nativeWindow, static_cast<int>(width), static_cast<int>(height));
	}

	if (_backend != nullptr) {
		_backend->resize(width, height);
	}
}

void flux_render::RenderManager::setSync(bool enabled)
{
	_vsync = enabled;

	if (_backend != nullptr) {
		_backend->setSync(enabled);
	}
}

flux_render::RenderSceneManager* flux_render::RenderManager::getSceneManager() const
{
	return _sceneMngr;
}

flux_render::UIManager* flux_render::RenderManager::getUIManager() const
{
	return _uiManager;
}

IRenderBackend* flux_render::RenderManager::getBackend() const
{
	return _backend.get();
}

OgreBackend* flux_render::RenderManager::getOgreBackend() const
{
	if (_backend == nullptr) return nullptr;
	if (_backend->getAPI() != BackendAPI::Ogre) return nullptr;

	return static_cast<OgreBackend*>(_backend.get());
}

SDL_Window* flux_render::RenderManager::getNativeWindow() const
{
	return _nativeWindow;
}