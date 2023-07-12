#pragma once

#include <functional>
#include <thread>
#include <mutex>

namespace dxgl {
	class DXGLTimer {
	public:
		DXGLTimer();
		~DXGLTimer();

		void start();
		void pause();
		void stop();

		void once(unsigned int duration, std::function<void()> callback);
		void every(unsigned int duration, std::function<void()> callback);

		void execute();

	private:
		void wait(unsigned int duration);

	private:
		std::thread m_thread;

		bool* m_notify = nullptr;

		unsigned int m_duration = 0;
		std::function<void()> m_function;
		bool m_isEvery = false;
	};
}