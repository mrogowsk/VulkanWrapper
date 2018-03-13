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

#pragma once

#include <string>

#include <Windows.h>

class Window
{
public:

    Window();
    Window(Window&& other);
    ~Window();

    bool    create(const std::string& strWindowName, unsigned int uiWidth, unsigned int uiHeight, unsigned int uiPosX, unsigned int uiPosY);
    void    open() const;
    void    close() const;

    void    minimized()
    {
        m_bMinimized = true;
    }
    void    restored()
    {
        m_bMinimized = false;
    }

    HWND getWindow() const
    {
        return m_hWND;
    }

    HINSTANCE getInstance() const
    {
        return m_hInstance;
    }

    unsigned int getWidth()  const
    {
        return m_uiWidth;
    }
    unsigned int getHeight() const
    {
        return m_uiHeight;
    }
    bool isMinimized()       const
    {
        return m_bMinimized;
    }

    operator bool() const
    {
        return m_hWND != NULL;
    }

protected:

    HWND                    m_hWND;
    HINSTANCE               m_hInstance;

    std::string             m_strWindowName;

    bool                    m_bMinimized;
    unsigned int            m_uiWidth;
    unsigned int            m_uiHeight;
    unsigned int            m_uiPosX;
    unsigned int            m_uiPosY;

    Window(Window const& w);

    Window operator=(Window const rhs);
};