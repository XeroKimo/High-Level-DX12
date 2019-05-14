#include "DX12/Objects/DX12R_Device.h"
#include "DX12R.h"

DX12R_Device::DX12R_Device()
{
	m_nodeMask = 0;
}

DX12R_Device::~DX12R_Device()
{
}

bool DX12R_Device::Initialize(DX12S_DeviceContext* deviceContext, D3D_FEATURE_LEVEL featureLevel, bool allowSoftwareDevices)
{
	m_deviceContext = deviceContext;
	ComPtr<IDXGIFactory4> dxgiFactory;
	CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	ComPtr<IDXGIAdapter1> baseAdapter;

	ComPtr<IDXGIAdapter3> adapter;
	UINT adapterIndex = 0;

	while (dxgiFactory->EnumAdapters1(adapterIndex, &baseAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		baseAdapter.As(&m_adapter);

		DXGI_ADAPTER_DESC1 desc;
		m_adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't look for a software device
			adapterIndex++;
			continue;
		}

		// Find a D3D12 compatible device ( minimum feature level for D3D12 is feature level 11_0 )
		if (SUCCEEDED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device))))
		{
			m_nodeMask = m_device->GetNodeCount();
			return true;
		}

		adapterIndex++;
	}
		return false;
}

HRESULT DX12R_Device::CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* commandAllocator, const IID& riid, void** ppCommandList)
{
	if (m_deviceContext->GetDX12Interface()->singleGPUMode)
		return m_device->CreateCommandList(0, type, commandAllocator, nullptr, riid, ppCommandList);
	else
		return m_device->CreateCommandList(m_nodeMask, type, commandAllocator, nullptr, riid, ppCommandList);
}

HRESULT DX12R_Device::CreateCommandQueue(D3D12_COMMAND_QUEUE_DESC* description, const IID& iid, void** commandQueue)
{
	return m_device->CreateCommandQueue(description, iid, commandQueue);
}

HRESULT DX12R_Device::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, const IID& iid, void** commandAllocator)
{
	return m_device->CreateCommandAllocator(type, iid, commandAllocator);
}

HRESULT DX12R_Device::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC* description, const IID& iid, void** heap)
{
	return m_device->CreateDescriptorHeap(description, iid, heap);
}

HRESULT DX12R_Device::CreateFence(UINT64 initialValue, D3D12_FENCE_FLAGS flags, const IID& iid, void** fence)
{
	return m_device->CreateFence(initialValue, flags, iid, fence);
}

void DX12R_Device::CreateRenderTargetView(ID3D12Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC* description, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
	m_device->CreateRenderTargetView(resource, description, handle);
}

HRESULT DX12R_Device::CreateRootSignature(ID3DBlob* blobWithRootSignature, const IID& iid, void** rootSignature)
{
	if (m_deviceContext->GetDX12Interface()->singleGPUMode)
		return m_device->CreateRootSignature(0,blobWithRootSignature->GetBufferPointer(),blobWithRootSignature->GetBufferSize(), iid,rootSignature);
	else
		return m_device->CreateRootSignature(m_nodeMask, blobWithRootSignature->GetBufferPointer(), blobWithRootSignature->GetBufferSize(), iid, rootSignature);
}

UINT DX12R_Device::GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
	return m_device->GetDescriptorHandleIncrementSize(type);
}

DXGI_QUERY_VIDEO_MEMORY_INFO DX12R_Device::GetMemoryInfo(DXGI_MEMORY_SEGMENT_GROUP group)
{
	DXGI_QUERY_VIDEO_MEMORY_INFO info;

	if (m_deviceContext->GetDX12Interface()->singleGPUMode)
		m_adapter->QueryVideoMemoryInfo(0, group, &info);
	else
		m_adapter->QueryVideoMemoryInfo(m_nodeMask, group, &info);
	return info;
}

ComPtr<ID3D12Device> DX12R_Device::GetDevice()
{
	return m_device;
}

UINT DX12R_Device::GetNodeMask()
{
	return (m_deviceContext->GetDX12Interface()->singleGPUMode) ? 0 : m_nodeMask;
}
