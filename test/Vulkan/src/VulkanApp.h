#pragma once

#ifndef VULKAN_APP_H_
#define VULKAN_APP_H_

#include <Windows.h>
#include <vector>

#include <vulkan/vulkan.h>

class VulkanApp
{
public:
	void init(HWND hwnd, int width, int height);
	void render();
	void waitForPreviousFrame();
	void destroy();

private:
	// General
	UINT m_width = 800;
	UINT m_height = 600;

	// Vulkan core
	VkInstance m_instance = VK_NULL_HANDLE;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;

	// Familias de colas
	uint32_t m_graphicsQueueFamilyIndex = UINT32_MAX;
	uint32_t m_presentQueueFamilyIndex = UINT32_MAX;
	VkQueue m_graphicsQueue = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;

	// Swapchain
	VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	VkFormat m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D m_swapChainExtent = {};

	// Render pass / pipeline
	VkRenderPass m_renderPass = VK_NULL_HANDLE;
	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_trianglePipeline = VK_NULL_HANDLE;
	VkPipeline m_axesPipeline = VK_NULL_HANDLE;

	// Framebuffers
	std::vector<VkFramebuffer> m_swapChainFramebuffers;

	// Sistema de comandos
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> m_commandBuffers;

	// Sincronización
	VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
	VkFence m_inFlightFence = VK_NULL_HANDLE;

	// Buffers
	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

	VkBuffer m_uniformBuffer = VK_NULL_HANDLE;
	VkDeviceMemory m_uniformBufferMemory = VK_NULL_HANDLE;

	// Descriptores
	VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
	VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

	// Shaders
	VkShaderModule m_vertShaderModule = VK_NULL_HANDLE;
	VkShaderModule m_fragShaderModule = VK_NULL_HANDLE;

	void createVertexBuffer();
	void createUniformBuffer();
	void createDescriptorSetLayoutAndPool();
	void createGraphicsPipelines();
	void createFramebuffers();
	void createCommandPoolAndBuffers();
	void createSyncObjects();

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

#endif // VULKAN_APP_H_