#pragma once
#include "DX12/DirectX12.h"

class DX12S_CommandSystem;

class DX12R_CommandQueue;
class DX12R_Device;

class DX12R_CommandAllocator :public  std::enable_shared_from_this<DX12R_CommandAllocator>
{
public:
	DX12R_CommandAllocator();
	bool Initialize(DX12S_CommandSystem* commandSystem, DX12R_Device* device,  D3D12_COMMAND_LIST_TYPE type);

	void ReEnlist();
	HRESULT Reset();

	ComPtr<ID3D12CommandAllocator> GetAllocator();
private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	DX12S_CommandSystem* m_commandSystem;
};