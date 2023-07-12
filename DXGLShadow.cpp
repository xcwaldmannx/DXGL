#include "DXGLShadow.h"

#include <algorithm>

#include "DXGLComponentTypes.h"

using namespace dxgl;

DXGLShadow::DXGLShadow() {
	InputLayoutDesc desc{};
	desc.add("POSITION", 0, FLOAT3, false);
	desc.add("INSTANCE_ID", 1, UINT1, true);
	desc.add("INSTANCE_SCALE", 1, FLOAT3, true);
	desc.add("INSTANCE_ROTATION", 1, FLOAT3, true);
	desc.add("INSTANCE_TRANSLATION", 1, FLOAT3, true);
	desc.add("INSTANCE_FLAGS", 1, SINT1, true);
	m_layout = DXGLMain::resource()->createInputLayout(desc, "Assets/Shaders/VS_Simple.cso");
	m_vs = DXGLMain::resource()->createShader<dxgl::DXGLVertexShader>("Assets/Shaders/VS_Simple.cso");
	m_ps = DXGLMain::resource()->createShader<dxgl::DXGLPixelShader>("Assets/Shaders/PS_Simple.cso");

	m_cbTrans = DXGLMain::resource()->createCBuffer(sizeof(TransformBuffer));
	m_cbShadow = DXGLMain::resource()->createCBuffer(sizeof(ShadowBuffer));

	m_srvs = new ID3D11ShaderResourceView*[4];

	createCascades();

}

DXGLShadow::~DXGLShadow() {
	for (ShadowCascade s : m_cascades) {
		s.srv->Release();
	}

	m_srvs = nullptr;
}

void DXGLShadow::create() {
	DXGLMain::governor()->group<TransformComponent, MeshComponent>(governor::GroupSort::GROUP_ANY, m_groupEntity);
}

void  DXGLShadow::update(Vec3f position, Vec3f target) {


	ShadowBuffer sb{};

	for (int i = 0; i < m_cascades.size(); i++) {
		m_cascades[i].buffer.view.lookAt(position, target, Vec3f{0, 1, 0});
		m_cascades[i].buffer.proj.setOrthographic(m_cascades[i].scale, m_cascades[i].scale, 0.01f, 1000.0f);

		sb.cascades[i] = m_cascades[i].buffer;
	}

	m_cbShadow->update(&sb);

	m_visibleEntities = {};
	//governor::DXGLGroup groupInRange = DXGLMain::renderer()->governor()->group<TransformComponent, MeshComponent>(dxgl::GroupSort::GROUP_ANY);
	for (governor::EntityId id : *m_groupEntity) {
		auto& transform = DXGLMain::governor()->getEntityComponent<TransformComponent>(id);
		SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

		if (Vec3f::dist(cam->getPosition(), transform.translation) < 500.0f) {
			m_visibleEntities.push_back(id);
		}
	}

	// map entities containing same meshes to that mesh
	m_meshGroups = {};
	for (governor::EntityId id : m_visibleEntities) {
		auto& mesh = DXGLMain::governor()->getEntityComponent<MeshComponent>(id);

		if ((mesh.instanceFlags & INSTANCE_USE_SHADOWING) == 0) {
			continue;
		}

		if (m_meshGroups.find(mesh.mesh) != m_meshGroups.end()) {
			m_meshGroups[mesh.mesh].push_back(id);
		}
		else {
			m_meshGroups[mesh.mesh] = {};
			m_meshGroups[mesh.mesh].push_back(id);
		}
	}
}

