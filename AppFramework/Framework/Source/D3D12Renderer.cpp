#include <string.h>
#include <assert.h>

#include "D3D12R.h"
namespace D3D12Renderer
{
	extern const int frameBufferCount = 2;
	extern const int threadCount = 1;

	ComPtr<ID3D12Device> graphicsDevice;												// The graphics device that will handle the rendering
	ComPtr<ID3D12CommandQueue> commandQueue;											// Responsible for sending command lists to the device for execution
	ComPtr<IDXGISwapChain3> swapChain;													// Swap chain used to switch between render targets
    unique_ptr<D3D12R_DescriptorHeapWrapper> rtvDescriptor;
    //ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;									// Descriptor for the render-targets
	//ComPtr<ID3D12Resource> renderTargets[frameBufferCount];							// Resources in the rtv Descriptor heap, number of render targets should equal the amount of render buffers
	ComPtr<ID3D12CommandAllocator> commandAllocators[frameBufferCount * threadCount];	// Have enough command allocators for each buffer * threads
	ComPtr<ID3D12GraphicsCommandList> commandList;										// Records commands for the device to execute
	ComPtr<ID3D12Fence>fence [frameBufferCount * threadCount];							// Utilized for syncing the GPU and CPU

	HANDLE fenceEvent;															// A Handle to our fence, to know when the gpu is unlocked
	UINT64 fenceValue[frameBufferCount * threadCount];							// This value is incremented each frame. Each fence has its own value

	D3D12_VIEWPORT viewport;													// The amount of pixels we will be rendering to
	D3D12_RECT scissorRect;														// How much of the viewport we will see when rendering

	unsigned int frameIndex;													// The current buffer we are currently on
	int rtvDescriptorSize;														// The size of the rtvDescriptorHeap on the device

	ComPtr<ID3D12RootSignature> defaultSignature;

	std::map<std::string, shared_ptr<D3D12R_RootSignatureWrapper>> ownedRootSignatureParams;
	std::map<std::string, ComPtr<ID3D12RootSignature>> ownedRootSignatures;
}

using namespace D3D12Renderer;

bool D3D12R_Initialize(int windowWidth, int windowHeight, HWND windowHandle)
{
	HRESULT hr;	// TODO: Delete all hr stuff when we know it works
	IDXGIFactory4* dxgiFactory;
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));
	if (FAILED(hr))
		return false;

#pragma region Graphics Device Creation

	IDXGIAdapter1* adapter;
	UINT adapterIndex = 0;
	bool adapterFound = false;

	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't look for a software device
			adapterIndex++;
			continue;
		}

		// Find a D3D12 compatible device ( minimum feature level for D3D12 is feature level 11_0 )
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&graphicsDevice));
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
		return false;

#pragma endregion
#pragma region Command Queue & Swap Chain Creation 
#pragma region Command Queue Creation

	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	hr = graphicsDevice->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue));
	if (FAILED(hr))
		return false;

#pragma endregion

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;	//How many samples per pixel are we drawing

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = windowWidth;
	swapChainDesc.Height = windowHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc = sampleDesc; // our multi-sampling description
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.BufferCount = frameBufferCount; // number of buffers we have
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present

	ComPtr<IDXGISwapChain1> tempSwapChain = nullptr;

	dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		windowHandle,
		&swapChainDesc,
		NULL,
		NULL,
		&tempSwapChain
	);

	tempSwapChain.As(&swapChain);
	frameIndex = swapChain->GetCurrentBackBufferIndex();

#pragma endregion
#pragma region Render-Targets View & Render Targets Creation 

    rtvDescriptor = make_unique<D3D12R_DescriptorHeapWrapper>();

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = frameBufferCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvHeapDesc.NodeMask = 0;

    hr = graphicsDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptor->heap));
    if (FAILED(hr))
        return false;

    rtvDescriptorSize = graphicsDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

