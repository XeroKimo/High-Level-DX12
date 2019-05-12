#include "DX12/Objects/DX12R_Fence.h"
#include "DX12R.h"

DX12R_Fence::DX12R_Fence()
{
	m_fenceValue = 0;
	m_fenceEvent = nullptr;
}

bool DX12R_Fence::Inititalize(DX12R_Device* device, D3D12_FENCE_FLAGS flags)
{
	if (FAILED(device->CreateFence(m_fenceValue,flags,IID_PPV_ARGS(&m_fence))))
		return false;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	
	if (m_fenceEvent == nullptr)
		return false;

	return true;
}

void DX12R_Fence::SignalCPU()
{
	m_fence->Signal(m_fenceValue);
}

void DX12R_Fence::StallGPU(DX12R_CommandQueue* commandQueue)
{
	commandQueue->StallGPU(m_fence.Get(), m_fenceValue);
}

void DX12R_Fence::SignalGPU(DX12R_CommandQueue* commandQueue)
{
	commandQueue->SignalGPU(m_fence.Get(), m_fenceValue);
}

void DX12R_Fence::StallCPU(DWORD milliseconds)
{

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, milliseconds);
	}

	m_fenceValue++;
}

UINT64 DX12R_Fence::GetValue()
{
	return m_fenceValue;
}

void DX12R_Fence::SetValue(UINT64 value)
{
	m_fenceValue = value;
}
