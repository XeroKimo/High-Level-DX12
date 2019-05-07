#pragma once
#include "DX12/DirectX12.h"

class DX12R_Device;
class DX12R_CommandQueue;

class DX12R_Fence
{
public:

	DX12R_Fence();
	bool Inititalize(DX12R_Device* device, D3D12_FENCE_FLAGS flags = D3D12_FENCE_FLAG_NONE);

	void SignalGPU(DX12R_CommandQueue* queue);
	void SignalCPU();

	void SyncDevices(DWORD milliseconds = INFINITE);

private:
	ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	UINT64 m_fenceValue;
};