#pragma region Render-Targets / Render Buffers Creation

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptor->heap->GetCPUDescriptorHandleForHeapStart());
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    rtvDesc.Texture2D.PlaneSlice = 0;

    rtvDescriptor->descriptors.resize(2);
    for (int i = 0; i < frameBufferCount; i++)
    {
        //Get a buffer in the swap chain
        hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&rtvDescriptor->descriptors[i]));
        if (FAILED(hr))
            return false;
        //Create a render-target in the rtvDescriptorHeap
        graphicsDevice->CreateRenderTargetView(rtvDescriptor->descriptors[i].Get(), &rtvDesc, rtvHandle);
        //Offset the value to store the next render-target
        rtvHandle.Offset(1, rtvDescriptorSize);
    }

#pragma endregion
//	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
//	rtvHeapDesc.NumDescriptors = frameBufferCount;
//	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
//	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
//	rtvHeapDesc.NodeMask = 0;
//
//	hr = graphicsDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
//	if (FAILED(hr))
//		return false;
//
//	rtvDescriptorSize = graphicsDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
//
//#pragma region Render-Targets / Render Buffers Creation
//
//	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
//
//	for (int i = 0; i < frameBufferCount; i++)
//	{
//		//Get a buffer in the swap chain
//		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));
//		if (FAILED(hr))
//			return false;
//		//Create a render-target in the rtvDescriptorHeap
//		graphicsDevice->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvHandle);
//		//Offset the value to store the next render-target
//		rtvHandle.Offset(1, rtvDescriptorSize);
//	}
//
//#pragma endregion
#pragma endregion
#pragma region Command Allocators & Command List Creation

	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = graphicsDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]));
		if (FAILED(hr))
			return false;
	}

	hr = graphicsDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[frameIndex].Get(), NULL, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
		return false;

#pragma endregion
#pragma region Fence & Fence Event Creation

	for (int i = 0; i < frameBufferCount; i++)
	{
		hr = graphicsDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence[i]));
		if (FAILED(hr))
			return false;
		fenceValue[i] = 0;
	}

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
		return false;


#pragma endregion
#pragma region Default Root Signature Creation

	defaultSignature = D3D12R_CreateRootSignature(nullptr, 0);

#pragma endregion
#pragma region Default Viewport & ScissorRect Creation

	viewport.TopLeftX = 0.f;
	viewport.TopLeftY = 0.f;
	viewport.Width = static_cast<FLOAT>(windowWidth);
	viewport.Height = static_cast<FLOAT>(windowHeight);
	viewport.MinDepth = 0.f;
	viewport.MaxDepth = 0.f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = static_cast<LONG>(windowWidth);
	scissorRect.bottom = static_cast<LONG>(windowHeight);

#pragma endregion

	dxgiFactory->Release();

	return true;
}

void D3D12R_Shutdown()
{
	D3D12R_WaitForPreviousFrame();

	// get swapchain out of full screen before exiting
	BOOL fs = false;
    if (SUCCEEDED(swapChain->GetFullscreenState(&fs, NULL)))
        swapChain->SetFullscreenState(false, NULL);
	graphicsDevice.Reset();
	swapChain.Reset();
	commandQueue.Reset();
    rtvDescriptor.reset();
	//rtvDescriptorHeap.Reset();
	commandList.Reset();
	defaultSignature.Reset();

	for (int i = 0; i < frameBufferCount; ++i)
	{
		//renderTargets[i].Reset();
		commandAllocators[i].Reset();
		fence[i].Reset();
	};

    ownedRootSignatureParams.clear();
    ownedRootSignatures.clear();
}

void D3D12R_BeginRender()
{
	D3D12R_WaitForPreviousFrame();

	commandAllocators[frameIndex]->Reset();
	commandList->Reset(commandAllocators[frameIndex].Get(), nullptr);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtvDescriptor->descriptors[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptor->heap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvDescriptorSize);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

}

void D3D12R_UsingPipeline(ID3D12PipelineState* pipelineState, ID3D12RootSignature* rootSignature)
{
	commandList->SetPipelineState(pipelineState);
	commandList->SetGraphicsRootSignature(rootSignature);
}

