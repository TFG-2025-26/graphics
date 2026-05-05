#include "OgreSceneBackend.h"

#include "OgreBackend.h"
#include "../RenderSceneManager.h"
#include "../UIManager.h"
#include "../RenderScene.h"
#include "../RenderObject.h"

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
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;

    scene->createSceneObject(entityID);
    return true;
}

bool flux_render::OgreSceneBackend::destroySceneObject(
    const std::string& sceneID,
    const std::string& entityID)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;

    scene->deleteSceneObject(entityID);
    return true;
}

bool flux_render::OgreSceneBackend::setObjectTransform(
    const std::string& sceneID,
    const std::string& entityID,
    const flux_utils::Vector3& position,
    const flux_utils::Vector4& rotation,
    const flux_utils::Vector3& scale)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;

    auto* obj = scene->getRenderObject(entityID);
    if (obj == nullptr) return false;

    obj->setPosition(position);
    obj->setOrientation(rotation);
    obj->setScale(scale);
    return true;
}

bool flux_render::OgreSceneBackend::attachMesh(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string& meshName,
    const std::string& materialName)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;
    return scene->createMesh(entityID, meshName, materialName);
}

bool flux_render::OgreSceneBackend::createLight(
    const std::string& sceneID,
    const std::string& entityID,
    const flux_utils::Vector3& diffuseColor,
    uint8_t lightType)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;
    return scene->createLight(entityID, diffuseColor, lightType);
}

bool flux_render::OgreSceneBackend::createCamera(
    const std::string& sceneID,
    const std::string& entityID,
    int8_t nearDist,
    int16_t farDist)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;
    return scene->createCamera(entityID, nearDist, farDist);
}

bool flux_render::OgreSceneBackend::addAnimation(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string& animationName)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;
    return scene->createAnimation(entityID, animationName);
}

bool flux_render::OgreSceneBackend::removeAnimation(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string& animationName)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;
    scene->deleteAnimation(entityID, animationName);
    return true;
}

bool flux_render::OgreSceneBackend::setAnimationEnabled(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string& animationName,
    bool enabled)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;
    scene->setAnimationEnabled(entityID, animationName, enabled);
    return true;
}

bool flux_render::OgreSceneBackend::setAnimationLoop(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string& animationName,
    bool loop)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;
    scene->setAnimationLoop(entityID, animationName, loop);
    return true;
}

bool flux_render::OgreSceneBackend::updateAnimation(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string& animationName,
    float dt)
{
    auto* scene = getScene(sceneID);
    if (scene == nullptr) return false;
    scene->updateAnimations(entityID, dt);
    return true;
}

bool flux_render::OgreSceneBackend::isCurrentScene(const std::string& sceneID) const
{
    if (_sceneManager == nullptr) return false;

    auto* scene = _sceneManager->getScene(sceneID);
    auto* currentScene = _sceneManager->getCurrentScene();

    return scene != nullptr && scene == currentScene;
}

flux_render::RenderSceneManager* flux_render::OgreSceneBackend::getSceneManager() const
{
    return _sceneManager;
}

flux_render::RenderScene* flux_render::OgreSceneBackend::getScene(const std::string& sceneID) const
{
    if (_sceneManager == nullptr) return nullptr;
    return _sceneManager->getScene(sceneID);
}
