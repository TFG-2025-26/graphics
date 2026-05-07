#pragma once

#ifndef D3D12_BACKEND_H_
#define D3D12_BACKEND_H_

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include <Windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>

#include "IRenderBackend.h"
#include "D3D12Types.h"

#if defined(_MSC_VER)
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")
#endif

namespace flux_render {

    constexpr float worldScale = 1.0f;

    class D3D12Backend : public IRenderBackend {
    public:
        explicit D3D12Backend(const std::string& appName);
        ~D3D12Backend() override = default;

        BackendAPI getAPI() const override;

        bool init(const RenderBackendDesc& desc) override;
        void shutdown() override;
        void waitIdle() override;

        void resize(uint32_t width, uint32_t height) override;
        void setSync(bool enabled) override;

        bool beginFrame() override;
        void endFrame() override;

        ID3D12Device* getDevice() const { return _device.Get(); }
        ID3D12GraphicsCommandList* getCommandList() const { return _commandList.Get(); }
        ID3D12CommandQueue* getCommandQueue() const { return _commandQueue.Get(); }
        UINT getCurrentFrameIndex() const { return _frameIndex; }
        DXGI_FORMAT getBackBufferFormat() const { return DXGI_FORMAT_R8G8B8A8_UNORM; }

        void setRenderables(const std::vector<D3D12Renderable>& renderables);
        void setLights(const std::vector<D3D12LightData>& lights);
        void setCamera(const D3D12CameraData& camera);

    private:
        struct Vertex {
            float position[3];
            float normal[3];
            float color[4];
        };

        bool cacheWindowHandle();
        bool createFactoryAndDevice();
        bool createCommandObjects();
        bool createSwapChain();
        bool createRTVHeap();
        bool createRenderTargets();
        bool createFenceObjects();

        bool createViewportState();
        bool createRootSignature();
        bool createPipelineState();
        bool createDepthStencilBuffer();
        bool createPlaceholderCubeBuffers();

        void releaseRenderTargets();
        void moveToNextFrame();

        void recordRenderableCommands();

    private:
        static constexpr UINT kFrameCount = 2;

        std::string _appName;
        HWND _hwnd = nullptr;

        Microsoft::WRL::ComPtr<IDXGIFactory4> _factory;
        Microsoft::WRL::ComPtr<ID3D12Device> _device;
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> _commandQueue;
        Microsoft::WRL::ComPtr<IDXGISwapChain3> _swapChain;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _rtvHeap;
        UINT _rtvDescriptorSize = 0;

        std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kFrameCount> _renderTargets;
        std::array<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>, kFrameCount> _commandAllocators;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _commandList;

        Microsoft::WRL::ComPtr<ID3D12Fence> _fence;
        std::array<UINT64, kFrameCount> _fenceValues = { 0, 0 };
        HANDLE _fenceEvent = nullptr;

        Microsoft::WRL::ComPtr<ID3D12RootSignature> _rootSignature;
        Microsoft::WRL::ComPtr<ID3D12PipelineState> _pipelineState;

        Microsoft::WRL::ComPtr<ID3D12Resource> _vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW _vertexBufferView = {};

        Microsoft::WRL::ComPtr<ID3D12Resource> _indexBuffer;
        D3D12_INDEX_BUFFER_VIEW _indexBufferView = {};
        UINT _indexCount = 0;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _dsvHeap;
        Microsoft::WRL::ComPtr<ID3D12Resource> _depthStencilBuffer;
        DXGI_FORMAT _depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

        D3D12_VIEWPORT _viewport = {};
        D3D12_RECT _scissorRect = {};

        UINT _frameIndex = 0;

        float _clearColor[4] = { 0.08f, 0.10f, 0.16f, 1.0f };

        std::vector<D3D12Renderable> _renderables;
        std::vector<D3D12LightData> _lights;
        D3D12CameraData _camera;
    };
}

#endif // D3D12_BACKEND_H_
