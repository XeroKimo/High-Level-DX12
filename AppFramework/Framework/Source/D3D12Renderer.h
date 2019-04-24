#pragma once
#include <vector>
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "d3dx12.h"

#define SHADER_VERTEX "vs_"
#define SHADER_PIXEL "ps_"
#define SHADER_VERSION_5_0 "5_0"

struct D3D12RShaderWrapper
{
	LPCWSTR fileName;
	LPCSTR shaderType;
	LPCSTR shaderVersion;
	D3D12_SHADER_BYTECODE shaderByteCode = {};

	D3D12RShaderWrapper() {}
	D3D12RShaderWrapper(LPCWSTR filename, LPCSTR shadertype, LPCSTR shaderversion)
	{
		fileName = filename;
		shaderType = shadertype;
		shaderVersion = shaderversion;
	}
};

struct D3D12R_ResourceWrapper
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
	D3D12R_ResourceWrapper(ResourceViewType type)
	{
		viewType = type;
	}
	~D3D12R_ResourceWrapper()
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

struct D3D12R_RSPInfo
{
	D3D12_ROOT_PARAMETER_TYPE parameterType;
	unsigned int numberOfValues = 0;
	unsigned int* shaderRegister = nullptr;
	unsigned int* numberOfDescriptor = nullptr;

	~D3D12R_RSPInfo()
	{
		delete[] shaderRegister;
		delete[] numberOfDescriptor;
	}
};

struct D3D12R_RSP
{
	unsigned int parameterCount = 0;
	D3D12R_RSPInfo* parameterInfo;

	~D3D12R_RSP()
	{
		delete[] parameterInfo;
	}
};

class D3D12R_SignatureParametersHelper
{
public:

	~D3D12R_SignatureParametersHelper()
	{
		for (D3D12_ROOT_CONSTANTS* constants : rootConstants)
			delete constants;
		for (D3D12_ROOT_DESCRIPTOR* descriptors : rootDescriptors)
			delete descriptors;
		for (D3D12_DESCRIPTOR_RANGE* ranges : descriptorRanges)
			delete ranges;
		for (D3D12_ROOT_DESCRIPTOR_TABLE* table : rootTables)
			delete table;
	}

	void CreateRootConstant(int Num32BitValues, D3D12_SHADER_VISIBILITY shaderVisibility)
	{
		if (freeSpace == 0)
			return;

		D3D12_ROOT_CONSTANTS* constant = new D3D12_ROOT_CONSTANTS;
		constant->Num32BitValues = Num32BitValues;
		constant->RegisterSpace = 0;
		constant->ShaderRegister = constantBufferCount;
		constantBufferCount++;
		rootConstants.push_back(constant);

		D3D12_ROOT_PARAMETER rootParameter;
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		rootParameter.Constants = *constant;
		rootParameter.ShaderVisibility = shaderVisibility;

		rootParameters.push_back(rootParameter);
		freeSpace -= Num32BitValues;
	}

	void CreateRootDescriptors(D3D12_ROOT_PARAMETER_TYPE* ParameterType, int DescriptorAmount, D3D12_SHADER_VISIBILITY* shaderVisibility)
	{
		for (int i = 0; i < DescriptorAmount; i++)
		{
			if (freeSpace < 2)
				return;

			D3D12_ROOT_DESCRIPTOR* descriptor = new D3D12_ROOT_DESCRIPTOR;
			descriptor->RegisterSpace = 0;

			if (ParameterType[i] == D3D12_ROOT_PARAMETER_TYPE_CBV)
			{
				descriptor->ShaderRegister = constantBufferCount;
				constantBufferCount++;
			}			
			else if (ParameterType[i] == D3D12_ROOT_PARAMETER_TYPE_UAV)
			{
				descriptor->ShaderRegister = unorderedAccessCount;
				unorderedAccessCount++;
			}
			else if (ParameterType[i] == D3D12_ROOT_PARAMETER_TYPE_SRV)
			{
				descriptor->ShaderRegister = shaderResourceCount;
				shaderResourceCount++;
			}
			rootDescriptors.push_back(descriptor);


			D3D12_ROOT_PARAMETER rootParameter;
			rootParameter.ParameterType = ParameterType[i];
			rootParameter.Descriptor = *descriptor;
			rootParameter.ShaderVisibility = shaderVisibility[i];

			rootParameters.push_back(rootParameter);
			freeSpace -= 2;

		}
	}

