#pragma once

#include <Windows.h>
#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <memory>
#include <array>
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

#include "Helpers/DX12R_GlobalFunctions.h"

#include "Objects/DX12R_CommandAllocator.h"
#include "Objects/DX12R_CommandList.h"
#include "Objects/DX12R_CommandQueue.h"
#include "Objects/DX12R_Device.h"
#include "Objects/DX12R_SwapChain.h"

#include "DX12R_Interface.h"