void D3D12R_UsingVertexBuffer(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews)
{
	commandList->IASetVertexBuffers(StartSlot, NumViews, pViews);
}

void D3D12R_UsingIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView)
{
	commandList->IASetIndexBuffer(pView);
}

void D3D12R_DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation, D3D12_PRIMITIVE_TOPOLOGY Topology)
{
	commandList->IASetPrimitiveTopology(Topology);
	commandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void D3D12R_DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, UINT BaseVertexLocation, UINT StartInstanceLocation, D3D12_PRIMITIVE_TOPOLOGY Topology)
{
	commandList->IASetPrimitiveTopology(Topology);
	commandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void D3D12R_SetViewport(float width, float height)
{
	viewport.Width = width;
	viewport.Height = height;

	scissorRect.right = static_cast<LONG>(width);
	scissorRect.bottom = static_cast<LONG>(height);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}

void D3D12R_EndRender()
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(rtvDescriptor->descriptors[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	D3D12R_DispatchCommandList();
	commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]);

	swapChain->Present(0, 0);
}

void D3D12R_DispatchCommandList()
{
	commandList->Close();
	ID3D12CommandList* ppCommandList[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);
}

void D3D12R_WaitForPreviousFrame()
{
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
	{
		fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	fenceValue[frameIndex]++;
}

ComPtr<ID3D12RootSignature> D3D12R_CreateRootSignature(D3D12_ROOT_PARAMETER* rootParamters, unsigned int numOfParameters)
{
	ComPtr<ID3D12RootSignature> signature;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.NumParameters = numOfParameters;
	rootSignatureDesc.pParameters = rootParamters;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> blob;
	D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);

	graphicsDevice->CreateRootSignature(0, blob.Get()->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&signature));

	return signature;
}

bool D3D12R_CreateShaderByteCode(D3D12R_ShaderWrapper* shader)
{
	HRESULT hr;

	char shaderTypeVersion[32];
	strcpy_s(shaderTypeVersion, 8, shader->shaderType);
	strncat_s(shaderTypeVersion, 8, shader->shaderVersion, 16);

	ID3DBlob* shaderBlob;
	ID3DBlob* errorBlob;
	hr = D3DCompileFromFile(
		shader->fileName,
		nullptr, nullptr,
		"main", shaderTypeVersion,
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0, &shaderBlob, &errorBlob
	);
	if (FAILED(hr))
	{
		OutputDebugStringA((char*)(errorBlob->GetBufferPointer()));
		assert(false);
		return false;
	}

	shader->shaderByteCode.pShaderBytecode = shaderBlob->GetBufferPointer();
	shader->shaderByteCode.BytecodeLength = shaderBlob->GetBufferSize();

	return true;
}

ComPtr<ID3D12PipelineState>  D3D12R_CreatePipelineState(ID3D12RootSignature* rootSignature, D3D12_INPUT_ELEMENT_DESC* inputLayout, unsigned int numOfElements, D3D12R_ShaderWrapper** arrayOfShaders, unsigned int numOfShaders)
{
	ComPtr<ID3D12PipelineState> pipelineState;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {};
	inputLayoutDesc.NumElements = numOfElements;
	inputLayoutDesc.pInputElementDescs = inputLayout;

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1; //multisample count, 1 = 1 sample

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {}; // a structure to define a pso
	psoDesc.InputLayout = inputLayoutDesc; // the structure describing our input layout
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE; // type of topology we are drawing
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // format of the render target
	psoDesc.SampleDesc = sampleDesc; // must be the same sample description as the swapchain and depth/stencil buffer
	psoDesc.SampleMask = 0xffffffff; // sample mask has to do with multi-sampling. 0xffffffff means point sampling is done
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT); // a default rasterizer state.
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT); // a default blend state.
	psoDesc.NumRenderTargets = 1; // we are only binding one render target

	if (rootSignature)
		psoDesc.pRootSignature = rootSignature; // the root signature that describes the input data this pso needs
	else
		psoDesc.pRootSignature = defaultSignature.Get();

	for (unsigned int i = 0; i < numOfShaders; i++)
	{
		if (arrayOfShaders[i]->shaderType == SHADER_VERTEX)
		{
			psoDesc.VS = arrayOfShaders[i]->shaderByteCode; // structure describing where to find the vertex shader bytecode and how large it is
			continue;									   
		}		
		if (arrayOfShaders[i]->shaderType == SHADER_PIXEL)
		{
			psoDesc.PS = arrayOfShaders[i]->shaderByteCode; // same as VS but for pixel shader
			continue;
		}
	}

    graphicsDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));

	return pipelineState;
}

