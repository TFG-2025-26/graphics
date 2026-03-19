#pragma once

#ifndef D3D12_BACKEND_H_
#define D3D12_BACKEND_H_

#include <array>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <Windows.h>
#include <wrl.h>

#include "IRenderBackend.h"

class D3D12Backend : public IRenderBackend {
public:
	D3D12Backend(const std::string& appName);
	~D3D12Backend() override = default;

	BackendAPI getAPI() const override;

	bool init(const RenderBackendDesc& desc) override;
	void shutdown() override;
	void waitIdle() override;

	void resize(uint32_t width, uint32_t height) override;
	void setSync(bool enabled) override;

	bool beginFrame() override;
	void endFrame() override;

private:
	bool cacheWindowHandle();
	bool createFactoryAndDevice();
	bool createCommandObjects();
	bool createSwapChain();
	bool createRTVHeap();
	bool createRenderTargets();
	bool createFenceObjects();

	void releaseRenderTargets();
	void moveToNextFrame();

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

	UINT _frameIndex = 0;

	float _clearColor[4] = { 0.08f, 0.10f, 0.16f, 1.0f };
};


#endif // D3D12_BACKEND_H_