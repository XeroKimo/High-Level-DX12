#pragma once
#include "D3D12Renderer.h"

struct D3D12R_RSPInfo
{
	D3D12_ROOT_PARAMETER_TYPE parameterType;
	unsigned int numberOfValues = 0;	//if parameter type = 32bit constants, this value indicates how many 32bit constants
										//if the parameter type = root descriptor table, this value indicates how many table ranges there are
	unique_ptr<unsigned int[]> shaderRegister = nullptr;
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

struct D3D12R_RSP
{
	unsigned int parameterCount = 0;
	unique_ptr<D3D12R_RSPInfo[]> parameterInfo;
	unique_ptr<D3D12R_RSPInput[]> parameterInputs;

	void SetupInputs(unsigned int size, unsigned int parameterID)
	{
		parameterInputs.get()[parameterID].inputSize = size;
		parameterInputs.get()[parameterID].AlignBuffer();
	}
	void SetupInputs(unsigned int* size)
	{
		for (unsigned int i = 0; i < parameterCount; i++)
		{
			parameterInputs.get()[i].inputSize = size[i];
			parameterInputs.get()[i].AlignBuffer();
		}
	}

	~D3D12R_RSP()
	{

	}
};