void DXGLShadow::draw() {
	for (int i = 0; i < m_cascades.size(); i++) {
		D3D11_VIEWPORT shadowViewport{};
		shadowViewport.TopLeftX = 0;
		shadowViewport.TopLeftY = 0;
		shadowViewport.Width = m_cascades[i].buffer.resolution;
		shadowViewport.Height = m_cascades[i].buffer.resolution;
		shadowViewport.MinDepth = 0.0f;
		shadowViewport.MaxDepth = 1.0f;

		DXGLMain::graphics()->context()->RSSetViewports(1, &shadowViewport);

		float color[4] = { 0, 0, 0, 1 };
		DXGLMain::renderer()->setRenderTarget(nullptr, color, m_cascades[i].dsv);

		// set shadow cbuffer
		DXGLMain::renderer()->shader()->VS_setCBuffer(1, 1, m_cbShadow->get());
		DXGLMain::renderer()->shader()->PS_setCBuffer(1, 1, m_cbShadow->get());

		// set inputs that won't change per draw call
		DXGLMain::renderer()->input()->setInputLayout(m_layout);
		DXGLMain::renderer()->shader()->VS_setShader(m_vs);
		DXGLMain::renderer()->shader()->HS_setShader(nullptr);
		DXGLMain::renderer()->shader()->DS_setShader(nullptr);
		DXGLMain::renderer()->shader()->PS_setShader(m_ps);

		for (auto group = m_meshGroups.begin(); group != m_meshGroups.end(); group++) {
			const SP_DXGLMesh& mesh = group->first;
			const governor::DXGLGroup& entities = group->second;

			// set world transform to zero
			dxgl::TransformBuffer tbuff{};
			tbuff.world.setIdentity();
			tbuff.view = m_cascades[i].buffer.view;
			tbuff.proj = m_cascades[i].buffer.proj;
			m_cbTrans->update(&tbuff);
			DXGLMain::renderer()->shader()->VS_setCBuffer(0, 1, m_cbTrans->get());

			std::vector<InstanceData> entityData{};
			for (governor::EntityId id : entities) {
				auto& transform = DXGLMain::governor()->getEntityComponent<TransformComponent>(id);
				auto& mesh = DXGLMain::governor()->getEntityComponent<MeshComponent>(id);
				InstanceData data{};
				data.id = id;
				data.scale = transform.scale;
				data.rotation = transform.rotation;
				data.translation = transform.translation;
				data.flags = mesh.instanceFlags;
				entityData.push_back(data);
			}

			// create an instance buffer for each group of entities
			SP_DXGLInstanceBuffer buffer = DXGLMain::resource()->createInstanceBuffer(&entityData[0], entities.size(), sizeof(InstanceData));

			// set appropriate input data
			DXGLMain::renderer()->input()->setVertexBuffer(0, 1, &mesh->getVertexBuffer());
			DXGLMain::renderer()->input()->setInstanceBuffers(1, &buffer);
			DXGLMain::renderer()->input()->setIndexBuffer(mesh->getIndexBuffer());

			// draw entities based on mesh material
			DXGLMain::renderer()->drawIndexedTriangleListInstanced(mesh->getIndices().size(), entities.size(), 0, 0, 0);
		}
	}
}

ID3D11ShaderResourceView** DXGLShadow::getSRVs() {
	for (int i = 0; i < m_cascades.size(); i++) {
		m_srvs[i] = m_cascades[i].srv;
	}
	return m_srvs;
}

void DXGLShadow::createCascades() {
	int cascadeCount = 4;
	for (int i = 0; i < cascadeCount; i++) {
		ShadowCascade cascade{};
		cascade.scale = 64 + i * 64;
		cascade.buffer.resolution = (cascadeCount - i) * 512;
		cascade.buffer.pcfSampleCount = std::min<int>(4, (cascadeCount - i) + 2);

		D3D11_VIEWPORT shadowViewport{};
		shadowViewport.TopLeftX = 0;
		shadowViewport.TopLeftY = 0;
		shadowViewport.Width = cascade.buffer.resolution;
		shadowViewport.Height = cascade.buffer.resolution;
		shadowViewport.MinDepth = 0.0f;
		shadowViewport.MaxDepth = 1.0f;

		DXGLMain::graphics()->context()->RSSetViewports(1, &shadowViewport);

		ID3D11Texture2D* dsvBuffer = nullptr;
		D3D11_TEXTURE2D_DESC dsvTexDesc{};
		dsvTexDesc.Width = cascade.buffer.resolution;
		dsvTexDesc.Height = cascade.buffer.resolution;
		dsvTexDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		dsvTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		dsvTexDesc.Usage = D3D11_USAGE_DEFAULT;
		dsvTexDesc.MipLevels = 1;
		dsvTexDesc.ArraySize = 1;
		dsvTexDesc.SampleDesc.Count = 1;
		dsvTexDesc.SampleDesc.Quality = 0;
		dsvTexDesc.MiscFlags = 0;
		dsvTexDesc.CPUAccessFlags = 0;


		HRESULT result = DXGLMain::graphics()->device()->CreateTexture2D(&dsvTexDesc, nullptr, &dsvBuffer);

		if (FAILED(result)) {
			throw std::exception("DXGLShadow DepthStencilView Texture2D could not be created.");
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		dsvDesc.Flags = 0;

		ID3D11DepthStencilView* dsv = nullptr;
		result = DXGLMain::graphics()->device()->CreateDepthStencilView(dsvBuffer, &dsvDesc, &dsv);

		cascade.dsv = std::make_shared<DXGLDepthStencilView>(dsv);

		if (FAILED(result)) {
			throw std::exception("DXGLShadow DepthStencilView could not be created.");
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		result = DXGLMain::graphics()->device()->CreateShaderResourceView(dsvBuffer, &srvDesc, &cascade.srv);

		if (FAILED(result)) {
			throw std::exception("DXGLShadow ShaderResourceView could not be created.");
		}

		dsvBuffer->Release();

		m_cascades.push_back(cascade);
	}
}
