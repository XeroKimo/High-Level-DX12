#pragma once
#include "D3D12Renderer.h"

class D3D12R_PipelineStateObject
{
public:
	D3D12R_PipelineStateObject(ComPtr<ID3D12PipelineState> pipeline, ComPtr<ID3D12RootSignature> signature, weak_ptr<D3D12R_RSP> rootSignatureParams);
	~D3D12R_PipelineStateObject();
	void SetForRender();
private:
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	shared_ptr<D3D12R_RSP> m_rootSignatureParams;
    //std::vector<ComPtr<ID3D12DescriptorHeap>> m_DescriptorHeaps;
    int m_uniqueIDCount;
};
