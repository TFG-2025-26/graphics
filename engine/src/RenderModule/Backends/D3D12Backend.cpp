#include "D3D12Backend.h"

#include <SDL_syswm.h>

#include <cstring>

flux_render::D3D12Backend::D3D12Backend(const std::string& appName)
{
    _appName = appName;
}

BackendAPI flux_render::D3D12Backend::getAPI() const
{
    return BackendAPI::D3D12;
}

bool flux_render::D3D12Backend::init(const RenderBackendDesc& desc)
{
    _nativeWindow = desc.nativeWindow;
    _width = desc.width;
    _height = desc.height;
    _vsync = desc.vsync;
    _appName = desc.appName;

    if (_nativeWindow == nullptr || _width == 0 || _height == 0) {
        return false;
    }

    if (!cacheWindowHandle()) return false;
    if (!createFactoryAndDevice()) return false;
    if (!createCommandObjects()) return false;
    if (!createSwapChain()) return false;
    if (!createRTVHeap()) return false;
    if (!createRenderTargets()) return false;
    if (!createFenceObjects()) return false;

    if (!createViewportState()) return false;
    if (!createRootSignature()) return false;
    if (!createPipelineState()) return false;
    if (!createTriangleVertexBuffer()) return false;

    _frameIndex = _swapChain->GetCurrentBackBufferIndex();
    _fenceValues[_frameIndex] = 1;

    return true;
}


void flux_render::D3D12Backend::setSync(bool enabled)
{
    _vsync = enabled;
}

bool flux_render::D3D12Backend::cacheWindowHandle()
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if (!SDL_GetWindowWMInfo(_nativeWindow, &wmInfo)) {
        return false;
    }

    _hwnd = wmInfo.info.win.window;
    return _hwnd != nullptr;
}

bool flux_render::D3D12Backend::createFactoryAndDevice()
{
#if defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }

    UINT factoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
    UINT factoryFlags = 0;
#endif

    HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&_factory));
    if (FAILED(hr)) return false;

    hr = D3D12CreateDevice(
        nullptr,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&_device)
    );
    if (FAILED(hr)) return false;

    return true;
}

bool flux_render::D3D12Backend::createCommandObjects()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    HRESULT hr = _device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&_commandQueue));
    if (FAILED(hr)) return false;

    for (UINT i = 0; i < kFrameCount; ++i) {
        hr = _device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&_commandAllocators[i])
        );
        if (FAILED(hr)) return false;
    }

    hr = _device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        _commandAllocators[0].Get(),
        nullptr,
        IID_PPV_ARGS(&_commandList)
    );
    if (FAILED(hr)) return false;

    hr = _commandList->Close();
    if (FAILED(hr)) return false;

    return true;
}

bool flux_render::D3D12Backend::createSwapChain()
{
    DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
    swapDesc.BufferCount = kFrameCount;
    swapDesc.Width = _width;
    swapDesc.Height = _height;
    swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;

    HRESULT hr = _factory->CreateSwapChainForHwnd(
        _commandQueue.Get(),
        _hwnd,
        &swapDesc,
        nullptr,
        nullptr,
        &swapChain1
    );
    if (FAILED(hr)) return false;
    
    hr = swapChain1.As(&_swapChain);
    if (FAILED(hr)) return false;

    _factory->MakeWindowAssociation(_hwnd, DXGI_MWA_NO_ALT_ENTER);

    _frameIndex = _swapChain->GetCurrentBackBufferIndex();
    return true;
}

bool flux_render::D3D12Backend::createRTVHeap()
{
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = kFrameCount;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = _device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_rtvHeap));
    if (FAILED(hr)) return false;

    _rtvDescriptorSize =
        _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    return true;
}

bool flux_render::D3D12Backend::createRenderTargets()
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        _rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < kFrameCount; ++i) {
        HRESULT hr = _swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i]));
        if (FAILED(hr)) return false;

        _device->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += _rtvDescriptorSize;
    }

    return true;
}

void flux_render::D3D12Backend::releaseRenderTargets()
{
    for (auto& rt : _renderTargets) {
        rt.Reset();
    }
}

