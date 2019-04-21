#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>

#include "d3dx12.h"

#define SHADER_VERTEX "vs_"
#define SHADER_PIXEL "ps_"
#define SHADER_VERSION_5_0 "5_0"


struct D3D12_SHADER
{
	LPCWSTR fileName;
	LPCSTR shaderType;
	LPCSTR shaderVersion;
	D3D12_SHADER_BYTECODE shaderByteCode = {};

	D3D12_SHADER() {}
	D3D12_SHADER(LPCWSTR filename, LPCSTR shadertype, LPCSTR shaderversion)
	{
		fileName = filename;
		shaderType = shadertype;
		shaderVersion = shaderversion;
	}
};

namespace D3D12Renderer
{
	extern const int frameBufferCount;
	extern const int threadCount;

	extern ID3D12Device* graphicsDevice;												// The graphics device that will handle the rendering
	extern ID3D12CommandQueue* commandQueue;											// Responsible for sending command lists to the device for execution
	extern IDXGISwapChain3* swapChain;													// Swap chain used to switch between render targets
	extern ID3D12DescriptorHeap* rtvDescriptorHeap;										// Descriptor for the render-targets
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

	extern ID3D12RootSignature* defaultSignature;
}

bool D3D12_Initialize(int windowWidth, int windowHeight, HWND windowHandle);

void D3D12_BeginRender();
void D3D12_EndRender();
void D3D12_UsingPipeline(ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature);
void D3D12_DispatchCommandList();
void D3D12_WaitForPreviousFrame();

bool D3D12_CreateShaderByteCode(D3D12_SHADER* shader);
ID3D12RootSignature* D3D12_CreateRootSignature(D3D12_ROOT_PARAMETER* rootParamters);
D3D12_VERTEX_BUFFER_VIEW* D3D12_CreateVertexBuffer(void* vertices, unsigned int vertexCount, unsigned int sizeOfVertex);