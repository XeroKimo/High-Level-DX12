#pragma once
#include "DirectX12.h"

class DX12R_CommandAllocator;
class DX12R_CommandList;
class DX12R_CommandQueue;
class DX12R_Device;
class DX12R_SwapChain;

//namespace DX12Interface
//{
//	extern const int frameBufferCount;
//	extern const int threadCount;
//
//	extern shared_ptr<DX12R_Device> dxrDevice;
//	extern shared_ptr<DX12R_CommandQueue> dxrCommandQueue;									// Responsible for sending command lists to the device for execution
//	extern unique_ptr<DX12R_SwapChain> dxrSwapChain;										// Swap chain used to switch between render targets
//
//	extern ComPtr<ID3D12Fence> fence;														// Utilized for syncing the GPU and CPU
//	extern HANDLE fenceEvent;															// A Handle to our fence, to know when the gpu is unlocked
//	extern UINT64 fenceValue;															// This value is incremented each frame. Each fence has its own value
//
//	extern D3D12_VIEWPORT viewport;														// The amount of pixels we will be rendering to
//	extern D3D12_RECT scissorRect;														// How much of the viewport we will see when rendering
//
//	extern unsigned int frameIndex;														// The current buffer we are currently on
//
//	extern ComPtr<ID3D12RootSignature> defaultSignature;		//A default root signature with no params
//	extern bool SingleGPUMode;
//}
//
//bool DX12R_Initialize(int windowWidth, int windowHeight, bool fullscreen,  HWND windowHandle);
//void DX12R_Shutdown();

class DX12S_DeviceManager;
class DX12S_MemorySystem;
class DX12R_SwapChain;

class DX12Interface
{
public:
	static DX12Interface* Instance();
	bool Initialize(UINT windowWidth, UINT windowHeight, bool isFullscreen, HWND windowHandle, UINT amountOfDevice = 1, bool allowSoftwareDevices = false);
	void Shutdown();

	void SendAllocatorsToCurrentFrame(std::vector<shared_ptr<DX12R_CommandAllocator>> commandAllocators);

	UINT GetFrameIndex();
	UINT GetFrameBufferCount();
private:	
	DX12Interface();
	~DX12Interface();

public:
	unique_ptr<DX12S_DeviceManager> deviceManager;
	unique_ptr<DX12S_MemorySystem> memorySystem;
	unique_ptr<DX12R_SwapChain> swapChain;

	bool singleGPUMode;
};