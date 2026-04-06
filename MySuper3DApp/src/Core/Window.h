#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <string>
#include <functional>

class Window
{
public:
    Window(int width, int height, const std::wstring& title);
    ~Window();

    // Возвращает false если пришёл WM_QUIT
    bool ProcessMessages();

    HWND  GetHWND()   const { return hWnd_; }
    int   GetWidth()  const { return width_; }
    int   GetHeight() const { return height_; }

    // Callback: вызывается при изменении размера окна
    std::function<void(int, int)> OnResize;

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

    HWND         hWnd_      = nullptr;
    HINSTANCE    hInstance_ = nullptr;
    std::wstring className_;
    int          width_  = 0;
    int          height_ = 0;
};
