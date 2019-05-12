#include "DX12/Systems/DX12S_DeviceManager.h"
#include "DX12R.h"

DX12S_DeviceManager::DX12S_DeviceManager()
{
}

bool DX12S_DeviceManager::InitDevices(DX12Interface* dx12Interface, UINT numDevices, D3D_FEATURE_LEVEL featureLevel, bool allowSoftwareDevices)
{
	m_interface = dx12Interface;

	int initializedDevices = 0;
	for (UINT i = 0; i < numDevices; i++)
	{
		shared_ptr<DX12S_DeviceContext> device = make_shared<DX12S_DeviceContext>();
		if (!device->Initialize(this, featureLevel, allowSoftwareDevices))
			break;

		initializedDevices++;
		m_managedDevices.push_back(device);
	}
	if (initializedDevices == 0)
		return false;

	if (initializedDevices == 1)
		m_interface->singleGPUMode = true;
	else
		m_interface->singleGPUMode = false;

	return true;
}

shared_ptr<DX12S_DeviceContext> DX12S_DeviceManager::GetDeviceContext(UINT nodeMask)
{
	if (m_interface->singleGPUMode)
		return m_managedDevices[0];
	else
	{
		int bitShifts = 1;
		while (!(nodeMask & 1))
		{
			nodeMask = nodeMask >> 1;
			bitShifts++;
		}
		return m_managedDevices[bitShifts];
	}
}

DX12Interface* DX12S_DeviceManager::GetDX12Interface()
{
	return m_interface;
}
