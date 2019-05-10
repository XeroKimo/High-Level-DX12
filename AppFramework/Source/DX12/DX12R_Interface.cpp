#include "DX12/DX12R_Interface.h"
#include "DX12R.h"

namespace DX12Interface
{
	extern const int frameBufferCount = 2;
	extern const int threadCount = 1;

	shared_ptr<DX12R_Device> dxrDevice;
	shared_ptr<DX12R_CommandQueue> dxrCommandQueue;								// Responsible for sending command lists to the device for execution
	unique_ptr<DX12R_SwapChain> dxrSwapChain;									// Swap chain used to switch between render targets

	ComPtr<ID3D12Fence>fence;							// Utilized for syncing the GPU and CPU
	HANDLE fenceEvent;															// A Handle to our fence, to know when the gpu is unlocked
	UINT64 fenceValue;								// This value is incremented each frame. Each fence has its own value

	D3D12_VIEWPORT viewport;													// The amount of pixels we will be rendering to
	D3D12_RECT scissorRect;														// How much of the viewport we will see when rendering

	unsigned int frameIndex;													// The current buffer we are currently on

	ComPtr<ID3D12RootSignature> defaultSignature;

	bool SingleGPUMode;
}

using namespace DX12Interface;

bool DX12R_Initialize(int windowWidth, int windowHeight, bool fullscreen ,HWND windowHandle)
{
	HRESULT hr;

//Graphics Device Creation

	dxrDevice = make_shared<DX12R_Device>();
	if (!dxrDevice->Initialize())
		return false;
	SingleGPUMode = true;
	ComPtr<ID3D12Device> device = dxrDevice->GetDevice();

//Command Queue Creation
	dxrCommandQueue = make_shared<DX12R_CommandQueue>();
	dxrCommandQueue->Initialize(dxrDevice, D3D12_COMMAND_LIST_TYPE_DIRECT);
	ComPtr<ID3D12CommandQueue> commandQueue = dxrCommandQueue->GetQueue();
	
//Swap Chain Creation
	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;	//How many samples per pixel are we drawing
	
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = windowWidth;
	swapChainDesc.Height = windowHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.BufferCount = frameBufferCount; // number of buffers we have
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present

	dxrSwapChain = make_unique<DX12R_SwapChain>();

	if (fullscreen)
	{
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
		dxrSwapChain->Initialize(dxrDevice.get(), commandQueue.Get(), windowHandle, &swapChainDesc, &fullscreenDesc);
	}
	else
		dxrSwapChain->Initialize(dxrDevice.get(), commandQueue.Get(), windowHandle, &swapChainDesc);
	
//Fence & Fence Event Creation
	hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	if (FAILED(hr))
		return false;
	fenceValue = 0;

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
		return false;

////#pragma region Default Root Signature Creation
////
////	defaultSignature = D3D12R_CreateRootSignature(nullptr, 0);
////
////#pragma endregion

//Default Viewport & ScissorRect Creation
	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.Width = static_cast<FLOAT>(windowWidth);
	viewport.Height = static_cast<FLOAT>(windowHeight);
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 0.f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = static_cast<LONG>(windowWidth);
	scissorRect.bottom = static_cast<LONG>(windowHeight);

	return true;
}

void DX12R_Shutdown()
{
	dxrSwapChain->GetFrameBuffer(frameIndex)->m_fence->SignalGPU(dxrCommandQueue.get());
	dxrSwapChain->GetFrameBuffer(frameIndex)->m_fence->SyncDevices();
}