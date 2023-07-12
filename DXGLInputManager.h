#pragma once

#include "DXGLInputListener.h"

#include "DXGLDefinitions.h"
#include "DXGLMain.h"

typedef int MOUSE_STATE;

namespace dxgl {
	class DXGLInputManager : public DXGLInputListener {
    public:
        DXGLInputManager();
        ~DXGLInputManager();

        bool getKeyHoldState(char key, float duration);
        float getKeyPressState(char key);
        float getKeyTapState(char key);

        float getMouseState(MOUSE_STATE state);
        const Point2f& getMousePosition();
        const Point2f& getMouseDelta();

        virtual void onKeyDown(int key) override;
        virtual void onKeyUp(int key) override;

        virtual void onMouseMove(const Point2f& mousePos, const Point2f& mouseDelta) override;
        virtual void onLeftMouseDown(const Point2f& mousePos) override;
        virtual void onLeftMouseUp(const Point2f& mousePos) override;
        virtual void onRightMouseDown(const Point2f& mousePos) override;
        virtual void onRightMouseUp(const Point2f& mousePos) override;

    public:
        const static MOUSE_STATE LMB_STATE = 1;
        const static MOUSE_STATE MMB_STATE = 2;
        const static MOUSE_STATE RMB_STATE = 3;

    private:
        // Tracks the duration a key was held for.
        float m_keyHoldStates[256]{};
        // Tracks if a key is up or down.
        float m_keyPressStates[256]{};
        // Tracks if a key is tapped and released.
        float m_keyTapStates[256]{};

        // mouse position
        Point2f m_position{};

        // mouse change
        Point2f m_delta{};

        // left, middle, and right mouse button states
        float m_lmbState = 0;
        float m_mmbState = 0;
        float m_rmbState = 0;
	};
}