#include "Material.h"

#include <filesystem>

#include "DXGLImage.h"

using namespace dxgl;

Material::Material(const std::string& filepath) {

	DXGLImage* images[6] = { nullptr };

	for (const auto& entry : std::filesystem::directory_iterator(filepath)) {
		const std::string filename = entry.path().generic_string();

		if (filename.find("normal") != std::string::npos) {
			images[0] = new DXGLImage(filename);
			m_materialIndices |= MATERIAL_FLAG_USE_NORM_DISP;
		}
		else if (filename.find("albedo") != std::string::npos) {
			images[1] = new DXGLImage(filename);
			m_materialIndices |= MATERIAL_FLAG_USE_ALBEDO;
		}
		else if (filename.find("metallic") != std::string::npos) {
			images[2] = new DXGLImage(filename);
			m_materialIndices |= MATERIAL_FLAG_USE_METALLIC;
		}
		else if (filename.find("roughness") != std::string::npos) {
			images[3] = new DXGLImage(filename);
			m_materialIndices |= MATERIAL_FLAG_USE_ROUGHNESS;
		}
		else if (filename.find("ao") != std::string::npos) {
			images[4] = new DXGLImage(filename);
			m_materialIndices |= MATERIAL_FLAG_USE_AMB_OCC;
		}
	}

	ID3D11Texture2D* texture = nullptr;
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = images[0]->getWidth();
	texDesc.Height = images[0]->getHeight();
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 5;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;


	HRESULT result = Engine::graphics()->device()->CreateTexture2D(&texDesc, nullptr, &texture);

	if (FAILED(result)) {
		throw std::exception("DXGLMaterial texture could not be created.");
	}

	for (int i = 0; i < 5; i++) {
		UINT numMips = 1 + static_cast<UINT>(std::floor(std::log2(std::max<UINT>(texDesc.Width, texDesc.Height))));
		for (int j = 0; j < numMips; j++) {
			int subresourceIndex = D3D11CalcSubresource(j, i, numMips);  // Calculate the subresource index for each array element and mip level

			// Calculate the size and pitch for the current mip level
			UINT mipWidth = texDesc.Width >> j;
			UINT mipHeight = texDesc.Height >> j;
			UINT rowPitch = mipWidth * 4;  // Assuming 32-bit RGBA format (4 bytes per pixel)

			Engine::graphics()->context()->UpdateSubresource(texture, subresourceIndex, nullptr,
				images[i]->getImageData(), rowPitch, rowPitch * mipHeight);
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = 5;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	result = Engine::graphics()->device()->CreateShaderResourceView(texture, &srvDesc, &m_srv);

	if (FAILED(result)) {
		throw std::exception("DXGLMaterial SRV could not be created.");
	}

	Engine::graphics()->context()->GenerateMips(m_srv);

	for (int i = 0; i < 5; i++) {
		images[i]->free();
		delete images[i];
	}

	texture->Release();
}

Material::Material(const MaterialData& data) {
	auto& textures = data.textures;
	
	// set used material indices
	for (unsigned int i = 0; i < textures.size(); i++) {
		TextureData texData = textures[i];
		switch (texData.type) {
		case NORMAL_DISP:
			m_materialIndices |= MATERIAL_FLAG_USE_NORM_DISP;
			break;
		case ALBEDO:
			m_materialIndices |= MATERIAL_FLAG_USE_ALBEDO;
			break;
		case METALLIC:
			m_materialIndices |= MATERIAL_FLAG_USE_METALLIC;
			break;
		case ROUGHNESS:
			m_materialIndices |= MATERIAL_FLAG_USE_ROUGHNESS;
			break;
		case AMB_OCC:
			m_materialIndices |= MATERIAL_FLAG_USE_AMB_OCC;
			break;
		}
	}

	ID3D11Texture2D* texture = nullptr;
	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = textures[0].width;
	texDesc.Height = textures[0].height;
	texDesc.MipLevels = 0;
	texDesc.ArraySize = 5;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;


	HRESULT result = Engine::graphics()->device()->CreateTexture2D(&texDesc, nullptr, &texture);

	if (FAILED(result)) {
		throw std::exception("DXGLMaterial texture could not be created.");
	}

	UINT numMips = 1 + static_cast<UINT>(std::floor(std::log2(std::max<UINT>(texDesc.Width, texDesc.Height))));

	int skip = 0;

	for (int i = 0; i < 5; i++) {
		if (m_materialIndices & (1 << i)) {
			for (int j = 0; j < numMips; j++) {
				int subresourceIndex = D3D11CalcSubresource(j, i, numMips);  // Calculate the subresource index for each array element and mip level

				// Calculate the size and pitch for the current mip level
				UINT mipWidth = texDesc.Width >> j;
				UINT mipHeight = texDesc.Height >> j;
				UINT rowPitch = mipWidth * 4;  // Assuming 32-bit RGBA format (4 bytes per pixel)

				Engine::graphics()->context()->UpdateSubresource(texture, subresourceIndex, nullptr,
					textures[i - skip].data, rowPitch, rowPitch * mipHeight);
			}
		} else {
			skip++;
		}
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.ArraySize = 5;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = -1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	result = Engine::graphics()->device()->CreateShaderResourceView(texture, &srvDesc, &m_srv);

	if (FAILED(result)) {
		throw std::exception("DXGLMaterial SRV could not be created.");
	}

	Engine::graphics()->context()->GenerateMips(m_srv);

	texture->Release();
}

Material::~Material() {
	if (m_srv) m_srv->Release();
}

void Material::bind(int slot) {
	Engine::graphics()->context()->VSSetShaderResources(slot, 1, &m_srv);
	Engine::graphics()->context()->DSSetShaderResources(slot, 1, &m_srv);
	Engine::graphics()->context()->HSSetShaderResources(slot, 1, &m_srv);
	Engine::graphics()->context()->PSSetShaderResources(slot, 1, &m_srv);
}

ID3D11ShaderResourceView* Material::get() {
	return m_srv;
}

