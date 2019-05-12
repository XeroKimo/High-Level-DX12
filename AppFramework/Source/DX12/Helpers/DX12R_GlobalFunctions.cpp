//#include "DX12/Helpers/DX12R_GlobalFunctions.h"
//
//using namespace DX12Interface;
////using namespace D3D12Renderer;
//
//void DX12R_DeviceSync()
//{
//	if (fence->GetCompletedValue() < fenceValue)
//	{
//		fence->SetEventOnCompletion(fenceValue, fenceEvent);
//		WaitForSingleObject(fenceEvent, INFINITE);
//	}
//
//	fenceValue++;
//}
//
//void DX12R_BeginRender()
//{
//	//DX12R_DeviceSync();
//	//GetSwapChain()->GetFrameBuffer(frameIndex)->Reset();
//
//	shared_ptr<DX12R_CommandList> commandList = dxrCommandQueue->GetCommandList();
//
//	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxrSwapChain->GetRTVHandle();
//	rtvHandle.Offset(frameIndex, dxrSwapChain->GetRTVDescriptorSize());
//	const float clearColor[] = { 0.0f,0.2f,0.4f,1.0f };
//
//	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dxrSwapChain->GetFrameBufferResource(frameIndex).Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
//	commandList->OMSetRenderTargets(1, &rtvHandle,FALSE,nullptr);
//	commandList->ClearRenderTargetView(rtvHandle, clearColor,0,nullptr);
//	commandList->RSSetScissorRects(1, &scissorRect);
//	commandList->RSSetViewports(1, &viewport);
//
//	commandList->CloseForSubmit();
//}
//
//void DX12R_EndRender()
//{
//	shared_ptr<DX12R_CommandList> commandList = dxrCommandQueue->GetCommandList();
//	commandList->CloseForSubmit();
//	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(dxrSwapChain->GetFrameBufferResource(frameIndex).Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
//
//	dxrCommandQueue->ExecuteCommandLists();
//	//dxrCommandQueue->Signal(fence.Get(), fenceValue);
//
//	dxrSwapChain->Present(0, 0, dxrCommandQueue.get());
//}
//
//DX12R_SwapChain* GetSwapChain()
//{
//	return dxrSwapChain.get();
//}
