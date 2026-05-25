#include "D3D12Backend.h"

#include <SDL_syswm.h>

#include <algorithm>
#include <cstring>

#include <DirectXMath.h>

using namespace DirectX;

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
    if (!createDepthStencilBuffer()) return false;
    if (!createRootSignature()) return false;
    if (!createPipelineState()) return false;
    if (!createPlaceholderCubeBuffers()) return false;

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

    _allowTearing = checkTearingSupport();

    hr = D3D12CreateDevice(
        nullptr,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&_device)
    );
    if (FAILED(hr)) return false;

    return true;
}

bool flux_render::D3D12Backend::checkTearingSupport()
{
    Microsoft::WRL::ComPtr<IDXGIFactory5> factory5;

    HRESULT hr = _factory.As(&factory5);
    if (FAILED(hr)) {
        return false;
    }

    BOOL allowTearing = FALSE;

    hr = factory5->CheckFeatureSupport(
        DXGI_FEATURE_PRESENT_ALLOW_TEARING,
        &allowTearing,
        sizeof(allowTearing)
    );

    return SUCCEEDED(hr) && allowTearing == TRUE;
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
    swapDesc.Flags = _allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

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
    D3D12_ROOT_PARAMETER rootParameter = {};
    rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParameter.Constants.ShaderRegister = 0;
    rootParameter.Constants.RegisterSpace = 0;
    rootParameter.Constants.Num32BitValues = 48;
    rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.NumParameters = 1;
    rootSigDesc.pParameters = &rootParameter;
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

    return SUCCEEDED(hr);
}


bool flux_render::D3D12Backend::createPipelineState()
{
    static const char* vsSource = R"(
	cbuffer ObjectConstants : register(b0)
	{
		float4x4 worldViewProj;
		float4x4 world;
		float4 debugColor;
		float4 ambientColor;
		float4 lightDirection;
		float4 lightColor;
	};

	struct VSInput {
		float3 pos : POSITION;
		float3 normal : NORMAL;
		float4 col : COLOR;
	};

	struct PSInput {
		float4 pos : SV_POSITION;
		float4 col : COLOR;
	};

	PSInput main(VSInput input)
	{
		PSInput output;

		output.pos = mul(float4(input.pos, 1.0f), worldViewProj);

		float3 normalWS = normalize(mul(float4(input.normal, 0.0f), world).xyz);

		// lightDirection representa hacia donde apunta la luz.
		// Para iluminar la superficie necesitamos el vector desde la superficie hacia la luz.
		float3 lightDir = normalize(-lightDirection.xyz);

		float ndotl = saturate(dot(normalWS, lightDir));

		// Difuso más amable para preview/debug:
		// evita que las caras contrarias queden prácticamente negras.
		float wrappedDiffuse = saturate(ndotl * 0.75f + 0.12f);

		float3 lighting = ambientColor.rgb + lightColor.rgb * lightColor.a * wrappedDiffuse;

		// Exposicion moderada para que el placeholder no se lave a blanco.
		float3 litColor = debugColor.rgb * lighting * 0.95f;
		output.col = float4(saturate(litColor), debugColor.a);

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
        vsSource,
        strlen(vsSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        compileFlags,
        0,
        &vsBlob,
        &errorBlob
    );
    if (FAILED(hr)) {
        return false;
    }

    errorBlob.Reset();

    hr = D3DCompile(
        psSource,
        strlen(psSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        compileFlags,
        0,
        &psBlob,
        &errorBlob
    );
    if (FAILED(hr)) {
        return false;
    }

    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        {
            "POSITION",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "NORMAL",
            0,
            DXGI_FORMAT_R32G32B32_FLOAT,
            0,
            12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        },
        {
            "COLOR",
            0,
            DXGI_FORMAT_R32G32B32A32_FLOAT,
            0,
            24,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            0
        }
    };

    D3D12_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;
    rasterDesc.ForcedSampleCount = 0;
    rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = FALSE;
    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, static_cast<UINT>(sizeof(inputLayout) / sizeof(inputLayout[0])) };
    psoDesc.pRootSignature = _rootSignature.Get();
    psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
    psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
    psoDesc.RasterizerState = rasterDesc;
    psoDesc.BlendState = blendDesc;
    psoDesc.DepthStencilState = depthStencilDesc;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat = _depthStencilFormat;
    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleDesc.Quality = 0;

    hr = _device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&_pipelineState));
    return SUCCEEDED(hr);
}

