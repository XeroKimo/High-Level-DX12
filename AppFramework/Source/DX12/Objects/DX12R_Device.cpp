#include "DX12/Objects/DX12R_Device.h"

DX12R_Device::DX12R_Device()
{
	m_activeVRAM = 0;
	m_nodeMask = 0;
	m_totalVRAM = 0;
}

DX12R_Device::~DX12R_Device()
{
}

bool DX12R_Device::Initialize(D3D_FEATURE_LEVEL featureLevel)
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
			// Don't look for a software device
			adapterIndex++;
			continue;
		}

		// Find a D3D12 compatible device ( minimum feature level for D3D12 is feature level 11_0 )
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
		{
			m_nodeMask = m_device->GetNodeCount();
			m_activeVRAM = 0;
			m_totalVRAM = (desc.DedicatedVideoMemory);
			return true;
		}

		adapterIndex++;
	}
		return false;
}

bool DX12R_Device::Initialize(UINT adapterIndex, D3D_FEATURE_LEVEL featureLevel)
{
	ComPtr<IDXGIFactory4> dxgiFactory;
	CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	ComPtr<IDXGIAdapter1> adapter;
	if (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) == DXGI_ERROR_NOT_FOUND)
		return false;

	DXGI_ADAPTER_DESC1 desc;
	adapter->GetDesc1(&desc);

	if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), featureLevel, IID_PPV_ARGS(&m_device))))
	{
		m_nodeMask = m_device->GetNodeCount();
		m_activeVRAM = 0;
		m_totalVRAM = (desc.DedicatedVideoMemory);
		return true;
	}
	return false;
}

ComPtr<ID3D12Device> DX12R_Device::GetDevice()
{
	return m_device;
}

UINT DX12R_Device::GetNodeMask()
{
	return m_nodeMask;
}
