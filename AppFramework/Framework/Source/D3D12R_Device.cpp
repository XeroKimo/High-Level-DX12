#include "stdafx.h"
#include "D3D12R_Device.h"

D3D12R_Device::D3D12R_Device()
{

}

bool D3D12R_Device::Initialize(D3D_FEATURE_LEVEL featureLevel)
{
	ComPtr<IDXGIFactory4> dxgiFactory;
	CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	ComPtr<IDXGIAdapter1> adapter;
	UINT adapterIndex = 0;
	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			adapterIndex++;
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&m_device))))
		{
			m_activeVRAM = 0;
			m_totalVRAM = static_cast<UINT>(desc.DedicatedVideoMemory);
			return true;
		}
	}
	return false;
}