bool flux_render::D3D12Backend::createDepthStencilBuffer()
{
    if (!_device || _width == 0 || _height == 0) {
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = _device->CreateDescriptorHeap(
        &dsvHeapDesc,
        IID_PPV_ARGS(&_dsvHeap)
    );
    if (FAILED(hr)) return false;

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Alignment = 0;
    depthDesc.Width = _width;
    depthDesc.Height = _height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.MipLevels = 1;
    depthDesc.Format = _depthStencilFormat;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = _depthStencilFormat;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    hr = _device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(&_depthStencilBuffer)
    );
    if (FAILED(hr)) return false;

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = _depthStencilFormat;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

    _device->CreateDepthStencilView(
        _depthStencilBuffer.Get(),
        &dsvDesc,
        _dsvHeap->GetCPUDescriptorHandleForHeapStart()
    );

    return true;
}

bool flux_render::D3D12Backend::createPlaceholderCubeBuffers()
{
    const Vertex vertices[] = {
        // Front (-Z)
        { { -0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1, 1, 1, 1 } },
        { { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1, 1, 1, 1 } },
        { {  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1, 1, 1, 1 } },
        { {  0.5f, -0.5f, -0.5f }, { 0.0f, 0.0f, -1.0f }, { 1, 1, 1, 1 } },

        // Back (+Z)
        { { -0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1, 1, 1, 1 } },
        { {  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1, 1, 1, 1 } },
        { {  0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1, 1, 1, 1 } },
        { { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1, 1, 1, 1 } },

        // Left (-X)
        { { -0.5f, -0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 1, 1, 1, 1 } },
        { { -0.5f,  0.5f,  0.5f }, { -1.0f, 0.0f, 0.0f }, { 1, 1, 1, 1 } },
        { { -0.5f,  0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1, 1, 1, 1 } },
        { { -0.5f, -0.5f, -0.5f }, { -1.0f, 0.0f, 0.0f }, { 1, 1, 1, 1 } },

        // Right (+X)
        { {  0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1, 1, 1, 1 } },
        { {  0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1, 1, 1, 1 } },
        { {  0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1, 1, 1, 1 } },
        { {  0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f, 0.0f }, { 1, 1, 1, 1 } },

        // Top (+Y)
        { { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1, 1, 1, 1 } },
        { { -0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1, 1, 1, 1 } },
        { {  0.5f,  0.5f,  0.5f }, { 0.0f, 1.0f, 0.0f }, { 1, 1, 1, 1 } },
        { {  0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1, 1, 1, 1 } },

        // Bottom (-Y)
        { { -0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 1, 1, 1, 1 } },
        { { -0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1, 1, 1, 1 } },
        { {  0.5f, -0.5f, -0.5f }, { 0.0f, -1.0f, 0.0f }, { 1, 1, 1, 1 } },
        { {  0.5f, -0.5f,  0.5f }, { 0.0f, -1.0f, 0.0f }, { 1, 1, 1, 1 } },
    };

    const uint16_t indices[] = {
        0, 1, 2,  0, 2, 3,
        4, 5, 6,  4, 6, 7,
        8, 9, 10, 8, 10, 11,
        12, 13, 14, 12, 14, 15,
        16, 17, 18, 16, 18, 19,
        20, 21, 22, 20, 22, 23
    };

    _indexCount = static_cast<UINT>(sizeof(indices) / sizeof(indices[0]));

    const UINT vertexBufferSize = static_cast<UINT>(sizeof(vertices));
    const UINT indexBufferSize = static_cast<UINT>(sizeof(indices));

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps.CreationNodeMask = 1;
    heapProps.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC vertexDesc = {};
    vertexDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    vertexDesc.Alignment = 0;
    vertexDesc.Width = vertexBufferSize;
    vertexDesc.Height = 1;
    vertexDesc.DepthOrArraySize = 1;
    vertexDesc.MipLevels = 1;
    vertexDesc.Format = DXGI_FORMAT_UNKNOWN;
    vertexDesc.SampleDesc.Count = 1;
    vertexDesc.SampleDesc.Quality = 0;
    vertexDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    vertexDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = _device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &vertexDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_vertexBuffer)
    );
    if (FAILED(hr)) return false;

    void* mappedData = nullptr;
    D3D12_RANGE readRange = { 0, 0 };

    hr = _vertexBuffer->Map(0, &readRange, &mappedData);
    if (FAILED(hr)) return false;

    memcpy(mappedData, vertices, vertexBufferSize);
    _vertexBuffer->Unmap(0, nullptr);

    _vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
    _vertexBufferView.StrideInBytes = sizeof(Vertex);
    _vertexBufferView.SizeInBytes = vertexBufferSize;

    D3D12_RESOURCE_DESC indexDesc = vertexDesc;
    indexDesc.Width = indexBufferSize;

    hr = _device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &indexDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&_indexBuffer)
    );
    if (FAILED(hr)) return false;

    mappedData = nullptr;
    hr = _indexBuffer->Map(0, &readRange, &mappedData);
    if (FAILED(hr)) return false;

    memcpy(mappedData, indices, indexBufferSize);
    _indexBuffer->Unmap(0, nullptr);

    _indexBufferView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
    _indexBufferView.Format = DXGI_FORMAT_R16_UINT;
    _indexBufferView.SizeInBytes = indexBufferSize;

    return true;
}

