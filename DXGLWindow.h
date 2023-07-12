#pragma once

#include <Windows.h>

namespace dxgl {
	class DXGLWindow {
	public:
		DXGLWindow();
		~DXGLWindow();

        bool isRunning();

        // window information
        HWND& getWindow();
        RECT getWindowSize();
        POINT getWindowCenter();

        void centerCursorOnWindow();

        // screen information
        RECT getScreenSize();

        // window functions
        void setFullscreen(bool fullscreen);

        // events
        virtual void onCreate();
        virtual void onUpdate();
        virtual void onDestroy();
        virtual void onFocus();
        virtual void onKillFocus();
        virtual void onMove();
        virtual void onSize();

    private:
        bool broadcast();

    protected:
        HWND m_hwnd{};

    private:
        RECT m_defaultWindowSize{0, 0, 0, 0};
        bool m_isWindowRunning = false;
        bool m_isWindowInitialized = false;

	};
}