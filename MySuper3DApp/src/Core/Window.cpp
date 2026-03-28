#include "Window.h"
#include "Input.h"
#include <stdexcept>
#include <iostream>

Window::Window(int width, int height, const std::wstring& title)
    : width_(width), height_(height), className_(title)
{
    hInstance_ = GetModuleHandle(nullptr);

    WNDCLASSEX wc     = {};
    wc.cbSize         = sizeof(WNDCLASSEX);
    wc.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc    = WndProc;
    wc.hInstance      = hInstance_;
    wc.hIcon          = LoadIcon(nullptr, IDI_WINLOGO);
    wc.hIconSm        = wc.hIcon;
    wc.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground  = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    wc.lpszClassName  = className_.c_str();

    if (!RegisterClassEx(&wc))
        throw std::runtime_error("Failed to register window class");

    RECT rect = { 0, 0, width_, height_ };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    int posX = (GetSystemMetrics(SM_CXSCREEN) - width_)  / 2;
    int posY = (GetSystemMetrics(SM_CYSCREEN) - height_) / 2;

    hWnd_ = CreateWindowEx(
        WS_EX_APPWINDOW,
        className_.c_str(), title.c_str(),
        WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME,
        posX, posY,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance_, this   // передаём this через lpParam
    );

    if (!hWnd_)
        throw std::runtime_error("Failed to create window");

    ShowWindow(hWnd_, SW_SHOW);
    SetForegroundWindow(hWnd_);
    SetFocus(hWnd_);
}

Window::~Window()
{
    if (hWnd_) DestroyWindow(hWnd_);
    UnregisterClass(className_.c_str(), hInstance_);
}

bool Window::ProcessMessages()
{
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT) return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    // При создании окна Windows передаёт указатель на Window через CREATESTRUCT
    Window* wnd = nullptr;
    if (msg == WM_NCCREATE)
    {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lp);
        wnd = static_cast<Window*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(wnd));
    }
    else
    {
        wnd = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    switch (msg)
    {
    case WM_KEYDOWN:
        Input::SetKeyDown(static_cast<int>(wp));
        if (static_cast<unsigned int>(wp) == VK_ESCAPE)
            PostQuitMessage(0);
        return 0;

    case WM_KEYUP:
        Input::SetKeyUp(static_cast<int>(wp));
        return 0;

    case WM_LBUTTONDOWN: Input::SetKeyDown(VK_LBUTTON); return 0;
    case WM_LBUTTONUP:   Input::SetKeyUp(VK_LBUTTON);   return 0;
    case WM_RBUTTONDOWN: Input::SetKeyDown(VK_RBUTTON);  return 0;
    case WM_RBUTTONUP:   Input::SetKeyUp(VK_RBUTTON);    return 0;
    case WM_MBUTTONDOWN: Input::SetKeyDown(VK_MBUTTON);  return 0;
    case WM_MBUTTONUP:   Input::SetKeyUp(VK_MBUTTON);    return 0;

    case WM_MOUSEMOVE:
    {
        static POINT lastPos = { -1, -1 };
        POINT cur = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) };
        if (lastPos.x != -1)
            Input::SetMouseDelta(cur.x - lastPos.x, cur.y - lastPos.y);
        lastPos = cur;
        return 0;
    }

    case WM_SIZE:
        if (wnd && wnd->OnResize)
        {
            wnd->width_  = LOWORD(lp);
            wnd->height_ = HIWORD(lp);
            wnd->OnResize(wnd->width_, wnd->height_);
        }
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}
