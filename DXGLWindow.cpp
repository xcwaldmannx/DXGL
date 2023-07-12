#include "DXGLWindow.h"

#include <exception>

using namespace dxgl;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_CREATE: {
        DXGLWindow* window = (DXGLWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (window) {
            window->onCreate();
        }
        break;
    }
    case WM_MOVE: {
        DXGLWindow* window = (DXGLWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (window) {
            window->onMove();
        }
        break;
    }
    case WM_SIZE: {
        DXGLWindow* window = (DXGLWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (window) {
            window->onSize();
        }
        break;
    }
    case WM_GETMINMAXINFO: {
        // Event fired when the window is resized
        // and stops at the width / height specified below
        LPMINMAXINFO minmax = (LPMINMAXINFO)lparam;
        minmax->ptMinTrackSize.x = 640;
        minmax->ptMinTrackSize.y = 480;
        break;
    }
    case WM_DESTROY: {
        DXGLWindow* window = (DXGLWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        window->onDestroy();
        PostQuitMessage(0);
        break;
    }
    case WM_SETFOCUS: {
        DXGLWindow* window = (DXGLWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (window) {
            window->onFocus();
        }
        break;
    }
    case WM_KILLFOCUS: {
        DXGLWindow* window = (DXGLWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        window->onKillFocus();
        break;
    }
    case WM_CLOSE: {
        DXGLWindow* window = (DXGLWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        window->onDestroy();
        PostQuitMessage(0);
        break;
    }
    default: {
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
    }

    return NULL;
}

DXGLWindow::DXGLWindow() {
    WNDCLASSEX wc;
    wc.cbClsExtra = NULL;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wc.hInstance = NULL;
    wc.lpfnWndProc = &WndProc;
    wc.lpszClassName = L"DXGLWindowClass";
    wc.lpszMenuName = L"";
    wc.style = NULL;

    if (!RegisterClassEx(&wc)) {
        throw std::exception("Window could not be created.");
    }

    m_defaultWindowSize.left = 0;
    m_defaultWindowSize.top = 0;
    m_defaultWindowSize.right = (GetSystemMetrics(SM_CXSCREEN) * (4 / 3)) / 2;
    m_defaultWindowSize.bottom = (GetSystemMetrics(SM_CYSCREEN) * (4 / 3)) / 2;

    m_hwnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, L"DXGLWindowClass", L"DXGL",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, m_defaultWindowSize.right, m_defaultWindowSize.bottom, NULL, NULL, NULL, NULL);

    if (!m_hwnd) {
        throw std::exception("DXGLWindow could not be created.");
    }

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    m_isWindowRunning = true;
}

DXGLWindow::~DXGLWindow() {
}

bool DXGLWindow::isRunning() {
    if (m_isWindowRunning) {
        broadcast();
    }
    return m_isWindowRunning;
}

HWND& DXGLWindow::getWindow() {
    return m_hwnd;
}

RECT DXGLWindow::getWindowSize() {
    RECT r{};
    GetClientRect(m_hwnd, &r);
    return r;
}

POINT DXGLWindow::getWindowCenter() {
    RECT clientRect;
    GetClientRect(m_hwnd, &clientRect);

    int centerX = (clientRect.left + clientRect.right) / 2;
    int centerY = (clientRect.top + clientRect.bottom) / 2;

    POINT center;
    center.x = centerX;
    center.y = centerY;

    ClientToScreen(m_hwnd, &center);

    return center;
}

RECT DXGLWindow::getScreenSize() {
    RECT r{};
    r.left = 0;
    r.bottom = 0;
    r.right = GetSystemMetrics(SM_CXSCREEN);
    r.bottom = GetSystemMetrics(SM_CYSCREEN);
    return r;
}

//POINT DXGLWindow::getWindowCenter() {
//    RECT bounds{};
//    GetClientRect(m_hwnd, &bounds);
//
//    RECT position{};
//    GetWindowRect(m_hwnd, &position);
//
//    int width = (int)(bounds.right - bounds.left);
//    int height = (int)(bounds.bottom - bounds.top);
//
//    int middleX = (int)position.right - (width / 2);
//    int middleY = (int)position.bottom - (height / 2);
//
//    return POINT{ middleX, middleY };
//}

bool DXGLWindow::broadcast() {
    if (!m_isWindowInitialized) {
        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR) this);
        this->onCreate();
        m_isWindowInitialized = true;
    }

    this->onUpdate();

    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

void DXGLWindow::setFullscreen(bool fullscreen) {
    if (fullscreen) {
        SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(m_hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER);
    } else {
        LONG_PTR windowStyle = GetWindowLongPtr(m_hwnd, GWL_STYLE);
        windowStyle |= (WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
        SetWindowLongPtr(m_hwnd, GWL_STYLE, windowStyle);
        RECT windowRect = { 32, 32, 800, 600 };
        AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);
        SetWindowPos(m_hwnd, NULL, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
            SWP_FRAMECHANGED | SWP_SHOWWINDOW);
    }
}

void DXGLWindow::onCreate() {
}

void DXGLWindow::onUpdate() {
}

void DXGLWindow::onDestroy() {
    m_isWindowRunning = false;
}

void DXGLWindow::onFocus() {
}

void DXGLWindow::onKillFocus() {
}

void DXGLWindow::onMove() {
}

void DXGLWindow::onSize() {
}