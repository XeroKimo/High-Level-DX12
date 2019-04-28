#pragma once
#include "D3D12R.h"


class D3D12R_Device
{
public:
	D3D12R_Device();
	bool Initialize(D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0);

	ComPtr<ID3D12Resource> CreateDescriptor();

private:
	ComPtr<ID3D12Device> m_device;
	UINT m_activeVRAM;
	UINT m_totalVRAM;
};