bool flux_render::D3D12Backend::createFenceObjects()
{
    HRESULT hr = _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
    if (FAILED(hr)) return false;

    _fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (_fenceEvent == nullptr) return false;

    for (UINT i = 0; i < kFrameCount; ++i) {
        _fenceValues[i] = 0;
    }

    return true;
}

bool flux_render::D3D12Backend::createViewportState()
{
    _viewport.TopLeftX = 0.0f;
    _viewport.TopLeftY = 0.0f;
    _viewport.Width = static_cast<float>(_width);
    _viewport.Height = static_cast<float>(_height);
    _viewport.MinDepth = 0.0f;
    _viewport.MaxDepth = 1.0f;

    _scissorRect.left = 0;
    _scissorRect.top = 0;
    _scissorRect.right = static_cast<LONG>(_width);
    _scissorRect.bottom = static_cast<LONG>(_height);

    return true;
}

bool flux_render::D3D12Backend::createRootSignature()
{
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 0;
    rootSigDesc.pParameters = nullptr;
    rootSigDesc.NumStaticSamplers = 0;
    rootSigDesc.pStaticSamplers = nullptr;
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    Microsoft::WRL::ComPtr<ID3DBlob> signature;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &errorBlob
    );
    if (FAILED(hr)) {
        return false;
    }

    hr = _device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&_rootSignature)
    );
    if (FAILED(hr)) return false;

    return true;
}

bool flux_render::D3D12Backend::createPipelineState()
{
    static const char* vsSource = R"(
        struct VSInput {
            float3 pos : POSITION;
            float4 col : COLOR;
        };

        struct PSInput {
            float4 pos : SV_POSITION;
            float4 col : COLOR;
        };

        PSInput main(VSInput input)
        {
            PSInput output;
            output.pos = float4(input.pos, 1.0f);
            output.col = input.col;
            return output;
        }
    )";

    static const char* psSource = R"(
        struct PSInput {
            float4 pos : SV_POSITION;
            float4 col : COLOR;
        };

        float4 main(PSInput input) : SV_TARGET
        {
            return input.col;
        }
    )";

    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    HRESULT hr = D3DCompile(
        vsSource, strlen(vsSource),
        nullptr, nullptr, nullptr,
        "main", "vs_5_0",
        compileFlags, 0,
        &vsBlob, &errorBlob
    );
    if (FAILED(hr)) return false;

    hr = D3DCompile(
        psSource, strlen(psSource),
        nullptr, nullptr, nullptr,
        "main", "ps_5_0",
        compileFlags, 0,
        &psBlob, &errorBlob
    );
    if (FAILED(hr)) return false;

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {
            "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
            0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        },
        {
            "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,
            0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0
        }
    };

    D3D12_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;

    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {
        FALSE, FALSE,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
        D3D12_LOGIC_OP_NOOP,
        D3D12_COLOR_WRITE_ENABLE_ALL
    };
    blendDesc.RenderTarget[0] = defaultRenderTargetBlendDesc;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = _rootSignature.Get();
    psoDesc.VS = {
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize()
    };
    psoDesc.PS = {
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize()
    };
    psoDesc.RasterizerState = rasterDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    hr = _device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pipelineState));
    if (FAILED(hr)) return false;

    return true;
}

bool flux_render::D3D12Backend::createTriangleVertexBuffer()
{
    const Vertex vertices[] = {
        { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.2f, 0.2f, 1.0f } },
        { {  0.5f, -0.5f, 0.0f }, { 0.2f, 1.0f, 0.2f, 1.0f } },
        { { -0.5f, -0.5f, 0.0f }, { 0.2f, 0.4f, 1.0f, 1.0f } }
    };

    const UINT bufferSize = sizeof(vertices);

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = bufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = _device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_vertexBuffer)
    );
    if (FAILED(hr)) return false;

    void* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };

    hr = _vertexBuffer->Map(0, &readRange, &mappedData);
    if (FAILED(hr)) return false;

    memcpy(mappedData, vertices, sizeof(vertices));
    _vertexBuffer->Unmap(0, nullptr);

    _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
    _vertexBufferView.StrideInBytes = sizeof(Vertex);
    _vertexBufferView.SizeInBytes = bufferSize;

    return true;
}

