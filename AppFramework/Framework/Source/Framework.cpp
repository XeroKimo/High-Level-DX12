#include "stdafx.h"

Framework::Framework()
{
	m_hInstance = nullptr;
	m_windowHandle = nullptr;
}

Framework::~Framework()
{
	D3D12_Shutdown();
}

bool Framework::Initialize(unsigned int width, unsigned int height)
{
	if (!InitWindow(width, height))
		return false;

	if (!D3D12_Initialize(width, height, m_windowHandle))
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
	D3D12ResourceWrapper* vbufferView = D3D12_CreateVertexBuffer(vertices, 3, sizeof(VertexDesc));

	DWORD indices[3] =
	{
		0,1,2
	};
	D3D12ResourceWrapper* iBufferView = D3D12_CreateIndexBuffer(indices, 3);

	D3D12ShaderWrapper vertexShader = D3D12ShaderWrapper(L"Framework/Source/VertexShader.hlsl", SHADER_VERTEX, SHADER_VERSION_5_0);
	D3D12_CreateShaderByteCode(&vertexShader);


	D3D12ShaderWrapper pixelShader = D3D12ShaderWrapper(L"Framework/Source/PixelShader.hlsl", SHADER_PIXEL, SHADER_VERSION_5_0);
	D3D12_CreateShaderByteCode(&pixelShader);

	D3D12_INPUT_ELEMENT_DESC inputLayout[2]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12ShaderWrapper* shaders[] = { &vertexShader, &pixelShader };

	ID3D12PipelineState* pipelineState = D3D12_CreatePipelineState(nullptr, inputLayout, 2 ,shaders,2);

	D3D12_DispatchCommandList();
#pragma endregion

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
			D3D12_BeginRender();
#pragma region D3D12 Render Testing

			D3D12_UsingPipeline(pipelineState, nullptr);
			D3D12_UsingVertexBuffer(0, 1, vbufferView->view.vertexBuffer);
			D3D12_UsingIndexBuffer(iBufferView->view.indexBuffer);
			D3D12_DrawIndexedInstanced(3, 1, 0, 0, 0, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#pragma endregion



			D3D12_EndRender();
		}
    }

#pragma region D3D12 Render Testing Freeing memory

	delete vbufferView;

	pipelineState->Release();

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
