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
    _camera = D3D12CameraData{};

    if (_renderBackend != nullptr) {
        _renderBackend->setRenderables({});
        _renderBackend->setLights({});
        _renderBackend->setCamera(D3D12CameraData{});
    }

    _initialized = false;
}

bool flux_render::D3D12SceneBackend::createScene(const std::string& sceneID)
{
    if (!_initialized || sceneID.empty()) {
        return false;
    }

    if (_scenes.find(sceneID) != _scenes.end()) {
        return true;
    }

    _scenes.emplace(sceneID, D3D12SceneData{});

    return true;
}

bool flux_render::D3D12SceneBackend::destroyScene(const std::string& sceneID)
{
    if (!_initialized) return false;

    auto it = _scenes.find(sceneID);
    if (it == _scenes.end()) return false;

    _scenes.erase(it);

    if (_currentSceneID == sceneID) {
        _currentSceneID.clear();
        syncCurrentSceneToBackend();
        syncLightsToBackend();
    }

    if (_camera.sceneID == sceneID) {
        _camera = D3D12CameraData{};
        syncCameraToBackend();
    }

    return true;
}

bool flux_render::D3D12SceneBackend::setCurrentScene(const std::string& sceneID)
{
    if (!_initialized) {
        return false;
    }

    if (_scenes.find(sceneID) == _scenes.end()) {
        return false;
    }

    _currentSceneID = sceneID;
    syncCurrentSceneToBackend();
    syncLightsToBackend();

    // Si la cámara activa pertenece a otra escena, desactivamos la cámara de D3D12
    // hasta que CCamera::createCamera() registre la cámara de la nueva escena.
    if (!_camera.valid || _camera.sceneID != _currentSceneID) {
        D3D12CameraData invalidCamera;
        _renderBackend->setCamera(invalidCamera);
    }
    else {
        syncCameraToBackend();
    }

    return true;
}

bool flux_render::D3D12SceneBackend::createSceneObject(
    const std::string& sceneID,
    const std::string& entityID)
{
    if (!_initialized || sceneID.empty() || entityID.empty()) {
        return false;
    }

    auto sceneIt = _scenes.find(sceneID);

    if (sceneIt == _scenes.end()) {
        return false;
    }

    auto& renderables = sceneIt->second.renderables;

    if (renderables.find(entityID) == renderables.end()) {
        D3D12Renderable renderable;
        renderable.sceneID = sceneID;
        renderable.entityID = entityID;
        renderable.position = flux_utils::Vector3(0, 0, 0);
        renderable.rotation = flux_utils::Vector4(0, 0, 0, 1);
        renderable.scale = flux_utils::Vector3(1, 1, 1);
        renderable.hasMesh = false;
        renderable.visible = true;

        renderables.emplace(entityID, renderable);
    }

    if (sceneID == _currentSceneID) {
        syncCurrentSceneToBackend();
        syncLightsToBackend();
    }

    return true;
}

bool flux_render::D3D12SceneBackend::destroySceneObject(
    const std::string& sceneID,
    const std::string& entityID)
{
    if (!_initialized) {
        return false;
    }

    auto sceneIt = _scenes.find(sceneID);

    if (sceneIt == _scenes.end()) {
        return false;
    }

    sceneIt->second.renderables.erase(entityID);
    sceneIt->second.lights.erase(entityID);

    if (_camera.valid && _camera.sceneID == sceneID && _camera.entityID == entityID) {
        _camera = D3D12CameraData{};
        syncCameraToBackend();
    }

    if (sceneID == _currentSceneID) {
        syncCurrentSceneToBackend();
        syncLightsToBackend();
    }

    return true;
}

bool flux_render::D3D12SceneBackend::setObjectTransform(
    const std::string& sceneID,
    const std::string& entityID,
    const flux_utils::Vector3& position,
    const flux_utils::Vector4& rotation,
    const flux_utils::Vector3& scale)
{
    if (!_initialized) {
        return false;
    }

    auto sceneIt = _scenes.find(sceneID);

    if (sceneIt == _scenes.end()) {
        return false;
    }

    auto& renderables = sceneIt->second.renderables;

    auto renderableIt = renderables.find(entityID);

    if (renderableIt == renderables.end()) {
        createSceneObject(sceneID, entityID);
        renderableIt = renderables.find(entityID);
    }

    if (renderableIt == renderables.end()) {
        return false;
    }

    renderableIt->second.position = position;
    renderableIt->second.rotation = rotation;
    renderableIt->second.scale = scale;

    auto lightIt = sceneIt->second.lights.find(entityID);
    if (lightIt != sceneIt->second.lights.end()) {
        lightIt->second.position = position;
        lightIt->second.rotation = rotation;

        if (sceneID == _currentSceneID) {
            syncLightsToBackend();
        }
    }

    if (_camera.valid && _camera.sceneID == sceneID && _camera.entityID == entityID) {
        _camera.position = position;
        _camera.rotation = rotation;
        syncCameraToBackend();
    }

    if (sceneID == _currentSceneID) {
        syncCurrentSceneToBackend();
    }

    return true;
}

