#include "DX12/DX12R.h"

using namespace DX12Interface;
//using namespace D3D12Renderer;

void DX12R_DeviceSync()
{

	if (fence->GetCompletedValue() < fenceValue)
	{
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	fenceValue++;
}

void DX12R_BeginRender()
{
	DX12R_DeviceSync();
	dxrCommandAllocator[frameIndex]->Reset();
	dxrCommandList->GetCommandList()->Reset(dxrCommandAllocator[frameIndex]->GetAllocator().Get(),nullptr);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxrSwapChain->GetRTVHandle();
	rtvHandle.Offset(frameIndex, dxrSwapChain->GetRTVDescriptorSize());
	const float clearColor[] = { 0.0f,0.2f,0.4f,1.0f };

	dxrCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dxrSwapChain->GetFrameBuffer(frameIndex).Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	dxrCommandList->OMSetRenderTargets(1, &rtvHandle,FALSE,nullptr);
	dxrCommandList->ClearRenderTargetView(rtvHandle, clearColor,0,nullptr);
	dxrCommandList->RSSetScissorRects(1, &scissorRect);
	dxrCommandList->RSSetViewports(1, &viewport);
}

void DX12R_EndRender()
{
	dxrCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dxrSwapChain->GetFrameBuffer(frameIndex).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	dxrCommandList->Close();

	ID3D12CommandList* ppCommandList[] = { dxrCommandList->GetCommandList().Get() };
	dxrCommandQueue->ExecuteCommandLists(1, ppCommandList);
	dxrCommandQueue->Signal(fence.Get(), fenceValue);

	dxrSwapChain->Present(0, 0);
}
