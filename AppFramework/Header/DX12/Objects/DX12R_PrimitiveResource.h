#pragma once
#include "DX12/DirectX12.h"

enum DX12H_PrimitiveType
{
	DX12H_PrimitiveType_Vertex = 0x01,
	DX12H_PrimitiveType_Index = 0x02,
};

union DX12H_PrimitiveView
{
	D3D12_VERTEX_BUFFER_VIEW vertexView;
	D3D12_INDEX_BUFFER_VIEW indexView;
};

struct DX12H_PrimitiveWrapper
{
	DX12H_PrimitiveType primitiveType;
	DX12H_PrimitiveView primitiveView;
};

class DX12R_PrimitiveResource
{
public:
	DX12R_PrimitiveResource();
	~DX12R_PrimitiveResource();

	void CreateVertices(void* vertexData, UINT sizeofVertexData, UINT vertexCount, UINT nodeMask = 0);
	void CreateIndices(void* indexData, UINT indexCount, UINT nodeMask = 0);

	ID3D12Resource* GetPrimitiveResource();
	DX12H_PrimitiveWrapper* GetPrimitive();
private:
	ComPtr<ID3D12Resource> m_resource;
	DX12H_PrimitiveWrapper m_primitive;

};