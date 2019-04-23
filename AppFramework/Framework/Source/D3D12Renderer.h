#pragma once
#include <memory>
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "d3dx12.h"

#define SHADER_VERTEX "vs_"
#define SHADER_PIXEL "ps_"
#define SHADER_VERSION_5_0 "5_0"

struct D3D12ShaderWrapper
{
	LPCWSTR fileName;
	LPCSTR shaderType;
	LPCSTR shaderVersion;
	D3D12_SHADER_BYTECODE shaderByteCode = {};

	D3D12ShaderWrapper() {}
	D3D12ShaderWrapper(LPCWSTR filename, LPCSTR shadertype, LPCSTR shaderversion)
	{
		fileName = filename;
		shaderType = shadertype;
		shaderVersion = shaderversion;
	}
};

struct D3D12ResourceWrapper
{
	enum ResourceViewType
	{
		ResourceViewType_VertexBuffer = 0x01,
		ResourceViewType_IndexBuffer = 0x02,
		ResourceViewType_DescriptorBuffer = 0x03,
	};
	union ResourceView
	{
		D3D12_VERTEX_BUFFER_VIEW* vertexBuffer;
		D3D12_INDEX_BUFFER_VIEW* indexBuffer;
		ID3D12DescriptorHeap* descriptorBuffer;
	};

	ID3D12Resource* pResource = nullptr;
	ID3D12Resource** pResourceArray = nullptr;
	unsigned int resourceArrayCount = 0;
	ResourceView view;

private:
	ResourceViewType viewType;

public:
	D3D12ResourceWrapper(ResourceViewType type)
	{
		viewType = type;
	}
	~D3D12ResourceWrapper()
	{
		if (pResource)
			pResource->Release();
		if (pResourceArray)
		{
			for (unsigned int i = 0; i < resourceArrayCount; i++)
			{
				pResourceArray[i]->Release();
			}
			delete[] pResourceArray;
		}
		if (viewType == ResourceViewType_VertexBuffer)
			delete view.vertexBuffer;
		if (viewType == ResourceViewType_IndexBuffer)
			delete view.indexBuffer;
		if (viewType == ResourceViewType_DescriptorBuffer)
			view.descriptorBuffer->Release();
	}
};

class D3D12RootSignatureParameters
{

public:
    D3D12RootSignatureParameters(int NumParameters)
    {
        parameterCount = static_cast<char>(NumParameters);
        rootParameters.reset(new D3D12_ROOT_PARAMETER[NumParameters]);
    }

	void CreateRootConstant(int Num32BitValues, D3D12_SHADER_VISIBILITY shaderVisibility)
	{
		if (freeSpace == 0)
			return;

		D3D12_ROOT_CONSTANTS constant;
		constant.Num32BitValues = Num32BitValues;
		constant.RegisterSpace = 0;
		constant.ShaderRegister = constantBufferCount;
		constantBufferCount++;

        rootParameters.get()[filledParameters].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        rootParameters.get()[filledParameters].Constants = constant;
        rootParameters.get()[filledParameters].ShaderVisibility = shaderVisibility;
        filledParameters++;
		freeSpace -= Num32BitValues;
	}

	void CreateRootDescriptors(D3D12_ROOT_PARAMETER_TYPE* ParameterType, int DescriptorAmount, D3D12_SHADER_VISIBILITY* shaderVisibility)
	{
		for (int i = 0; i < DescriptorAmount; i++)
		{
			if (freeSpace < 2)
				return;

			D3D12_ROOT_DESCRIPTOR descriptor;
			descriptor.RegisterSpace = 0;

			if (ParameterType[i] == D3D12_ROOT_PARAMETER_TYPE_CBV)
			{
				descriptor.ShaderRegister = constantBufferCount;
				constantBufferCount++;
			}			
			else if (ParameterType[i] == D3D12_ROOT_PARAMETER_TYPE_UAV)
			{
				descriptor.ShaderRegister = unorderedAccessCount;
				unorderedAccessCount++;
			}
			else if (ParameterType[i] == D3D12_ROOT_PARAMETER_TYPE_SRV)
			{
				descriptor.ShaderRegister = shaderResourceCount;
				shaderResourceCount++;
			}

            rootParameters.get()[filledParameters].ParameterType = ParameterType[i];
            rootParameters.get()[filledParameters].Descriptor = descriptor;
            rootParameters.get()[filledParameters].ShaderVisibility = shaderVisibility[i];

			filledParameters++;
			freeSpace -= 2;

		}
	}

