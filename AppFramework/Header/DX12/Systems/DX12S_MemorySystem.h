#pragma once
#include "DX12/DirectX12.h"

class DX12Interface;

class DX12S_MemorySystem
{
public:
	DX12S_MemorySystem();
	bool Initialize(DX12Interface* dx12Interface);

	DX12Interface* GetDX12Interface();
private:

};