#pragma once

#ifndef DX12_APP_H_
#define DX12_APP_H_

#include <Windows.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <wrl.h>

#include <DirectXMath.h>

class DX12App
{
public:
	void init(HWND hwnd, int width, int height);
	void render();
	void waitForPreviousFrame();
	void destroy();

private:
	static const UINT FrameCount = 2;

	UINT m_frameIndex = 0;

	// DXGI
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;

	// Cola de comandos
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;

	// Swapchain buffer y RTV
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	UINT m_rtvDescriptorSize = 0;

	// Fence
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue = 0;
	HANDLE m_fenceEvent = nullptr;

	// Tamaño
	UINT m_width = 800;
	UINT m_height = 600;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView = {};
	UINT m_vertexCount = 0;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
	UINT8* m_pCbvDataBegin = nullptr;
};

#endif // DX12_APP_H