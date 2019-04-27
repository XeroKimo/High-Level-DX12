#include "D3D12R_SignatureParametersHelper.h"

using namespace D3D12Renderer;

void D3D12R_SignatureParametersHelper::CreateRootConstant(int Num32BitValues, D3D12_SHADER_VISIBILITY shaderVisibility)
{
	if (freeSpace == 0)
		return;

	shared_ptr<D3D12_ROOT_CONSTANTS> constant = make_shared<D3D12_ROOT_CONSTANTS>();
	constant->Num32BitValues = Num32BitValues;
	constant->RegisterSpace = 0;
	constant->ShaderRegister = constantBufferCount;
	constantBufferCount++;
	rootConstants.push_back(constant);

	D3D12_ROOT_PARAMETER rootParameter;
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameter.Constants = *constant;
	rootParameter.ShaderVisibility = shaderVisibility;

	rootParameters.push_back(rootParameter);
	freeSpace -= Num32BitValues;
}

void D3D12R_SignatureParametersHelper::CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE ParameterType, D3D12_SHADER_VISIBILITY shaderVisibility)
{
	if (freeSpace < 2)
		return;

	shared_ptr<D3D12_ROOT_DESCRIPTOR> descriptor = make_shared <D3D12_ROOT_DESCRIPTOR>();
	descriptor->RegisterSpace = 0;

	if (ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV)
	{
		descriptor->ShaderRegister = constantBufferCount;
		constantBufferCount++;
	}
	else if (ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV)
	{
		descriptor->ShaderRegister = unorderedAccessCount;
		unorderedAccessCount++;
	}
	else if (ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV)
	{
		descriptor->ShaderRegister = shaderResourceCount;
		shaderResourceCount++;
	}
	rootDescriptors.push_back(descriptor);

	D3D12_ROOT_PARAMETER rootParameter;
	rootParameter.ParameterType = ParameterType;
	rootParameter.Descriptor = *descriptor;
	rootParameter.ShaderVisibility = shaderVisibility;

	rootParameters.push_back(rootParameter);
	freeSpace -= 2;
}

void D3D12R_SignatureParametersHelper::CreateRootDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE* RangeType, int* NumDescriptorsInTable, int NumDescriptorRangeType, D3D12_SHADER_VISIBILITY shaderVisibility)
{
	std::vector<shared_ptr<D3D12_DESCRIPTOR_RANGE>> descriptorViewRange;
	std::vector<shared_ptr<D3D12_DESCRIPTOR_RANGE>> descriptorSamplerRange;
	if (freeSpace == 0)
		return;

	for (int i = 0; i < NumDescriptorRangeType; i++)
	{
		if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_CBV ||
			RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_UAV ||
			RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
		{
			shared_ptr<D3D12_DESCRIPTOR_RANGE> range = make_shared <D3D12_DESCRIPTOR_RANGE>();
			range->RangeType = RangeType[i];
			range->NumDescriptors = NumDescriptorsInTable[i];
			range->RegisterSpace = 0;
			range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			
			if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_CBV)
			{
				range->BaseShaderRegister = constantBufferCount;
				constantBufferCount += NumDescriptorsInTable[i];
			}
			else if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
			{
				range->BaseShaderRegister = unorderedAccessCount;
				unorderedAccessCount += NumDescriptorsInTable[i];
			}
			else if (RangeType[i] == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
			{
				range->BaseShaderRegister = shaderResourceCount;
				shaderResourceCount += NumDescriptorsInTable[i];
			}
			descriptorViewRange.push_back(range);
			descriptorRanges.push_back(range);
		}
		else
		{
			shared_ptr<D3D12_DESCRIPTOR_RANGE> range = make_shared <D3D12_DESCRIPTOR_RANGE>();
			range->RangeType = RangeType[i];
			range->NumDescriptors = NumDescriptorsInTable[i];
			range->RegisterSpace = 0;
			range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
			range->BaseShaderRegister = samplerCount;

			descriptorSamplerRange.push_back(range);
			descriptorRanges.push_back(range);
		}
	}

	if (descriptorViewRange.size() > 0)
	{
		shared_ptr<D3D12_ROOT_DESCRIPTOR_TABLE> descriptorViewTable = make_shared<D3D12_ROOT_DESCRIPTOR_TABLE>();
		descriptorViewTable->NumDescriptorRanges = static_cast<UINT>(descriptorViewRange.size());
		descriptorViewTable->pDescriptorRanges = descriptorViewRange[0].get();
		rootTables.push_back(descriptorViewTable);

		D3D12_ROOT_PARAMETER rootParameter;
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable = *descriptorViewTable;
		rootParameter.ShaderVisibility = shaderVisibility;
		rootParameters.push_back(rootParameter);

		freeSpace--;
	}
	if (descriptorSamplerRange.size() > 0)
	{
		shared_ptr<D3D12_ROOT_DESCRIPTOR_TABLE> descriptorSamplerTable = make_shared<D3D12_ROOT_DESCRIPTOR_TABLE>();
		descriptorSamplerTable->NumDescriptorRanges = static_cast<UINT>(descriptorSamplerRange.size());
		descriptorSamplerTable->pDescriptorRanges = descriptorSamplerRange[0].get();
		rootTables.push_back(descriptorSamplerTable);

		D3D12_ROOT_PARAMETER rootParameter;
		rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameter.DescriptorTable = *descriptorSamplerTable;
		rootParameter.ShaderVisibility = shaderVisibility;
		rootParameters.push_back(rootParameter);

		freeSpace--;
	}
}

