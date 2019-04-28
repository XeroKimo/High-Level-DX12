#pragma once
#include "D3D12R.h"
struct D3D12R_DescriptorHeapWrapper;

class D3D12R_PipelineStateObject
{
public:
	D3D12R_PipelineStateObject(ComPtr<ID3D12PipelineState> pipeline, weak_ptr<D3D12R_RootSignatureWrapper> rootSignatureParams, unsigned int* constBufferSizes);
	~D3D12R_PipelineStateObject();

    void SetForRender();
	void Render(unsigned int uniqueID);

    void UpdateConstBuffer(unsigned int uniqueID, unsigned int descriptorID, void* data);

    unsigned int GenerateUniqueInputID();
private:
    void SetupInputs(unsigned int* size, int countOfConstBuffers);
    void GenerateInputHeaps();
private:

	struct RSPConstBufferInput
	{
		ComPtr<ID3D12Resource> cbDescriptor[2];
		unsigned int inputSize;
		unsigned int bufferOffset;
		bool inHeap;
		UINT8* GPUAddress[2];

		void AlignBuffer()
		{
			bufferOffset = (inputSize + 255) & ~255;
		}
		~RSPConstBufferInput()
		{

		}
	};

	ComPtr<ID3D12PipelineState> m_pipelineState;
	shared_ptr<D3D12R_RootSignatureWrapper> m_rootSignatureParams;
    std::vector<unique_ptr<RSPConstBufferInput>> m_cbInputs;
	std::vector<unique_ptr<D3D12R_DescriptorHeapWrapper>> m_descriptorHeaps;
    unsigned int m_uniqueIDCount;
};
