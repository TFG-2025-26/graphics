#include "D3D12SceneBackend.h"
#include "D3D12Backend.h"

flux_render::D3D12SceneBackend::D3D12SceneBackend(D3D12Backend* renderBackend)
{
    _renderBackend = renderBackend;
}

bool flux_render::D3D12SceneBackend::init()
{
    if (_renderBackend == nullptr) return false;
    _initialized = true;
    return true;
}

void flux_render::D3D12SceneBackend::shutdown()
{
    _scenes.clear();
    _currentSceneID.clear();
    _initialized = false;
}

bool flux_render::D3D12SceneBackend::createScene(const std::string& sceneID)
{
    if (!_initialized || sceneID.empty()) return false;
    return _scenes.emplace(sceneID, std::unordered_set<std::string>{}).second;
}

bool flux_render::D3D12SceneBackend::destroyScene(const std::string& sceneID)
{
    if (!_initialized) return false;

    auto it = _scenes.find(sceneID);
    if (it == _scenes.end()) return false;

    _scenes.erase(it);

    if (_currentSceneID == sceneID) {
        _currentSceneID.clear();
    }

    return true;
}

bool flux_render::D3D12SceneBackend::setCurrentScene(const std::string& sceneID)
{
    if (!_initialized) return false;
    if (_scenes.find(sceneID) == _scenes.end()) return false;

    _currentSceneID = sceneID;
    return true;
}

bool flux_render::D3D12SceneBackend::createSceneObject(
    const std::string& sceneID,
    const std::string& entityID)
{
    if (!_initialized || entityID.empty()) return false;

    auto it = _scenes.find(sceneID);
    if (it == _scenes.end()) return false;

    return it->second.insert(entityID).second;
}

bool flux_render::D3D12SceneBackend::destroySceneObject(
    const std::string& sceneID,
    const std::string& entityID)
{
    if (!_initialized) return false;

    auto it = _scenes.find(sceneID);
    if (it == _scenes.end()) return false;

    return it->second.erase(entityID) > 0;
}

bool flux_render::D3D12SceneBackend::setObjectTransform(
    const std::string& sceneID,
    const std::string& entityID,
    const flux_utils::Vector3&,
    const flux_utils::Vector4&,
    const flux_utils::Vector3&)
{
    return hasSceneObject(sceneID, entityID);
}

bool flux_render::D3D12SceneBackend::attachMesh(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string&,
    const std::string&)
{
    return hasSceneObject(sceneID, entityID);
}

bool flux_render::D3D12SceneBackend::createLight(
    const std::string& sceneID,
    const std::string& entityID,
    const flux_utils::Vector3&,
    uint8_t)
{
    return hasSceneObject(sceneID, entityID);
}

bool flux_render::D3D12SceneBackend::createCamera(
    const std::string& sceneID,
    const std::string& entityID,
    int8_t,
    int16_t)
{
    return hasSceneObject(sceneID, entityID);
}

bool flux_render::D3D12SceneBackend::addAnimation(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string&)
{
    return hasSceneObject(sceneID, entityID);
}

bool flux_render::D3D12SceneBackend::removeAnimation(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string&)
{
    return hasSceneObject(sceneID, entityID);
}

bool flux_render::D3D12SceneBackend::setAnimationEnabled(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string&,
    bool)
{
    return hasSceneObject(sceneID, entityID);
}

bool flux_render::D3D12SceneBackend::setAnimationLoop(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string&,
    bool)
{
    return hasSceneObject(sceneID, entityID);
}

bool flux_render::D3D12SceneBackend::updateAnimation(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string& animationName,
    float)
{
    return hasSceneObject(sceneID, entityID);
}

bool flux_render::D3D12SceneBackend::isCurrentScene(const std::string& sceneID) const
{
    return _currentSceneID == sceneID;
}

bool flux_render::D3D12SceneBackend::hasScene(const std::string& sceneID) const
{
    return _scenes.find(sceneID) != _scenes.end();
}

bool flux_render::D3D12SceneBackend::hasSceneObject(
    const std::string& sceneID,
    const std::string& entityID) const
{
    auto it = _scenes.find(sceneID);
    if (it == _scenes.end()) return false;

    return it->second.find(entityID) != it->second.end();
}

const std::string& flux_render::D3D12SceneBackend::getCurrentSceneID() const
{
    return _currentSceneID;
}
