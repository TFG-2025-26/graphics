#pragma once

#ifndef RENDER_SCENE_MANAGER_H_
#define RENDER_SCENE_MANAGER_H_

#include <unordered_map>
#include <string>

#if defined(_MSC_VER) && defined(_DEBUG)
#define OGRE_DEBUG_NS_BEGIN namespace DEBUG_BUILD_REQUIRED {
#define OGRE_DEBUG_NS_END }
namespace Ogre { namespace DEBUG_BUILD_REQUIRED {} using namespace DEBUG_BUILD_REQUIRED; }
#else
#define OGRE_DEBUG_NS_BEGIN
#define OGRE_DEBUG_NS_END
#endif

namespace Ogre {
    OGRE_DEBUG_NS_BEGIN
    class Root;
    OGRE_DEBUG_NS_END
    class SceneManager;
}

namespace flux_render {
	class RenderScene;
    class UIManager;
	
	class RenderSceneManager
	{
    public:
        RenderSceneManager(Ogre::Root* root);
        virtual ~RenderSceneManager();

        RenderScene* createScene(const std::string& sceneID);
        void destroyScene(const std::string& sceneID);
        bool setCurrentScene(const std::string& sceneID);
        void setUIManager(flux_render::UIManager* ui);

        RenderScene* getCurrentScene() const;
        RenderScene* getScene(const std::string& sceneID) const;
        Ogre::SceneManager* getOgreSceneManager() const;
        const std::unordered_map<std::string, RenderScene*>& getAllScenes() const;
    private:
        Ogre::Root* _root;
        Ogre::SceneManager* _sceneManager;
        RenderScene* _currentScene;
        std::unordered_map<std::string, RenderScene*> _scenes;
        UIManager* _uiManager;
	};
}


#endif // RENDER_SCENE_MANAGER_H_