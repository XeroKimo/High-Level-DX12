#include "stdafx.h"

Framework::Framework()
{
	m_hInstance = nullptr;
	m_windowHandle = nullptr;
}

Framework::~Framework()
{
	D3D12R_Shutdown();
}

bool Framework::Initialize(unsigned int width, unsigned int height)
{
	if (!InitWindow(width, height))
		return false;

	if (!D3D12R_Initialize(width, height, m_windowHandle))
		return false;

	return true;
}

bool Framework::InitWindow(unsigned int width, unsigned  int height)
{
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    WNDCLASS wc = {};

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
    wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2);
	wc.lpszMenuName = NULL;
    wc.lpszClassName = CLASS_NAME;

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, L"Error registering class", L"ERROR", MB_OK | MB_ICONERROR);
		return false;
	}
    // Create the window.

    m_windowHandle = CreateWindowEx(
        NULL,                           // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,	// Size
		width, height,					// Position
        NULL,							// Parent window    
        NULL,							// Menu
        m_hInstance,						// Instance handle
        NULL							// Additional application data
    );

    if (!m_windowHandle)
    {
		MessageBox(NULL, L"Error registering class", L"ERROR", MB_OK | MB_ICONERROR);
        return false;
    }

    ShowWindow(m_windowHandle, SW_SHOW);
    return true;
}

void Framework::Run()
{
#pragma region D3D12 Renderer Testing

	struct VertexDesc {
		XMFLOAT3 position;
		XMFLOAT4 color;

		VertexDesc(XMFLOAT3 pos,  XMFLOAT4 col)
		{
			position = pos;
			color = col;
		}
	};

	VertexDesc vertices[3] =
	{
		VertexDesc(XMFLOAT3(0.0f, 0.5f ,0.5f)  , XMFLOAT4(1.0f,0.0f,0.0f,1.0f)),
		VertexDesc(XMFLOAT3(0.5f,-0.5f,0.5f)   , XMFLOAT4(0.0f,0.0f,1.0f,1.0f)),
		VertexDesc(XMFLOAT3(-0.5f, -0.5f ,0.5f), XMFLOAT4(0.0f,1.0f,0.0f,1.0f)),
	};
	std::unique_ptr<D3D12R_PrimitiveResource> vbufferView = D3D12R_CreateVertexBuffer(vertices, 3, sizeof(VertexDesc));

    //vbufferView.release();
	DWORD indices[3] =
	{
		0,1,2
	};
	std::unique_ptr < D3D12R_PrimitiveResource> iBufferView = D3D12R_CreateIndexBuffer(indices, 3);

	D3D12R_ShaderWrapper vertexShader = D3D12R_ShaderWrapper(L"Framework/Source/VertexShader.hlsl", SHADER_VERTEX, SHADER_VERSION_5_0);
	D3D12R_CreateShaderByteCode(&vertexShader);


	D3D12R_ShaderWrapper pixelShader = D3D12R_ShaderWrapper(L"Framework/Source/PixelShader.hlsl", SHADER_PIXEL, SHADER_VERSION_5_0);
	D3D12R_CreateShaderByteCode(&pixelShader);

	D3D12_INPUT_ELEMENT_DESC inputLayout[2]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12R_ShaderWrapper* shaders[] = { &vertexShader, &pixelShader };


	unique_ptr<D3D12R_SignatureParametersHelper> test = make_unique<D3D12R_SignatureParametersHelper>();
	test->CreateRootConstant(4, D3D12_SHADER_VISIBILITY_ALL);
	//test->CreateRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, D3D12_SHADER_VISIBILITY_ALL);
	//D3D12_DESCRIPTOR_RANGE_TYPE type = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	//int numDesc = 1;
	//int numRange = 1;
	//test->CreateRootDescriptorTable(&type, &numDesc, numRange, D3D12_SHADER_VISIBILITY_ALL);
	weak_ptr<D3D12R_RootSignatureWrapper> params = test->GenerateRootSignatureWrapper("params");




	//unsigned int inputSizes[] = { 0,257 };
	////D3D12R_GenerateUniqueRSPResources(testParams, &inputSizes[0]);

	ComPtr<ID3D12PipelineState> pipelineState = D3D12R_CreatePipelineState(params.lock()->rootSignature.Get(), inputLayout, 2 ,shaders,2);
	unsigned int sizes[1] = { sizeof(float) * 4 };
	//unique_ptr<D3D12R_PipelineStateObject> pipelineStateObj = make_unique<D3D12R_PipelineStateObject>(pipelineState,params,&sizes[0]);
	//int uniqueID = pipelineStateObj->GenerateUniqueInputID();

	float color[4] = { 0.0f,1.0f,0.0f,0.0f };

	D3D12R_DispatchCommandList();

#pragma endregion
	int count = params.use_count();

    MSG msg = {};
    bool done = false;
    while (!done)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
			if (msg.message == WM_QUIT)
				done = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }
		else
		{
			//pipelineStateObj->UpdateConstBuffer(uniqueID, 0, &color);
			D3D12R_BeginRender();
#pragma region D3D12 Render Testing

			D3D12R_UsingPipeline(pipelineState.Get(), params.lock()->rootSignature.Get());
			//pipelineStateObj->SetForRender();
			D3D12Renderer::commandList->SetGraphicsRoot32BitConstants(0, params.lock().get()[0].parameterInfo.get()->numberOfValues , & color[0], 0);

			D3D12R_UsingVertexBuffer(0, 1, vbufferView->view.vertexBuffer);
			D3D12R_UsingIndexBuffer(iBufferView->view.indexBuffer);
			D3D12R_DrawIndexedInstanced(3, 1, 0, 0, 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#pragma endregion



			D3D12R_EndRender();
		}
    }

#pragma region D3D12 Render Testing Freeing memory

	int i = 0;
#pragma endregion

}

LRESULT Framework::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
		if (MessageBox(0, L"Are you sure you want to exit?", L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
			DestroyWindow(hwnd);
		}
		return 0;
	}
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
