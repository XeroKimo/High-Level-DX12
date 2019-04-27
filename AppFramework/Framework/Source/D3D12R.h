#pragma once

#ifndef DDEBUG
#define DDEBUG
#endif // !DDEBUG

#include <Windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <memory>
#include <vector>
#include <map>
#include <string>

#include "d3dx12.h"

using Microsoft::WRL::ComPtr;
using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;

#include "D3D12Renderer.h"
#include "D3D12R_SignatureParametersHelper.h"
#include "D3D12R_RootSignatureWrapper.h"
#include "D3D12R_PipelineStateObject.h"
#include "D3D12R_DescriptorHeapWrapper.h"

