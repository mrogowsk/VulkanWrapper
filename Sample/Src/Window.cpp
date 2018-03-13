//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "Window.h"

#define WINDOW_CLASSNAME "DX11WindowClass"

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

Window::Window()
    : m_hWND(NULL)
    , m_bMinimized(false)
    , m_uiWidth(0)
    , m_uiHeight(0)
    , m_uiPosX(0)
    , m_uiPosY(0)
{}

Window::Window(Window&& other)
    : m_hWND(other.m_hWND)
    , m_strWindowName(std::move(other.m_strWindowName))
    , m_bMinimized(other.m_bMinimized)
    , m_uiWidth(other.m_uiWidth)
    , m_uiHeight(other.m_uiHeight)
    , m_uiPosX(other.m_uiPosX)
    , m_uiPosY(other.m_uiPosY)
{
    other.m_hWND = NULL;
}

Window::~Window(void)
{
    if (m_hWND)
    {
        DestroyWindow(m_hWND);
    }
}

bool Window::create(const std::string& strWindowName, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPosX, unsigned int uiPosY)
{
    m_strWindowName = strWindowName;
    m_uiWidth = uiWidth;
    m_uiHeight = uiHeight;
    m_uiPosX = uiPosX;
    m_uiPosY = uiPosY;

    WNDCLASSEX wndClass = {};
    if (!GetClassInfoEx(static_cast<HINSTANCE>(GetModuleHandle(NULL)), WINDOW_CLASSNAME, &wndClass))
    {
        wndClass.cbSize = sizeof(WNDCLASSEX);
        wndClass.lpfnWndProc = WndProc;
        wndClass.hInstance = static_cast<HINSTANCE>(GetModuleHandle(NULL));
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        wndClass.lpszClassName = WINDOW_CLASSNAME;
        wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        if (!RegisterClassEx(&wndClass))
        {
            return false;
        }
    }

    RECT wndRect = {0, 0, static_cast<LONG>(m_uiWidth), static_cast<LONG>(m_uiHeight)};
    AdjustWindowRect(&wndRect, WS_OVERLAPPEDWINDOW, false);

    m_hWND = CreateWindow(WINDOW_CLASSNAME,
                          m_strWindowName.c_str(),
                          WS_OVERLAPPEDWINDOW,
                          m_uiPosX,
                          m_uiPosY,
                          wndRect.right - wndRect.left,
                          wndRect.bottom - wndRect.top,
                          NULL,
                          NULL,
                          static_cast<HINSTANCE>(GetModuleHandle(NULL)),
                          nullptr);

    if (!m_hWND)
    {
        return false;
    }

    m_hInstance = static_cast<HINSTANCE>(GetModuleHandle(NULL));

    return true;
}

void Window::open() const
{
    if (m_hWND)
    {
        ShowWindow(m_hWND, SW_SHOWDEFAULT);

        UpdateWindow(m_hWND);

        SetWindowLongPtr(m_hWND, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }
}

void Window::close() const
{
    if (m_hWND)
    {
        ShowWindow(m_hWND, SW_HIDE);

        UpdateWindow(m_hWND);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    Window* pWin = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (!pWin)
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
        case WM_CHAR:
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}