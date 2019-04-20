#pragma once

class Framework
{
public:
	Framework();
	~Framework();
	bool Initialize(unsigned int width, unsigned int height);

	void Run();
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	bool InitWindow(unsigned int width, unsigned int height);

private:
	HWND m_windowHandle;
	HINSTANCE m_hInstance;
};