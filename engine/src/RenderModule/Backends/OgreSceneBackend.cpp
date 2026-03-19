#include "OgreSceneBackend.h"

#include "OgreBackend.h"
#include "../RenderSceneManager.h"
#include "../UIManager.h"
#include "../RenderScene.h"

flux_render::OgreSceneBackend::OgreSceneBackend(OgreBackend* renderBackend, UIManager* uiManager)
{
    _renderBackend = renderBackend;
    _uiManager = uiManager;
}

bool flux_render::OgreSceneBackend::init()
{
    if (_renderBackend == nullptr) return false;

    _sceneManager = new RenderSceneManager(_renderBackend->getRoot());

    if (_uiManager != nullptr) {
        _uiManager->setSceneManager(_sceneManager->getOgreSceneManager());
        _sceneManager->setUIManager(_uiManager);
    }

    _renderBackend->addSceneManagerToRTShaderSystem(
        _sceneManager->getOgreSceneManager()
    );

    return true;
}

void flux_render::OgreSceneBackend::shutdown()
{
    delete _sceneManager;
    _sceneManager = nullptr;
}

bool flux_render::OgreSceneBackend::createScene(const std::string& sceneID)
{
    return _sceneManager != nullptr && _sceneManager->createScene(sceneID) != nullptr;
}

bool flux_render::OgreSceneBackend::destroyScene(const std::string& sceneID)
{
    if (_sceneManager == nullptr) return false;
    _sceneManager->destroyScene(sceneID);
    return true;
}

bool flux_render::OgreSceneBackend::setCurrentScene(const std::string& sceneID)
{
    return _sceneManager != nullptr && _sceneManager->setCurrentScene(sceneID);
}

bool flux_render::OgreSceneBackend::createSceneObject(
    const std::string& sceneID,
    const std::string& entityID)
{
    if (_sceneManager == nullptr) return false;

    auto* scene = _sceneManager->getScene(sceneID);
    if (scene == nullptr) return false;

    scene->createSceneObject(entityID);
    return true;
}

bool flux_render::OgreSceneBackend::destroySceneObject(
    const std::string& sceneID,
    const std::string& entityID)
{
    if (_sceneManager == nullptr) return false;

    auto* scene = _sceneManager->getScene(sceneID);
    if (scene == nullptr) return false;

    scene->deleteSceneObject(entityID);
    return true;
}
