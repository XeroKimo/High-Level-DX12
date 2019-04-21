#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include "d3dx12.h"

namespace D3D12Renderer
{
	extern const int frameBufferCount;
	extern const int threadCount;

	extern ID3D12Device* graphicsDevice;												// The graphics device that will handle the rendering
	extern ID3D12CommandQueue* commandQueue;											// Responsible for sending command lists to the device for execution
	extern IDXGISwapChain3* swapChain;													// Swap chain used to switch between render targets
	extern ID3D12DescriptorHeap* rtvDecriptorHeap;										// Descriptor for the render-targets
	extern ID3D12Resource* renderTargets[];								// Resources in the rtv Descriptor heap, number of render targets should equal the amount of render buffers
	extern ID3D12CommandAllocator* commandAllocators[];	// Have enough command allocators for each buffer * threads
	extern ID3D12GraphicsCommandList* commandList;										// Records commands for the device to execute
	extern ID3D12Fence* fence[];							// Utilized for syncing the GPU and CPU

	extern HANDLE fenceEvent;															// A Handle to our fence, to know when the gpu is unlocked
	extern UINT64 fenceValue[];							// This value is incremented each frame. Each fence has its own value

	extern D3D12_VIEWPORT viewport;														// The amount of pixels we will be rendering to
	extern D3D12_RECT scissorRect;														// How much of the viewport we will see when rendering

	extern unsigned int frameIndex;														// The current buffer we are currently on
	extern int rtvDescriptorSize;														// The size of the rtvDescriptorHeap on the device
}

bool D3D12_Initialize(int windowWidth, int windowHeight, HWND windowHandle);