void flux_render::D3D12Backend::recordTriangleCommands()
{
    _commandList->RSSetViewports(1, &_viewport);
    _commandList->RSSetScissorRects(1, &_scissorRect);

    _commandList->SetGraphicsRootSignature(_rootSignature.Get());
    _commandList->SetPipelineState(_pipelineState.Get());
    _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
    _commandList->DrawInstanced(3, 1, 0, 0);
}

bool flux_render::D3D12Backend::beginFrame()
{
    if (!_device || !_swapChain || !_commandQueue || !_commandList) return false;

    HRESULT hr = _commandAllocators[_frameIndex]->Reset();
    if (FAILED(hr)) return false;

    hr = _commandList->Reset(_commandAllocators[_frameIndex].Get(), nullptr);
    if (FAILED(hr)) return false;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = _renderTargets[_frameIndex].Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    _commandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        _rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += static_cast<SIZE_T>(_frameIndex) * _rtvDescriptorSize;

    _commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    _commandList->ClearRenderTargetView(rtvHandle, _clearColor, 0, nullptr);

    recordTriangleCommands();

    return true;
}

void flux_render::D3D12Backend::endFrame()
{
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = _renderTargets[_frameIndex].Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

    _commandList->ResourceBarrier(1, &barrier);

    HRESULT hr = _commandList->Close();
    if (FAILED(hr)) return;

    ID3D12CommandList* lists[] = { _commandList.Get() };
    _commandQueue->ExecuteCommandLists(1, lists);

    hr = _swapChain->Present(_vsync ? 1 : 0, 0);
    if (FAILED(hr)) return;

    moveToNextFrame();
}

void flux_render::D3D12Backend::moveToNextFrame()
{
    const UINT64 currentFenceValue = _fenceValues[_frameIndex];

    _commandQueue->Signal(_fence.Get(), currentFenceValue);

    _frameIndex = _swapChain->GetCurrentBackBufferIndex();

    if (_fence->GetCompletedValue() < _fenceValues[_frameIndex]) {
        _fence->SetEventOnCompletion(_fenceValues[_frameIndex], _fenceEvent);
        WaitForSingleObject(_fenceEvent, INFINITE);
    }

    _fenceValues[_frameIndex] = currentFenceValue + 1;
}

void flux_render::D3D12Backend::waitIdle()
{
    if (!_commandQueue || !_fence || !_fenceEvent) return;

    const UINT64 value = _fenceValues[_frameIndex];

    if (FAILED(_commandQueue->Signal(_fence.Get(), value))) {
        return;
    }

    if (_fence->GetCompletedValue() < value) {
        if (FAILED(_fence->SetEventOnCompletion(value, _fenceEvent))) {
            return;
        }
        WaitForSingleObject(_fenceEvent, INFINITE);
    }

    _fenceValues[_frameIndex] = value + 1;
}

void flux_render::D3D12Backend::resize(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) return;
    if (!_swapChain || !_device) return;

    waitIdle();

    _width = width;
    _height = height;

    releaseRenderTargets();

    HRESULT hr = _swapChain->ResizeBuffers(
        kFrameCount,
        _width,
        _height,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        0
    );
    if (FAILED(hr)) return;

    _frameIndex = _swapChain->GetCurrentBackBufferIndex();

    for (UINT i = 0; i < kFrameCount; ++i) {
        _fenceValues[i] = 0;
    }
    _fenceValues[_frameIndex] = 1;

    if (!createRenderTargets()) return;
    createViewportState();
}

void flux_render::D3D12Backend::shutdown()
{
    waitIdle();

    releaseRenderTargets();

    _vertexBuffer.Reset();
    _pipelineState.Reset();
    _rootSignature.Reset();

    _commandList.Reset();

    for (auto& allocator : _commandAllocators) {
        allocator.Reset();
    }

    _rtvHeap.Reset();
    _swapChain.Reset();
    _commandQueue.Reset();
    _fence.Reset();
    _device.Reset();
    _factory.Reset();

    if (_fenceEvent != nullptr) {
        CloseHandle(_fenceEvent);
        _fenceEvent = nullptr;
    }

    _hwnd = nullptr;
    _nativeWindow = nullptr;
    _frameIndex = 0;
}