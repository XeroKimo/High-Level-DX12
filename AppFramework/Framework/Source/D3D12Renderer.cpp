#include "D3D12Renderer.h"

namespace D3D12Renderer
{
	extern const int frameBufferCount = 2;
	extern const int threadCount = 1;

	ID3D12Device* graphicsDevice;												// The graphics device that will handle the rendering
	ID3D12CommandQueue* commandQueue;											// Responsible for sending command lists to the device for execution
	IDXGISwapChain3* swapChain;													// Swap chain used to switch between render targets
	ID3D12DescriptorHeap* rtvDescriptorHeap;										// Descriptor for the render-targets
	ID3D12Resource* renderTargets[frameBufferCount];							// Resources in the rtv Descriptor heap, number of render targets should equal the amount of render buffers
	ID3D12CommandAllocator* commandAllocators[frameBufferCount * threadCount];	// Have enough command allocators for each buffer * threads
	ID3D12GraphicsCommandList* commandList;										// Records commands for the device to execute
	ID3D12Fence* fence[frameBufferCount * threadCount];							// Utilized for syncing the GPU and CPU

	HANDLE fenceEvent;															// A Handle to our fence, to know when the gpu is unlocked
	UINT64 fenceValue[frameBufferCount * threadCount];							// This value is incremented each frame. Each fence has its own value

	D3D12_VIEWPORT viewport;													// The amount of pixels we will be rendering to
	D3D12_RECT scissorRect;														// How much of the viewport we will see when rendering

	unsigned int frameIndex;														// The current buffer we are currently on
	int rtvDescriptorSize;														// The size of the rtvDescriptorHeap on the device
}

using namespace D3D12Renderer;

bool D3D12_Initialize(int windowWidth, int windowHeight, HWND windowHandle)
{
	HRESULT hr;	// TODO: Delete all hr stuff when we know it works

	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
		return false;

#pragma region Graphics Device Creation

	IDXGIAdapter1* adapter;
	UINT adapterIndex = 0;
	bool adapterFound = false;

	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't look for a software device
			adapterIndex++;
			continue;
		}

		// Find a D3D12 compatible device ( minimum feature level for D3D12 is feature level 11_0 )
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&graphicsDevice));
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
		return false;

#pragma endregion
#pragma region Command Queue & Swap Chain Creation 
#pragma region Command Queue Creation

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	hr = graphicsDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue));
	if (FAILED(hr))
		return false;

#pragma endregion

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

	IDXGISwapChain1* tempSwapChain = nullptr;

	dxgiFactory->CreateSwapChainForHwnd(
		commandQueue,
		windowHandle,
		&swapChainDesc,
		NULL,
		NULL,
		&tempSwapChain
	);

	swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
	frameIndex = swapChain->GetCurrentBackBufferIndex();

#pragma endregion
#pragma region Render-Targets View & Render Targets Creation 

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	hr = graphicsDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	if (FAILED(hr))
		return false;

	rtvDescriptorSize = graphicsDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

#pragma region Render-Targets / Render Buffers Creation

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (int i = 0; i < frameBufferCount; i++)
	{
		//Get a buffer in the swap chain
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
		if (FAILED(hr))
			return false;
		//Create a render-target in the rtvDescriptorHeap
		graphicsDevice->CreateRenderTargetView(renderTargets[i], nullptr, rtvHandle);
		//Offset the value to store the next render-target
		rtvHandle.Offset(1, rtvDescriptorSize);
	}

#pragma endregion
#pragma endregion
#pragma region Command Allocators & Command List Creation

	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = graphicsDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]));
		if (FAILED(hr))
			return false;
	}

	hr = graphicsDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[frameIndex], NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
		return false;

	commandList->Close();

#pragma endregion
#pragma region Fence & Fence Event Creation

	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = graphicsDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
			return false;
		fenceValue[i] = 0;
	}

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
		return false;


#pragma endregion
#pragma region Default Viewport & ScissorRect Creation

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

#pragma endregion

	dxgiFactory->Release();

	return true;
}

void D3D12_BeginRender()
{
	D3D12_WaitForPreviousFrame();
	commandAllocators[frameIndex]->Reset();
	D3D12_UsingPipeline(nullptr,nullptr);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void D3D12_EndRender()
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	D3D12_DispatchCommandList();
	commandQueue->Signal(fence[frameIndex], fenceValue[frameIndex]);

	swapChain->Present(0, 0);
}

void D3D12_UsingPipeline(ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature)
{
	if (!pipelineState)
	{
		commandList->Reset(commandAllocators[frameIndex], nullptr);
		return;
	}
	D3D12_DispatchCommandList();

	commandList->Reset(commandAllocators[frameIndex], pipelineState);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

}

void D3D12_DispatchCommandList()
{
	commandList->Close();
	ID3D12CommandList* ppCommandList[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);
}

void D3D12_WaitForPreviousFrame()
{
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	fenceValue[frameIndex]++;
}