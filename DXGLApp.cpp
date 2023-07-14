#include "DXGLApp.h"

#include <iostream>

#include <chrono>

// uses buffer zero
struct alignas(16) EntityBuffer {
	// projection of entity
	Mat4f entityWorld{};
	Mat4f entityView{};
	Mat4f entityProj{};

	Mat4f boneSpace[100];

	// camera data
	Vec3f camPosition{};
	float pad0;
	Vec3f camDirection{};
	float pad1;

	// screen data
	float width = 0;
	float height = 0;
	float pad2[2];

	// global flags for shader
	int globalFlags = 0;
	float pad3[3];
};

DXGLApp::DXGLApp() {
}

DXGLApp::~DXGLApp() {
}

void DXGLApp::create() {

	// back buffer RTV and DSV
	RESOURCE_VIEW_DESC rtvDesc = {
		FLOAT16,
		1,
		0,
	};
	renderer()->createRenderTargetView(&rtvDesc, RESOURCE_VIEW_SLOT_BACK_BUFFER, &m_backBufferRTV);

	RESOURCE_VIEW_DESC dsvDesc = {
		D24_S8,
		4,
		D3D11_STANDARD_MULTISAMPLE_PATTERN,
	};
	renderer()->createDepthStencilView(&dsvDesc, RESOURCE_VIEW_SLOT_BACK_BUFFER, &m_backBufferDSV);

	dxgl::InputLayoutDesc desc{};
	desc.add("POSITION", 0, FLOAT3, false);
	desc.add("TEXCOORD", 0, FLOAT2, false);
	desc.add("NORMAL", 0, FLOAT3, false);
	desc.add("TANGENT", 0, FLOAT3, false);
	resource()->storeInputLayout(desc, "Assets/Shaders/VS_Shadow.cso", "simpleLayout");

	dxgl::InputLayoutDesc descInst{};
	descInst.add("POSITION", 0, FLOAT3, false);
	descInst.add("TEXCOORD", 0, FLOAT2, false);
	descInst.add("NORMAL", 0, FLOAT3, false);
	descInst.add("TANGENT", 0, FLOAT3, false);
	descInst.add("INSTANCE_ID", 1, UINT1, true);
	descInst.add("INSTANCE_SCALE", 1, FLOAT3, true);
	descInst.add("INSTANCE_ROTATION", 1, FLOAT3, true);
	descInst.add("INSTANCE_TRANSLATION", 1, FLOAT3, true);
	descInst.add("INSTANCE_FLAGS", 1, SINT1, true);
	//resource()->storeInputLayout(descInst, "Assets/Shaders/VS_Instance.cso", "instanceLayout");

	dxgl::InputLayoutDesc descFbx{};
	descFbx.add("POSITION", 0, FLOAT3, false);
	descFbx.add("TEXCOORD", 0, FLOAT2, false);
	descFbx.add("NORMAL",   0, FLOAT3, false);
	descFbx.add("TANGENT",  0, FLOAT3, false);

	descFbx.add("BONE_ID",     2, UINT4, false);
	descFbx.add("BONE_WEIGHT", 2, FLOAT4, false);

	descFbx.add("INSTANCE_ID",          1, UINT1, true);
	descFbx.add("INSTANCE_SCALE",       1, FLOAT3, true);
	descFbx.add("INSTANCE_ROTATION",    1, FLOAT3, true);
	descFbx.add("INSTANCE_TRANSLATION", 1, FLOAT3, true);
	descFbx.add("INSTANCE_FLAGS",       1, SINT1, true);
	resource()->storeInputLayout(descFbx, "Assets/Shaders/VS_Instance.cso", "fbxLayout");

	DXGLShaderSet shaderSet = {
		renderer()->shader()->create<dxgl::DXGLVertexShader>("Assets/Shaders/VS_Simple.cso"),
		nullptr,
		nullptr,
		renderer()->shader()->create<dxgl::DXGLPixelShader>("Assets/Shaders/PS_Simple.cso"),
	};
	renderer()->shader()->addSet(shaderSet, "simple");

	shaderSet = {
		renderer()->shader()->create<dxgl::DXGLVertexShader>("Assets/Shaders/VS_MousePick.cso"),
		nullptr,
		nullptr,
		renderer()->shader()->create<dxgl::DXGLPixelShader>("Assets/Shaders/PS_MousePick.cso"),
	};
	renderer()->shader()->addSet(shaderSet, "mousepick");

	shaderSet = {
		renderer()->shader()->create<dxgl::DXGLVertexShader>("Assets/Shaders/VS_Instance.cso"),
		nullptr,
		nullptr,
		renderer()->shader()->create<dxgl::DXGLPixelShader>("Assets/Shaders/PS_Instance.cso"),
	};
	renderer()->shader()->addSet(shaderSet, "instance");

	shaderSet = {
		renderer()->shader()->create<dxgl::DXGLVertexShader>("Assets/Shaders/VS_Tessellation.cso"),
		renderer()->shader()->create<dxgl::DXGLHullShader>("Assets/Shaders/HullShader.cso"),
		renderer()->shader()->create<dxgl::DXGLDomainShader>("Assets/Shaders/DomainShader.cso"),
		renderer()->shader()->create<dxgl::DXGLPixelShader>("Assets/Shaders/PS_Tessellation.cso"),
	};
	renderer()->shader()->addSet(shaderSet, "tessellation");

	resource()->storeTexture2D("Assets/Textures/brdf.png", "brdf");

	resource()->storeMaterial("Assets/Materials/default/", "None");
	resource()->storeMaterial("Assets/Materials/barren-ground-rock/", "rock");
	resource()->storeMaterial("Assets/Materials/pine-tree-bark/", "bark");
	resource()->storeMaterial("Assets/Materials/metal-grid/", "metal");
	resource()->storeMaterial("Assets/Materials/gray-granite/", "granite");
	resource()->storeMaterial("Assets/Materials/military-panel/", "panel");
	resource()->storeMaterial("Assets/Materials/greasy-pan/", "greasy");

	resource()->storeMesh("Assets/Meshes/landscapes/landscape_01.obj", "landscape");
	resource()->storeMesh("Assets/Meshes/sphereFlipped.obj", "sphereFlipped");
	resource()->storeMesh("Assets/Meshes/light_pillar.obj", "light_pillar");

	resource()->storeMesh("Assets/Meshes/shapes/cube.obj", "cube");
	resource()->storeMesh("Assets/Meshes/shapes/sphere.obj", "sphere");
	resource()->storeMesh("Assets/Meshes/shapes/cone.obj", "cone");
	resource()->storeMesh("Assets/Meshes/shapes/cylinder.obj", "cylinder");
	resource()->storeMesh("Assets/Meshes/shapes/torus.obj", "torus");

	resource()->storeMesh("Assets/Meshes/person/person_01.obj", "person");

	m_shapes.push_back("cube");
	m_shapes.push_back("sphere");
	m_shapes.push_back("cone");
	m_shapes.push_back("cylinder");
	m_shapes.push_back("torus");

	m_camera = renderer()->camera()->create(this, input(), "primary");
	m_camera->world().setTranslation(Vec3f{ 0, 0, -5.0f });

	m_cbEntityBuffer = resource()->createCBuffer(sizeof(EntityBuffer));

	// Texture SamplerState
	resource()->storeSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0, "textureSampler");

	renderer()->shader()->DS_setSampler(0, 1, resource()->get<SP_DXGLSamplerState>("textureSampler"));
	renderer()->shader()->PS_setSampler(0, 1, resource()->get<SP_DXGLSamplerState>("textureSampler"));

	// Shadow SamplerState
	resource()->storeSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_ALWAYS, 1, "shadowSampler");
	renderer()->setSamplerState(1, 1, resource()->get<SP_DXGLSamplerState>("shadowSampler")->get());

	m_camera->update(0);

	// setup shadow updates
	shadowTimer.every(25, [&]() {
		float camX = m_camera->getPosition().x;
		float camY = m_camera->getPosition().y;
		float camZ = m_camera->getPosition().z;

		float x = camX;
		float y = 512 * sin(m_timePassed / 100.0f);
		float z = camZ + 512 * cos(m_timePassed / 100.0f);

		renderer()->shadow()->update(Vec3f{ x, y, z }, Vec3f{ camX, 0, camZ });
		renderer()->shadow()->draw();
		});
	//shadowTimer.start();

	// rasterizer states
	D3D11_RASTERIZER_DESC descWireframe{};
	descWireframe.FillMode = D3D11_FILL_WIREFRAME;
	descWireframe.CullMode = D3D11_CULL_NONE;
	descWireframe.FrontCounterClockwise = false;
	descWireframe.DepthClipEnable = true;
	descWireframe.MultisampleEnable = true;
	descWireframe.AntialiasedLineEnable = false;
	renderer()->raster()->create(descWireframe, "wireframe");

	// rasterizer states
	D3D11_RASTERIZER_DESC descCullNone{};
	descCullNone.FillMode = D3D11_FILL_SOLID;
	descCullNone.CullMode = D3D11_CULL_NONE;
	descCullNone.FrontCounterClockwise = false;
	descCullNone.DepthClipEnable = true;
	descCullNone.MultisampleEnable = true;
	descCullNone.AntialiasedLineEnable = false;
	renderer()->raster()->create(descCullNone, "cull_none");
	
	D3D11_RASTERIZER_DESC descCullBack{};
	descCullBack.FillMode = D3D11_FILL_SOLID;
	descCullBack.CullMode = D3D11_CULL_BACK;
	descCullBack.FrontCounterClockwise = false;
	descCullBack.DepthClipEnable = true;
	descCullBack.MultisampleEnable = true;
	descCullBack.AntialiasedLineEnable = false;
	renderer()->raster()->create(descCullBack, "cull_back");

	// lights
	
	dxgl::Light light{};
	light.position = Vec3f{ -16.0f, 4.0f, -12.0f };
	light.color = Vec3f{ 10, 0, 0 };
	renderer()->light()->addLight(light);

	light = {};
	light.position = Vec3f{-8.0f, 4.0f, -12.0f};
	light.color = Vec3f{ 20, 20, 0 };
	renderer()->light()->addLight(light);

	light = {};
	light.position = Vec3f{ 8.0f, 4.0f, -12.0f };
	light.color = Vec3f{ 0, 30, 0 };
	renderer()->light()->addLight(light);

	light = {};
	light.position = Vec3f{ 16.0f, 4.0f, -12.0f };
	light.color = Vec3f{ 0, 0, 10 };
	renderer()->light()->addLight(light);

	light = {};
	light.direction = Vec3f{ 0, -1, 1 };
	light.color = Vec3f{ 1, 1, 1 };
	renderer()->light()->addLight(light);

	// ECS
	governor()->registerComponent<TransformComponent>();
	governor()->registerComponent<MeshComponent>();
	governor()->registerComponent<PickableComponent>();
	governor()->registerComponent<FoliageComponent>();

	governor()->registerComponent<VS_CBufferComponent>();
	governor()->registerComponent<HS_CBufferComponent>();
	governor()->registerComponent<DS_CBufferComponent>();
	governor()->registerComponent<PS_CBufferComponent>();

	// shapes
	for (int i = -5; i < 5; i++) {
		for (int j = -5; j < 5; j++) {
			dxgl::governor::EntityId id = governor()->createEntity();

			TransformComponent transform{};
			transform.scale = { 0.5f + std::rand() % 2, 0.5f + std::rand() % 2, 0.5f + std::rand() % 2 };
			float rx = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float) (std::rand() % 6);
			float ry = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float) (std::rand() % 6);
			float rz = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float) (std::rand() % 6);
			transform.rotation = { rx, ry, rz };
			transform.translation = { (float) i * 4, 15, (float) j * 4 };
			governor()->addEntityComponent<TransformComponent>(transform, id);

			MeshComponent mesh{};
			mesh.mesh = resource()->get<SP_DXGLMesh>(m_shapes[std::rand() % 5]);
			mesh.useTessellation = false;
			mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;
			governor()->addEntityComponent<MeshComponent>(mesh, id);

			PickableComponent pickable{};
			pickable.isSelected = false;
			governor()->addEntityComponent<PickableComponent>(pickable, id);
		}
	}

	// landscape
	{
		MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_ALL;
		resource()->storeBasicMesh(desc, "Assets/Meshes/landscapes/landscape.fbx", "landscape");

		dxgl::governor::EntityId id = governor()->createEntity();

		TransformComponent transform{};
		transform.scale = { 10, 10, 10 };
		transform.rotation = { 0, 0, 0 };
		transform.translation = { 0, 0, 0 };
		governor()->addEntityComponent<TransformComponent>(transform, id);

		MeshComponent mesh{};
		mesh.mesh = nullptr;
		mesh.basicmesh = resource()->get<SP_DXGLBasicMesh>("landscape");
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;
		governor()->addEntityComponent<MeshComponent>(mesh, id);
	}

	// grass

	for (int i = 0; i < 500; i++) {
		for (int j = 0; j < 500; j++) {
			FoliageInstance foliage{};
			float scale = 0.25f + (std::rand() % 25) * 0.01f;
			foliage.scale = Vec3f{ 0.1f, scale, 0.1f };

			float rotation = (std::rand() % 314) * 0.01f;
			foliage.rotation = Vec3f{ 0, rotation, 0 };

			float x = (std::rand() % 125) * 0.001f;
			float y = (std::rand() % 125) * 0.001f;
			foliage.translation = Vec3f{(float) i * 0.125f + x, 0, (float) j * 0.125f + y };

			foliage.color0 = Vec3f{ 0.1f, 0.8f, 0.0f };
			foliage.color1 = Vec3f{ 0.0f, 0.6f, 0.0f };
			foliage.color2 = Vec3f{ 0.0f, 0.4f, 0.0f };
			foliage.color3 = Vec3f{ 0.0f, 0.2f, 0.0f };

			foliage.timeOffset = i * 0.025f + j * 0.025f;
			renderer()->foliage()->add(foliage);
		}
	}

	{ // guitar
		MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_ALL;
		resource()->storeBasicMesh(desc, "Assets/Meshes/material test cube/explorer guitar.fbx", "guitar");
		resource()->storeBasicMesh(desc, "Assets/Meshes/material test cube/cube.fbx", "cube02");

		m_guitar = governor()->createEntity();

		TransformComponent transform{};
		transform.scale = { 1, 1, 1 };
		transform.rotation = { 0, 0, 0 };
		transform.translation = { 0, 0, 0 };
		governor()->addEntityComponent<TransformComponent>(transform, m_guitar);

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_DXGLMesh>("person");
		mesh.basicmesh = resource()->get<SP_DXGLBasicMesh>("guitar");
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;
		governor()->addEntityComponent<MeshComponent>(mesh, m_guitar);

		PickableComponent pickable{};
		pickable.isSelected = false;
		governor()->addEntityComponent<PickableComponent>(pickable, m_guitar);
	}

	// gun
	{
		MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_ALL;
		m_fbxMesh = resource()->createBasicMesh(desc, "Assets/Meshes/material test cube/gun.fbx");

		m_gun = governor()->createEntity();

		TransformComponent transform{};
		transform.scale = { 1, 1, 1 };
		transform.rotation = { 0, 0, 0 };
		transform.translation = { 0, 0, 0 };
		governor()->addEntityComponent<TransformComponent>(transform, m_gun);

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_DXGLMesh>("person");
		mesh.basicmesh = m_fbxMesh;
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;
		governor()->addEntityComponent<MeshComponent>(mesh, m_gun);

		PickableComponent pickable{};
		pickable.isSelected = false;
		governor()->addEntityComponent<PickableComponent>(pickable, m_gun);
	}

	governor()->group<TransformComponent, MeshComponent>(dxgl::governor::GroupSort::GROUP_ANY, m_groupEntity);
	governor()->group<PickableComponent>(dxgl::governor::GroupSort::GROUP_ANY, m_groupPickable);
	governor()->group<FoliageComponent>(dxgl::governor::GroupSort::GROUP_ANY, m_groupFoliage);

	// Depth Stencils
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	renderer()->merger()->createDepthStencil(depthStencilDesc, "basic");
}

