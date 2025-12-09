#include "VulkanBackend.h"

VulkanBackend::~VulkanBackend()
{
    shutdown();
}

bool VulkanBackend::init(const GraphicsConfig& cfg)
{
    m_cfg = cfg;
    m_hwnd = static_cast<HWND>(cfg.windowHandle);

    if (!createInstanceAndDevice())     return false;
    if (!createSwapchainAndRTVs())      return false;
    // if (!createRenderPass())            return false; // si se separa
    if (!createCommandObjects())        return false;
    if (!createSyncObjects())           return false;
    if (!createTestSceneResources())    return false;

    return true;
}

void VulkanBackend::shutdown()
{
}

void VulkanBackend::beginFrame(const float clearColor[4])
{
}

void VulkanBackend::endFrame()
{
}

void VulkanBackend::drawTestScene()
{
}

bool VulkanBackend::createInstanceAndDevice()
{
    return false;
}

bool VulkanBackend::createSwapchainAndRTVs() 
{
    return false;
}

bool VulkanBackend::createCommandObjects()
{
    return false;
}

bool VulkanBackend::createSyncObjects()
{
    return false;
}

bool VulkanBackend::createTestSceneResources()
{
    return false;
}
