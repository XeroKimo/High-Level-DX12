#pragma once
#include "D3D12R.h"
struct D3D12R_RootSignatureWrapper;

class D3D12R_SignatureParametersHelper
{
	//Use this class to create root signature parameters
	//Delete the object when no longer in use
	//Use the D3D12R_RSP to store any key information that was made in this class
public:

	void CreateRootConstant(int Num32BitValues, D3D12_SHADER_VISIBILITY shaderVisibility);
	void CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE ParameterType, D3D12_SHADER_VISIBILITY shaderVisibility);
	void CreateRootDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE* RangeType, int* NumDescriptorsInTable, int NumDescriptorRangeType, D3D12_SHADER_VISIBILITY shaderVisibility);
	char GetParameterCount() { return static_cast<char>(rootParameters.size()); }

	D3D12_ROOT_PARAMETER* GetRootParameters()
	{
		return &rootParameters[0];
	}

	weak_ptr<D3D12R_RootSignatureWrapper> GenerateRootSignature(std::string signatureName);
private:
	char freeSpace = 64;
	unsigned long constantBufferCount = 0;
	unsigned long shaderResourceCount = 0;
	unsigned long unorderedAccessCount = 0;
	unsigned long samplerCount = 0;
	std::vector<shared_ptr<D3D12_ROOT_CONSTANTS>> rootConstants;
	std::vector<shared_ptr<D3D12_ROOT_DESCRIPTOR>> rootDescriptors;
	std::vector<shared_ptr<D3D12_DESCRIPTOR_RANGE>> descriptorRanges;
	std::vector<shared_ptr<D3D12_ROOT_DESCRIPTOR_TABLE>> rootTables;

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;
};