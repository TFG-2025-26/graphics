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
	_FSLayer = new Ogre::FileSystemLayer(_appName);
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
	delete _FSLayer;
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
	if (!createRoot()) {
		throwFluxError(false, "Error al crear la raíz en el módulo render");
	}

	setup();

	_sceneMngr = new RenderSceneManager(_root);

	_uiManager = new UIManager();
	_uiManager->init();
	_uiManager->setSceneManager(_sceneMngr->getOgreSceneManager());
	_sceneMngr->setUIManager(_uiManager);
	// _sceneMngr->addRenderQueueListener(_overlaySystem);

	RenderScene* scene = _sceneMngr->createScene("SampleScene");
	if (!_sceneMngr->setCurrentScene("SampleScene")) {
		throwFluxError(false, "Error al setear una escena");
	}

	//_shaderGenerator->addSceneManager(_sceneMngr->getOgreSceneManager());
	//scene->createSceneObject("SampleObject");

	//_sceneMngr = _root->createSceneManager();
	//_sceneMngr->getOgreSceneManager()->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));
	_shaderGenerator->addSceneManager(_sceneMngr->getOgreSceneManager());

	isInitialized = true;
	return true;
}

void flux_render::RenderManager::update(float dt)
{
	_root->renderOneFrame();
	_uiManager->update(dt);
	_window.render->update();
	if (_debugDrawer) {
		_debugDrawer->clear();
	}
}

bool flux_render::RenderManager::shutdown()
{
	if (isInitialized) {
		_uiManager->shutdown();
		delete _uiManager;
		closeAll();

		if (_debugDrawer != nullptr) {
			delete _debugDrawer;
			_debugDrawer = nullptr;
		}

		delete _sceneMngr;
		_sceneMngr = nullptr;

		

		if (_root != nullptr) {
			_root->saveConfig();
		}

		delete _root;
		_root = nullptr;

	
		isInitialized = false;
		return true;
	}
	else return false;
}

bool flux_render::RenderManager::createRoot()
{
	std::string pluginsPath;
	std::string nameFile = "plugins.cfg";
	pluginsPath = _FSLayer->getConfigFilePath("plugins.cfg");

	if (!Ogre::FileSystemLayer::fileExists(pluginsPath)) {
		//OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND, "plugins.cfg", "RenderManager::createRoot");
		throwFluxError(false, "El archivo plugins.cfg no ha sido encontrado");
	}
	_solutionPath = pluginsPath;

	_solutionPath.resize(_solutionPath.size() - nameFile.size());


	_root = new Ogre::Root(pluginsPath, _FSLayer->getWritablePath("ogre.cfg"), _FSLayer->getWritablePath("ogre.log"));

	return true;
}

void flux_render::RenderManager::closeAll()
{
	destroyRTShaderSystem();

	if (_window.render != nullptr) {
		_root->destroyRenderTarget(_window.render);
		_window.render = nullptr;
	}

	if (_window.native != nullptr) {
		SDL_DestroyWindow(_window.native);
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		_window.native = nullptr;
	}
}

void flux_render::RenderManager::setup()
{
	_root->showConfigDialog(nullptr);
	_root->initialise(false);
	createWindow(_appName);
	setWindowGrab(false);

	// --- [1] Detectar resolución máxima real del sistema ---
	_fullW = 0;
	_fullH = 0;

	int displayIndex = 0; // pantalla principal
	int numModes = SDL_GetNumDisplayModes(displayIndex);

	std::vector<std::pair<int, int>> validRes;

	if (numModes > 0) {
		std::cout << "----------------- CREACION DE RESOLUCIONES -----------------" << std::endl;
		for (int i = 0; i < numModes; ++i) {
			SDL_DisplayMode mode;
			if (SDL_GetDisplayMode(displayIndex, i, &mode) == 0) {
				// Guardar resolución máxima
				if (mode.w * mode.h > _fullW * _fullH) {
					_fullW = mode.w;
					_fullH = mode.h;
				}

				std::pair<int, int> res = { mode.w, mode.h };

				// evitar duplicados
				if (std::find(validRes.begin(), validRes.end(), res) == validRes.end()) {
					std::cout << res.first << "x" << res.second << std::endl;
					validRes.push_back(res);
				}
			}
		}

		std::sort(validRes.begin(), validRes.end(), [](const auto& a, const auto& b) {
			return a.first * a.second < b.first * b.second;
			});
	}

	std::cout << "[Max Resolution Detected]: " << _fullW << "x" << _fullH << std::endl;

	// --- [2] Establecer resoluciones válidas ---
	setResolutions(validRes);

	// --- [3] Cargar recursos y shaders ---
	locateResources();
	initialiseRTShaderSystem();
	loadResources();
}

