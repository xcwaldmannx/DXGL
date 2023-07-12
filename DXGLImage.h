#pragma once

#include <string>

namespace dxgl {

	class DXGLImage {
	public:
		DXGLImage(const std::string& filename);
		~DXGLImage();

		static unsigned char* loadFromMemory(unsigned char* data, unsigned int length, int& width, int& height, int& channels);
		void free();

		int getWidth();
		int getHeight();
		int getChannels();
		unsigned char* getImageData();

	private:
		int m_width = 0;
		int m_height = 0;
		int m_channels = 0;
		unsigned char* m_imageData = nullptr;
	};

}
