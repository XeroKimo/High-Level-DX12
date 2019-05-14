#pragma once
#include "DirectX12.h"

class DX12R_CommandAllocator;
class DX12R_CommandList;
class DX12R_CommandQueue;
class DX12R_Device;
class DX12R_SwapChain;

class DX12S_DeviceManager;

class DX12Interface
{
public:
	static DX12Interface* Instance();
	bool Initialize(UINT windowWidth, UINT windowHeight, bool isFullscreen, HWND windowHandle, UINT amountOfDevice = 1, bool allowSoftwareDevices = false);
	void Shutdown();

	void SendAllocatorsToCurrentFrame(std::vector<shared_ptr<DX12R_CommandAllocator>> commandAllocators);

	DX12R_Device* GetDevice(UINT nodeMask = 1);
	shared_ptr<DX12R_CommandList> GetCommandList(D3D12_COMMAND_LIST_TYPE type, UINT nodeMask = 1);
	shared_ptr<DX12R_CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type, UINT nodeMask = 1);

	UINT GetFrameIndex();
	UINT GetFrameBufferCount();
private:	
	DX12Interface();
	~DX12Interface();

	unique_ptr<DX12S_DeviceManager> deviceManager;
public:
	unique_ptr<DX12R_SwapChain> swapChain;

	bool singleGPUMode;
};