#pragma once
#include "DX12/DirectX12.h"

class DX12R_Device;
class DX12R_CommandQueue;
class DX12R_CommandAllocator;

class DX12R_CommandList :public  std::enable_shared_from_this<DX12R_CommandList>
{
public:
	DX12R_CommandList();
	bool Initialize(weak_ptr<DX12R_Device> device, D3D12_COMMAND_LIST_TYPE type, weak_ptr < DX12R_CommandAllocator> commandAllocator, weak_ptr< DX12R_CommandQueue> commandQueue);

	//Things that are required to be called after Reset
	void ClearRenderTargetView(D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView, const float* colorRGBA, UINT numRects = 0, D3D12_RECT* rect = nullptr);
	void OMSetRenderTargets(UINT numRenderTargetDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE* pRenderTargetDescirptors, BOOL rtsSingleHandleToDescriptorRange = FALSE, const D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilDescriptor = nullptr);
	void RSSetScissorRects(UINT numRects, const D3D12_RECT* rects);
	void RSSetViewports(UINT numViewports, const  D3D12_VIEWPORT* viewports);

	//Things required for drawing
	void SetGraphicsRootSignature(ID3D12RootSignature* signature);
	void SetGraphics32BitConstants(UINT rootParamIndex, UINT num32BitValuesToSet, const void* srcData, UINT DestOffsetIn32BitValues = 0);
	void SetGraphicsConstantBufferView(UINT rootParamIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
	void SetGraphicsShaderResourceView(UINT rootParamIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
	void SetGraphicsUnorderedAccessView(UINT rootParamIndex, D3D12_GPU_VIRTUAL_ADDRESS bufferLocation);
	void SetGraphicsDescriptorTable(UINT rootParamIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor);

	//General use
	HRESULT Close();
	void CloseForSubmit();
	HRESULT Reset(weak_ptr<DX12R_CommandAllocator> commandAllocator, ID3D12PipelineState* pipelineState = nullptr);

	void SetPipelineState(ID3D12PipelineState* pipelineState);
	void ResourceBarrier(int numBarraiers, const  D3D12_RESOURCE_BARRIER* barriers);

	ComPtr<ID3D12GraphicsCommandList> GetCommandList();

	weak_ptr<DX12R_CommandAllocator> GetCommandAllocator();

private:
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	weak_ptr<DX12R_CommandQueue> m_commandQueue;
	shared_ptr<DX12R_CommandAllocator> m_commandAllocator;
};