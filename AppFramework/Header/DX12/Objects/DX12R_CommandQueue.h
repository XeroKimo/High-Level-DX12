#pragma once
#include "DX12/DirectX12.h"

class DX12S_CommandSystem;

class DX12R_Device;
class DX12R_CommandList;
class DX12R_CommandAllocator;

class DX12R_CommandQueue : public std::enable_shared_from_this<DX12R_CommandQueue>
{
public:
	DX12R_CommandQueue();
	bool Initialize(DX12S_CommandSystem* commandSystem, DX12R_Device* device, D3D12_COMMAND_LIST_TYPE type, D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE);

	void ExecuteCommandList(DX12R_CommandList* commandList);
	void ExecuteCommandLists(UINT numCommandLists, ID3D12CommandList* const* commandLists);
	HRESULT SignalGPU(ID3D12Fence* fence, UINT64 value);
	HRESULT StallGPU(ID3D12Fence* fence, UINT64 value);


	ComPtr<ID3D12CommandQueue> GetQueue();

private:
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	DX12S_CommandSystem* m_commandSystem;
	D3D12_COMMAND_LIST_TYPE m_type;


};