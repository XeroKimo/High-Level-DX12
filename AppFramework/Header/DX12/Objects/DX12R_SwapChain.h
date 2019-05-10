#pragma once
#include "DX12/DirectX12.h"

class DX12R_CommandQueue;
class DX12R_Device;
struct DX12R_FrameBuffer;

class DX12R_SwapChain
{
public:
	DX12R_SwapChain();
	
	bool Initialize(DX12R_Device* device, ID3D12CommandQueue* commandQueue, HWND windowHandle, DXGI_SWAP_CHAIN_DESC1* swapChainDesc, DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullScreenDesc = nullptr, IDXGIOutput* restrictOutputTo = nullptr);

	HRESULT Present(UINT syncInterval, UINT flags, DX12R_CommandQueue* queue);
	HRESULT ResizeBuffers(UINT width, UINT height);
	HRESULT SetFullScreenState(bool fullscreen);

	ComPtr<IDXGISwapChain3> GetSwapChain();
	ComPtr<ID3D12Resource> GetFrameBufferResource(UINT bufferIndex);
	DX12R_FrameBuffer* GetFrameBuffer(UINT bufferIndex);

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTVHandle();
	UINT GetRTVDescriptorSize();
	UINT GetCurrentBackBufferIndex();

private:
	void CreateRenderTargets(DX12R_Device* device);

private:
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
	std::vector<ComPtr<ID3D12Resource>> m_renderTargets;
	std::vector<unique_ptr<DX12R_FrameBuffer>> m_frameBuffers;

	UINT m_rtvDescriptorSize;
};