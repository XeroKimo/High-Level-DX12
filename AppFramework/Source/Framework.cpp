#include "stdafx.h"

Framework::Framework()
{
	m_hInstance = nullptr;
	m_windowHandle = nullptr;
	fullscreen = false;
	m_windowedHeight = 0;
	m_windowedWidth = 0;
}

Framework::~Framework()
{
	//D3D12R_Shutdown();
	DX12R_Shutdown();
}

bool Framework::Initialize(HINSTANCE hinstance, unsigned int width, unsigned int height)
{
	if (!InitWindow(hinstance, width, height))
		return false;

	if (!DX12R_Initialize(width, height, fullscreen, m_windowHandle))
		return false;

	return true;
}

bool Framework::InitWindow(HINSTANCE hinstance, unsigned int width, unsigned  int height)
{
	m_hInstance = hinstance;
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

	RECT windowRect = { 0,0, static_cast<LONG>(width),static_cast<LONG>(height) };
	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    m_windowHandle = CreateWindowEx(
        NULL,                           // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT,	// Position
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,					// Size
        NULL,							// Parent window    
        NULL,							// Menu
        m_hInstance,						// Instance handle
        this							// Additional application data
    );

    if (!m_windowHandle)
    {
		MessageBox(NULL, L"Error registering class", L"ERROR", MB_OK | MB_ICONERROR);
        return false;
    }

	m_windowedHeight = height;
	m_windowedWidth = width;
    ShowWindow(m_windowHandle, SW_SHOW);
    return true;
}

void Framework::Run()
{


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
			DX12R_BeginRender();
			DX12R_EndRender();
		}
    }

}

LRESULT Framework::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Framework* framework = reinterpret_cast<Framework*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (uMsg)
    {
	case WM_CREATE:
	{
		LPCREATESTRUCT pStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pStruct->lpCreateParams));
	}
		return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
		if (MessageBox(0, L"Are you sure you want to exit?", L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
			DestroyWindow(hwnd);
		}
		if (wParam == VK_F11)
		{
			framework->fullscreen = !framework->fullscreen;
			HRESULT hr = DX12Interface::dxrSwapChain->SetFullScreenState(framework->fullscreen);
			int i = 0;
		}
		return 0;
	case WM_SIZE:
		UINT height = HIWORD(lParam);
		UINT width = LOWORD(lParam);
		if (DX12Interface::dxrSwapChain)
			DX12Interface::dxrSwapChain->ResizeBuffers(width, height);
		return 0;
	}
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