	void CreateRootDescriptorTables(D3D12_DESCRIPTOR_RANGE_TYPE* RangeType, int* NumDescriptorsInTable, int NumDescriptorRangeType, D3D12_SHADER_VISIBILITY shaderVisibility)
	{

		std::vector<D3D12_DESCRIPTOR_RANGE*> descriptorViewRange;
		std::vector<D3D12_DESCRIPTOR_RANGE*> descriptorSamplerRange;
		if (freeSpace == 0)
			return;

		for (int i = 0; i < NumDescriptorRangeType; i++)
		{
			if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_CBV ||
				RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_UAV ||
				RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
			{
				D3D12_DESCRIPTOR_RANGE* range = new D3D12_DESCRIPTOR_RANGE;
				range->RangeType = RangeType[i];
				range->NumDescriptors = NumDescriptorsInTable[i];
				range->RegisterSpace = 0;
				range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

				if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
				{
					range->BaseShaderRegister = constantBufferCount;
					constantBufferCount += NumDescriptorsInTable[i];
				}
				else if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
				{
					range->BaseShaderRegister = unorderedAccessCount;
					unorderedAccessCount += NumDescriptorsInTable[i];
				}
				else if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
				{
					range->BaseShaderRegister = shaderResourceCount;
					shaderResourceCount += NumDescriptorsInTable[i];
				}
				descriptorViewRange.push_back(range);
				descriptorRanges.push_back(range);
			}
			else
			{
				D3D12_DESCRIPTOR_RANGE* range = new D3D12_DESCRIPTOR_RANGE;
				range->RangeType = RangeType[i];
				range->NumDescriptors = NumDescriptorsInTable[i];
				range->RegisterSpace = 0;
				range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
				range->BaseShaderRegister = samplerCount;

				descriptorSamplerRange.push_back(range);
				descriptorRanges.push_back(range);
			}
		}

		if (descriptorViewRange.size() > 0)
		{
			D3D12_ROOT_DESCRIPTOR_TABLE* descriptorViewTable = new D3D12_ROOT_DESCRIPTOR_TABLE;
			descriptorViewTable->NumDescriptorRanges = static_cast<UINT>(descriptorViewRange.size());
			descriptorViewTable->pDescriptorRanges = descriptorViewRange[0];
			rootTables.push_back(descriptorViewTable);

			D3D12_ROOT_PARAMETER rootParameter;
			rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter.DescriptorTable = *descriptorViewTable;
			rootParameter.ShaderVisibility = shaderVisibility;
			rootParameters.push_back(rootParameter);

			freeSpace--;
		}
		if (descriptorSamplerRange.size() > 0)
		{
			D3D12_ROOT_DESCRIPTOR_TABLE* descriptorSamplerTable = new D3D12_ROOT_DESCRIPTOR_TABLE;
			descriptorSamplerTable->NumDescriptorRanges = static_cast<UINT>(descriptorSamplerRange.size());
			descriptorSamplerTable->pDescriptorRanges = descriptorSamplerRange[0];
			rootTables.push_back(descriptorSamplerTable);

			D3D12_ROOT_PARAMETER rootParameter;
			rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParameter.DescriptorTable = *descriptorSamplerTable;
			rootParameter.ShaderVisibility = shaderVisibility;
			rootParameters.push_back(rootParameter);

			freeSpace--;
		}
	}

    char GetParameterCount() { return static_cast<char>(rootParameters.size()); }