void DXGLApp::update(long double delta) {
	m_timeDelta = delta;
	m_timePassed += m_timeDelta;

	if (input()->getKeyPressState(VK_ESCAPE)) {
		quit();
	}

	if (input()->getKeyTapState('F')) {
		m_fullscreen = !m_fullscreen;
		std::cout << "FULLSCREEN: " << m_fullscreen << "\n";
		renderer()->setFullscreen(m_fullscreen);
	}

	if (input()->getKeyTapState('P')) {
		m_playState = !m_playState;
		dxgl::DXGLInputSystem::get()->showCursor(!m_playState);
	}

	if (m_playState) {
		POINT center = getWindowCenter();
		dxgl::DXGLInputSystem::get()->setCursorPosition(Point2f{ (float) center.x, (float) center.y });
		m_camera->update(delta);
	}

	RECT dim = getScreenSize();
	UINT width = dim.right - dim.left;
	UINT height = dim.bottom - dim.top;

	m_postProcessor.update(delta, width, height);

	m_skybox.update(delta);

	renderer()->foliage()->update(delta);

	// add entity start

	if (input()->getKeyTapState('E')) {
		dxgl::governor::EntityId id = governor()->createEntity();

		TransformComponent transform{};
		transform.scale = { 1, 1, 1 };
		float rx = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
		float ry = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
		float rz = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
		transform.rotation = { rx, ry, rz };
		transform.translation = m_camera->getPosition() + m_camera->getDirection() * 2.0f;
		governor()->addEntityComponent<TransformComponent>(transform, id);

		MeshComponent mesh{};
		//mesh.mesh = resource()->get<SP_DXGLMesh>(m_shapes[std::rand() % 5]);
		mesh.basicmesh = m_fbxMesh;
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;
		governor()->addEntityComponent<MeshComponent>(mesh, id);

		PickableComponent pickable{};
		pickable.isSelected = false;
		governor()->addEntityComponent<PickableComponent>(pickable, id);
	}

	// add entity end

	// point gun start
	{
		Vec3f aim(-0.75f, -0.25f, 1.0f);
		if (input()->getMouseState(DXGLInputManager::RMB_STATE)) {
			aim = Vec3f(0.0f, -0.18f, 1.0f);
		}

		auto& transform = governor()->getEntityComponent<TransformComponent>(m_gun);
		transform.rotation = m_camera->world().getRotation();
		Vec3f offsetX = m_camera->world().getXDirection() * aim.x;
		Vec3f offsetY = m_camera->world().getYDirection() * aim.y;
		Vec3f offsetZ = m_camera->world().getZDirection() * aim.z;
		Vec3f offset = offsetX + offsetY + offsetZ;
		transform.translation = m_camera->world().getTranslation() + offset;
	}
	// point gun end

	// rotate guitar start
	
	{
		auto& transform = governor()->getEntityComponent<TransformComponent>(m_guitar);
		transform.rotation = Vec3f(0, m_timePassed, 0);
	}

	// rotate guitar end

	// process foliage start
	
	for (dxgl::governor::EntityId id : *m_groupFoliage) {
		auto& transform = governor()->getEntityComponent<TransformComponent>(id);
		auto& foliage = governor()->getEntityComponent<FoliageComponent>(id);

		float distToCam = Vec3f::dist(m_camera->getPosition(), transform.translation);
		transform.translation.y = foliage.position.y - (distToCam / foliage.size) * 3;

		if (std::abs(m_camera->getPosition().x - transform.translation.x) > (foliage.size / 2) && transform.translation.y < foliage.position.y - 1.0f) {
			if (transform.translation.x < m_camera->getPosition().x) {
				transform.translation.x += foliage.size;
			} else {
				transform.translation.x -= foliage.size;
			}
		}

		if (std::abs(m_camera->getPosition().z - transform.translation.z) > (foliage.size / 2) && transform.translation.y < foliage.position.y - 1.0f) {
			if (transform.translation.z < m_camera->getPosition().z) {
				transform.translation.z += foliage.size;
			}
			else {
				transform.translation.z -= foliage.size;
			}
		}
	}

	// process foliage end

	// frustum cull check
	m_visibleEntities = {};
	for (dxgl::governor::EntityId id : *m_groupEntity) {
		auto& transform = governor()->getEntityComponent<TransformComponent>(id);
		auto& mesh = governor()->getEntityComponent<MeshComponent>(id);

		if (mesh.basicmesh) {
			m_visibleEntities.push_back(id);
		} else

		if (!m_camera->cull(transform.translation, transform.scale, mesh.mesh->getAABBMin(), mesh.mesh->getAABBMax())) {
			m_visibleEntities.push_back(id);
		}
	}
}

