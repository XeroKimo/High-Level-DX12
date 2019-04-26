#include "D3D12R_PipelineStateObject.h"

using namespace D3D12Renderer;

D3D12R_PipelineStateObject::D3D12R_PipelineStateObject(ComPtr<ID3D12PipelineState> pipeline, weak_ptr<D3D12R_RootSignatureWrapper> rootSignature, unsigned int* inputSizeArray)
{
	m_pipelineState = pipeline;
	m_rootSignatureParams = rootSignature.lock();
    m_uniqueIDCount = 0;
    SetupInputs(inputSizeArray);
    GenerateInputHeaps();
}

D3D12R_PipelineStateObject::~D3D12R_PipelineStateObject()
{
	m_pipelineState.Get()->Release();
	m_rootSignatureParams.reset();
}

void D3D12R_PipelineStateObject::UpdateParameter(unsigned int uniqueID, unsigned int parameterID, void * data, unsigned int dataSize)
{
    //memCpy(RSPInput->address[i] + uniqueID*RSPInput->BufferOffset[i],data, dataSize);
}

void D3D12R_PipelineStateObject::SetupInputs(unsigned int* size)
{
    for (unsigned int i = 0; i < m_rootSignatureParams->parameterCount; i++)
    {
        parameterInputs.get()[i].inputSize = size[i];
        parameterInputs.get()[i].AlignBuffer();
    }
}

void D3D12R_PipelineStateObject::SetForRender()
{
	D3D12R_UsingPipeline(m_pipelineState.Get(), m_rootSignatureParams->rootSignature.Get());
}

unsigned int D3D12R_PipelineStateObject::GenerateUniqueInputID()
{
    unsigned int ID = m_uniqueIDCount;
    m_uniqueIDCount++;

    for (unsigned int i = 0; i < m_rootSignatureParams->parameterCount; i++)
    {
        if (m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
            continue;        
        else if (m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_CBV ||
                m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_SRV ||
                m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_UAV)
        {

        }
    }

    return ID;
}

void D3D12R_PipelineStateObject::GenerateInputHeaps()
{
    for (unsigned int i = 0; i < m_rootSignatureParams->parameterCount; i++)
    {
        if (m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
            continue;
        else if (m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_CBV ||
                m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_SRV ||
                m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_UAV)
        {
            m_descriptorHeaps.push_back(nullptr);
            m_descriptorHeaps[m_descriptorHeaps.size() - 1] = make_unique<D3D12R_DescriptorHeapWrapper>();
            ComPtr<ID3D12Resource> resource = D3D12R_CreateDescriptor(D3D12_HEAP_TYPE_DEFAULT, DescriptorBufferUse_ConstantBuffer, 64*1024*1024);
            m_descriptorHeaps[m_descriptorHeaps.size() - 1]->descriptors.push_back(resource);
        }
    }
}
