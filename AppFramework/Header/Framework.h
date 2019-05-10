#pragma once

class Framework
{
public:
	Framework();
	~Framework();
	bool Initialize(HINSTANCE hinstance, unsigned int width, unsigned int height);

	void Run();
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	bool InitWindow(HINSTANCE hinstance, unsigned int width, unsigned int height);

public:
	bool fullscreen;
private:
	HWND m_windowHandle;
	HINSTANCE m_hInstance;

	UINT m_windowedWidth;
	UINT m_windowedHeight;

};