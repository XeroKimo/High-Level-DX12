#include "DX12/Systems/DX12S_CommandSystem.h"
#include "DX12R.h"

DX12S_CommandSystem::DX12S_CommandSystem()
{
}

bool DX12S_CommandSystem::Initialize(DX12S_DeviceContext* deviceContext, D3D12_COMMAND_LIST_TYPE type)
{
	m_type = type;
	m_deviceContext = deviceContext;
	m_commandQueue = make_unique<DX12R_CommandQueue>();
	if (!m_commandQueue->Initialize(this, m_deviceContext->GetDevice(), type))
		return false;

	return true;
}

void DX12S_CommandSystem::EnlistAllocator(weak_ptr<DX12R_CommandAllocator> allocator)
{
	m_inactiveAllocators.push_back(allocator.lock());
}

void DX12S_CommandSystem::CloseList(weak_ptr<DX12R_CommandList> commandList)
{
	m_waitingCommandLists.push_back(commandList.lock());
	m_waitingAllocators.push_back(commandList.lock()->GetCommandAllocator().lock());
}

void DX12S_CommandSystem::ExecuteCommandList(weak_ptr<DX12R_CommandList> commandList)
{
	m_commandQueue->ExecuteCommandList(commandList.lock().get());
	m_inactiveCommandLists.push_back(commandList.lock());
}

void DX12S_CommandSystem::ExecuteWaitingCommandLists()
{
	std::vector<ID3D12CommandList*> lists;
	//lists.reserve(m_waitingCommandLists.size());
	for (int i = 0; i < m_waitingCommandLists.size(); i++)
	{
		lists.push_back(m_waitingCommandLists[i]->GetCommandList().Get());
		m_inactiveCommandLists.push_back(m_waitingCommandLists[i]);
	}

	m_commandQueue->ExecuteCommandLists(static_cast<UINT>(m_waitingCommandLists.size()), &lists[0]);

	m_waitingCommandLists.clear();
	m_deviceContext->GetDX12Interface()->SendAllocatorsToCurrentFrame(m_waitingAllocators);
	m_waitingAllocators.clear();
	
	//GetSwapChain()->GetFrameBuffer(frameIndex)->m_activeAllocators.swap(m_waitingAllocators);
}

shared_ptr<DX12R_CommandList> DX12S_CommandSystem::GetCommandList()
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

shared_ptr<DX12R_CommandQueue> DX12S_CommandSystem::GetCommandQueue()
{
	return m_commandQueue;
}

DX12S_DeviceContext* DX12S_CommandSystem::GetDeviceContext()
{
	return m_deviceContext;
}

DX12Interface* DX12S_CommandSystem::GetDX12Interface()
{
	return m_deviceContext->GetDX12Interface();
}

void DX12S_CommandSystem::CreateCommandAllocator()
{
	shared_ptr<DX12R_CommandAllocator> newAllocator = make_shared<DX12R_CommandAllocator>();
	newAllocator->Initialize(this, m_deviceContext->GetDevice(), m_type);
	m_inactiveAllocators.push_back(newAllocator);
}

shared_ptr<DX12R_CommandList> DX12S_CommandSystem::CreateCommandList()
{
	shared_ptr<DX12R_CommandAllocator> cAllocator = m_inactiveAllocators.back();
	m_inactiveAllocators.pop_back();
	cAllocator->Reset();

	shared_ptr<DX12R_CommandList> cList = make_shared<DX12R_CommandList>();
	cList->Initialize(this, m_deviceContext->GetDevice(), m_type, cAllocator);

	return cList;
}
