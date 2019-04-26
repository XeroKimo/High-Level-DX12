#pragma once
#include <Windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <map>
#include <string>

#include "d3dx12.h"

#define SHADER_VERTEX "vs_"
#define SHADER_PIXEL "ps_"
#define SHADER_VERSION_5_0 "5_0"

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;

enum DescriptorBufferUse
{
    DescriptorBufferUse_Generic = 0x01,
    DescriptorBufferUse_ConstantBuffer,
    DescriptorBufferUse_MultiSample
};
struct D3D12R_RootSignatureWrapper;
struct D3D12R_DescriptorHeapWrapper;
class D3D12R_SignatureParametersHelper;

struct D3D12R_ShaderWrapper
{
	LPCWSTR fileName;
	LPCSTR shaderType;
	LPCSTR shaderVersion;
	D3D12_SHADER_BYTECODE shaderByteCode = {};

	D3D12R_ShaderWrapper() {}
	D3D12R_ShaderWrapper(LPCWSTR filename, LPCSTR shadertype, LPCSTR shaderversion)
	{
		fileName = filename;
		shaderType = shadertype;
		shaderVersion = shaderversion;
	}
};

struct D3D12R_DrawResource
{
	enum ResourceType
	{
		ResourceType_VertexBuffer = 0x01,
		ResourceType_IndexBuffer = 0x02,
	};

    union ResourceView
	{
		D3D12_VERTEX_BUFFER_VIEW* vertexBuffer;
		D3D12_INDEX_BUFFER_VIEW* indexBuffer;
	};

	ComPtr<ID3D12Resource> pResource = nullptr;
    ComPtr<ID3D12Resource> pUpload = nullptr;
	ResourceView view;
private:
	ResourceType viewType;

public:
	D3D12R_DrawResource(ResourceType type)
	{
		viewType = type;
		view.vertexBuffer = nullptr;
	}

	ResourceType* GetViewType() { return &viewType; }
	~D3D12R_DrawResource()
	{
		if (viewType == ResourceType_VertexBuffer)
			delete view.vertexBuffer;
		if (viewType == ResourceType_IndexBuffer)
			delete view.indexBuffer;

	}
};



namespace D3D12Renderer
{
	extern const int frameBufferCount;
	extern const int threadCount;

	extern ComPtr<ID3D12Device> graphicsDevice;												// The graphics device that will handle the rendering
	extern ComPtr<ID3D12CommandQueue> commandQueue;											// Responsible for sending command lists to the device for execution
	extern ComPtr<IDXGISwapChain3> swapChain;													// Swap chain used to switch between render targets
    extern unique_ptr<D3D12R_DescriptorHeapWrapper> rtvDescriptor;
	//extern ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;										// Descriptor for the render-targets
	//extern ComPtr<ID3D12Resource> renderTargets[];												// Resources in the rtv Descriptor heap, number of render targets should equal the amount of render buffers
	extern ComPtr<ID3D12CommandAllocator> commandAllocators[];									// Have enough command allocators for each buffer * threads
	extern ComPtr<ID3D12GraphicsCommandList> commandList;										// Records commands for the device to execute
	extern ComPtr<ID3D12Fence> fence[];														// Utilized for syncing the GPU and CPU

	extern HANDLE fenceEvent;															// A Handle to our fence, to know when the gpu is unlocked
	extern UINT64 fenceValue[];															// This value is incremented each frame. Each fence has its own value

	extern D3D12_VIEWPORT viewport;														// The amount of pixels we will be rendering to
	extern D3D12_RECT scissorRect;														// How much of the viewport we will see when rendering

	extern unsigned int frameIndex;														// The current buffer we are currently on
	extern int rtvDescriptorSize;														// The size of the rtvDescriptorHeap on the device

	extern ComPtr<ID3D12RootSignature> defaultSignature;

	extern std::map<std::string, shared_ptr<D3D12R_RootSignatureWrapper>> ownedRootSignatureParams;
}

bool D3D12R_Initialize(int windowWidth, int windowHeight, HWND windowHandle);
void D3D12R_Shutdown();

#pragma region Rendering Functions

void D3D12R_BeginRender();

//Must use to render objects
void D3D12R_UsingPipeline(ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature);
void D3D12R_UsingVertexBuffer(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
void D3D12R_UsingIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView);
void D3D12R_DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation, D3D12_PRIMITIVE_TOPOLOGY Topology);
void D3D12R_DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, UINT BaseVertexLocation, UINT StartInstanceLocation, D3D12_PRIMITIVE_TOPOLOGY Topology);

//Optional use
void D3D12R_SetViewport(float width, float height);

void D3D12R_EndRender();

#pragma endregion

void D3D12R_DispatchCommandList();
void D3D12R_WaitForPreviousFrame();

#pragma region DirextX Object Creation

ComPtr<ID3D12RootSignature> D3D12R_CreateRootSignature(D3D12_ROOT_PARAMETER* rootParamters, unsigned int numOfParameters);
bool D3D12R_CreateShaderByteCode(D3D12R_ShaderWrapper* shader);
ComPtr<ID3D12PipelineState> D3D12R_CreatePipelineState(ID3D12RootSignature* rootSignature, D3D12_INPUT_ELEMENT_DESC* inputLayout, unsigned int numOfElements, D3D12R_ShaderWrapper** arrayOfShaders, unsigned int numOfShaders);
unique_ptr<D3D12R_DrawResource> D3D12R_CreateVertexBuffer(void* vertices, unsigned int vertexCount, unsigned int sizeOfVertex);
unique_ptr<D3D12R_DrawResource> D3D12R_CreateIndexBuffer(DWORD* indices, DWORD indexCount);
ComPtr<ID3D12Resource> D3D12R_CreateDescriptor(D3D12_HEAP_TYPE heapType, DescriptorBufferUse bufferUse, unsigned int bufferSize, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON, LPCWSTR bufferName = nullptr);

//void D3D12R_GenerateUniqueRSPResources(const D3D12R_RSP* rootSignatureParams, unsigned int* inputDataSizes);

#pragma endregion

