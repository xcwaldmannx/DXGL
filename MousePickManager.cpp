#include "MousePickManager.h"

#include "Engine.h"
#include "Renderer.h"
#include "EntityManager.h"
#include "DXGLShaderResourceView.h"

using namespace dxgl;

MousePickManager::MousePickManager() {

}

MousePickManager::~MousePickManager() {

}

uint32_t MousePickManager::getColorId(Point2f position) {

	if (!m_entities) {
		Engine::entities()->group<PickableComponent>(governor::GROUP_ANY, m_entities);
	}

	SP_DXGLShaderResourceView tex = Engine::renderer()->getRenderToTexture(RESOURCE_VIEW_SLOT_1);

	UINT width = 0;
	UINT height = 0;

	// Get the original resource from the shader resource view
	ID3D11Resource* pOriginalResource = nullptr;
	tex->get()->GetResource(&pOriginalResource);

	// get width and height of resource
	ID3D11Texture2D* pTexture = nullptr;
	HRESULT hr = pOriginalResource->QueryInterface<ID3D11Texture2D>(&pTexture);
	if (SUCCEEDED(hr)) {
		D3D11_TEXTURE2D_DESC desc;
		pTexture->GetDesc(&desc);

		width = desc.Width;
		height = desc.Height;
	}

	if (position.x < 0 || position.x > width || position.y < 0 || position.y > height) {
		return INT32_MAX;
	}

	pTexture->Release();

	ID3D11Texture2D* pStagingTexture = nullptr;

	// Create the staging texture
	D3D11_TEXTURE2D_DESC stagingDesc{};
	stagingDesc.Width = width;
	stagingDesc.Height = height;
	stagingDesc.MipLevels = 1;
	stagingDesc.ArraySize = 1;
	stagingDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	stagingDesc.SampleDesc.Count = 1;
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	hr = Engine::graphics()->device()->CreateTexture2D(&stagingDesc, nullptr, &pStagingTexture);

	if (FAILED(hr)) {
		throw std::runtime_error("Could not create texture.");
	}

	// Copy the data from the original resource to the staging texture
	Engine::graphics()->context()->CopySubresourceRegion(pStagingTexture, 0, 0, 0, 0, pOriginalResource, 0, nullptr);


	// Map the staging texture to read the pixel data
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	hr = Engine::graphics()->context()->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);

	if (FAILED(hr)) {
		throw std::runtime_error("Could not map resource.");
	}

	// get the pixel data
	uint32_t* data = reinterpret_cast<uint32_t*>(mappedResource.pData);
	UINT rowPitch = mappedResource.RowPitch / sizeof(uint32_t);

	UINT offset = (position.y * rowPitch) + position.x;

	// Read the color value
	uint32_t color = *(data + offset);

	// Extract the red, green, and blue components
	int red = static_cast<int>((color & 0xFF));
	int green = static_cast<int>(((color >> 8) & 0xFF));
	int blue = static_cast<int>(((color >> 16) & 0xFF));

	// Pack the color components (with alpha set to zero)
	uint32_t colorId = (red << 16) | (green << 8) | blue;

	// unmap when done with resource
	Engine::graphics()->context()->Unmap(pStagingTexture, 0);

	// Release resources
	pOriginalResource->Release();
	pStagingTexture->Release();

	// set selected entity
	for (auto& id : *m_entities) {
		auto& component = Engine::entities()->getEntityComponent<PickableComponent>(id);
		component.isSelected = false;
		if (id == colorId) {
			component.isSelected = true;
		}
	}

	return colorId;
}