static DirectX::XMFLOAT3 FluxPositionToD3D(
    const flux_utils::Vector3& position,
    float worldScale)
{
    return DirectX::XMFLOAT3(
        position.getX() * worldScale,
        position.getY() * worldScale,
        -position.getZ() * worldScale
    );
}

static DirectX::XMFLOAT3 FluxDirectionToD3D(const DirectX::XMFLOAT3& direction)
{
    return DirectX::XMFLOAT3(
        direction.x,
        direction.y,
        -direction.z
    );
}

static DirectX::XMVECTOR SafeQuaternionFromFlux(const flux_utils::Vector4& rotation)
{
    DirectX::XMVECTOR rotationQuat = DirectX::XMVectorSet(
        rotation.getX(),
        rotation.getY(),
        rotation.getZ(),
        rotation.getW()
    );

    if (DirectX::XMVectorGetX(DirectX::XMVector4LengthSq(rotationQuat)) <= 0.000001f) {
        return DirectX::XMQuaternionIdentity();
    }

    return DirectX::XMQuaternionNormalize(rotationQuat);
}

static float NormalizeColorChannel(float value)
{
    return value > 1.0f ? value / 255.0f : value;
}

static DirectX::XMFLOAT4 BuildAmbientColor()
{
    return DirectX::XMFLOAT4(0.34f, 0.34f, 0.38f, 1.0f);
}

static DirectX::XMFLOAT4 BuildDefaultLightDirection()
{
    // Dirección hacia la que apunta la luz fallback.
    // En el shader se invierte para obtener el vector superficie -> luz.
    return DirectX::XMFLOAT4(-0.35f, -0.85f, -0.40f, 0.0f);
}

static void BuildDominantLightConstants(
    const std::vector<flux_render::D3D12LightData>& lights,
    DirectX::XMFLOAT4& lightDirection,
    DirectX::XMFLOAT4& lightColor)
{
    lightDirection = BuildDefaultLightDirection();
    lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 0.95f);

    if (lights.empty()) {
        return;
    }

    const flux_render::D3D12LightData* selectedLight = nullptr;

    for (const auto& light : lights) {
        if (light.valid) {
            selectedLight = &light;
            break;
        }
    }

    if (selectedLight == nullptr) {
        return;
    }

    lightColor.x = NormalizeColorChannel(selectedLight->diffuseColor.getX());
    lightColor.y = NormalizeColorChannel(selectedLight->diffuseColor.getY());
    lightColor.z = NormalizeColorChannel(selectedLight->diffuseColor.getZ());

    const float colorSum = lightColor.x + lightColor.y + lightColor.z;
    if (colorSum <= 0.01f) {
        lightColor.x = 1.0f;
        lightColor.y = 1.0f;
        lightColor.z = 1.0f;
    }

    lightColor.w = std::clamp<float>(selectedLight->intensity, 0.75f, 1.15f);

    DirectX::XMVECTOR rotationQuat = SafeQuaternionFromFlux(selectedLight->rotation);

    DirectX::XMVECTOR fluxForward = DirectX::XMVector3Rotate(
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
        rotationQuat
    );

    DirectX::XMFLOAT3 forwardFlux;
    DirectX::XMStoreFloat3(&forwardFlux, fluxForward);
    DirectX::XMFLOAT3 forwardD3D = FluxDirectionToD3D(forwardFlux);

    DirectX::XMVECTOR dir = DirectX::XMVectorSet(
        forwardD3D.x,
        forwardD3D.y,
        forwardD3D.z,
        0.0f
    );

    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(dir)) <= 0.000001f) {
        return;
    }

    dir = DirectX::XMVector3Normalize(dir);
    DirectX::XMStoreFloat4(&lightDirection, dir);
    lightDirection.w = 0.0f;
}

