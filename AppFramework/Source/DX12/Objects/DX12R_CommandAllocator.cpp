#include "DX12/Objects/DX12R_CommandAllocator.h"

DX12R_CommandAllocator::DX12R_CommandAllocator()
{
}

bool DX12R_CommandAllocator::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
{
	if (FAILED(device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_commandAllocator))))
		return false;

	return true;
}

HRESULT DX12R_CommandAllocator::Reset()
{
	return m_commandAllocator->Reset();
}

ComPtr<ID3D12CommandAllocator> DX12R_CommandAllocator::GetAllocator()
{
	return m_commandAllocator;
}
