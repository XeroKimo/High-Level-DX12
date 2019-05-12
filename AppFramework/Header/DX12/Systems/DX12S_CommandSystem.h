#pragma once
#include "DX12/DirectX12.h"

class DX12Interface;

class DX12R_CommandAllocator;
class DX12R_CommandList;
class DX12R_CommandQueue;

class DX12S_DeviceContext;

class DX12S_CommandSystem
{
public:
	DX12S_CommandSystem();

	bool Initialize(DX12S_DeviceContext* deviceContext, D3D12_COMMAND_LIST_TYPE type);

	void EnlistAllocator(weak_ptr<DX12R_CommandAllocator> allocator);
	void CloseList(weak_ptr<DX12R_CommandList> commandList);

	void ExecuteCommandList(weak_ptr<DX12R_CommandList> commandList);
	void ExecuteWaitingCommandLists();

	shared_ptr<DX12R_CommandList> GetCommandList();
	shared_ptr<DX12R_CommandQueue> GetCommandQueue();

	DX12S_DeviceContext* GetDeviceContext();

	DX12Interface* GetDX12Interface();
private:
	void CreateCommandAllocator();
	shared_ptr<DX12R_CommandList> CreateCommandList();
private:
	DX12S_DeviceContext* m_deviceContext;
	shared_ptr<DX12R_CommandQueue> m_commandQueue;

	std::vector<shared_ptr<DX12R_CommandAllocator>> m_inactiveAllocators;
	std::vector<shared_ptr<DX12R_CommandAllocator>> m_waitingAllocators;

	std::vector<shared_ptr<DX12R_CommandList>> m_inactiveCommandLists;
	std::vector<shared_ptr<DX12R_CommandList>> m_waitingCommandLists;

	D3D12_COMMAND_LIST_TYPE m_type;
};