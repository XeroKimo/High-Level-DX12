#include "DX12/Objects/DX12R_RootSignature.h"
#include "DX12R.h"

DX12R_RootSignature::DX12R_RootSignature()
{
}

void DX12R_RootSignature::InitRootConstant(D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues, UINT shaderRegister, UINT registerSpace)
{
#ifdef DEBUG
	if (m_signatureLimit - num32BitValues < 0)
		return;
	m_signatureLimit -= num32BitValues;
#endif // DEBUG

	D3D12_ROOT_PARAMETER param;
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	param.Constants = CD3DX12_ROOT_CONSTANTS(num32BitValues, shaderRegister, registerSpace);
	param.ShaderVisibility = visibility;

	m_rootParameters.push_back(param);
}

void DX12R_RootSignature::InitRootDescriptor(D3D12_SHADER_VISIBILITY visibility, D3D12_ROOT_PARAMETER_TYPE type, UINT shaderRegister, UINT registerSpace)
{
#ifdef DEBUG
	if (m_signatureLimit - 2 < 0)
		return;
	m_signatureLimit -= 2;
#endif // DEBUG

	D3D12_ROOT_PARAMETER param;
	param.ParameterType = type;
	param.Descriptor = CD3DX12_ROOT_DESCRIPTOR(shaderRegister, registerSpace);
	param.ShaderVisibility = visibility;

	m_rootParameters.push_back(param);
}

void DX12R_RootSignature::InitRootDescriptorTable(D3D12_SHADER_VISIBILITY visibility, UINT numDescriptorRanges, D3D12_DESCRIPTOR_RANGE* descriptorRanges)
{
#ifdef DEBUG
	if (m_signatureLimit - 1 < 0)
		return;
	m_signatureLimit -= 1;
#endif // DEBUG

	D3D12_ROOT_PARAMETER param;
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param.DescriptorTable = CD3DX12_ROOT_DESCRIPTOR_TABLE(numDescriptorRanges,descriptorRanges);
	param.ShaderVisibility = visibility;

	m_rootParameters.push_back(param);
}

void DX12R_RootSignature::InitStaticSampler(UINT shaderRegister)
{
	CD3DX12_STATIC_SAMPLER_DESC desc;
	desc.Init(shaderRegister);
	m_staticSamplers.push_back(desc);
}

void DX12R_RootSignature::InitStaticSampler(D3D12_STATIC_SAMPLER_DESC staticSampler)
{
	m_staticSamplers.push_back(staticSampler);
}

void DX12R_RootSignature::InitializeRootSignature(DX12R_Device* device, D3D12_ROOT_SIGNATURE_FLAGS flags)
{
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = static_cast<UINT>(m_rootParameters.size());
	rootSignatureDesc.pParameters = m_rootParameters.data();
	rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(m_staticSamplers.size());
	rootSignatureDesc.pStaticSamplers = m_staticSamplers.data();
	rootSignatureDesc.Flags = flags;

	m_rootParameters.shrink_to_fit();
	m_staticSamplers.shrink_to_fit();

	ComPtr<ID3DBlob> blob;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);
	HRESULT hr = device->CreateRootSignature(blob.Get(), IID_PPV_ARGS(&m_rootSignature));
	int i = 0;
}

ComPtr<ID3D12RootSignature> DX12R_RootSignature::GetRootSignature()
{
	return m_rootSignature;
}
