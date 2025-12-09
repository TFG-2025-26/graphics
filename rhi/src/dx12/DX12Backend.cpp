#include "DX12Backend.h"

#include "../common/TestSceneGeometry.h"

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <directx/d3dx12.h>

#include <cassert>

DX12Backend::~DX12Backend()
{
    shutdown();
}

bool DX12Backend::init(const GraphicsConfig& cfg)
{
    m_cfg = cfg;
    m_hwnd = static_cast<HWND>(cfg.windowHandle);

#if defined(_DEBUG)
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
    }
#endif

    if (!createInstanceAndDevice())     return false;
    if (!createSwapchainAndRTVs())      return false;
    if (!createCommandObjects())        return false;
    if (!createSyncObjects())           return false;
    if (!createTestSceneResources())    return false;

    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width = static_cast<float>(m_cfg.width);
    m_viewport.Height = static_cast<float>(m_cfg.height);
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    m_scissorRect.left = 0;
    m_scissorRect.top = 0;
    m_scissorRect.right = static_cast<LONG>(m_cfg.width);
    m_scissorRect.bottom = static_cast<LONG>(m_cfg.height);

    return true;
}

void DX12Backend::shutdown()
{
    if (!m_device) return; // ya está apagado

    // Asegurar que la GPU termina antes de liberar cosas
    waitForGpu();

    if (m_fenceEvent) {
        CloseHandle(m_fenceEvent);
        m_fenceEvent = nullptr;
    }

    // ComPtr se encarga del resto
    m_pipelineState.Reset();
    m_rootSignature.Reset();
    m_vertexBuffer.Reset();

    for (UINT i = 0; i < FrameCount; ++i) {
        m_renderTargets[i].Reset();
        m_cmdAllocator[i].Reset();
    }

    m_cmdList.Reset();
    m_rtvHeap.Reset();
    m_swapchain.Reset();
    m_cmdQueue.Reset();
    m_device.Reset();
    m_factory.Reset();
}

void DX12Backend::beginFrame(const float clearColor[4])
{
    // Reset allocator + command list para el frame actual
    HRESULT hr = m_cmdAllocator[m_frameIndex]->Reset();
    assert(SUCCEEDED(hr));

    hr = m_cmdList->Reset(m_cmdAllocator[m_frameIndex].Get(), 
        m_pipelineState.Get());
    assert(SUCCEEDED(hr));

    // Cambiar RT de PRESENT a RENDER_TARGET
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore =    D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter =     D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource =    D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_cmdList->ResourceBarrier(1, &barrier);

    // RTV handle (el i-ésimo)
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_frameIndex * m_rtvDescriptorSize;

    // Viewport / scissor
    m_cmdList->RSSetViewports(1, &m_viewport);
    m_cmdList->RSSetScissorRects(1, &m_scissorRect);

    // Set render target y clear
    m_cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    m_cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void DX12Backend::endFrame()
{
    // Pasar RT de RENDER_TARGET a PRESENT
    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_cmdList->ResourceBarrier(1, &barrier);

    HRESULT hr = m_cmdList->Close();
    assert(SUCCEEDED(hr));

    ID3D12CommandList* cmdLists[] = { m_cmdList.Get() };
    m_cmdQueue->ExecuteCommandLists(1, cmdLists);

    // Present
    UINT syncInterval = m_cfg.vsync ? 1 : 0;
    hr = m_swapchain->Present(syncInterval, 0);
    assert(SUCCEEDED(hr));

    moveToNextFrame();
}

void DX12Backend::drawTestScene()
{
    // Aquí solo nos centramos en la parte "de dibujo":
    m_cmdList->SetPipelineState(m_pipelineState.Get());
    m_cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
    m_cmdList->IASetVertexBuffers(0, 1, &m_vbView);

    // CBuffer con la matriz ortográfica
    m_cmdList->SetGraphicsRootConstantBufferView(
        0, m_constantBuffer->GetGPUVirtualAddress());

    // 1) Triángulo
    m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_cmdList->DrawInstanced(
        TestSceneGeometry::kTriangleVertexCount,
        1,
        TestSceneGeometry::kTriangleStart,
        0);

    // 2) Ejes
    m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    m_cmdList->DrawInstanced(
        TestSceneGeometry::kAxesVertexCount,
        1,
        TestSceneGeometry::kAxesStart,
        0);
}

bool DX12Backend::createInstanceAndDevice()
{
    UINT factoryFlags = 0;
#if defined(_DEBUG)
    factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_factory));
    if (FAILED(hr)) return false;

    // Crear dispositivo con el adaptador por defecto
    hr = D3D12CreateDevice(
        nullptr,
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)
    );
    if (FAILED(hr)) return false;

    // Cola de comandos
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.NodeMask = 0;

    hr = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_cmdQueue));
    if (FAILED(hr)) return false;

    // Swapchain
    DXGI_SWAP_CHAIN_DESC1 scDesc{};
    scDesc.Width = m_cfg.width;
    scDesc.Height = m_cfg.height;
    scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.Stereo = FALSE;
    scDesc.SampleDesc.Count = 1;
    scDesc.SampleDesc.Quality = 0;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.BufferCount = FrameCount;
    scDesc.Scaling = DXGI_SCALING_STRETCH;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    scDesc.Flags = 0;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapchain1;
    hr = m_factory->CreateSwapChainForHwnd(
        m_cmdQueue.Get(),
        m_hwnd,
        &scDesc,
        nullptr,
        nullptr,
        &swapchain1
    );
    if (FAILED(hr)) return false;

    hr = swapchain1.As(&m_swapchain);
    if (FAILED(hr)) return false;

    m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();

    return true;
}

