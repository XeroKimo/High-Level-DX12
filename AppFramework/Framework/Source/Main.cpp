#include "stdafx.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    Framework* framework = new Framework();

    if (!framework->Initialize(WINDOW_WIDTH, WINDOW_HEIGHT))
	{
		MessageBox(0, L"Window Initialization - Failed", L"Error", MB_OK);
		return 1;
	}

	framework->Run();

    return 0;
}