bool flux_render::RenderManager::oneTimeConfig()
{
	if (!_root->restoreConfig()) {
		// return _root->showConfigDialog(OgreBites::getNativeConfigDialog());
		return false;
	}
	else return true;
}

bool flux_render::RenderManager::initialiseRTShaderSystem()
{
	if (Ogre::RTShader::ShaderGenerator::initialize()) {
		_shaderGenerator = Ogre::RTShader::ShaderGenerator::getSingletonPtr();

		if (_RTShaderLibPath.empty())
		{
			//throwFluxError(false, "No se encontro RTShaderLibPath");
			return false;
		}
	}

	return true;
}

void flux_render::RenderManager::destroyRTShaderSystem()
{
	Ogre::MaterialManager::getSingleton().setActiveScheme(Ogre::MaterialManager::DEFAULT_SCHEME_NAME);

	if (_shaderGenerator != nullptr) {
		Ogre::RTShader::ShaderGenerator::destroy();
		_shaderGenerator = nullptr;
	}
}

NativeWindowPair flux_render::RenderManager::createWindow(const std::string& name)
{
	Ogre::NameValuePairList miscParams;

	Ogre::ConfigOptionMap ropts = _root->getRenderSystem()->getConfigOptions();

	std::istringstream mode(ropts["Video Mode"].currentValue);
	std::string token;
	mode >> _currW;
	mode >> token;
	mode >> _currH;

	miscParams["FSAA"] = ropts["FSAA"].currentValue;
	miscParams["vsync"] = ropts["VSync"].currentValue;
	miscParams["gamma"] = ropts["sRGB Gamma Conversion"].currentValue;

	if (!SDL_WasInit(SDL_INIT_VIDEO)) SDL_InitSubSystem(SDL_INIT_VIDEO);

	Uint32 flags = 0;

	if (ropts["Full Screen"].currentValue == "Yes") flags = SDL_WINDOW_FULLSCREEN;

	_window.native = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, 
		SDL_WINDOWPOS_UNDEFINED, _currW, _currH, flags);

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(_window.native, &wmInfo);

	miscParams["externalWindowHandle"] = Ogre::StringConverter::toString(size_t(wmInfo.info.win.window));

	_window.render = _root->createRenderWindow(name, _currW, _currH, false, &miscParams);

	return _window;
}

void flux_render::RenderManager::setWindowName(const std::string& windowName)
{
	_appName = windowName;
	SDL_SetWindowTitle(_window.native, windowName.c_str());
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
		changeWindowSize(_resolutions[_currRes].first, _resolutions[_currRes].second);
		_currH = _resolutions[_currRes].first;
		_currW = _resolutions[_currRes].second;
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

void flux_render::RenderManager::changeWindowSize(int w, int h)
{
	SDL_SetWindowSize(_window.native, w, h);
	SDL_SetWindowPosition(_window.native, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	_window.render->windowMovedOrResized();
	updateAspectRatio();
}

flux_render::RenderSceneManager* flux_render::RenderManager::getSceneManager() const
{
	return _sceneMngr;
}

flux_render::UIManager* flux_render::RenderManager::getUIManager() const
{
	return _uiManager;
}

void flux_render::RenderManager::setWindowGrab(bool _grab)
{
	SDL_bool grab = SDL_bool(_grab);
	SDL_SetWindowGrab(_window.native, grab);
	// SDL_SetRelativeMouseMode(grab);
	SDL_ShowCursor(grab);
}

void flux_render::RenderManager::loadResources()
{
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void flux_render::RenderManager::locateResources()
{
	std::string assetsPath = Ogre::FileSystemLayer::resolveBundlePath(_solutionPath + "assets\\");

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
		assetsPath + "animations",
		"FileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
		assetsPath + "fonts",
		"FileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
		assetsPath + "materials",
		"FileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
		assetsPath + "meshes",
		"FileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
		assetsPath + "textures",
		"FileSystem", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
}