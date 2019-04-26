#include "stdafx.h"
#include "D3D12R_PipelineStateObject.h"

D3D12R_PipelineStateObject::D3D12R_PipelineStateObject(ComPtr<ID3D12PipelineState> pipeline, ComPtr<ID3D12RootSignature> signature, weak_ptr<D3D12R_RSP> rootSignatureParams)
{
	m_pipelineState = pipeline;
	m_rootSignature = signature;
	m_rootSignatureParams = rootSignatureParams.lock();
    m_uniqueIDCount = 0;
}

D3D12R_PipelineStateObject::~D3D12R_PipelineStateObject()
{
	m_pipelineState.Get()->Release();
	m_rootSignature.Get()->Release();
	m_rootSignatureParams.reset();
}

void D3D12R_PipelineStateObject::SetForRender()
{
	D3D12R_UsingPipeline(m_pipelineState.Get(), m_rootSignature.Get());
}
