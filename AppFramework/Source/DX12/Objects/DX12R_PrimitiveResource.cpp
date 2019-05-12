#include "DX12/Objects/DX12R_PrimitiveResource.h"
#include "DX12R.h"

DX12Interface* DX12R_PrimitiveResource::m_interface = nullptr;

DX12R_PrimitiveResource::DX12R_PrimitiveResource()
{
}

DX12R_PrimitiveResource::~DX12R_PrimitiveResource()
{
}

void DX12R_PrimitiveResource::CreateVertices(void* vertexData, UINT sizeofVertexData, UINT vertexCount, UINT nodeMask)
{
	m_primitive.primitiveType = DX12H_PrimitiveType_Vertex;
}

void DX12R_PrimitiveResource::CreateIndices(void* indexData, UINT indexCount, UINT nodeMask)
{
	m_primitive.primitiveType = DX12H_PrimitiveType_Index;
}

ID3D12Resource* DX12R_PrimitiveResource::GetPrimitiveResource()
{
	return m_resource.Get();
}

DX12H_PrimitiveWrapper* DX12R_PrimitiveResource::GetPrimitive()
{
	return &m_primitive;
}

void DX12R_PrimitiveResource::SetInterface(DX12Interface* dx12Interface)
{
	m_interface = dx12Interface;
}
