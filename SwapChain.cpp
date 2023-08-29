#include "SwapChain.h"

using namespace dxgl;

SwapChain::SwapChain() {
	m_rtvs.resize(RESOURCE_VIEW_COUNT_MAX);
	m_dsvs.resize(RESOURCE_VIEW_COUNT_MAX);
}

SwapChain::~SwapChain() {
	if (m_swapchain) m_swapchain->Release();
	m_window = nullptr;
}

void SwapChain::create(DXGLWindow* window) {
	m_window = window;
	m_width = m_window->getWindowSize().right;
	m_height = m_window->getWindowSize().bottom;

	// create the swapchain
	DXGI_SWAP_CHAIN_DESC desc{};
	desc.BufferCount = 1;
	desc.BufferDesc.Width = m_width;
	desc.BufferDesc.Height = m_height;
	desc.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.OutputWindow = m_window->getWindow();
	desc.SampleDesc.Count = 4;
	desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	desc.Windowed = true;

	// create swapchain for the window specified
	HRESULT result = Engine::graphics()->factory()->CreateSwapChain(Engine::graphics()->device(), &desc, &m_swapchain);

	if (FAILED(result)) {
		throw std::exception("DXGLSwapChain could not be created.");
	}

	// temp blend state
	Engine::graphics()->context()->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

	reload();
}

bool SwapChain::present(bool vsync) {
	m_swapchain->Present(vsync, 0);
	return true;
}

bool SwapChain::isPaused() {
	return m_isPaused;
}

void SwapChain::resize(unsigned int width, unsigned int height) {
	if (m_swapchain) {	
		for (int i = 0; i < m_rtvSlots.size(); i++) {
			m_rtvs[m_rtvSlots[i]].target->reset();
		}

		for (int i = 0; i < m_dsvSlots.size(); i++) {
			m_dsvs[m_dsvSlots[i]].target->reset();
		}
	}

	m_width = width;
	m_height = height;

	if (m_width == 0 || m_height == 0) {
		m_swapchain->Release();
		m_swapchain = nullptr;
		m_isPaused = true;
	} else {
		if (m_swapchain) {

			m_swapchain->ResizeBuffers(1, m_width, m_height, DXGI_FORMAT_R16G16B16A16_FLOAT, 0);
			reload();

			DXGI_MODE_DESC modeDesc{};
			modeDesc.Width = m_width;
			modeDesc.Height = m_height;
			modeDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			modeDesc.RefreshRate.Numerator = 60;
			modeDesc.RefreshRate.Denominator = 1;
			modeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			modeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			m_swapchain->ResizeTarget(&modeDesc);
		} else {
			create(m_window);

			reload();

			m_isPaused = false;
		}
	}
}

void SwapChain::reload() {

	for (int i = 0; i < m_rtvSlots.size(); i++) {
		createRenderTargetView(nullptr, (ResourceViewSlot) m_rtvSlots[i], nullptr);
	}

	for (int i = 0; i < m_dsvSlots.size(); i++) {
		createDepthStencilView(nullptr, (ResourceViewSlot) m_dsvSlots[i], nullptr);
	}
}

void SwapChain::setFullscreen(bool fullscreen) {
	unsigned int width = fullscreen ? GetSystemMetrics(SM_CXSCREEN) : GetSystemMetrics(SM_CXSCREEN) / 2;
	unsigned int height = fullscreen ? GetSystemMetrics(SM_CYSCREEN) : GetSystemMetrics(SM_CYSCREEN) / 2;
	resize(width, height);
	m_window->setFullscreen(fullscreen);
}

void SwapChain::createRenderTargetView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLRenderTargetView* rtv) {
	ID3D11Texture2D* buffer = nullptr;

	if (slot == RESOURCE_VIEW_SLOT_BACK_BUFFER) {
		HRESULT result = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer);

		if (FAILED(result)) {
			throw std::exception("DXGLSwapChain could not get SwapChain buffer.");
		}
	} else {
		if (desc) {
			buffer = createBuffer(desc, true);
		} else {
			buffer = createBuffer(&m_rtvs[slot].desc, true);
		}
	}

	ID3D11RenderTargetView* tempRTV = nullptr;
	HRESULT result = Engine::graphics()->device()->CreateRenderTargetView(buffer, nullptr, &tempRTV);

	if (FAILED(result)) {
		throw std::exception("DXGLSwapChain RTV texture could not be created.");
	}

	buffer->Release();

	SP_DXGLRenderTargetView newRTV = std::make_shared<DXGLRenderTargetView>(tempRTV);

	if (rtv) {
		m_rtvs[slot].target = rtv;
		if (desc) m_rtvs[slot].desc = *desc;
	}

	*m_rtvs[slot].target = newRTV;

	if (std::find(m_rtvSlots.begin(), m_rtvSlots.end(), slot) == m_rtvSlots.end()) {
		m_rtvSlots.push_back(slot);
	}
}

