#ifndef I_GRAPHICS_BACKEND_H_
#define I_GRAPHICS_BACKEND_H_

#pragma once
#include "GraphicsConfig.h"

class IGraphicsBackend {
public:
	virtual ~IGraphicsBackend() = default;

	virtual bool init(const GraphicsConfig& cfg) = 0;
	virtual void shutdown() = 0;

	virtual void beginFrame(const float clearColor[4]) = 0;
	virtual void endFrame() = 0;

	// De momento: dibujar escena de test ya preparada
	virtual void drawTestScene() = 0;
};

#endif // I_GRAPHICS_BACKEND_H_