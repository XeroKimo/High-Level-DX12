#pragma once
#include "DX12/DirectX12.h"

class DX12Interface;

class DX12S_DeviceContext;

class DX12S_DeviceManager
{
public:
	DX12S_DeviceManager();
	
	bool InitDevices(DX12Interface* dx12Interface, UINT numDevices, D3D_FEATURE_LEVEL featureLevel, bool allowSoftwareDevices);

	shared_ptr<DX12S_DeviceContext> GetDeviceContext(UINT nodeMask = 1);

	DX12Interface* GetDX12Interface();
private:
	std::vector<shared_ptr<DX12S_DeviceContext>> m_managedDevices;
	DX12Interface* m_interface;
};