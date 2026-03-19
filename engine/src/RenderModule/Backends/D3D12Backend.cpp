#include "D3D12Backend.h"

#include <SDL_syswm.h>

D3D12Backend::D3D12Backend(const std::string& appName)
{
    _appName = appName;
}

BackendAPI D3D12Backend::getAPI() const
{
    return BackendAPI::D3D12;
}

bool D3D12Backend::init(const RenderBackendDesc& desc)
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

    _frameIndex = _swapChain->GetCurrentBackBufferIndex();
    _fenceValues[_frameIndex] = 1;

    return true;
}


void D3D12Backend::setSync(bool enabled)
{
    _vsync = enabled;
}

bool D3D12Backend::cacheWindowHandle()
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if (!SDL_GetWindowWMInfo(_nativeWindow, &wmInfo)) {
        return false;
    }

    _hwnd = wmInfo.info.win.window;
    return _hwnd != nullptr;
}

bool D3D12Backend::createFactoryAndDevice()
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

bool D3D12Backend::createCommandObjects()
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

bool D3D12Backend::createSwapChain()
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

bool D3D12Backend::createRTVHeap()
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

bool D3D12Backend::createRenderTargets()
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

void D3D12Backend::releaseRenderTargets()
{
    for (auto& rt : _renderTargets) {
        rt.Reset();
    }
}

bool D3D12Backend::createFenceObjects()
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

bool D3D12Backend::beginFrame()
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

    return true;
}

void D3D12Backend::endFrame()
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

void D3D12Backend::moveToNextFrame()
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

void D3D12Backend::waitIdle()
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

void D3D12Backend::resize(uint32_t width, uint32_t height)
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

    createRenderTargets();
}

void D3D12Backend::shutdown()
{
    waitIdle();

    releaseRenderTargets();

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