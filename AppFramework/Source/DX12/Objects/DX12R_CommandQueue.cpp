#include "DX12/Objects/DX12R_CommandQueue.h"
#include "DX12R.h"

using namespace DX12Interface;

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

bool DX12R_CommandQueue::Initialize(weak_ptr<DX12R_Device> device, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority, D3D12_COMMAND_QUEUE_FLAGS flags)
{
	m_device = device;

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = type;
	desc.Priority = priority;
	desc.Flags = flags;
	desc.NodeMask = device.lock()->GetNodeMask();

	if (!device.lock()->CreateCommandQueue(&desc,IID_PPV_ARGS(&m_commandQueue)))
		return false;

	return true;
}

void DX12R_CommandQueue::EnlistAllocator(weak_ptr<DX12R_CommandAllocator> allocator)
{
	m_inactiveAllocators.push_back(allocator.lock());
}

void DX12R_CommandQueue::CloseList(weak_ptr<DX12R_CommandList> commandList)
{
	m_waitingCommandLists.push_back(commandList.lock());
	m_waitingAllocators.push_back(commandList.lock()->GetCommandAllocator().lock());
}

void DX12R_CommandQueue::ExecuteCommandLists()
{
	std::vector<ID3D12CommandList*> lists;

	for (int i = 0; i < m_waitingCommandLists.size(); i++)
	{
		lists.push_back(m_waitingCommandLists[i]->GetCommandList().Get());
		m_inactiveCommandLists.push_back(m_waitingCommandLists[i]);
	}

	m_commandQueue->ExecuteCommandLists(static_cast<UINT>(m_waitingCommandLists.size()), lists.data());

	m_waitingCommandLists.clear();
	GetSwapChain()->GetFrameBuffer(frameIndex)->m_activeAllocators.swap(m_waitingAllocators);
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
		cAllocator->Reset();
		cList->Reset(cAllocator);
		return cList;
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
	newAllocator->Initialize(m_device.lock().get(), m_type, shared_from_this());
	m_inactiveAllocators.push_back(newAllocator);
}

shared_ptr<DX12R_CommandList> DX12R_CommandQueue::CreateCommandList()
{
	shared_ptr<DX12R_CommandAllocator> cAllocator = m_inactiveAllocators.back();
	m_inactiveAllocators.pop_back();
	cAllocator->Reset();

	shared_ptr<DX12R_CommandList> cList = make_shared<DX12R_CommandList>();
	cList->Initialize(m_device, m_type, cAllocator, weak_from_this());

	return cList;
}

