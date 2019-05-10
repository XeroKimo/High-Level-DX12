#pragma once
#include "DX12/DirectX12.h"

class DX12R_Device;

class DX12R_RootSignature
{
public:
	DX12R_RootSignature();

	void InitRootConstant(D3D12_SHADER_VISIBILITY visibility, UINT num32BitValues, UINT shaderRegister, UINT registerSpace = 0);
	void InitRootDescriptor(D3D12_SHADER_VISIBILITY visibility, D3D12_ROOT_PARAMETER_TYPE type, UINT shaderRegister, UINT registerSpace);
	void InitRootDescriptorTable(D3D12_SHADER_VISIBILITY visibility, UINT numDescriptorRanges, D3D12_DESCRIPTOR_RANGE* descriptorRanges);
	void InitStaticSampler(UINT shaderRegister);
	void InitStaticSampler(D3D12_STATIC_SAMPLER_DESC staticSampler);

	void InitializeRootSignature(DX12R_Device* device, D3D12_ROOT_SIGNATURE_FLAGS flags);
	ComPtr<ID3D12RootSignature> GetRootSignature();
private:
	ComPtr<ID3D12RootSignature> m_rootSignature;
	std::vector<D3D12_ROOT_PARAMETER> m_rootParameters;
	std::vector<D3D12_STATIC_SAMPLER_DESC> m_staticSamplers;

#ifdef DEBUG
	UINT m_signatureLimit = 64;
#endif // DEBUG

};