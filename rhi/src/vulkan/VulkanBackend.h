#ifndef VULKAN_BACKEND_H_
#define VULKAN_BACKEND_H_

#pragma once
#include <rhi/IGraphicsBackend.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include <vector>
#include <optional>

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
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    // ==== helpers de creación / destrucción ====
    bool createInstance();
    bool createSurface();
    bool pickPhysicalDevice();
    bool createLogicalDevice();
    bool createSwapChain();
    bool createImageViews();
    bool createRenderPass();
    bool createDescriptorSetLayout();
    bool createGraphicsPipelines();
    bool createFramebuffers();
    bool createCommandPool();
    bool createVertexBuffer();
    bool createUniformBuffers();
    bool createDescriptorPool();
    bool createDescriptorSets();
    bool createCommandBuffers();
    bool createSyncObjects();

    void cleanupSwapChain();
    bool recreateSwapChain();

    // ==== utilidades ====
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    void updateUniformBuffer(uint32_t imageIndex);

private:
    GraphicsConfig m_cfg{};
    HWND           m_hwnd = nullptr;

    VkInstance       m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice         m_device = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;

    VkSurfaceKHR m_surface = VK_NULL_HANDLE;

    VkSwapchainKHR             m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage>       m_swapChainImages;
    std::vector<VkImageView>   m_swapChainImageViews;
    VkFormat                   m_swapChainImageFormat{};
    VkExtent2D                 m_swapChainExtent{};

    VkRenderPass         m_renderPass = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout      m_pipelineLayout = VK_NULL_HANDLE;

    // Dos pipelines: uno para triángulo y otro para ejes
    VkPipeline            m_trianglePipeline = VK_NULL_HANDLE;
    VkPipeline            m_linesPipeline = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    VkCommandPool                m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers; // uno por frame in flight

    VkBuffer       m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;

    std::vector<VkBuffer>       m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;

    VkDescriptorPool              m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet>  m_descriptorSets;

    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence>     m_inFlightFences;

    uint32_t m_currentFrame = 0;
    uint32_t m_currentImageIndex = 0;
    bool     m_framebufferResized = false;
};

#endif // VULKAN_BACKEND_H_