void DXGLApp::draw() {

	renderer()->merger()->setDepthStencil("basic");
	shadowTimer.execute();

	RECT dim = getWindowSize();
	UINT width = dim.right- dim.left;
	UINT height = dim.bottom - dim.top;
	renderer()->setViewport(width, height);

	// mousepick start
	governor::EntityId selectedEntity = -1;
	if (input()->getKeyPressState('Q')) {
		renderer()->mousePicker()->update();
		renderer()->mousePicker()->draw();
		Point2f mouse = DXGLMain::input()->getMousePosition();
		POINT clientMouse = { mouse.x, mouse.y };
		ScreenToClient(getWindow(), &clientMouse);
		selectedEntity = renderer()->mousePicker()->getColorId(Point2f{(float) clientMouse.x, (float) clientMouse.y });
	}
	// mousepick end
	
	// render start

	float color[4] = { 1, 1, 1, 1 };
	renderer()->setRenderTarget(m_backBufferRTV, color, m_backBufferDSV);

	// set cascaded shadow textures
	renderer()->shader()->PS_setResources(6, 4, renderer()->shadow()->getSRVs());

	// Skybox
	m_skybox.draw();
	renderer()->shader()->PS_setResource(1, m_skybox.getCube()->get());
	renderer()->shader()->PS_setResource(2, resource()->get<SP_DXGLTexture2D>("brdf")->get());


	// render meshes
	renderer()->merger()->setDepthStencil("basic");

	// grass
	renderer()->raster()->RS_setState("cull_none");
	renderer()->foliage()->draw();
	renderer()->raster()->RS_setState("cull_back");

	if (input()->getKeyPressState('Z')) {
		renderer()->raster()->RS_setState("wireframe");
	}
	else {
		renderer()->raster()->RS_setState("cull_back");
	}

	// general cbuffers
	renderer()->shader()->VS_setCBuffer(0, 1, m_cbEntityBuffer->get());
	renderer()->shader()->HS_setCBuffer(0, 1, m_cbEntityBuffer->get());
	renderer()->shader()->PS_setCBuffer(0, 1, m_cbEntityBuffer->get());
	renderer()->shader()->PS_setCBuffer(3, 1, renderer()->light()->getBuffer()->get());

	// automatic instancing with assimp
	{
		// map entities containing same meshes to that mesh
		std::unordered_map<SP_DXGLBasicMesh, dxgl::governor::DXGLGroup> meshGroups{};
		for (dxgl::governor::EntityId id : m_visibleEntities) {
			auto& mesh = governor()->getEntityComponent<MeshComponent>(id);
			if (meshGroups.find(mesh.basicmesh) != meshGroups.end()) {
				meshGroups[mesh.basicmesh].push_back(id);
			} else {
				meshGroups[mesh.basicmesh] = {};
				meshGroups[mesh.basicmesh].push_back(id);
			}
		}

		// iterate each mesh group
		for (auto group = meshGroups.begin(); group != meshGroups.end(); group++) {
			if (!group->first) continue;

			const SP_DXGLBasicMesh& mesh = group->first;
			const dxgl::governor::DXGLGroup& entities = group->second;

			// set world transform to zero
			dxgl::TransformBuffer tbuff{};
			tbuff.world.setIdentity();
			tbuff.view = m_camera->view();
			tbuff.proj = m_camera->proj();

			// get boneSpace
			std::vector<Mat4f> boneSpace{};
			mesh->getBoneTransforms(0, m_timePassed, boneSpace);

			// update entity
			EntityBuffer ebuff{};
			ebuff.entityWorld = tbuff.world;
			ebuff.entityView = tbuff.view;
			ebuff.entityProj = tbuff.proj;
			std::copy(boneSpace.begin(), boneSpace.end(), ebuff.boneSpace);
			ebuff.camPosition = m_camera->getPosition();
			ebuff.camDirection = m_camera->getDirection();
			ebuff.width = width;
			ebuff.height = height;
			ebuff.globalFlags = (m_fullscreen ? GLOBAL_USE_FULLSCREEN : 0);
			m_cbEntityBuffer->update(&ebuff);

			// get all entity transform data
			bool useTessellation = false;

			std::vector<InstanceData> entityData{};
			for (dxgl::governor::EntityId id : entities) {
				auto& transform = governor()->getEntityComponent<TransformComponent>(id);
				auto& mesh = governor()->getEntityComponent<MeshComponent>(id);
				InstanceData data{};
				data.id = id;
				data.scale = transform.scale;
				data.rotation = transform.rotation;
				data.translation = transform.translation;
				data.flags = mesh.instanceFlags;

				if (id == selectedEntity) {
					data.flags |= INSTANCE_IS_SELECTED;
				}

				entityData.push_back(data);

				useTessellation = mesh.useTessellation;
			}

			// create an instance buffer for each group of entities
			SP_DXGLInstanceBuffer buffer = resource()->createInstanceBuffer(&entityData[0], entities.size(), sizeof(InstanceData));

			// set appropriate input data
			renderer()->input()->setInputLayout(resource()->get<SP_DXGLInputLayout>("fbxLayout"));
			renderer()->input()->setVertexBuffer(0, 1, &mesh->getMeshVertexBuffer());
			if (mesh->getBoneVertexBuffer())
				renderer()->input()->setVertexBuffer(2, 1, &mesh->getBoneVertexBuffer());
			renderer()->input()->setInstanceBuffers(1, &buffer);
			renderer()->input()->setIndexBuffer(mesh->getIndexBuffer());

			// set shaders
			if (useTessellation) {
				renderer()->shader()->setShaderSet("tessellation");
			}
			else {
				renderer()->shader()->setShaderSet("instance");
			}

			// draw entities based on mesh material
			std::vector<BasicMesh> meshes = mesh->getMeshes();
			if (meshes.size() > 0) {
				for (int i = 0; i < meshes.size(); i++) {
					//dxgl::MeshMaterialSlot mat = mesh->getMaterials()[i];
					SP_DXGLMaterial material = resource()->get<SP_DXGLMaterial>(meshes[i].materialName);
					renderer()->shader()->PS_setMaterial(0, 1, material);
					if (useTessellation) {
						renderer()->shader()->DS_setMaterial(0, 1, resource()->get<SP_DXGLMaterial>(meshes[i].materialName));
						renderer()->drawIndexedTriangleListInstancedTess(meshes[i].indexCount, entities.size(), meshes[i].baseIndex, meshes[i].baseVertex, 0);
					} else {
						renderer()->drawIndexedTriangleListInstanced(meshes[i].indexCount, entities.size(), meshes[i].baseIndex, meshes[i].baseVertex, 0);
					}
				}
			}
		}
	}

	// automatic instancing
	/*{
		// map entities containing same meshes to that mesh
		std::unordered_map<SP_DXGLMesh, dxgl::governor::DXGLGroup> meshGroups{};
		for (dxgl::governor::EntityId id : m_visibleEntities) {
			auto& mesh = governor()->getEntityComponent<MeshComponent>(id);
			if (meshGroups.find(mesh.mesh) != meshGroups.end()) {
				meshGroups[mesh.mesh].push_back(id);
			} else {
				meshGroups[mesh.mesh] = {};
				meshGroups[mesh.mesh].push_back(id);
			}
		}

		// iterate each mesh group
		for (auto group = meshGroups.begin(); group != meshGroups.end(); group++) {
			const SP_DXGLMesh& mesh = group->first;
			const dxgl::governor::DXGLGroup& entities = group->second;

			// set world transform to zero
			dxgl::TransformBuffer tbuff{};
			tbuff.world.setIdentity();
			tbuff.view = m_camera->view();
			tbuff.proj = m_camera->proj();

			// update entity
			EntityBuffer ebuff{};
			ebuff.entityWorld = tbuff.world;
			ebuff.entityView = tbuff.view;
			ebuff.entityProj = tbuff.proj;
			ebuff.camPosition = m_camera->getPosition();
			ebuff.camDirection = m_camera->getDirection();
			ebuff.width = width;
			ebuff.height = height;
			ebuff.globalFlags = (m_fullscreen ? GLOBAL_USE_FULLSCREEN : 0);
			m_cbEntityBuffer->update(&ebuff);

			// get all entity transform data
			bool useTessellation = false;

			std::vector<InstanceData> entityData{};
			for (dxgl::governor::EntityId id : entities) {
				auto& transform = governor()->getEntityComponent<TransformComponent>(id);
				auto& mesh = governor()->getEntityComponent<MeshComponent>(id);
				InstanceData data{};
				data.id = id;
				data.scale = transform.scale;
				data.rotation = transform.rotation;
				data.translation = transform.translation;
				data.flags = mesh.instanceFlags;

				if (id == selectedEntity) {
					data.flags |= INSTANCE_IS_SELECTED;
				}

				entityData.push_back(data);

				useTessellation = mesh.useTessellation;
			}

			// create an instance buffer for each group of entities
			SP_DXGLInstanceBuffer buffer = resource()->createInstanceBuffer(&entityData[0], entities.size(), sizeof(InstanceData));

			// set appropriate input data
			renderer()->input()->setInputLayout(resource()->get<SP_DXGLInputLayout>("instanceLayout"));
			renderer()->input()->setVertexBuffers(1, &mesh->getVertexBuffer());
			renderer()->input()->setInstanceBuffers(1, &buffer);
			renderer()->input()->setIndexBuffer(mesh->getIndexBuffer());

			// set shaders
			if (useTessellation) {
				renderer()->shader()->setShaderSet("tessellation");
			}
			else {
				renderer()->shader()->setShaderSet("instance");
			}

			// draw entities based on mesh material
			std::vector<dxgl::MeshMaterialSlot> mats = mesh->getMaterials();
			if (mats.size() > 0) {
				for (int i = 0; i < mats.size(); i++) {
					dxgl::MeshMaterialSlot mat = mesh->getMaterials()[i];
					renderer()->shader()->PS_setMaterial(0, 1, resource()->get<SP_DXGLMaterial>(mat.material));
					if (useTessellation) {
						renderer()->shader()->DS_setMaterial(0, 1, resource()->get<SP_DXGLMaterial>(mat.material));
						renderer()->drawIndexedTriangleListInstancedTess(mat.indexCount, entities.size(), mat.startIndex, 0, 0);
					} else {
						renderer()->drawIndexedTriangleListInstanced(mat.indexCount, entities.size(), mat.startIndex, 0, 0);
					}
				}
			} else {
				if (useTessellation) {
					renderer()->drawIndexedTriangleListInstancedTess(mesh->getIndices().size(), entities.size(), 0, 0, 0);
				} else {
					renderer()->drawIndexedTriangleListInstanced(mesh->getIndices().size(), entities.size(), 0, 0, 0);
				}
			}
		}
	}*/

	// render meshes end

	renderer()->raster()->RS_setState(nullptr);

	// post process start

	m_postProcessor.draw();

	// post process end

	if (input()->getKeyTapState('X')) {
		std::cout << renderer()->getDrawCallCount() << "\n";
	}

	renderer()->resetDrawCallCount();

}
