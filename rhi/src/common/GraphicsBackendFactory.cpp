#include "GraphicsBackendFactory.h"

#include "../dx12/DX12Backend.h"
#include "../vulkan/VulkanBackend.h"

std::unique_ptr<IGraphicsBackend> CreateGraphicsBackend(const GraphicsConfig& cfg)
{
	switch (cfg.api) {
	case GraphicsAPI::DX12:
		return std::make_unique<DX12Backend>();
	case GraphicsAPI::Vulkan:
		return std::make_unique<VulkanBackend>();
	}


	return nullptr;
}