bool DX12Backend::createSwapchainAndRTVs()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    HRESULT hr = m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
    if (FAILED(hr)) return false;

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < FrameCount; ++i) {
        hr = m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
        if (FAILED(hr)) return false;


        m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);

        rtvHandle.ptr += m_rtvDescriptorSize;
    }

    return true;
}

bool DX12Backend::createCommandObjects()
{
    HRESULT hr;

    for (UINT i = 0; i < FrameCount; ++i) {
        hr = m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_cmdAllocator[i])
        );
        if (FAILED(hr)) return false;
    }

    hr = m_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        m_cmdAllocator[0].Get(),
        nullptr, // PSO se asignará en beginFrame
        IID_PPV_ARGS(&m_cmdList)
    );
    if (FAILED(hr)) return false;

    // Lista empieza abierta, se cierra
    hr = m_cmdList->Close();
    if (FAILED(hr)) return false;

    return true;
}

bool DX12Backend::createSyncObjects()
{
    HRESULT hr = m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(hr)) return false;

    m_fenceValues[0] = 1;
    m_fenceValues[1] = 1;

    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent) return false;

    return true;
}

bool DX12Backend::createTestSceneResources()
{
    HRESULT hr;

    // 1) Vertex buffer
    const UINT vertexBufferSize =
        static_cast<UINT>(sizeof(TestVertex) * TestSceneGeometry::kTotalVertexCount);

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc =
        CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

    hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)
    );
    if (FAILED(hr)) return false;

    // Copiar los vértices CPU -> GPU
    std::uint8_t* pVertexDataBegin = nullptr;
    CD3DX12_RANGE readRange(0, 0); // no vamos a leer desde CPU
    hr = m_vertexBuffer->Map(0, &readRange,
        reinterpret_cast<void**>(&pVertexDataBegin));
    if (FAILED(hr)) return false;

    std::memcpy(pVertexDataBegin, TestSceneGeometry::gVertices,
        sizeof(TestVertex) * TestSceneGeometry::kTotalVertexCount);

    m_vertexBuffer->Unmap(0, nullptr);

    // Rellenar la vista del VB
    m_vbView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vbView.StrideInBytes = sizeof(TestVertex);
    m_vbView.SizeInBytes = vertexBufferSize;

    // 2) Constant buffer para la matriz ortográfica (MVP)
    CD3DX12_HEAP_PROPERTIES cbHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC cbResourceDesc = // 256-aligned
        CD3DX12_RESOURCE_DESC::Buffer((sizeof(DirectX::XMMATRIX) + 255) & ~255);

    hr = m_device->CreateCommittedResource(
        &cbHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &cbResourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer));
    if (FAILED(hr)) return false;

    CD3DX12_RANGE cbReadRange(0, 0);
    hr = m_constantBuffer->Map(0, &cbReadRange,
        reinterpret_cast<void**>(&m_cbvMappedData));
    if (FAILED(hr)) return false;

    // Construir la misma ortográfica que en DX12App
    float aspect = static_cast<float>(m_cfg.width) / m_cfg.height;
    float halfHeight = 300.0f;
    float halfWidth = aspect * halfHeight;

    DirectX::XMMATRIX ortho = DirectX::XMMatrixOrthographicOffCenterLH(
        -halfWidth, halfWidth,
        -halfHeight, halfHeight,
        0.0f, 1.0f);

    std::memcpy(m_cbvMappedData, &ortho, sizeof(ortho));

    // 3) Shaders (reutilizando vcolors_vertex.hlsl / vcolors_pixel.hlsl)
    Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

    // Usamos shaderBasePath del config
    std::wstring basePath(m_cfg.shaderBasePath.begin(), m_cfg.shaderBasePath.end());
    std::wstring vsPath = basePath + L"vcolors_vertex.hlsl";
    std::wstring psPath = basePath + L"vcolors_pixel.hlsl";

    hr = D3DCompileFromFile(
        vsPath.c_str(), nullptr, nullptr,
        "main", "vs_5_0", 0, 0,
        &vertexShader, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        return false;
    }

    errorBlob.Reset();
    hr = D3DCompileFromFile(
        psPath.c_str(), nullptr, nullptr,
        "main", "ps_5_0", 0, 0,
        &pixelShader, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        return false;
    }

    // 4) Root signature
    CD3DX12_ROOT_PARAMETER rootParam;
    rootParam.InitAsConstantBufferView(0);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init(
        1, &rootParam,
        0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig;
    Microsoft::WRL::ComPtr<ID3DBlob> error;
    hr = D3D12SerializeRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &serializedRootSig,
        &error);

    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        return false;
    }

    hr = m_device->CreateRootSignature(
        0,
        serializedRootSig->GetBufferPointer(),
        serializedRootSig->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature));
    if (FAILED(hr)) return false;

    // 5) Pipeline state (PSO)
    D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,
          0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,
          0, sizeof(float) * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
    psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());

    D3D12_RASTERIZER_DESC rastDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    rastDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
    psoDesc.RasterizerState = rastDesc;

    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;

    // IMPORTANTE: vamos a usar triángulo + líneas. Lo más limpio sería 2 PSOs;
    // de momento, para clavar la demo, mantenemos TRIANGLE + cambiamos la topología
    // entre draw calls, como ya hacías.
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    hr = m_device->CreateGraphicsPipelineState(
        &psoDesc, IID_PPV_ARGS(&m_pipelineState));
    if (FAILED(hr)) return false;

    return true;
}

void DX12Backend::waitForGpu()
{
    if (!m_cmdQueue || !m_fence) return;

    // Enviar señal
    const UINT64 fenceValue = m_fenceValues[m_frameIndex];
    HRESULT hr = m_cmdQueue->Signal(m_fence.Get(), fenceValue);
    if (FAILED(hr)) return;

    // Esperar a que la GPU alcance ese valor
    if (m_fence->GetCompletedValue() < fenceValue) {
        hr = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
        if (FAILED(hr)) return;
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    // Avanzar el valor para el siguiente uso
    m_fenceValues[m_frameIndex]++;
}

void DX12Backend::moveToNextFrame()
{
    const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];

    // Se señala la cola con el valor actual
    HRESULT hr = m_cmdQueue->Signal(m_fence.Get(), currentFenceValue);
    assert(SUCCEEDED(hr));

    // Se avanza índice del backbuffer
    m_frameIndex = m_swapchain->GetCurrentBackBufferIndex();

    // Si la GPU aún no ha alcanzado el valor para este frame, se espera
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex]) {
        hr = m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
        assert(SUCCEEDED(hr));
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    // Siguiente valor de fence para el frame actual
    m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}
