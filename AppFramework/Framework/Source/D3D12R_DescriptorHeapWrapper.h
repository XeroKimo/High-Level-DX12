#pragma once
#include "D3D12R.h"
struct D3D12R_DescriptorHeapWrapper
{

    ComPtr<ID3D12DescriptorHeap> heap;
    std::vector<ComPtr<ID3D12Resource>> descriptors;

    void GenerateDescriptor(D3D12_HEAP_TYPE heapProperty, DescriptorBufferUse bufferUse, unsigned int bufferSize, D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON, LPCWSTR bufferName = nullptr)
    {

    }

    ~D3D12R_DescriptorHeapWrapper()
    {
        heap.ReleaseAndGetAddressOf();
        for (int i = 0; i < descriptors.size(); i++)
            descriptors[i].ReleaseAndGetAddressOf();
    }
};