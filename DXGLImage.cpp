#include "DXGLImage.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace dxgl;

DXGLImage::DXGLImage(const std::string& filename) {
	m_imageData = stbi_load(filename.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
}

DXGLImage::~DXGLImage() {
}

unsigned char* DXGLImage::loadFromMemory(unsigned char* data, unsigned int length, int& width, int& height, int& channels) {
	return stbi_load_from_memory(data, length, &width, &height, &channels, STBI_rgb_alpha);
}

void DXGLImage::free() {
	stbi_image_free(m_imageData);
}

int DXGLImage::getWidth() {
	return m_width;
}

int DXGLImage::getHeight() {
	return m_height;
}

int DXGLImage::getChannels() {
	return m_channels;
}

unsigned char* DXGLImage::getImageData() {
	return m_imageData;
}
