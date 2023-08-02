#include <iostream>
#include <chrono>
#include <thread>

#include "DXGLApp.h"

int main() {
	std::cout << "Running..." << std::endl;
	
	DXGLApp dxglApp{};
	try {
		while (dxglApp.isRunning());
	} catch (...) {
		return -1;
	}

	std::cout << "terminated." << std::endl;
	return 0;
}