#include "DX12/Objects/DX12R_CommandAllocator.h"
#include "DX12R.h"

DX12R_CommandAllocator::DX12R_CommandAllocator()
{
}

bool DX12R_CommandAllocator::Initialize(DX12S_CommandSystem* commandSystem, DX12R_Device* device, D3D12_COMMAND_LIST_TYPE type)
{
	if (FAILED(device->CreateCommandAllocator(type,IID_PPV_ARGS(&m_commandAllocator))))
		return false;

	m_commandSystem = commandSystem;

	return true;
}

void DX12R_CommandAllocator::ReEnlist()
{
	m_commandSystem->EnlistAllocator(shared_from_this());
}

HRESULT DX12R_CommandAllocator::Reset()
{
	return m_commandAllocator->Reset();
}

ComPtr<ID3D12CommandAllocator> DX12R_CommandAllocator::GetAllocator()
{
	return m_commandAllocator;
}
