#pragma once
#include "DX12/DirectX12.h"

class DX12R_Device;
class DX12R_CommandList;
class DX12R_CommandAllocator;

class DX12R_CommandQueue : public std::enable_shared_from_this<DX12R_CommandQueue>
{
public:
	DX12R_CommandQueue();
	bool Initialize(weak_ptr<DX12R_Device> device, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE);

	void EnlistAllocator(weak_ptr<DX12R_CommandAllocator> allocator);
	void CloseList(weak_ptr<DX12R_CommandList> commandList);
	void ExecuteCommandLists();
	void ExecuteCommandLists(UINT numCommandLists, ID3D12CommandList* const* commandLists);
	HRESULT Signal(ID3D12Fence* fence, UINT64 value);
	HRESULT Wait(ID3D12Fence* fence, UINT64 value);

	shared_ptr<DX12R_CommandList> GetCommandList();
	ComPtr<ID3D12CommandQueue> GetQueue();

private:
	void CreateCommandAllocator();
	shared_ptr<DX12R_CommandList> CreateCommandList();
private:
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	weak_ptr<DX12R_Device> m_device;
	D3D12_COMMAND_LIST_TYPE m_type;

	std::vector<shared_ptr<DX12R_CommandAllocator>> m_inactiveAllocators;
	std::vector<shared_ptr<DX12R_CommandAllocator>> m_waitingAllocators;

	std::vector<shared_ptr<DX12R_CommandList>> m_inactiveCommandLists;
	std::vector<shared_ptr<DX12R_CommandList>> m_waitingCommandLists;
};