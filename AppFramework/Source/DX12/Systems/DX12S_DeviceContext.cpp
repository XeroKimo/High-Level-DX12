#include "DX12/Systems/DX12S_DeviceContext.h"
#include "DX12R.h"

DX12S_DeviceContext::DX12S_DeviceContext()
{
}

bool DX12S_DeviceContext::Initialize(DX12S_DeviceManager* deviceManager, D3D_FEATURE_LEVEL level, bool allowSoftwareDevices)
{
	m_deviceManager = deviceManager;

	m_device = make_unique<DX12R_Device>();
	if (!m_device->Initialize(this, level,allowSoftwareDevices))
		return false;

	m_directCommandModule = make_unique<DX12S_CommandSystem>();
	if (!m_directCommandModule->Initialize(this, D3D12_COMMAND_LIST_TYPE_DIRECT))
		return false;

	m_copyCommandModule = make_unique<DX12S_CommandSystem>();
	if (!m_copyCommandModule->Initialize(this, D3D12_COMMAND_LIST_TYPE_COPY))
		return false;

	m_computeCommandModule = make_unique<DX12S_CommandSystem>();
	if (!m_computeCommandModule->Initialize(this, D3D12_COMMAND_LIST_TYPE_COMPUTE))
		return false;

	return true;
}

DX12R_Device* DX12S_DeviceContext::GetDevice()
{
	return m_device.get();
}

DX12S_CommandSystem* DX12S_DeviceContext::GetCommandSystem(D3D12_COMMAND_LIST_TYPE type)
{
	switch (type)
	{
	case D3D12_COMMAND_LIST_TYPE_DIRECT:
		return m_directCommandModule.get();
	case D3D12_COMMAND_LIST_TYPE_COPY:
		return m_copyCommandModule.get();
	case D3D12_COMMAND_LIST_TYPE_COMPUTE:
		return m_computeCommandModule.get();
	}
	return nullptr;
}

DX12Interface* DX12S_DeviceContext::GetDX12Interface()
{
	return m_deviceManager->GetDX12Interface();
}
