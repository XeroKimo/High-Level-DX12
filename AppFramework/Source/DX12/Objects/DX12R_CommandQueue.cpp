#include "DX12/Objects/DX12R_CommandQueue.h"

DX12R_CommandQueue::DX12R_CommandQueue()
{
}

bool DX12R_CommandQueue::Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority, D3D12_COMMAND_QUEUE_FLAGS flags)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = priority;
	desc.Flags = flags;
	desc.NodeMask = device->GetNodeCount();

	if (FAILED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue))))
		return false;

	return true;
}

void DX12R_CommandQueue::ExecuteCommandLists(UINT numCommandLists, ID3D12CommandList* const* commandLists)
{
	m_commandQueue->ExecuteCommandLists(numCommandLists, commandLists);
}

HRESULT DX12R_CommandQueue::Signal(ID3D12Fence* fence, UINT64 value)
{
	return m_commandQueue->Signal(fence,value);
}

HRESULT DX12R_CommandQueue::Wait(ID3D12Fence* fence, UINT64 value)
{
	return m_commandQueue->Wait(fence,value);
}

shared_ptr<DX12R_CommandList> DX12R_CommandQueue::GetCommandList()
{
	if (m_inactiveAllocators.empty())
	{
		CreateCommandAllocator();//Create command allocator
	}
	if (m_inactiveCommandLists.empty())
	{
		return CreateCommandList();//Create command list
	}
	else
	{
		shared_ptr<DX12R_CommandList> cList = m_inactiveCommandLists.back();
		shared_ptr<DX12R_CommandAllocator> cAllocator = m_inactiveAllocators.back();
		m_inactiveCommandLists.pop_back();
		m_inactiveAllocators.pop_back();
		//cList->Reset()
		//return //Return a inactive commandList with inactive allocator
	}
}

ComPtr<ID3D12CommandQueue> DX12R_CommandQueue::GetQueue()
{
	return m_commandQueue;
}

void DX12R_CommandQueue::CreateCommandAllocator()
{
	shared_ptr<DX12R_CommandAllocator> newAllocator = make_unique<DX12R_CommandAllocator>();
	newAllocator->Initialize(m_device->GetDevice().Get(), m_type);
	m_inactiveAllocators.push_back(newAllocator);
}

shared_ptr<DX12R_CommandList> DX12R_CommandQueue::CreateCommandList()
{
	return shared_ptr<DX12R_CommandList>();
}

