#include "DX12/Objects/DX12R_SwapChain.h"
#include "DX12R.h"

using namespace DX12Interface;

DX12R_SwapChain::DX12R_SwapChain()
{
	m_swapChain = nullptr;
	m_rtvDescriptorHeap = nullptr;
	m_rtvDescriptorSize = 0;
}

bool DX12R_SwapChain::Initialize(DX12R_Device* device, ID3D12CommandQueue* commandQueue, HWND windowHandle, DXGI_SWAP_CHAIN_DESC1* swapChainDesc, DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullScreenDesc, IDXGIOutput* restrictOutputTo)
{
	ComPtr<IDXGIFactory4> dxgiFactory;
	CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	ComPtr<IDXGISwapChain1> tempSwapChain = nullptr;

	dxgiFactory->CreateSwapChainForHwnd(
		commandQueue,
		windowHandle,
		swapChainDesc,
		fullScreenDesc,
		restrictOutputTo,
		&tempSwapChain
	);

	tempSwapChain.As(&m_swapChain);
	if (!m_swapChain)
		return false;

	frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = device->GetNodeMask();

	if (!device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap)))
		return false;

	m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRTVHandle();
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = swapChainDesc->Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	m_frameBuffers.resize(frameBufferCount);
	for (int i = 0; i < frameBufferCount; i++)
	{
		m_frameBuffers[i] = make_unique<DX12R_FrameBuffer>();
		m_frameBuffers[i]->Initialize(device, this, i, &rtvDesc, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	return true;
}

HRESULT DX12R_SwapChain::Present(UINT syncInterval, UINT flags)
{
	HRESULT hr = m_swapChain->Present(syncInterval, flags);
	frameIndex = dxrSwapChain->GetCurrentBackBufferIndex();
	return hr;
}

ComPtr<IDXGISwapChain3> DX12R_SwapChain::GetSwapChain()
{
	return m_swapChain;
}

ComPtr<ID3D12Resource> DX12R_SwapChain::GetFrameBufferResource(UINT bufferIndex)
{
	return m_frameBuffers[bufferIndex]->m_frameResource;
}

DX12R_FrameBuffer* DX12R_SwapChain::GetFrameBuffer(UINT bufferIndex)
{
	return m_frameBuffers[bufferIndex].get();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DX12R_SwapChain::GetRTVHandle()
{
	return static_cast<CD3DX12_CPU_DESCRIPTOR_HANDLE>(m_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

UINT DX12R_SwapChain::GetRTVDescriptorSize()
{
	return m_rtvDescriptorSize;
}

UINT DX12R_SwapChain::GetCurrentBackBufferIndex()
{
	return m_swapChain->GetCurrentBackBufferIndex();
}
