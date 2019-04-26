#pragma once
#include "D3D12R.h"
struct D3D12R_DescriptorHeapWrapper;

class D3D12R_PipelineStateObject
{
public:
	D3D12R_PipelineStateObject(ComPtr<ID3D12PipelineState> pipeline, weak_ptr<D3D12R_RootSignatureWrapper> rootSignatureParams, unsigned int* inputSizeArray);
	~D3D12R_PipelineStateObject();

    void SetForRender();
    void UpdateParameter(unsigned int uniqueID, unsigned int parameterID, void* data, unsigned int dataSize);

    unsigned int GenerateUniqueInputID();
private:
    void SetupInputs(unsigned int* size);
    void GenerateInputHeaps();   
private:
	ComPtr<ID3D12PipelineState> m_pipelineState;
	shared_ptr<D3D12R_RootSignatureWrapper> m_rootSignatureParams;
    unique_ptr<D3D12R_RSPInput[]> parameterInputs;
    std::vector<unique_ptr<D3D12R_DescriptorHeapWrapper>> m_descriptorHeaps;
    unsigned int m_uniqueIDCount;
};
