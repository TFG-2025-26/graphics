#include "DX12App.h"

#include <d3dcompiler.h>

#include <stdexcept>
#include <cassert>

struct Vertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

// Triángulo y ejes
static const Vertex vertices[] = {
	// Triángulo (cian)
	{{ 0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f }},
	{{ 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f }},
	{{-0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 1.0f, 1.0f }},

	// Ejes
	{{0, 0, 0}, {1, 0, 0, 1}}, {{1, 0, 0}, {1, 0, 0, 1}}, // X
	{{0, 0, 0}, {0, 1, 0, 1}}, {{0, 1, 0}, {0, 1, 0, 1}}, // Y
	{{0, 0, 0}, {0, 0, 1, 1}}, {{0, 0, 1}, {0, 0, 1, 1}}, // Z
};


void DX12App::init(HWND hwnd, int width, int height)
{
	m_width = width;
	m_height = height;

	// Crear el dispositivo
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory1(IID_PPV_ARGS(&factory));

	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(i, &adapter); ++i) {
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
			break;
	}

	// Crear cola de comandos
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));

	// Describir el swapchain
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.BufferCount = FrameCount;
	scDesc.Width = m_width;
	scDesc.Height = m_height;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(), hwnd, &scDesc, nullptr, nullptr, &swapChain1);

	swapChain1.As(&m_swapChain);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Crear el descriptor de pila para los RTV
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < FrameCount; ++i) {
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	// Crear asignador de comandos y listas
	m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
	m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));
	m_commandList->Close();

	// Crear verja
	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_fenceValue = 1;
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	m_vertexCount = _countof(vertices);

	const UINT vertexBufferSize = sizeof(vertices);

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

	m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer));

	// Copiar datos al buffer
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0); // No vamos a leer desde CPU
	m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	memcpy(pVertexDataBegin, vertices, sizeof(vertices));
	m_vertexBuffer->Unmap(0, nullptr);

	// Vista del buffer
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;

	// Compilar shaders
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	HRESULT hr = D3DCompileFromFile(
		L"assets/shaders/vcolors_vertex.hlsl", nullptr, nullptr,
		"main", "vs_5_0", 0, 0, &vertexShader, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		throw std::runtime_error("Error al compilar vertex shader");
	}

	hr = D3DCompileFromFile(
		L"assets/shaders/vcolors_pixel.hlsl", nullptr, nullptr,
		"main", "ps_5_0", 0, 0, &pixelShader, &errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		throw std::runtime_error("Error al compilar pixel shader");
	}

	// Crear constant buffer para MVP
	CD3DX12_HEAP_PROPERTIES cbHeapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC cbResourceDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(DirectX::XMMATRIX) + 255) & ~255);

	m_device->CreateCommittedResource(
		&cbHeapProps,
		D3D12_HEAP_FLAG_NONE,
		&cbResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constantBuffer));

	// Mapear e inicializar MVP = identidad
	CD3DX12_RANGE cbReadRange(0, 0);
	m_constantBuffer->Map(0, &cbReadRange, reinterpret_cast<void**>(&m_pCbvDataBegin));

	DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
	memcpy(m_pCbvDataBegin, &identity, sizeof(identity));

	// Root signature
	CD3DX12_ROOT_PARAMETER rootParam;
	rootParam.InitAsConstantBufferView(0);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc;
	rootSigDesc.Init(
		1,                // 1 parámetro raíz (el cbuffer)
		&rootParam,       // puntero al parámetro
		0, nullptr,       // sin static samplers
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSig;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &error);

	if (FAILED(hr)) {
		if (error) {
			OutputDebugStringA((char*)error->GetBufferPointer());
		}
		throw std::runtime_error("Error al serializar root signature");
	}

	hr = m_device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), 
		serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

	if (FAILED(hr)) {
		throw std::runtime_error("Error al crear root signature");
	}

	// Input layout
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, sizeof(float) * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	// Crear pipeline state object (PSO)
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
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
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	hr = m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
	if (FAILED(hr)) {
		char msg[128];
		sprintf_s(msg, "CreateGraphicsPipelineState failed with HRESULT: 0x%08X\n", (unsigned int)hr);
		OutputDebugStringA(msg);
		throw std::runtime_error("Error al crear el Pipeline State Object");
	}
}

void DX12App::render()
{
	// Reinicio de la lista de comandos
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	// Cambiar el estado del backbuffer a RENDER_TARGET
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_commandList->ResourceBarrier(1, &barrier);

	// Obtener handle y hacer clean
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_frameIndex,
		m_rtvDescriptorSize);

	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { 0.6f, 0.7f, 0.8f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->SetPipelineState(m_pipelineState.Get());
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	// m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	// m_commandList->DrawInstanced(m_vertexCount, 1, 0, 0);

	m_commandList->SetGraphicsRootConstantBufferView(0, m_constantBuffer->GetGPUVirtualAddress());

	D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)m_width, (float)m_height, 0.0f, 1.0f };
	D3D12_RECT scissorRect = { 0, 0, (LONG)m_width, (LONG)m_height };
	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->DrawInstanced(3, 1, 0, 0); // triángulo

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
	m_commandList->DrawInstanced(6, 1, 3, 0); // ejes

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTargets[m_frameIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	m_commandList->ResourceBarrier(1, &barrier);

	m_commandList->Close();

	ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

	m_swapChain->Present(1, 0);

	waitForPreviousFrame();
}

void DX12App::waitForPreviousFrame()
{
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);
	if (m_fence->GetCompletedValue() < m_fenceValue) {
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_fenceValue++;
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void DX12App::destroy()
{
	waitForPreviousFrame();
	CloseHandle(m_fenceEvent);
}