	D3D12_ROOT_PARAMETER* GetRootParameters()
	{
		return &rootParameters[0];
	}

	D3D12R_RSP* MakeParameterInfo()
	{
		D3D12R_RSP* info = new D3D12R_RSP();

		info->parameterCount = static_cast<UINT>(rootParameters.size());
		info->parameterInfo = new D3D12R_RSPInfo[rootParameters.size()];
		for (int i = 0; i < rootParameters.size(); i++)
		{
			info->parameterInfo[i].parameterType = rootParameters[i].ParameterType;
			if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
			{
				info->parameterInfo[i].shaderRegister = new unsigned int[1];
				info->parameterInfo[i].shaderRegister[0] = rootParameters[i].Constants.ShaderRegister;
				info->parameterInfo[i].numberOfValues = rootParameters[i].Constants.Num32BitValues;
			}
			else if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV ||
				rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV ||
				rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV)
			{
				info->parameterInfo[i].shaderRegister = new unsigned int[1];
				info->parameterInfo[i].shaderRegister[0] = rootParameters[i].Descriptor.ShaderRegister;
				info->parameterInfo[i].numberOfValues = 1;
			}
			else if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			{
				info->parameterInfo[i].numberOfValues = rootParameters[i].DescriptorTable.NumDescriptorRanges;
				info->parameterInfo[i].shaderRegister = new unsigned int[rootParameters[i].DescriptorTable.NumDescriptorRanges];
				info->parameterInfo[i].numberOfDescriptor = new unsigned int[rootParameters[i].DescriptorTable.NumDescriptorRanges];
				for (int v = 0; v < static_cast<int>(rootParameters[i].DescriptorTable.NumDescriptorRanges); v++)
				{
					info->parameterInfo[i].shaderRegister[v] = rootParameters[i].DescriptorTable.pDescriptorRanges[v].BaseShaderRegister;
					info->parameterInfo[i].numberOfDescriptor[v] = rootParameters[i].DescriptorTable.pDescriptorRanges[v].NumDescriptors;
				}
			}
		}

		return info;
	}
private:
	char freeSpace = 64;
	unsigned long constantBufferCount = 0;
	unsigned long shaderResourceCount = 0;
	unsigned long unorderedAccessCount = 0;
	unsigned long samplerCount = 0;
	std::vector<D3D12_ROOT_CONSTANTS*> rootConstants;
	std::vector<D3D12_ROOT_DESCRIPTOR*> rootDescriptors;
	std::vector<D3D12_DESCRIPTOR_RANGE*> descriptorRanges;
	std::vector<D3D12_ROOT_DESCRIPTOR_TABLE*> rootTables;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
};

namespace D3D12Renderer
{
	extern const int frameBufferCount;
	extern const int threadCount;

	extern ID3D12Device* graphicsDevice;												// The graphics device that will handle the rendering
	extern ID3D12CommandQueue* commandQueue;											// Responsible for sending command lists to the device for execution
	extern IDXGISwapChain3* swapChain;													// Swap chain used to switch between render targets
	extern D3D12R_ResourceWrapper* renderTargetResource;
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

ID3D12RootSignature* D3D12R_CreateRootSignature(D3D12_ROOT_PARAMETER* rootParamters, unsigned int numOfParameters);
bool D3D12R_CreateShaderByteCode(D3D12RShaderWrapper* shader);
ID3D12PipelineState* D3D12R_CreatePipelineState(ID3D12RootSignature* rootSignature, D3D12_INPUT_ELEMENT_DESC* inputLayout, unsigned int numOfElements, D3D12RShaderWrapper** arrayOfShaders, unsigned int numOfShaders);
D3D12R_ResourceWrapper* D3D12R_CreateVertexBuffer(void* vertices, unsigned int vertexCount, unsigned int sizeOfVertex);
D3D12R_ResourceWrapper* D3D12R_CreateIndexBuffer(DWORD* indices, DWORD indexCount);

#pragma endregion

