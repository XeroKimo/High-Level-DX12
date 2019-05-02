#include "DX12/Objects/DX12R_CommandAllocator.h"
#include "DX12R.h"

DX12R_CommandAllocator::DX12R_CommandAllocator()
{
}

bool DX12R_CommandAllocator::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type)
{
	if (FAILED(device->CreateCommandAllocator(type, IID_PPV_ARGS(&m_commandAllocator))))
		return false;

	return true;
}

bool DX12R_CommandAllocator::Initialize(DX12R_Device* device, D3D12_COMMAND_LIST_TYPE type, weak_ptr<DX12R_CommandQueue> queue)
{
	if (!device->CreateCommandAllocator(type,IID_PPV_ARGS(&m_commandAllocator)))
		return false;

	m_commandQueue = queue;

	return true;
}

void DX12R_CommandAllocator::ReEnlist()
{
	m_commandQueue.lock()->EnlistAllocator(shared_from_this());
}

HRESULT DX12R_CommandAllocator::Reset()
{
	return m_commandAllocator->Reset();
}

ComPtr<ID3D12CommandAllocator> DX12R_CommandAllocator::GetAllocator()
{
	return m_commandAllocator;
}

weak_ptr<DX12R_CommandQueue> DX12R_CommandAllocator::GetCommandQueue()
{
	return m_commandQueue;
}
