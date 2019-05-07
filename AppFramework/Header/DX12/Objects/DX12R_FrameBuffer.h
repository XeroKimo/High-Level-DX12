#pragma once
#include "DX12/DirectX12.h"

class DX12R_Device;
class DX12R_SwapChain;
class DX12R_CommandAllocator;
class DX12R_Fence;

struct DX12R_FrameBuffer
{
public:
	DX12R_FrameBuffer();

	void Initialize(DX12R_Device* device, DX12R_SwapChain* swapChain, int frameIndex, D3D12_RENDER_TARGET_VIEW_DESC* description, D3D12_CPU_DESCRIPTOR_HANDLE handle);

	void Reset();

	ComPtr<ID3D12Resource> m_frameResource;
	std::vector<shared_ptr<DX12R_CommandAllocator>> m_activeAllocators;
	unique_ptr<DX12R_Fence> m_fence;
};