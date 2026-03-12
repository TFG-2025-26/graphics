#include "OgreBackend.h"

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

#include <iostream>

#include "FluxError.h"

OgreBackend::OgreBackend(const std::string& appName)
{
	_appName = appName;
	_FSLayer = new Ogre::FileSystemLayer(_appName);
}

BackendAPI OgreBackend::getAPI() const
{
	return BackendAPI::Ogre;
}

bool OgreBackend::init(const RenderBackendDesc& desc)
{
	_nativeWindow = desc.nativeWindow;
	_width = desc.width;
	_height = desc.height;
	_vsync = desc.vsync;
	_appName = desc.appName.empty() ? _appName : desc.appName;

	if (_nativeWindow == nullptr) {
		throwFluxError(false, "OgreBackend::init -> nativeWindow es nullptr");
	}

	_currW = _width;
	_currH = _height;

	if (!createRoot()) {
		throwFluxError(false, "Error al crear la raiz en OgreBackend");
	}

	if (!setup()) {
		throwFluxError(false, "Error durante la inicializacion de Ogre");
	}

	setSync(_vsync);

	return true;
}

void OgreBackend::shutdown()
{
	if (_root != nullptr) {
		_root->saveConfig();
	}

	if (_renderWindow != nullptr && _root != nullptr) {
		_root->destroyRenderTarget(_renderWindow);
		_renderWindow = nullptr;
	}

	delete _root;
	_root = nullptr;

	delete _FSLayer;
	_FSLayer = nullptr;
}

void OgreBackend::waitIdle()
{
	// ...
}

void OgreBackend::resize(uint32_t width, uint32_t height)
{
	if (height == 0) return;

	_width = width;
	_height = height;
	_currW = width;
	_currH = height;

	if (_renderWindow == nullptr) return;

	_renderWindow->resize(width, height);
	_renderWindow->windowMovedOrResized();

	Ogre::Viewport* vp = _renderWindow->getViewport(0);
	if (!vp) return;

	Ogre::Camera* cam = vp->getCamera();
	if (!cam) return;

	cam->setAspectRatio(
		static_cast<Ogre::Real>(width) / static_cast<Ogre::Real>(height)
	);
}

void OgreBackend::setSync(bool enabled)
{
	_vsync = enabled;
	if (_renderWindow != nullptr) {
		_renderWindow->setVSyncEnabled(enabled);
	}
}

bool OgreBackend::beginFrame()
{
	if (_root == nullptr || _renderWindow == nullptr) return false;
	if (_renderWindow->isClosed()) return false;

	return true;
}

void OgreBackend::endFrame()
{
	if (_root == nullptr || _renderWindow == nullptr) return;
	if (_renderWindow->isClosed()) return;

	_root->renderOneFrame();
}

bool OgreBackend::createRenderWindow()
{
	if (_nativeWindow == nullptr) return false;

	Ogre::NameValuePairList miscParams;
	Ogre::ConfigOptionMap ropts = _root->getRenderSystem()->getConfigOptions();

	miscParams["FSAA"] = ropts["FSAA"].currentValue;
	miscParams["vsync"] = _vsync ? "Yes" : "No";
	miscParams["gamma"] = ropts["sRGB Gamma Conversion"].currentValue;

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);

	if (!SDL_GetWindowWMInfo(_nativeWindow, &wmInfo)) {
		return false;
	}

	miscParams["externalWindowHandle"] =
		Ogre::StringConverter::toString(size_t(wmInfo.info.win.window));

	_renderWindow = _root->createRenderWindow(
		_appName,
		_currW,
		_currH,
		false,
		&miscParams
	);

	return _renderWindow != nullptr;
}

bool OgreBackend::addSceneManagerToRTShaderSystem(Ogre::SceneManager* sm)
{
	if (_shaderGenerator == nullptr || sm == nullptr) return false;
	_shaderGenerator->addSceneManager(sm);
	return true;
}

bool OgreBackend::createRoot()
{
	std::string pluginsPath;
	std::string nameFile = "plugins.cfg";
	pluginsPath = _FSLayer->getConfigFilePath("plugins.cfg");

	if (!Ogre::FileSystemLayer::fileExists(pluginsPath)) {
		throwFluxError(false, "El archivo plugins.cfg no ha sido encontrado");
	}

	_solutionPath = pluginsPath;
	_solutionPath.resize(_solutionPath.size() - nameFile.size());

	_root = new Ogre::Root(pluginsPath, _FSLayer->getWritablePath("ogre.cfg"),
		_FSLayer->getWritablePath("ogre.log"));

	return true;
}

bool OgreBackend::setup()
{
	_root->showConfigDialog(nullptr);
	_root->initialise(false);

	if (!createRenderWindow()) {
		throwFluxError(false, "No se pudo crear la RenderWindow de Ogre");
	}

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

	return true;
}

void OgreBackend::setResolutions(const std::vector<std::pair<int, int>>& resolutions)
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

void OgreBackend::locateResources()
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

void OgreBackend::loadResources()
{
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

bool OgreBackend::initialiseRTShaderSystem()
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