	void CreateRootDescriptorTables(D3D12_DESCRIPTOR_RANGE_TYPE* RangeType, int* NumDescriptorsInTable, int NumTables, D3D12_SHADER_VISIBILITY* shaderVisibility)
	{
		int i = 0;
        std::unique_ptr<D3D12_DESCRIPTOR_RANGE> descriptorRange(new D3D12_DESCRIPTOR_RANGE[NumTables]);
		for ( ; i < NumTables; i++)
		{
            if (freeSpace == 0)
                break;

            descriptorRange.get()[i].RangeType = RangeType[i];
            descriptorRange.get()[i].NumDescriptors = NumDescriptorsInTable[i];
            descriptorRange.get()[i].RegisterSpace = 0;
            descriptorRange.get()[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
			{
                descriptorRange.get()[i].BaseShaderRegister = constantBufferCount;
				constantBufferCount += NumDescriptorsInTable[i];
			}
			else if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
			{
                descriptorRange.get()[i].BaseShaderRegister = unorderedAccessCount;
				unorderedAccessCount += NumDescriptorsInTable[i];
			}
			else if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
			{
                descriptorRange.get()[i].BaseShaderRegister = shaderResourceCount;
				shaderResourceCount += NumDescriptorsInTable[i];
			}
			else if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			{
                descriptorRange.get()[i].BaseShaderRegister = samplerCount;
				samplerCount += NumDescriptorsInTable[i];
			}

		}

		D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
		descriptorTable.NumDescriptorRanges = i;
		descriptorTable.pDescriptorRanges = &descriptorRange.get()[0];

        rootParameters.get()[filledParameters].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        rootParameters.get()[filledParameters].DescriptorTable = descriptorTable;
        rootParameters.get()[filledParameters].ShaderVisibility = shaderVisibility[i];
		parameterCount++;
	}

    char GetParameterCount() { return parameterCount; }

    //std::unique_ptr<D3D12_ROOT_PARAMETER> GetRootParameters()
    //{
    //    return rootParameters;
    //}

private:
	char freeSpace = 64;
	char parameterCount = 0;
    char filledParameters = 0;
	unsigned long constantBufferCount = 0;
	unsigned long shaderResourceCount = 0;
	unsigned long unorderedAccessCount = 0;
	unsigned long samplerCount = 0;
	std::unique_ptr<D3D12_ROOT_PARAMETER> rootParameters;
};
namespace D3D12Renderer
{
	extern const int frameBufferCount;
	extern const int threadCount;

	extern ID3D12Device* graphicsDevice;												// The graphics device that will handle the rendering
	extern ID3D12CommandQueue* commandQueue;											// Responsible for sending command lists to the device for execution
	extern IDXGISwapChain3* swapChain;													// Swap chain used to switch between render targets
	extern D3D12ResourceWrapper* renderTargetResource;
	//extern ID3D12DescriptorHeap* rtvDescriptorHeap;										// Descriptor for the render-targets
	//extern ID3D12Resource* renderTargets[];												// Resources in the rtv Descriptor heap, number of render targets should equal the amount of render buffers
	extern ID3D12CommandAllocator* commandAllocators[];									// Have enough command allocators for each buffer * threads
	extern ID3D12GraphicsCommandList* commandList;										// Records commands for the device to execute
	extern ID3D12Fence* fence[];														// Utilized for syncing the GPU and CPU

	extern HANDLE fenceEvent;															// A Handle to our fence, to know when the gpu is unlocked
	extern UINT64 fenceValue[];															// This value is incremented each frame. Each fence has its own value

	extern D3D12_VIEWPORT viewport;														// The amount of pixels we will be rendering to
	extern D3D12_RECT scissorRect;														// How much of the viewport we will see when rendering

	extern unsigned int frameIndex;														// The current buffer we are currently on
	extern int rtvDescriptorSize;														// The size of the rtvDescriptorHeap on the device

	extern ID3D12RootSignature* defaultSignature;
}

bool D3D12_Initialize(int windowWidth, int windowHeight, HWND windowHandle);
void D3D12_Shutdown();

#pragma region Rendering Functions

void D3D12_BeginRender();

//Must use to render objects
void D3D12_UsingPipeline(ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature);
void D3D12_UsingVertexBuffer(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews);
void D3D12_UsingIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView);
void D3D12_DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation, D3D12_PRIMITIVE_TOPOLOGY Topology);
void D3D12_DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, UINT BaseVertexLocation, UINT StartInstanceLocation, D3D12_PRIMITIVE_TOPOLOGY Topology);

//Optional use
void D3D12_SetViewport(float width, float height);

void D3D12_EndRender();

#pragma endregion

void D3D12_DispatchCommandList();
void D3D12_WaitForPreviousFrame();

#pragma region DirextX Object Creation

ID3D12RootSignature* D3D12_CreateRootSignature(D3D12_ROOT_PARAMETER* rootParamters, unsigned int numOfParameters);
bool D3D12_CreateShaderByteCode(D3D12ShaderWrapper* shader);
ID3D12PipelineState* D3D12_CreatePipelineState(ID3D12RootSignature* rootSignature, D3D12_INPUT_ELEMENT_DESC* inputLayout, unsigned int numOfElements, D3D12ShaderWrapper** arrayOfShaders, unsigned int numOfShaders);
D3D12ResourceWrapper* D3D12_CreateVertexBuffer(void* vertices, unsigned int vertexCount, unsigned int sizeOfVertex);
D3D12ResourceWrapper* D3D12_CreateIndexBuffer(DWORD* indices, DWORD indexCount);

#pragma endregion

