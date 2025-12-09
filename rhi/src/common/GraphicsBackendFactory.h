#ifndef GRAPHICS_BACKEND_FACTORY_H_
#define GRAPHICS_BACKEND_FACTORY_H_

#pragma once

#include <memory>
#include <rhi/GraphicsConfig.h>
#include <rhi/IGraphicsBackend.h>

std::unique_ptr<IGraphicsBackend> CreateGraphicsBackend(const GraphicsConfig& cfg);

#endif // GRAPHICS_BACKEND_FACTORY_H_