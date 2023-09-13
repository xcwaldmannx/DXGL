#include <iostream>
#include <chrono>
#include <thread>

#include "DXGLApp.h"
#include "Game01.h"

int main() {
	std::cout << "Running..." << std::endl;
	
	// DXGLApp app{};
	Game01 app{};
	try {
		while (app.isRunning());
	} catch (const std::exception& e) {
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		std::string wait;
		std::cin >> wait;
		return -1;
	}

	std::cout << "terminated." << std::endl;
	return 0;
}