bool flux_render::D3D12SceneBackend::attachMesh(
    const std::string& sceneID,
    const std::string& entityID,
    const std::string& meshName,
    const std::string& materialName)
{
    if (!_initialized) {
        return false;
    }

    auto sceneIt = _scenes.find(sceneID);

    if (sceneIt == _scenes.end()) {
        return false;
    }

    auto& renderables = sceneIt->second.renderables;

    auto renderableIt = renderables.find(entityID);

    if (renderableIt == renderables.end()) {
        createSceneObject(sceneID, entityID);
        renderableIt = renderables.find(entityID);
    }

    if (renderableIt == renderables.end()) {
        return false;
    }

    renderableIt->second.meshName = meshName;
    renderableIt->second.hasMesh = true;

    size_t hash = std::hash<std::string>{}(entityID);

    float r = 0.35f + static_cast<float>((hash & 0xFF0000) >> 16) / 255.0f * 0.65f;
    float g = 0.35f + static_cast<float>((hash & 0x00FF00) >> 8) / 255.0f * 0.65f;
    float b = 0.35f + static_cast<float>((hash & 0x0000FF)) / 255.0f * 0.65f;

    renderableIt->second.debugColor[0] = r;
    renderableIt->second.debugColor[1] = g;
    renderableIt->second.debugColor[2] = b;
    renderableIt->second.debugColor[3] = 1.0f;

    if (sceneID == _currentSceneID) {
        syncCurrentSceneToBackend();
    }

    return true;
}

bool flux_render::D3D12SceneBackend::createLight(
    const std::string& sceneID,
    const std::string& entityID,
    const flux_utils::Vector3& diffuseColor,
    uint8_t lightType)
{
    if (!_initialized || sceneID.empty() || entityID.empty()) {
        return false;
    }

    auto sceneIt = _scenes.find(sceneID);

    if (sceneIt == _scenes.end()) {
        return false;
    }

    if (!hasSceneObject(sceneID, entityID)) {
        createSceneObject(sceneID, entityID);
    }

    auto& renderables = sceneIt->second.renderables;
    auto renderableIt = renderables.find(entityID);

    if (renderableIt == renderables.end()) {
        return false;
    }

    D3D12LightData light;
    light.sceneID = sceneID;
    light.entityID = entityID;
    light.position = renderableIt->second.position;
    light.rotation = renderableIt->second.rotation;
    light.diffuseColor = diffuseColor;
    light.lightType = lightType;
    light.intensity = 1.0f;
    light.valid = true;

    sceneIt->second.lights[entityID] = light;

    if (sceneID == _currentSceneID) {
        syncLightsToBackend();
    }

    return true;
}

bool flux_render::D3D12SceneBackend::createCamera(
    const std::string& sceneID,
    const std::string& entityID,
    int8_t nearDist,
    int16_t farDist)
{
    if (!_initialized || sceneID.empty() || entityID.empty()) {
        return false;
    }

    auto sceneIt = _scenes.find(sceneID);

    if (sceneIt == _scenes.end()) {
        return false;
    }

    auto& renderables = sceneIt->second.renderables;
    auto renderableIt = renderables.find(entityID);

    if (renderableIt == renderables.end()) {
        createSceneObject(sceneID, entityID);
        renderableIt = renderables.find(entityID);
    }

    if (renderableIt == renderables.end()) {
        return false;
    }

    _camera.sceneID = sceneID;
    _camera.entityID = entityID;
    _camera.position = renderableIt->second.position;
    _camera.rotation = renderableIt->second.rotation;
    _camera.nearPlane = nearDist > 0 ? static_cast<float>(nearDist) : 0.1f;
    _camera.farPlane = farDist > _camera.nearPlane ? static_cast<float>(farDist) : 1000.0f;
    _camera.fovYDegrees = 60.0f;
    _camera.valid = true;

    syncCameraToBackend();

    return true;
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
    auto sceneIt = _scenes.find(sceneID);

    if (sceneIt == _scenes.end()) {
        return false;
    }

    const auto& renderables = sceneIt->second.renderables;

    return renderables.find(entityID) != renderables.end();
}

const std::string& flux_render::D3D12SceneBackend::getCurrentSceneID() const
{
    return _currentSceneID;
}

void flux_render::D3D12SceneBackend::syncCurrentSceneToBackend()
{
    if (_renderBackend == nullptr) {
        return;
    }

    std::vector<D3D12Renderable> renderables;

    auto sceneIt = _scenes.find(_currentSceneID);

    if (sceneIt != _scenes.end()) {
        for (auto& pair : sceneIt->second.renderables) {
            renderables.push_back(pair.second);
        }
    }

    _renderBackend->setRenderables(renderables);
}

void flux_render::D3D12SceneBackend::syncLightsToBackend()
{
    if (_renderBackend == nullptr) {
        return;
    }

    std::vector<D3D12LightData> lights;

    auto sceneIt = _scenes.find(_currentSceneID);

    if (sceneIt != _scenes.end()) {
        for (auto& pair : sceneIt->second.lights) {
            if (pair.second.valid) {
                lights.push_back(pair.second);
            }
        }
    }

    _renderBackend->setLights(lights);
}

void flux_render::D3D12SceneBackend::syncCameraToBackend()
{
    if (_renderBackend == nullptr) {
        return;
    }

    _renderBackend->setCamera(_camera);
}
