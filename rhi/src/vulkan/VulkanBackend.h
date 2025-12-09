#ifndef VULKAN_BACKEND_H_
#define VULKAN_BACKEND_H_

#include <rhi/IGraphicsBackend.h>

#include <vulkan/vulkan.h>
#include <vector>

#include <wrl.h>

class VulkanBackend : public IGraphicsBackend {
public:
	VulkanBackend() = default;
	~VulkanBackend() override;

	bool init(const GraphicsConfig& cfg) override;
	void shutdown() override;

	void beginFrame(const float clearColor[4]) override;
	void endFrame() override;

	void drawTestScene() override;

private:
	// =============== Métodos internos ===============
	bool createInstanceAndDevice();
	bool createSwapchainAndRTVs();
	bool createCommandObjects();
	bool createSyncObjects();
	bool createTestSceneResources();

	void cleanupSwapchain();
	void recreateSwapchain(uint32_t width, uint32_t height);

	GraphicsConfig m_cfg;
	HWND m_hwnd = nullptr;

	VkInstance				m_instance = VK_NULL_HANDLE;
	VkPhysicalDevice		m_physicalDevice = VK_NULL_HANDLE;
	VkDevice				m_device = VK_NULL_HANDLE;
	VkQueue					m_graphicsQueue = VK_NULL_HANDLE;
	uint32_t				m_graphicsQueueFamily = 0;

	VkSurfaceKHR			m_surface = VK_NULL_HANDLE;
	VkSwapchainKHR			m_swapchain = VK_NULL_HANDLE;

	std::vector<VkImage>		m_swapchainImages;
	std::vector<VkImageView>	m_swapchainImageViews;
	std::vector<VkFramebuffer>	m_frameBuffers;

	VkRenderPass					m_renderPass = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer>	m_commandBuffers;
	VkCommandPool					m_cmdPool = VK_NULL_HANDLE;

	std::vector<VkSemaphore>		m_imageAvailableSemaphores;
	std::vector<VkSemaphore>		m_renderFinishedSemaphores;
	std::vector<VkFence>			m_inFlightFences;
	size_t							m_currentFrame = 0;

	// Recursos de la escena de prueba (triángulo + ejes)
	VkPipelineLayout			m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline					m_pipeline = VK_NULL_HANDLE;

	VkBuffer					m_vertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory				m_vertexBufferMemory = VK_NULL_HANDLE;
};

#endif // VULKAN_BACKEND_H_