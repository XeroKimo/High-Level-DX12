#include "stdafx.h"

Framework::Framework()
{
	m_hInstance = nullptr;
	m_windowHandle = nullptr;
}

Framework::~Framework()
{
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
    }

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
