#pragma once
#include "DX12/DirectX12.h"

class DX12Interface;

class DX12R_CommandList;
class DX12R_Device;

class DX12S_CommandSystem;
class DX12S_DeviceManager;

class DX12S_DeviceContext : public std::enable_shared_from_this<DX12S_DeviceContext>
{
public:
	DX12S_DeviceContext();
	
	bool Initialize(DX12S_DeviceManager* deviceManager, D3D_FEATURE_LEVEL level, bool allowSoftwareDevices);

	DX12R_Device* GetDevice();
	DX12S_CommandSystem* GetCommandSystem(D3D12_COMMAND_LIST_TYPE type);

	DX12Interface* GetDX12Interface();
private:
	unique_ptr<DX12R_Device> m_device;
	unique_ptr<DX12S_CommandSystem> m_directCommandModule;
	unique_ptr<DX12S_CommandSystem> m_copyCommandModule;
	unique_ptr<DX12S_CommandSystem> m_computeCommandModule;

	DX12S_DeviceManager* m_deviceManager;
};