weak_ptr<D3D12R_RootSignatureWrapper> D3D12R_SignatureParametersHelper::GenerateRootSignatureWrapper(std::string signatureName)
{
	shared_ptr<D3D12R_RootSignatureWrapper> info = make_shared<D3D12R_RootSignatureWrapper>();
    info->rootSignature = D3D12R_CreateRootSignature(GetRootParameters(), GetParameterCount());
	info->parameterCount = static_cast<UINT>(rootParameters.size());
	info->parameterInfo = make_unique<D3D12R_RSPInfo[]>(rootParameters.size());
	for (int i = 0; i < rootParameters.size(); i++)
	{
		info->parameterInfo.get()[i].parameterType = rootParameters[i].ParameterType;
		if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS)
		{
#ifdef DDEBUG
            info->parameterInfo.get()[i].shaderRegister = make_unique<unsigned int[]>(1);
            info->parameterInfo.get()[i].shaderRegister.get()[0] = rootParameters[i].Constants.ShaderRegister;
#endif // DDEBUG
			info->parameterInfo.get()[i].numberOfValues = rootParameters[i].Constants.Num32BitValues;
		}
		else if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV ||
			rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV ||
			rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV)
		{
#ifdef DDEBUG
            info->parameterInfo.get()[i].shaderRegister = make_unique<unsigned int[]>(1);
			info->parameterInfo.get()[i].shaderRegister.get()[0] = rootParameters[i].Descriptor.ShaderRegister;
#endif
			info->parameterInfo.get()[i].numberOfValues = 1;
		}
		else if (rootParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			info->parameterInfo.get()[i].numberOfValues = rootParameters[i].DescriptorTable.NumDescriptorRanges;
#ifdef DDEBUG
            info->parameterInfo.get()[i].shaderRegister = make_unique<unsigned int[]>(rootParameters[i].DescriptorTable.NumDescriptorRanges);
#endif
			info->parameterInfo.get()[i].numberOfDescriptor = make_unique<unsigned int[]>(rootParameters[i].DescriptorTable.NumDescriptorRanges);
			info->parameterInfo.get()[i].rangeType = make_unique<D3D12_DESCRIPTOR_RANGE_TYPE[]>(rootParameters[i].DescriptorTable.NumDescriptorRanges);
			for (int v = 0; v < static_cast<int>(rootParameters[i].DescriptorTable.NumDescriptorRanges); v++)
			{
#ifdef DDEBUG
				info->parameterInfo.get()[i].shaderRegister.get()[v] = rootParameters[i].DescriptorTable.pDescriptorRanges[v].BaseShaderRegister;
#endif
				info->parameterInfo.get()[i].numberOfDescriptor.get()[v] = rootParameters[i].DescriptorTable.pDescriptorRanges[v].NumDescriptors;
				info->parameterInfo.get()[i].rangeType.get()[v] = rootParameters[i].DescriptorTable.pDescriptorRanges[v].RangeType;
			}
		}
	}

	ownedRootSignatureParams[signatureName] = info;

	return info;
}
