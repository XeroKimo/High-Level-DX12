#pragma once
#include "../DX12R.h"

class DX12R_Device
{
public:
	DX12R_Device();
	~DX12R_Device();
	bool Initialize(D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0);
	bool Initialize(UINT adapterIndex, D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0);

	ComPtr<ID3D12Device> GetDevice();
	UINT GetNodeMask();
private:
	ComPtr<ID3D12Device> m_device;
	UINT m_nodeMask;
	size_t m_activeVRAM;
	size_t m_totalVRAM;
};