static DirectX::XMMATRIX BuildCameraViewMatrix(
    const flux_render::D3D12CameraData& camera,
    float worldScale)
{
    if (!camera.valid) {
        return DirectX::XMMatrixLookAtLH(
            DirectX::XMVectorSet(0.0f, 5.0f, -15.0f, 1.0f),
            DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f),
            DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
        );
    }

    DirectX::XMFLOAT3 eyePos = FluxPositionToD3D(camera.position, worldScale);
    DirectX::XMVECTOR eye = DirectX::XMVectorSet(eyePos.x, eyePos.y, eyePos.z, 1.0f);

    DirectX::XMVECTOR rotationQuat = SafeQuaternionFromFlux(camera.rotation);

    // Convención Ogre/Flux habitual: una cámara sin rotación mira hacia -Z.
    DirectX::XMVECTOR fluxForward = DirectX::XMVector3Rotate(
        DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f),
        rotationQuat
    );

    DirectX::XMVECTOR fluxUp = DirectX::XMVector3Rotate(
        DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f),
        rotationQuat
    );

    DirectX::XMFLOAT3 forwardFlux;
    DirectX::XMFLOAT3 upFlux;
    DirectX::XMStoreFloat3(&forwardFlux, fluxForward);
    DirectX::XMStoreFloat3(&upFlux, fluxUp);

    DirectX::XMFLOAT3 forwardD3D = FluxDirectionToD3D(forwardFlux);
    DirectX::XMFLOAT3 upD3D = FluxDirectionToD3D(upFlux);

    DirectX::XMVECTOR forward = DirectX::XMVector3Normalize(
        DirectX::XMVectorSet(forwardD3D.x, forwardD3D.y, forwardD3D.z, 0.0f)
    );

    DirectX::XMVECTOR up = DirectX::XMVector3Normalize(
        DirectX::XMVectorSet(upD3D.x, upD3D.y, upD3D.z, 0.0f)
    );

    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(forward)) <= 0.000001f) {
        forward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    }

    if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(up)) <= 0.000001f) {
        up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    }

    return DirectX::XMMatrixLookToLH(eye, forward, up);
}

