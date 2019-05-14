#pragma once
#include "DX12/DirectX12.h"

class DX12Interface;


class DX12S_DeviceContext;

class DX12R_CommandQueue;
class DX12R_Device;
struct DX12R_FrameBuffer;

class DX12R_SwapChain
{
public:
	DX12R_SwapChain();
	
	bool Initialize(DX12Interface* dx12Interface, DX12S_DeviceContext* deviceContext, HWND windowHandle, DXGI_SWAP_CHAIN_DESC1* swapChainDesc, DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullScreenDesc = nullptr, IDXGIOutput* restrictOutputTo = nullptr);

	HRESULT Present(UINT syncInterval, UINT flags, DX12R_CommandQueue* queue);
	HRESULT ResizeBuffers(UINT width, UINT height);
	HRESULT SetFullScreenState(bool fullscreen);

	ComPtr<IDXGISwapChain3> GetSwapChain();
	ComPtr<ID3D12Resource> GetFrameBufferResource(UINT bufferIndex);
	DX12R_FrameBuffer* GetFrameBuffer(UINT bufferIndex);

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTVHandle();
	UINT GetRTVDescriptorSize();
	UINT GetCurrentBackBufferIndex();
	ComPtr<ID3D12Resource> GetCurrentBackBufferResource();
	UINT GetFrameBufferCount();

private:
	void CreateRenderTargets(DX12R_Device* device);

private:
	ComPtr<IDXGISwapChain3> swapChain;
	ComPtr<ID3D12DescriptorHeap> m_rtvDescriptorHeap;
	std::vector<unique_ptr<DX12R_FrameBuffer>> m_frameBuffers;

	UINT m_rtvDescriptorSize;
	UINT m_frameBufferCount = 2;
	UINT m_frameIndex;

	DX12Interface* m_interface;
};