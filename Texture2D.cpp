#include "Texture2D.h"

#include "Engine.h"
#include "Graphics.h"

#include "DXGLImage.h"

using namespace dxgl;

Texture2D::Texture2D(const std::string& filename) {
	DXGLImage image = DXGLImage(filename);

	ID3D11Texture2D* texture = nullptr;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = image.getWidth();
	desc.Height = image.getHeight();
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.CPUAccessFlags = 0;

	HRESULT result = Engine::graphics()->device()->CreateTexture2D(&desc, nullptr, &texture);

	if (FAILED(result)) {
		throw std::exception("DXGLTexture2D texture could not be created.");
	}

	Engine::graphics()->context()->UpdateSubresource(texture, 0, nullptr,
		image.getImageData(), image.getWidth() * 4, 0);


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	result = Engine::graphics()->device()->CreateShaderResourceView(texture, &srvDesc, &m_srv);

	if (FAILED(result)) {
		throw std::exception("DXGLTexture2D SRV could not be created.");
	}

	Engine::graphics()->context()->GenerateMips(m_srv);

	image.free();
	texture->Release();

}

Texture2D::Texture2D(unsigned int width, unsigned int height, unsigned char* data) {
	ID3D11Texture2D* texture = nullptr;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.CPUAccessFlags = 0;

	HRESULT result = Engine::graphics()->device()->CreateTexture2D(&desc, nullptr, &texture);

	if (FAILED(result)) {
		throw std::exception("DXGLTexture2D texture could not be created.");
	}

	Engine::graphics()->context()->UpdateSubresource(texture, 0, nullptr, data, width * 4, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	result = Engine::graphics()->device()->CreateShaderResourceView(texture, &srvDesc, &m_srv);

	if (FAILED(result)) {
		throw std::exception("DXGLTexture2D SRV could not be created.");
	}

	Engine::graphics()->context()->GenerateMips(m_srv);

	texture->Release();
}

Texture2D::~Texture2D() {
	if (m_srv) m_srv->Release();
}

void Texture2D::bind(int slot) {
	Engine::graphics()->context()->VSSetShaderResources(slot, 1, &m_srv);
	Engine::graphics()->context()->HSSetShaderResources(slot, 1, &m_srv);
	Engine::graphics()->context()->DSSetShaderResources(slot, 1, &m_srv);
	Engine::graphics()->context()->PSSetShaderResources(slot, 1, &m_srv);
}

ID3D11ShaderResourceView* Texture2D::get() {
	return m_srv;
}
