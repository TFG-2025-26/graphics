#ifndef DX12_BACKEND_H_
#define DX12_BACKEND_H_

#include <rhi/IGraphicsBackend.h>

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>

class DX12Backend : public IGraphicsBackend {
public:
	DX12Backend() = default;
	~DX12Backend() override;

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

	void waitForGpu();
	void moveToNextFrame();

	// === Datos comunes ===
    static const UINT FrameCount = 2;

    GraphicsConfig m_cfg{};
    HWND m_hwnd = nullptr;

    // DXGI/D3D12 core
    Microsoft::WRL::ComPtr<IDXGIFactory6>           m_factory;
    Microsoft::WRL::ComPtr<ID3D12Device>            m_device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>      m_cmdQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain4>         m_swapchain;

    // RTVs
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource>          m_renderTargets[FrameCount];
    UINT                                            m_rtvDescriptorSize = 0;
    UINT                                            m_frameIndex = 0;

    // Commands
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>  m_cmdAllocator[FrameCount];
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_cmdList;

    // Sync
    Microsoft::WRL::ComPtr<ID3D12Fence>             m_fence;
    UINT64                                          m_fenceValues[FrameCount] = {};
    HANDLE                                          m_fenceEvent = nullptr;

    // Estado de render
    D3D12_VIEWPORT                                  m_viewport{};
    D3D12_RECT                                      m_scissorRect{};

    // Recursos de la escena de test (triángulo + ejes)
    Microsoft::WRL::ComPtr<ID3D12RootSignature>     m_rootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState>     m_pipelineState;

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW                        m_vbView{};

    Microsoft::WRL::ComPtr<ID3D12Resource>          m_constantBuffer;
    std::uint8_t*                                   m_cbvMappedData = nullptr;
};

#endif // DX12_BACKEND_H_