void flux_render::D3D12Backend::recordRenderableCommands()
{
    _commandList->RSSetViewports(1, &_viewport);
    _commandList->RSSetScissorRects(1, &_scissorRect);

    _commandList->SetGraphicsRootSignature(_rootSignature.Get());
    _commandList->SetPipelineState(_pipelineState.Get());
    _commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _commandList->IASetVertexBuffers(0, 1, &_vertexBufferView);
    _commandList->IASetIndexBuffer(&_indexBufferView);

    const float aspectRatio =
        _height != 0 ? static_cast<float>(_width) / static_cast<float>(_height) : 1.0f;

    XMMATRIX view = BuildCameraViewMatrix(_camera, worldScale);

    const float nearPlane = _camera.valid && _camera.nearPlane > 0.0f
        ? _camera.nearPlane
        : 0.1f;

    const float farPlane = _camera.valid && _camera.farPlane > nearPlane
        ? _camera.farPlane
        : 1000.0f;

    const float fovY = _camera.valid && _camera.fovYDegrees > 0.0f
        ? _camera.fovYDegrees
        : 60.0f;

    XMMATRIX projection = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(fovY),
        aspectRatio,
        nearPlane,
        farPlane
    );

    XMFLOAT4 ambientColor = BuildAmbientColor();
    XMFLOAT4 lightDirection = BuildDefaultLightDirection();
    XMFLOAT4 lightColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.95f);
    BuildDominantLightConstants(_lights, lightDirection, lightColor);

    for (const auto& renderable : _renderables) {
        if (!renderable.visible || !renderable.hasMesh) {
            continue;
        }

        float sx = renderable.scale.getX();
        float sy = renderable.scale.getY();
        float sz = renderable.scale.getZ();

        if (sx == 0.0f) sx = 1.0f;
        if (sy == 0.0f) sy = 1.0f;
        if (sz == 0.0f) sz = 1.0f;

        XMMATRIX scale = XMMatrixScaling(sx, sy, sz);

        XMVECTOR rotationQuat = XMVectorSet(
            renderable.rotation.getX(),
            renderable.rotation.getY(),
            renderable.rotation.getZ(),
            renderable.rotation.getW()
        );

        if (XMVectorGetX(XMVector4LengthSq(rotationQuat)) <= 0.000001f) {
            rotationQuat = XMQuaternionIdentity();
        }
        else {
            rotationQuat = XMQuaternionNormalize(rotationQuat);
        }

        XMMATRIX rotation = XMMatrixRotationQuaternion(rotationQuat);

        DirectX::XMFLOAT3 d3dPosition =
            FluxPositionToD3D(renderable.position, worldScale);

        XMMATRIX translation = XMMatrixTranslation(
            d3dPosition.x,
            d3dPosition.y,
            d3dPosition.z
        );

        XMMATRIX world = scale * rotation * translation;
        XMMATRIX worldViewProj = world * view * projection;

        XMMATRIX wvpTransposed = XMMatrixTranspose(worldViewProj);
        XMMATRIX worldTransposed = XMMatrixTranspose(world);

        float constants[48];

        XMStoreFloat4x4(
            reinterpret_cast<XMFLOAT4X4*>(&constants[0]),
            wvpTransposed
        );

        XMStoreFloat4x4(
            reinterpret_cast<XMFLOAT4X4*>(&constants[16]),
            worldTransposed
        );

        constants[32] = renderable.debugColor[0];
        constants[33] = renderable.debugColor[1];
        constants[34] = renderable.debugColor[2];
        constants[35] = renderable.debugColor[3];

        constants[36] = ambientColor.x;
        constants[37] = ambientColor.y;
        constants[38] = ambientColor.z;
        constants[39] = ambientColor.w;

        constants[40] = lightDirection.x;
        constants[41] = lightDirection.y;
        constants[42] = lightDirection.z;
        constants[43] = lightDirection.w;

        constants[44] = lightColor.x;
        constants[45] = lightColor.y;
        constants[46] = lightColor.z;
        constants[47] = lightColor.w;

        _commandList->SetGraphicsRoot32BitConstants(
            0,
            48,
            constants,
            0
        );

        _commandList->DrawIndexedInstanced(_indexCount, 1, 0, 0, 0);
    }
}

void flux_render::D3D12Backend::setRenderables(const std::vector<D3D12Renderable>& renderables)
{
    _renderables = renderables;
}

void flux_render::D3D12Backend::setLights(const std::vector<D3D12LightData>& lights)
{
    _lights = lights;
}

void flux_render::D3D12Backend::setCamera(const D3D12CameraData& camera)
{
    _camera = camera;
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

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
    if (_dsvHeap != nullptr) {
        dsvHandle = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
        _commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
    }
    else {
        _commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    }

    _commandList->ClearRenderTargetView(rtvHandle, _clearColor, 0, nullptr);

    if (_dsvHeap != nullptr) {
        _commandList->ClearDepthStencilView(
            dsvHandle,
            D3D12_CLEAR_FLAG_DEPTH,
            1.0f,
            0,
            0,
            nullptr
        );
    }

    recordRenderableCommands();

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

    const UINT syncInterval = _vsync ? 1 : 0;
    const UINT presentFlags = (!_vsync && _allowTearing)
        ? DXGI_PRESENT_ALLOW_TEARING
        : 0;

    hr = _swapChain->Present(syncInterval, presentFlags);
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
        _allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0
    );
    if (FAILED(hr)) return;

    _frameIndex = _swapChain->GetCurrentBackBufferIndex();

    for (UINT i = 0; i < kFrameCount; ++i) {
        _fenceValues[i] = 0;
    }
    _fenceValues[_frameIndex] = 1;

    if (!createRenderTargets()) return;

    _depthStencilBuffer.Reset();
    _dsvHeap.Reset();

    if (!createDepthStencilBuffer()) return;
    createViewportState();
}

void flux_render::D3D12Backend::shutdown()
{
    waitIdle();

    releaseRenderTargets();

    _indexBuffer.Reset();
    _vertexBuffer.Reset();
    _depthStencilBuffer.Reset();
    _dsvHeap.Reset();

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