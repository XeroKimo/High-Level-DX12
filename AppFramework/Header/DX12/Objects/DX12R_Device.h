#pragma once
#include "DX12/DirectX12.h"

class DX12R_Device
{
public:
	DX12R_Device();
	~DX12R_Device();
	bool Initialize(D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0);
	bool Initialize(UINT adapterIndex, D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0);

	bool CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type, const IID& iid, void** commandAllocator);
	bool CreateCommandList(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* commandAllocator, const IID& riid, void** ppCommandList);
	bool CreateCommandQueue(D3D12_COMMAND_QUEUE_DESC* description, const IID& iid, void** commandQueue);
	bool CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_DESC* description, const IID& iid, void** heap);
	void CreateRenderTargetView(ID3D12Resource* resource, D3D12_RENDER_TARGET_VIEW_DESC* description, D3D12_CPU_DESCRIPTOR_HANDLE handle);

	UINT GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE type);

	ComPtr<ID3D12Device> GetDevice();
	UINT GetNodeMask();
private:
	ComPtr<ID3D12Device> m_device;
	UINT m_nodeMask;
	size_t m_activeVRAM;
	size_t m_totalVRAM;
};