#ifndef GRAPHICS_CONFIG_H_
#define GRAPHICS_CONFIG_H_

#pragma once
#include <cstdint>
#include <string>

enum class GraphicsAPI {
	DX12,
	Vulkan
};

struct GraphicsConfig {
	GraphicsAPI api = GraphicsAPI::DX12;

	void* windowHandle = nullptr; // HWND en Win32
	uint32_t width = 1280;
	uint32_t height = 720;
	bool vsync = true;

	// Opcional: ruta base para shaders, etc.
	std::string shaderBasePath = "assets/shaders/";
};

#endif // GRAPHICS_CONFIG_H_