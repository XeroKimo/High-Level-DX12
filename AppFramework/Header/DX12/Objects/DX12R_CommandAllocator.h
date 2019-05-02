#pragma once
#include "DX12/DirectX12.h"

class DX12R_CommandQueue;
class DX12R_Device;

class DX12R_CommandAllocator : std::enable_shared_from_this<DX12R_CommandAllocator>
{
public:
	DX12R_CommandAllocator();
	bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);
	bool Initialize(DX12R_Device* device, D3D12_COMMAND_LIST_TYPE type, weak_ptr<DX12R_CommandQueue> queue);

	void ReEnlist();
	HRESULT Reset();

	ComPtr<ID3D12CommandAllocator> GetAllocator();
	weak_ptr<DX12R_CommandQueue> GetCommandQueue();
private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	weak_ptr<DX12R_CommandQueue> m_commandQueue;
};