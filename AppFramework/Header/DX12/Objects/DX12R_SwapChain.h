#pragma once
#include "DX12/DX12R.h"

class DX12R_SwapChain
{
public:
	DX12R_SwapChain();
	
	bool Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, HWND windowHandle, DXGI_SWAP_CHAIN_DESC1* swapChainDesc, DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullScreenDesc = nullptr, IDXGIOutput* restrictOutputTo = nullptr);

	HRESULT Present(UINT syncInterval, UINT flags);

	ComPtr<IDXGISwapChain3> GetSwapChain();
	ComPtr<ID3D12Resource> GetFrameBuffer(UINT bufferIndex);

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTVHandle();
	UINT GetRTVDescriptorSize();
	UINT GetCurrentBackBufferIndex();
private:
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
	std::vector<ComPtr<ID3D12Resource>> m_renderTargets;

	UINT m_rtvDescriptorSize;
};