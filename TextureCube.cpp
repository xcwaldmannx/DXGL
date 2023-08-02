#include "TextureCube.h"

#include <filesystem>

#include "DXGLImage.h"

using namespace dxgl;

TextureCube::TextureCube(const std::string& filepath) {

	// TODO: Create loop for all cubemap faces

	DXGLImage* images[6] = { nullptr };

	for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
		const std::string filename = entry.path().generic_string();
		if (filename.find("negx") != std::string::npos) {
			images[1] = new DXGLImage(filename);
		} else if (filename.find("negy") != std::string::npos) {
			images[3] = new DXGLImage(filename);
		} else if (filename.find("negz") != std::string::npos) {
			images[5] = new DXGLImage(filename);
		} else if (filename.find("posx") != std::string::npos) {
			images[0] = new DXGLImage(filename);
		} else if (filename.find("posy") != std::string::npos) {
			images[2] = new DXGLImage(filename);
		} else if (filename.find("posz") != std::string::npos) {
			images[4] = new DXGLImage(filename);
		}
	}

	ID3D11Texture2D* texture = nullptr;

	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = images[0]->getWidth();
	desc.Height = images[0]->getHeight();
	desc.MipLevels = 0;
	desc.ArraySize = 6;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS;
	desc.CPUAccessFlags = 0;

	HRESULT result = DXGLMain::graphics()->device()->CreateTexture2D(&desc, nullptr, &texture);

	if (FAILED(result)) {
		throw std::exception("TextureCube texture could not be created.");
	}

	for (int i = 0; i < 6; i++) {
		UINT numMips = 1 + static_cast<UINT>(std::floor(std::log2(std::max<UINT>(desc.Width, desc.Height))));
		for (int j = 0; j < numMips; j++) {
			// Calculate the subresource index for each array element and mip level
			int subresourceIndex = D3D11CalcSubresource(j, i, numMips);

			// Calculate the size and pitch for the current mip level
			UINT mipWidth = desc.Width >> j;
			UINT mipHeight = desc.Height >> j;
			UINT rowPitch = mipWidth * 4;

			DXGLMain::graphics()->context()->UpdateSubresource(texture, subresourceIndex, nullptr,
				images[i]->getImageData(), rowPitch, rowPitch * mipHeight);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	result = DXGLMain::graphics()->device()->CreateShaderResourceView(texture, &srvDesc, &m_srv);

	if (FAILED(result)) {
		throw std::exception("TextureCube SRV could not be created.");
	}

	DXGLMain::graphics()->context()->GenerateMips(m_srv);

	for (int i = 0; i < 6; i++) {
		images[i]->free();
		delete images[i];
	}

	texture->Release();
}

TextureCube::~TextureCube() {
	if (m_srv) m_srv->Release();
}

void TextureCube::bind(int slot) {
	DXGLMain::graphics()->context()->VSSetShaderResources(slot, 1, &m_srv);
	DXGLMain::graphics()->context()->HSSetShaderResources(slot, 1, &m_srv);
	DXGLMain::graphics()->context()->DSSetShaderResources(slot, 1, &m_srv);
	DXGLMain::graphics()->context()->PSSetShaderResources(slot, 1, &m_srv);
}

ID3D11ShaderResourceView* TextureCube::get() {
	return m_srv;
}
