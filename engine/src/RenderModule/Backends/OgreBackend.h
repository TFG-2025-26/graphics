#pragma once

#ifndef OGRE_BACKEND_H_
#define OGRE_BACKEND_H_

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

class SGTechniqueResolverListener;


#include <string>
#include <vector>

#include "IRenderBackend.h"

class OgreBackend : public IRenderBackend {
public:
	OgreBackend(const std::string& appName);
	~OgreBackend() override = default;

	BackendAPI getAPI() const override;

	bool init(const RenderBackendDesc& desc) override;
	void shutdown() override;
	void waitIdle() override;

	void resize(uint32_t width, uint32_t height) override;
	void setSync(bool enabled) override;

	bool beginFrame() override;
	void endFrame() override;

	bool createRoot();
	bool setup();

	void setResolutions(const std::vector<std::pair<int, int>>& resolutions);
	void locateResources();
	void loadResources();
	bool initialiseRTShaderSystem();
	bool removeSceneManagerFromRTShaderSystem(Ogre::SceneManager* sm);

	Ogre::Root* getRoot() const { return _root; }
	Ogre::RenderWindow* getRenderWindow() const { return _renderWindow; }

	bool addSceneManagerToRTShaderSystem(Ogre::SceneManager* sm);
private:
	bool createRenderWindow();

private:
	Ogre::Root* _root = nullptr;
	Ogre::RenderWindow* _renderWindow = nullptr;

	Ogre::FileSystemLayer* _FSLayer = nullptr;
	std::string _appName;
	std::string _solutionPath;

	std::string _RTShaderLibPath;
	Ogre::RTShader::ShaderGenerator* _shaderGenerator = nullptr;
	SGTechniqueResolverListener* _materialMgrListener = nullptr;

	std::vector<std::pair<int, int>> _resolutions;
	int _currRes = 0;
	bool _isFullScreen = false;
	uint32_t _fullW = 0, _fullH = 0;
	uint32_t _currW = 0, _currH = 0;
};

#endif // OGRE_BACKEND_H_