void SwapChain::createDepthStencilView(RESOURCE_VIEW_DESC* desc, ResourceViewSlot slot, SP_DXGLDepthStencilView* dsv) {
	ID3D11Texture2D* buffer = nullptr;

	if (desc) {
		buffer = createBuffer(desc, false);
	} else {
		buffer = createBuffer(&m_dsvs[slot].desc, false);
	}

	ID3D11DepthStencilView* tempDSV = nullptr;
	HRESULT result = Engine::graphics()->device()->CreateDepthStencilView(buffer, nullptr, &tempDSV);

	if (FAILED(result)) {
		throw std::exception("DXGLSwapChain DSV could not be created.");
	}

	buffer->Release();

	SP_DXGLDepthStencilView newDSV = std::make_shared<DXGLDepthStencilView>(tempDSV);

	if (dsv) {
		m_dsvs[slot].target = dsv;
		if (desc) m_dsvs[slot].desc = *desc;
	}

	*m_dsvs[slot].target = newDSV;

	if (std::find(m_dsvSlots.begin(), m_dsvSlots.end(), slot) == m_dsvSlots.end()) {
		m_dsvSlots.push_back(slot);
	}
}

ID3D11Texture2D* SwapChain::createBuffer(RESOURCE_VIEW_DESC* desc, bool rtv) {
	DXGI_FORMAT format = (DXGI_FORMAT) desc->format;

	if (rtv) {
		ID3D11Texture2D* buffer = nullptr;

		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = m_width;
		textureDesc.Height = m_height;
		textureDesc.Format = format;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = desc->sampleCount;
		textureDesc.SampleDesc.Quality = desc->sampleQuality;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		HRESULT result = Engine::graphics()->device()->CreateTexture2D(&textureDesc, nullptr, &buffer);

		if (FAILED(result)) {
			throw std::exception("DXGLSwapChain buffer could not be created.");
		}

		return buffer;
	} else {
		ID3D11Texture2D* buffer = nullptr;
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_width;
		depthStencilDesc.Height = m_height;
		depthStencilDesc.Format = format;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.SampleDesc.Count = desc->sampleCount;
		depthStencilDesc.SampleDesc.Quality = desc->sampleQuality;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
		dsv_desc.Format = format;
		dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsv_desc.Texture2D.MipSlice = 0;
		dsv_desc.Flags = 0;

		HRESULT result = Engine::graphics()->device()->CreateTexture2D(&depthStencilDesc, nullptr, &buffer);

		if (FAILED(result)) {
			throw std::exception("DXGLSwapChain RTV could not be created.");
		}

		return buffer;
	}
}

SP_DXGLRenderTargetView SwapChain::getRTV(ResourceViewSlot slot) {
	return *m_rtvs[slot].target;
}

SP_DXGLDepthStencilView SwapChain::getDSV(ResourceViewSlot slot) {
	return *m_dsvs[slot].target;
}

SP_DXGLShaderResourceView SwapChain::getRenderToTexture(ResourceViewSlot slot) {

	ID3D11ShaderResourceView* renderToTexture = nullptr;

	ID3D11Texture2D* resolvedBuffer = createBuffer(&m_rtvs[slot].desc, true);

	DXGI_FORMAT format = (DXGI_FORMAT)m_rtvs[slot].desc.format;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	HRESULT result = Engine::graphics()->device()->CreateShaderResourceView(resolvedBuffer, &srvDesc, &renderToTexture);

	if (FAILED(result)) {
		throw std::exception("DXGLSwapChain Resolved SRV could not be created.");
	}

	ID3D11Texture2D* buffer = nullptr;
	if (slot == RESOURCE_VIEW_SLOT_BACK_BUFFER) {
		result = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buffer);
	} else {
		ID3D11Resource* resource = nullptr;
		m_rtvs[slot].target->get()->get()->GetResource(&resource);

		// Query the buffer from the resource
		result = resource->QueryInterface<ID3D11Texture2D>(&buffer);

		// Release the resource interface
		resource->Release();
	}

	if (FAILED(result)) {
		throw std::exception("DXGLSwapChain Resolved could not get SwapChain buffer.");
	}

	Engine::graphics()->context()->ResolveSubresource(resolvedBuffer, 0, buffer, 0, format);

	resolvedBuffer->Release();
	buffer->Release();

	return std::make_shared<DXGLShaderResourceView>(renderToTexture);
}
