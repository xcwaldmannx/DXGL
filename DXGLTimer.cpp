#include "DXGLTimer.h"

#include <thread>
#include <chrono>

using namespace dxgl;

DXGLTimer::DXGLTimer() {
    m_notify = new bool(false);
}

DXGLTimer::~DXGLTimer() {
    *m_notify = false;

    if (m_thread.joinable()) {
        m_thread.join();
    }

    delete m_notify;
}

void DXGLTimer::start() {
    m_thread = std::thread([&]() {
        wait(m_duration);
        });
    m_thread.detach();
}

void DXGLTimer::stop() {
    *m_notify = true;
    m_isEvery = false;
}

void DXGLTimer::once(unsigned int duration, std::function<void()> callback) {
    m_duration = duration;
    m_function = callback;
    m_isEvery = false;
}

void DXGLTimer::every(unsigned int duration, std::function<void()> callback) {
    m_duration = duration;
    m_function = callback;
    m_isEvery = true;
}

void DXGLTimer::execute() {
    if (*m_notify) {
        m_function();
        *m_notify = false;
        if (m_isEvery) {
            start();
        }
    }
}

void DXGLTimer::wait(unsigned int duration) {
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    *m_notify = true;
}

