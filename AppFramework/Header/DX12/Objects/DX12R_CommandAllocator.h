#pragma once
#include "DX12/DX12R.h"

class DX12R_CommandAllocator
{
public:
	DX12R_CommandAllocator();
	bool Initialize(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE type);

	HRESULT Reset();

	ComPtr<ID3D12CommandAllocator> GetAllocator();

private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
};