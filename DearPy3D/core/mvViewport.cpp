#include "mvViewport.h"
#include "mvTypes.h"
#include "imgui_impl_win32.h"
#include <objbase.h> // For COM headers

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK
HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void
initialize_viewport(mvViewport& viewport, int width, int height)
{

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    viewport.wc = { 0 };
    viewport.wc.cbSize = sizeof(WNDCLASSEX);
    viewport.wc.style = CS_HREDRAW | CS_VREDRAW;
    viewport.wc.lpfnWndProc = HandleMsgSetup;
    viewport.wc.cbClsExtra = 0;
    viewport.wc.cbWndExtra = 0;
    viewport.wc.hInstance = GetModuleHandle(nullptr);
    viewport.wc.hIcon = nullptr;
    viewport.wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    viewport.wc.hbrBackground = nullptr;
    viewport.wc.lpszMenuName = nullptr;
    viewport.wc.lpszClassName = "Dear Py3D";
    viewport.wc.hIconSm = nullptr;
    RegisterClassEx(&viewport.wc);
    viewport.hinstance = viewport.wc.hInstance;

    // calculate window size based on desired client region size
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    // create window & get hWnd
    viewport.handle = CreateWindowA(
        "Dear Py3D",
        "Dear Py3D Sandbox",
        WS_OVERLAPPEDWINDOW,
        0, 0, wr.right - wr.left, wr.bottom - wr.top,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        &viewport // app data
    );

    ShowWindow(viewport.handle, SW_SHOWDEFAULT);
    viewport.running = true;
}

void
process_viewport_events(mvViewport& viewport)
{
    viewport.deltaX = 0.0;
    viewport.deltaY = 0.0;

    MSG msg = {};

    // while queue has messages, remove and dispatch them (but do not block on empty queue)
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        // check for quit because peekmessage does not signal this via return val
        if (msg.message == WM_QUIT)
        {
            viewport.running = false;
            return;
        }

        // TranslateMessage will post auxilliary WM_CHAR messages from key msgs
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

static LRESULT CALLBACK
HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    mvViewport* viewport;
    if (msg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        viewport = reinterpret_cast<mvViewport*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)viewport);
    }
    else
    {
        LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        viewport = reinterpret_cast<mvViewport*>(ptr);
    }

    switch (msg)
    {

    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED)
        {
            RECT rect;
            RECT crect;
            int awidth = 0;
            int aheight = 0;
            int cwidth = 0;
            int cheight = 0;
            if (GetWindowRect(hWnd, &rect))
            {
                awidth = rect.right - rect.left;
                aheight = rect.bottom - rect.top;
            }

            if (GetClientRect(hWnd, &crect))
            {
                cwidth = crect.right - crect.left;
                cheight = crect.bottom - crect.top;
            }

            //if (window->width != cwidth || window->height != cheight)
            viewport->resized = true;

            viewport->width = cwidth;
            viewport->height = cheight;

        }
        return 0;

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        break;
    }

    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void
cleanup_viewport(mvViewport& viewport)
{
    CoUninitialize();
    DestroyWindow(viewport.handle);
    UnregisterClass(viewport.wc.lpszClassName, viewport.wc.hInstance);
}

