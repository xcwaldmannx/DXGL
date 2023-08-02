#include "DXGLMousePicker.h"

#include "DXGLComponentTypes.h"

using namespace dxgl;

DXGLMousePicker::DXGLMousePicker() {
	InputLayoutDesc desc{};
	desc.add("POSITION", 0, FLOAT3, false);
	desc.add("INSTANCE_ID", 1, UINT1, true);
	desc.add("INSTANCE_SCALE", 1, FLOAT3, true);
	desc.add("INSTANCE_ROTATION", 1, FLOAT3, true);
	desc.add("INSTANCE_TRANSLATION", 1, FLOAT3, true);
	desc.add("INSTANCE_FLAGS", 1, SINT1, true);
	m_layout = DXGLMain::resource()->createInputLayout(desc, "Assets/Shaders/VS_MousePick.cso");
	m_vs = DXGLMain::resource()->createShader<dxgl::DXGLVertexShader>("Assets/Shaders/VS_MousePick.cso");
	m_ps = DXGLMain::resource()->createShader<dxgl::DXGLPixelShader>("Assets/Shaders/PS_MousePick.cso");

	m_cbTrans = DXGLMain::resource()->createVSConstantBuffer(sizeof(TransformBuffer));

}

DXGLMousePicker::~DXGLMousePicker() {
}

void DXGLMousePicker::create() {
	DXGLMain::governor()->group<TransformComponent, MeshComponent>(governor::GroupSort::GROUP_ANY, m_groupEntity);
	DXGLMain::governor()->group<PickableComponent>(dxgl::governor::GroupSort::GROUP_ANY, m_groupPickable);

	// mouse pick RTV
	RESOURCE_VIEW_DESC rtvDesc = {
		UNORM8,
		1,
		0,
	};
	DXGLMain::renderer()->createRenderTargetView(&rtvDesc, RESOURCE_VIEW_SLOT_1, &m_rtv);

	RESOURCE_VIEW_DESC dsvDesc = {
		D24_S8,
		1,
		0,
	};
	DXGLMain::renderer()->createDepthStencilView(&dsvDesc, RESOURCE_VIEW_SLOT_1, &m_dsv);
}

#include <iostream>

void DXGLMousePicker::update() {
	// frustum cull check
	m_visibleEntities = {};
	for (governor::EntityId id : *m_groupEntity) {
		auto& transform = DXGLMain::governor()->getEntityComponent<TransformComponent>(id);
		auto& mesh = DXGLMain::governor()->getEntityComponent<MeshComponent>(id);

		if (!DXGLMain::renderer()->camera()->get("primary")->cull(transform.translation, transform.scale,
			mesh.mesh->getAABB().min, mesh.mesh->getAABB().max)) {
			m_visibleEntities.push_back(id);
		}
	}
}

void DXGLMousePicker::draw() {
	float color[4] = { 1, 1, 1, 1 };
	DXGLMain::renderer()->raster()->RS_setState(nullptr);
	DXGLMain::renderer()->setRenderTarget(m_rtv, color, m_dsv);

	// map entities containing same meshes to that mesh
	std::unordered_map<SP_Mesh, governor::DXGLGroup> meshGroups{};
	for (governor::EntityId id : m_visibleEntities) {
		auto& mesh = DXGLMain::governor()->getEntityComponent<MeshComponent>(id);
		if (meshGroups.find(mesh.mesh) != meshGroups.end()) {
			meshGroups[mesh.mesh].push_back(id);
		}
		else {
			meshGroups[mesh.mesh] = {};
			meshGroups[mesh.mesh].push_back(id);
		}
	}

	// set inputs that won't change per draw call
	DXGLMain::renderer()->input()->setInputLayout(m_layout);
	DXGLMain::renderer()->shader()->VS_setShader(m_vs);
	DXGLMain::renderer()->shader()->HS_setShader(nullptr);
	DXGLMain::renderer()->shader()->DS_setShader(nullptr);
	DXGLMain::renderer()->shader()->PS_setShader(m_ps);

	for (auto group = meshGroups.begin(); group != meshGroups.end(); group++) {
		const SP_Mesh& mesh = group->first;
		const governor::DXGLGroup& entities = group->second;

		// set world transform to zero
		dxgl::TransformBuffer tbuff{};
		tbuff.world.setIdentity();
		tbuff.view = DXGLMain::renderer()->camera()->get("primary")->view();
		tbuff.proj = DXGLMain::renderer()->camera()->get("primary")->proj();
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
		SP_InstanceBuffer buffer = DXGLMain::resource()->createInstanceBuffer(&entityData[0], entities.size(), sizeof(InstanceData));

		// set appropriate input data
		DXGLMain::renderer()->input()->setVertexBuffer(0, 1, &mesh->getMeshVertexBuffer());
		DXGLMain::renderer()->input()->setInstanceBuffers(1, &buffer);
		DXGLMain::renderer()->input()->setIndexBuffer(mesh->getIndexBuffer());

		// draw entities based on mesh material

		std::vector<BasicMesh> meshes = mesh->getMeshes();
		if (meshes.size() > 0) {
			for (int i = 0; i < meshes.size(); i++) {
				//dxgl::MeshMaterialSlot mat = mesh->getMaterials()[i];
				SP_Material material = DXGLMain::resource()->get<SP_Material>(meshes[i].materialName);
				DXGLMain::renderer()->drawIndexedTriangleListInstanced(meshes[i].indexCount, entities.size(), meshes[i].baseIndex, meshes[i].baseVertex, 0);
			}
		}
	}
}

uint32_t DXGLMousePicker::getColorId(Point2f position) {

	SP_DXGLShaderResourceView tex = DXGLMain::renderer()->getRenderToTexture(RESOURCE_VIEW_SLOT_1);

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

	hr = DXGLMain::graphics()->device()->CreateTexture2D(&stagingDesc, nullptr, &pStagingTexture);

	if (FAILED(hr)) {
		throw std::runtime_error("Could not create texture.");
	}

	// Copy the data from the original resource to the staging texture
	DXGLMain::graphics()->context()->CopySubresourceRegion(pStagingTexture, 0, 0, 0, 0, pOriginalResource, 0, nullptr);


	// Map the staging texture to read the pixel data
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	hr = DXGLMain::graphics()->context()->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);

	if (FAILED(hr)) {
		throw std::runtime_error("Could not map resource.");
	}

	// get the pixel data
	uint32_t* data = reinterpret_cast<uint32_t*>(mappedResource.pData);
	UINT rowPitch = mappedResource.RowPitch / sizeof(uint32_t);

	UINT offset = (position.y * rowPitch) + position.x;

	// Read the color value
	uint32_t color = *(data + offset);

	int red   = static_cast<int>((color & 0xFF));
	int green = static_cast<int>(((color >> 8) & 0xFF));
	int blue  = static_cast<int>(((color >> 16) & 0xFF));

	// Pack the color components into a 32-bit format
	uint32_t colorId = (red << 16) | (green << 8) | blue;

	colorId /= 65536;

	// unmap when done with resource
	DXGLMain::graphics()->context()->Unmap(pStagingTexture, 0);

	// Release resources
	pOriginalResource->Release();
	pStagingTexture->Release();

	// set selected entity
	for (auto& id : *m_groupPickable) {
		auto& component = DXGLMain::governor()->getEntityComponent<PickableComponent>(id);
		component.isSelected = false;
		if (id == colorId) {
			component.isSelected = true;
		}
	}

	return colorId;
}
