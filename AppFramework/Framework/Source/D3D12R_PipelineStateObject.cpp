#include "D3D12R_PipelineStateObject.h"

using namespace D3D12Renderer;

D3D12R_PipelineStateObject::D3D12R_PipelineStateObject(ComPtr<ID3D12PipelineState> pipeline, weak_ptr<D3D12R_RootSignatureWrapper> rootSignature, UINT* constBufferSizes)
{
	m_pipelineState = pipeline;
	m_rootSignatureParams = rootSignature.lock();
    m_uniqueIDCount = 0;
	int constBufferCount = 0;
	for (UINT i = 0; i < m_rootSignatureParams->parameterCount; i++)
	{
		if (m_rootSignatureParams->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_CBV)
		{
			m_cbInputs.push_back(nullptr);
			m_cbInputs[i] = make_unique<RSPConstBufferInput>();
			m_cbInputs[i]->inHeap = false;
			constBufferCount++;
		}
		else if (m_rootSignatureParams->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			for (UINT v = 0; v < m_rootSignatureParams->parameterInfo.get()[i].numberOfValues; v++)
			{
				if (m_rootSignatureParams->parameterInfo.get()[i].rangeType[v] == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
				{
					for (UINT x = 0; x < m_rootSignatureParams->parameterInfo.get()[i].numberOfDescriptor[v]; x++)
					{
						m_cbInputs.push_back(nullptr);
						m_cbInputs[i + x] = make_unique<RSPConstBufferInput>();
						m_cbInputs[i]->inHeap = true;
						constBufferCount++;
					}
				}
			}
		}
	}
	m_cbInputs.shrink_to_fit();

    SetupInputs(constBufferSizes, constBufferCount);
    GenerateInputHeaps();
}

D3D12R_PipelineStateObject::~D3D12R_PipelineStateObject()
{
	m_pipelineState.Get()->Release();
	m_rootSignatureParams.reset();
}

void D3D12R_PipelineStateObject::Set32BitConstants(UINT RootParameterIndex, UINT Num32BitValuesToSet, const void* pSrcData, UINT DestOffsetIn32BitValues)
{
	commandList->SetGraphicsRoot32BitConstants(RootParameterIndex, Num32BitValuesToSet, pSrcData, DestOffsetIn32BitValues);
}

void D3D12R_PipelineStateObject::UpdateConstBuffer(UINT uniqueID, UINT descriptorID, void * data)
{
    memcpy(m_cbInputs[descriptorID]->GPUAddress[frameIndex] + uniqueID * m_cbInputs[descriptorID]->bufferOffset, data, m_cbInputs[descriptorID]->inputSize);
}

void D3D12R_PipelineStateObject::SetupInputs(UINT* size, int countOfConstBuffers)
{
    for (int i = 0; i < countOfConstBuffers; i++)
    {
        m_cbInputs[i]->inputSize = size[i];
        m_cbInputs[i]->AlignBuffer();
    }
}

void D3D12R_PipelineStateObject::SetForRender()
{
	D3D12R_UsingPipeline(m_pipelineState.Get(), m_rootSignatureParams->rootSignature.Get());
}

void D3D12R_PipelineStateObject::Render(UINT uniqueID)
{
	for (UINT i = 0; i < m_rootSignatureParams->parameterCount; i++)
	{
		if (m_rootSignatureParams->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			commandList->SetGraphicsRootDescriptorTable(i, m_descriptorHeaps[i]->heap->GetGPUDescriptorHandleForHeapStart());
		else if (m_rootSignatureParams->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_CBV)
			commandList->SetGraphicsRootConstantBufferView(i, m_descriptorHeaps[i]->descriptors[0]->GetGPUVirtualAddress() + uniqueID * m_cbInputs[i]->bufferOffset);
		else if (m_rootSignatureParams->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_UAV)
			commandList->SetGraphicsRootUnorderedAccessView(i, m_descriptorHeaps[i]->descriptors[0]->GetGPUVirtualAddress() + uniqueID * m_cbInputs[i]->bufferOffset);
		else if (m_rootSignatureParams->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_SRV)
			commandList->SetGraphicsRootShaderResourceView(i, m_descriptorHeaps[i]->descriptors[0]->GetGPUVirtualAddress() + uniqueID * m_cbInputs[i]->bufferOffset);
	}
}

UINT D3D12R_PipelineStateObject::GenerateUniqueInputID()
{
    UINT ID = m_uniqueIDCount;
    m_uniqueIDCount++;
    return ID;
}

void D3D12R_PipelineStateObject::GenerateInputHeaps()
{
    for (UINT i = 0, notHeapCounter = 0; i < m_rootSignatureParams->parameterCount; i++)
    {
        if (m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
            continue;
        else 
        {
			m_descriptorHeaps.push_back(nullptr);
			m_descriptorHeaps[m_descriptorHeaps.size() - 1] = make_unique<D3D12R_DescriptorHeapWrapper>();		

			if (m_rootSignatureParams->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
			{
				D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
				UINT totalNumDescriptors = 0;
				for (UINT v = 0; v < m_rootSignatureParams->parameterInfo.get()[i].numberOfValues; v++)
				{
					totalNumDescriptors += m_rootSignatureParams->parameterInfo.get()[i].numberOfDescriptor[v];
				}
				heapDesc.NumDescriptors = totalNumDescriptors;
				heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
				heapDesc.Type = (m_rootSignatureParams->parameterInfo.get()[i].rangeType[0] != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER) 
					? D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV : D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
				heapDesc.NodeMask = 0;

				graphicsDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptorHeaps[m_descriptorHeaps.size() - 1]->heap));

				for (UINT v = 0, y=0; v < totalNumDescriptors; v++)
				{
					for (int x = 0; x < frameBufferCount; x++)
					{
						ComPtr<ID3D12Resource> resource = nullptr;
						m_descriptorHeaps[m_descriptorHeaps.size() - 1]->descriptors.push_back(resource);
					}
				}

				int descOffset = 0;
				for (UINT v = 0, inHeapCounter = 0; v < m_rootSignatureParams->parameterInfo.get()[i].numberOfValues; v++)	//loop for every descriptor range
				{
					if (m_rootSignatureParams->parameterInfo.get()[i].rangeType[v] == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)	//if range == cbv
					{
						for (UINT z = 0; z < m_rootSignatureParams->parameterInfo.get()[i].numberOfDescriptor[v] && inHeapCounter < m_cbInputs.size(); inHeapCounter++)	//loop until we filled up all the cbv
						{
							if (m_cbInputs[inHeapCounter]->inHeap == true)
							{
								for (int x = 0; x < frameBufferCount; x++)
								{
									m_cbInputs[inHeapCounter]->cbDescriptor[x] = D3D12R_CreateDescriptor(D3D12_HEAP_TYPE_UPLOAD, DescriptorBufferUse_CBV_SRV_UAV_SAMPLER, 64 * 1024);
									m_descriptorHeaps[m_descriptorHeaps.size() - 1]->descriptors[descOffset + x];

									CD3DX12_RANGE readRange(0, 0);
									m_cbInputs[inHeapCounter]->cbDescriptor[x]->Map(0, &readRange, reinterpret_cast<void**>(m_cbInputs[inHeapCounter]->GPUAddress[x]));
								}
								z++;
							}
						}
					}
					descOffset += m_rootSignatureParams->parameterInfo.get()[i].numberOfDescriptor[v] * frameBufferCount;
				}
			}
			else
			{
				for (int x = 0; x < frameBufferCount; x++)
					m_descriptorHeaps[m_descriptorHeaps.size() - 1]->descriptors.push_back(nullptr);

				if (m_rootSignatureParams->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_CBV)
				{
					for (; ;)	//loop until we find a constant buffer that's not part of a heap desc
					{
						if (m_cbInputs[notHeapCounter]->inHeap == false)
						{
							for (int x = 0; x < frameBufferCount; x++)
							{
								m_cbInputs[notHeapCounter]->cbDescriptor[x] = D3D12R_CreateDescriptor(D3D12_HEAP_TYPE_UPLOAD, DescriptorBufferUse_CBV_SRV_UAV_SAMPLER, 64 * 1024 -1,D3D12_RESOURCE_STATE_GENERIC_READ);
								m_descriptorHeaps[m_descriptorHeaps.size() - 1]->descriptors[x] = m_cbInputs[notHeapCounter]->cbDescriptor[x];

								CD3DX12_RANGE readRange(0, 0);
								m_cbInputs[notHeapCounter]->cbDescriptor[x]->Map(0, &readRange, reinterpret_cast<void**>(&m_cbInputs[notHeapCounter]->GPUAddress[x]));
							}

							notHeapCounter++;
							break;
						}
						notHeapCounter++;
					}
				}
			}
        }
    }
}

//if (m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_CBV ||
//	m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_SRV ||
//	m_rootSignatureParams.get()->parameterInfo.get()[i].parameterType == D3D12_ROOT_PARAMETER_TYPE_UAV)
