#pragma once
#include "D3D12R.h"

struct D3D12R_RSPInfo
{
	D3D12_ROOT_PARAMETER_TYPE parameterType;
	UINT numberOfValues = 0;	//if parameter type = 32bit constants, this value indicates how many 32bit constants
										//if the parameter type = root descriptor table, this value indicates how many table ranges there are
#ifdef DDEBUG
    unique_ptr<UINT[]> shaderRegister = nullptr;
#endif // DEBUG

	unique_ptr<UINT[]> numberOfDescriptor = nullptr;
	unique_ptr<D3D12_DESCRIPTOR_RANGE_TYPE[]> rangeType = nullptr;

	~D3D12R_RSPInfo()
	{

	}
};

struct D3D12R_RootSignatureWrapper
{
    ComPtr<ID3D12RootSignature> rootSignature;
	UINT parameterCount = 0;
	unique_ptr<D3D12R_RSPInfo[]> parameterInfo;

	~D3D12R_RootSignatureWrapper()
	{

	}
};