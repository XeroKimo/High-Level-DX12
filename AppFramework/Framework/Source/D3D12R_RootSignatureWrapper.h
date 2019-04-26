#pragma once
#include "D3D12R.h"

struct D3D12R_RSPInfo
{
	D3D12_ROOT_PARAMETER_TYPE parameterType;
	unsigned int numberOfValues = 0;	//if parameter type = 32bit constants, this value indicates how many 32bit constants
										//if the parameter type = root descriptor table, this value indicates how many table ranges there are
#ifdef DDEBUG
    unique_ptr<unsigned int[]> shaderRegister = nullptr;
#endif // DEBUG

	unique_ptr<unsigned int[]> numberOfDescriptor = nullptr;
	unique_ptr<D3D12_DESCRIPTOR_RANGE_TYPE[]> rangeType = nullptr;

	~D3D12R_RSPInfo()
	{

	}
};

struct D3D12R_RSPInput
{
	unsigned int inputSize;
	unsigned int bufferOffset;

	void AlignBuffer()
	{
		bufferOffset = (inputSize + 255) & ~255;
	}
	~D3D12R_RSPInput()
	{

	}
};

struct D3D12R_RootSignatureWrapper
{
    ComPtr<ID3D12RootSignature> rootSignature;
	unsigned int parameterCount = 0;
	unique_ptr<D3D12R_RSPInfo[]> parameterInfo;

	~D3D12R_RootSignatureWrapper()
	{

	}
};