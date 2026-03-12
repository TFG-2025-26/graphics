#pragma once

#ifndef I_RENDER_BACKEND_H_
#define I_RENDER_BACKEND_H_

#include <cstdint>
#include <string>

class SDL_Window;

enum class BackendAPI { Ogre, D3D12, Vulkan };

struct RenderBackendDesc {
	SDL_Window* nativeWindow = nullptr;
	std::string appName;
	uint32_t width = 0;
	uint32_t height = 0;
	bool vsync = true;
};

class IRenderBackend {
protected:
	SDL_Window* _nativeWindow = nullptr;
	uint32_t _width = 0;
	uint32_t _height = 0;
	bool _vsync = true;
public:
	virtual ~IRenderBackend() = default;

	virtual BackendAPI getAPI() const = 0;

	// Ciclo de vida
	virtual bool init(const RenderBackendDesc& desc) = 0;
	virtual void shutdown() = 0;
	virtual void waitIdle() = 0;

	// Swapchain / superficie
	virtual void resize(uint32_t width, uint32_t height) = 0;
	virtual void setSync(bool enabled) = 0;

	// Bucle de renderizado
	virtual bool beginFrame() = 0;
	virtual void endFrame() = 0;
};

#endif // I_RENDER_BACKEND_H_