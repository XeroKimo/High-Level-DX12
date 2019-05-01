#include "DX12/Objects/DX12R_SwapChain.h"

using namespace DX12Interface;

DX12R_SwapChain::DX12R_SwapChain()
{
	m_swapChain = nullptr;
	m_rtvDescriptorHeap = nullptr;
	m_rtvDescriptorSize = 0;
}

bool DX12R_SwapChain::Initialize(ID3D12Device* device, ID3D12CommandQueue* commandQueue, HWND windowHandle, DXGI_SWAP_CHAIN_DESC1* swapChainDesc, DXGI_SWAP_CHAIN_FULLSCREEN_DESC* fullScreenDesc, IDXGIOutput* restrictOutputTo)
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
	HRESULT hr;


	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = frameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = device->GetNodeCount();

	if (FAILED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvDescriptorHeap))))
		return false;

	m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = GetRTVHandle();
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = swapChainDesc->Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.Texture2D.PlaneSlice = 0;

	m_renderTargets.resize(frameBufferCount);
	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));
		if (FAILED(hr))
			return false;

		device->CreateRenderTargetView(m_renderTargets[i].Get(), &rtvDesc, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	return true;
}

HRESULT DX12R_SwapChain::Present(UINT syncInterval, UINT flags)
{
	HRESULT hr= m_swapChain->Present(syncInterval, flags);
	frameIndex = dxrSwapChain->GetCurrentBackBufferIndex();
	return hr;
}

ComPtr<IDXGISwapChain3> DX12R_SwapChain::GetSwapChain()
{
	return m_swapChain;
}

ComPtr<ID3D12Resource> DX12R_SwapChain::GetFrameBuffer(UINT bufferIndex)
{
	return m_renderTargets[bufferIndex];
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