unique_ptr<D3D12R_PrimitiveResource>  D3D12R_CreateVertexBuffer(void* vertices, unsigned int vertexCount, unsigned int sizeOfVertex)
{
	unique_ptr<D3D12R_PrimitiveResource> vertexBuffer = make_unique<D3D12R_PrimitiveResource>(D3D12R_PrimitiveResource::PrimitiveType_Vertex);

	int vBufferSize = sizeOfVertex * vertexCount;

    vertexBuffer->pResource = D3D12R_CreateDescriptor(D3D12_HEAP_TYPE_DEFAULT, DescriptorBufferUse_Generic, vBufferSize, D3D12_RESOURCE_STATE_COPY_DEST, L"Vertex Buffer Resource Heap");
    vertexBuffer->pUpload = D3D12R_CreateDescriptor(D3D12_HEAP_TYPE_UPLOAD, DescriptorBufferUse_Generic, vBufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, L"Vertex Buffer Resource Upload Heap");

	//graphicsDevice->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	//	&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), D3D12_RESOURCE_STATE_COPY_DEST,
	//	nullptr, IID_PPV_ARGS(&vertexBuffer.get()->pResource));
	//vertexBuffer->pResource->SetName(L"Vertex Buffer Resource Heap");

	//ID3D12Resource* uploadBuffer;
	//graphicsDevice->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
	//	&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ,
	//	nullptr, IID_PPV_ARGS(&uploadBuffer));

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(vertices);
	vertexData.RowPitch = vBufferSize;
	vertexData.SlicePitch = vBufferSize;

	UpdateSubresources(commandList.Get(), vertexBuffer->pResource.Get(), vertexBuffer->pUpload.Get(), 0, 0, 1, &vertexData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(vertexBuffer.get()->pResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	vertexBuffer->view.vertexBuffer = new D3D12_VERTEX_BUFFER_VIEW;

	vertexBuffer->view.vertexBuffer->BufferLocation = vertexBuffer.get()->pResource->GetGPUVirtualAddress();
	vertexBuffer->view.vertexBuffer->StrideInBytes = sizeOfVertex;
	vertexBuffer->view.vertexBuffer->SizeInBytes = vBufferSize;


	return vertexBuffer;
}
//
unique_ptr<D3D12R_PrimitiveResource>  D3D12R_CreateIndexBuffer(DWORD* indices, DWORD indexCount)
{
	std::unique_ptr <D3D12R_PrimitiveResource> indexBuffer = make_unique<D3D12R_PrimitiveResource>(D3D12R_PrimitiveResource::PrimitiveType_Index);
	unsigned int bufferSize = sizeof(DWORD) * indexCount;

    indexBuffer->pResource = D3D12R_CreateDescriptor(D3D12_HEAP_TYPE_DEFAULT, DescriptorBufferUse_Generic, bufferSize, D3D12_RESOURCE_STATE_COPY_DEST, L"Index Buffer Resource Heap");
    indexBuffer->pUpload = D3D12R_CreateDescriptor(D3D12_HEAP_TYPE_UPLOAD, DescriptorBufferUse_Generic, bufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, L"Index Buffer Resource Upload Heap");
	//graphicsDevice->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
	//	D3D12_HEAP_FLAG_NONE, // no flags
	//	&CD3DX12_RESOURCE_DESC::Buffer(bufferSize), // resource description for a buffer
	//	D3D12_RESOURCE_STATE_COPY_DEST, // start in the copy destination state
	//	nullptr, // optimized clear value must be null for this type of resource
	//	IID_PPV_ARGS(&indexBuffer.get()->pResource));
	//// we can give resource heaps a name so when we debug with the graphics debugger we know what resource we are looking at
	//indexBuffer->pResource->SetName(L"Index Buffer Resource Heap");
    

	// create upload heap to upload index buffer
	//ID3D12Resource* iBufferUploadHeap;
	//graphicsDevice->CreateCommittedResource(
	//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // upload heap
	//	D3D12_HEAP_FLAG_NONE, // no flags
	//	&CD3DX12_RESOURCE_DESC::Buffer(bufferSize), // resource description for a buffer
	//	D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
	//	nullptr,
	//	IID_PPV_ARGS(&iBufferUploadHeap));
	//iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<BYTE*>(indices); // pointer to our index array
	indexData.RowPitch = bufferSize; // size of all our index buffer
	indexData.SlicePitch = bufferSize; // also the size of our index buffer

	UpdateSubresources(commandList.Get(), indexBuffer->pResource.Get(), indexBuffer->pUpload.Get(), 0, 0, 1, &indexData);

	// transition the vertex buffer data from copy destination state to vertex buffer state
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(indexBuffer.get()->pResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));

	indexBuffer->view.indexBuffer = new D3D12_INDEX_BUFFER_VIEW;
	indexBuffer->view.indexBuffer->BufferLocation = indexBuffer->pResource->GetGPUVirtualAddress();
	indexBuffer->view.indexBuffer->Format = DXGI_FORMAT_R32_UINT;
	indexBuffer->view.indexBuffer->SizeInBytes = bufferSize;

	return indexBuffer;
}
ComPtr<ID3D12Resource> D3D12R_CreateDescriptor(D3D12_HEAP_TYPE heapType, DescriptorBufferUse bufferUse, unsigned int bufferSize, D3D12_RESOURCE_STATES initialState, LPCWSTR bufferName)
{
    unsigned int bSize = bufferSize;
    if (bufferUse == DescriptorBufferUse_CBV_SRV_UAV_SAMPLER)
        bSize = (bSize + (1024 * 64)) & ~(1024 * 64);
    else if (bufferUse == DescriptorBufferUse_MultiSampleTexture)
        bSize = (bSize + (1024 * 1024 * 4)) & ~(1024 * 1024 * 4);

    ComPtr<ID3D12Resource> resource;
    graphicsDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(heapType), D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bSize), initialState,
        nullptr, IID_PPV_ARGS(&resource)
    );
    if (bufferName)
        resource->SetName(bufferName);

    return resource;
}
//
//void D3D12R_GenerateUniqueRSPResources(const D3D12R_RSP* rootSignatureParams, unsigned int* inputDataSizes)
//{
//	for (int i = 0; i < rootSignatureParams->parameterCount; i++)
//	{
//		if (rootSignatureParams->parameterInfo[i].parameterType != D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS &&
//			rootSignatureParams->parameterInfo[i].parameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
//		{
//			for (int v = 0; v < frameBufferCount; v++)
//			{
//				ID3D12Resource* uploadBuffer[frameBufferCount];
//				graphicsDevice->CreateCommittedResource(
//					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
//					D3D12_HEAP_FLAG_NONE, // no flags
//					&CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
//					D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
//					nullptr, // we do not have use an optimized clear value for constant buffers
//					IID_PPV_ARGS(&uploadBuffer[v]));
//
//				UINT8* bufferAddress[frameBufferCount];
//
//				CD3DX12_RANGE readRange(0, 0);
//				uploadBuffer[v]->Map(0, &readRange, reinterpret_cast<void**>(&bufferAddress[i]));
//
//				int allignment = (inputDataSizes[i] + 255) & ~255;
//				int f = 0;
//			}
//		}
//	}
//}

