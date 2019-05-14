#include "DX12/DX12R_Interface.h"
#include "DX12R.h"

DX12Interface* DX12Interface::Instance()
{
	static DX12Interface dx12Interface;
	return &dx12Interface;
}

bool DX12Interface::Initialize(UINT windowWidth, UINT windowHeight, bool isFullscreen, HWND windowHandle, UINT amountOfDevice, bool allowSoftwareDevices)
{
	deviceManager = make_unique<DX12S_DeviceManager>();

	if (!deviceManager->InitDevices(this, amountOfDevice, D3D_FEATURE_LEVEL_11_0, allowSoftwareDevices))
		return false;

	swapChain = make_unique<DX12R_SwapChain>();

	shared_ptr<DX12S_DeviceContext> mainDevice = deviceManager->GetDeviceContext();
	shared_ptr<DX12R_CommandQueue> directCommandQueue = mainDevice->GetCommandSystem(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetCommandQueue();

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;	//How many samples per pixel are we drawing
	
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = windowWidth;
	swapChainDesc.Height = windowHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.BufferCount = swapChain->GetFrameBufferCount(); // number of buffers we have
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present

	if (isFullscreen)
	{
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc = {};
		if (!swapChain->Initialize(this, mainDevice.get(), windowHandle, &swapChainDesc, &fullscreenDesc))
			return false;
	}
	else
	{
		if (!swapChain->Initialize(this, mainDevice.get(), windowHandle, &swapChainDesc))
			return false;
	}

	return true;
}

void DX12Interface::Shutdown()
{
}

void DX12Interface::SendAllocatorsToCurrentFrame(std::vector<shared_ptr<DX12R_CommandAllocator>> commandAllocators)
{
	swapChain->GetFrameBuffer(swapChain->GetCurrentBackBufferIndex())->AttachAllocators(commandAllocators);
}

DX12R_Device* DX12Interface::GetDevice(UINT nodeMask)
{
	return deviceManager->GetDeviceContext(nodeMask)->GetDevice();
}

shared_ptr<DX12R_CommandList> DX12Interface::GetCommandList(D3D12_COMMAND_LIST_TYPE type, UINT nodeMask)
{
	return deviceManager->GetDeviceContext(nodeMask)->GetCommandSystem(type)->GetCommandList();
}

shared_ptr<DX12R_CommandQueue> DX12Interface::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type, UINT nodeMask)
{
	return deviceManager->GetDeviceContext(nodeMask)->GetCommandSystem(type)->GetCommandQueue();
}

UINT DX12Interface::GetFrameIndex()
{
	return swapChain->GetCurrentBackBufferIndex();
}

UINT DX12Interface::GetFrameBufferCount()
{
	return 0;
}

DX12Interface::DX12Interface()
{
}

DX12Interface::~DX12Interface()
{
}
