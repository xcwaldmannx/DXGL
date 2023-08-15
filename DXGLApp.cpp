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

	// flags
	int materialFlags = 0;
	int globalFlags = 0;
	float pad3[2];
};

DXGLApp::DXGLApp() {
}

DXGLApp::~DXGLApp() {
}

void DXGLApp::create() {

	// back buffer RTV and DSV
	RESOURCE_VIEW_DESC rtvDesc = {
		FLOAT16,
		4,
		D3D11_STANDARD_MULTISAMPLE_PATTERN,
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

	{
		MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_INDEX;
		resource()->storeMesh(desc, "Assets/Meshes/cubeFlipped.fbx", "cubeFlipped");
	}

	m_camera = renderer()->camera()->create("primary");
	m_camera->world().setTranslation(Vec3f{ 0.0f, 0.0f, 0.0f });

	m_vscbEntityBuffer = resource()->createVSConstantBuffer(sizeof(EntityBuffer));
	m_pscbEntityBuffer = resource()->createPSConstantBuffer(sizeof(EntityBuffer));

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
	// shadowTimer.start();

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
	entities()->registerComponent<TransformComponent>();
	entities()->registerComponent<MeshComponent>();
	entities()->registerComponent<PickableComponent>();
	entities()->registerComponent<DestroyableComponent>();
	entities()->registerComponent<RigidBodyComponent>();

	// landscape
	{
		MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_ALL;
		renderer()->terrain()->load(desc, "Assets/Meshes/landscapes/landscape_grass.fbx");
	}

	{ // guitar
		MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_DEFAULT;
		resource()->storeMesh(desc, "Assets/Meshes/material test cube/explorer guitar.fbx", "guitar");
		resource()->storeMesh(desc, "Assets/Meshes/material test cube/cube.fbx", "cube02");

		TransformComponent transform{};
		transform.scale = { 1, 1, 1 };
		transform.rotation = { 0, 0, 0 };
		transform.translation = { 0, 0, 0 };

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_Mesh>("guitar");
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;

		m_guitar = entities()->createEntity(transform, mesh);

		PickableComponent pickable{};
		pickable.isSelected = false;
		entities()->addEntityComponent<PickableComponent>(pickable, m_guitar);
	}

	// gun
	{
		MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_DEFAULT;
		m_fbxMesh = resource()->createMesh(desc, "Assets/Meshes/material test cube/gun.fbx");

		TransformComponent transform{};
		transform.scale = { 1, 1, 1 };
		transform.rotation = { 0, 0, 0 };
		transform.translation = { 0, 0, 0 };

		MeshComponent mesh{};
		mesh.mesh = m_fbxMesh;
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;

		m_gun = entities()->createEntity(transform, mesh);
	}

	// rocks
	{
		MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_DEFAULT | MISC_FACE;
		resource()->storeMesh(desc, "Assets/Meshes/box.fbx", "rock1");
	}

	for (int i = -10; i < 10; i++) {
		for (int j = -10; j < 10; j++) {
			for (int k = -10; k < 10; k++) {
				TransformComponent transform{};
				transform.scale = { 5, 5, 5 };// { 2.0f + std::rand() % 5, 2.0f + std::rand() % 5, 2.0f + std::rand() % 5 };
				float rx = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
				float ry = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
				float rz = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
				transform.rotation = { rx, ry, rz };

				float tx = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 96);
				float ty = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 96);
				float tz = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 96);
				transform.translation = { (float) i * 256 + tx, (float) j * 256 + ty, (float) k * 256 + tz };

				MeshComponent mesh{};
				mesh.mesh = resource()->get<SP_Mesh>("rock1");
				mesh.useTessellation = false;
				mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;

				dxgl::governor::EntityId id = entities()->createEntity(transform, mesh);

				PickableComponent pickable{};
				pickable.isSelected = false;
				entities()->addEntityComponent<PickableComponent>(pickable, id);

				DestroyableComponent destroyable{};
				entities()->addEntityComponent<DestroyableComponent>(destroyable, id);

				RigidBodyComponent rigidbody{};
				rigidbody.elasticity = 0.3f;
				rigidbody.mass = transform.scale.x * 100.0f;
				entities()->addEntityComponent<RigidBodyComponent>(rigidbody, id);
			}
		}
	}

	//{
	//	// colliding player
	//	TransformComponent transform{};
	//	transform.scale = { 0.5f, 0.5f, 0.5f };
	//	transform.translation = { 0, 0, 0 };
	//	MeshComponent mesh{};
	//	mesh.mesh = resource()->get<SP_Mesh>("rock1");
	//	m_player = entities()->createEntity(transform, mesh);

	//	RigidBodyComponent rigidbody{};
	//	rigidbody.elasticity = 0.1f;
	//	rigidbody.mass = 75.0f;
	//	entities()->addEntityComponent<RigidBodyComponent>(rigidbody, m_player);
	//}

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

	renderer()->terrain()->update(delta);

	renderer()->foliage()->update(delta);

	// add entity start

	if (input()->getKeyTapState('E')) {

		TransformComponent transform{};
		transform.scale = { 1, 1, 1 };
		float rx = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
		float ry = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
		float rz = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
		transform.rotation = { rx, ry, rz };
		transform.translation = m_camera->getPosition() + m_camera->getDirection() * 2.0f;

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_Mesh>("rock1");

		dxgl::governor::EntityId id = entities()->createEntity(transform, mesh);

		RigidBodyComponent rigidbody{};
		rigidbody.linearVelocity = m_camera->getDirection() * 25.0f;
		rigidbody.elasticity = 0.5f;
		rigidbody.mass = 10.0f;
		entities()->addEntityComponent<RigidBodyComponent>(rigidbody, id);
	}

	// add entity end

	// point gun start
	{
		Vec3f aim(-0.75f, -0.25f, 1.0f);
		if (input()->getMouseState(DXGLInputManager::RMB_STATE)) {
			aim = Vec3f(0.0f, -0.185f, 1.0f);
		}

		auto& transform = entities()->getEntityComponent<TransformComponent>(m_gun);
		transform.rotation = m_camera->world().getRotation();
		Vec3f offsetX = m_camera->world().getXDirection() * aim.x;
		Vec3f offsetY = m_camera->world().getYDirection() * aim.y;
		Vec3f offsetZ = m_camera->world().getZDirection() * aim.z;
		Vec3f offset = offsetX + offsetY + offsetZ;
		transform.translation = m_camera->world().getTranslation() + offset;
		entities()->relocateEntity(m_gun);
	}
	// point gun end

	// rotate guitar start
	
	{
		auto& transform = entities()->getEntityComponent<TransformComponent>(m_guitar);
		transform.rotation = Vec3f(0, m_timePassed, 0);
	}

	// rotate guitar end

	// player start
	//{
	//	auto& transform = entities()->getEntityComponent<TransformComponent>(m_player);
	//	transform.rotation = m_camera->world().getRotation();
	//	transform.translation = m_camera->world().getTranslation();

	//	auto& rigidbody = entities()->getEntityComponent<RigidBodyComponent>(m_player);
	//	rigidbody.linearVelocity = m_camera->getDirection() * m_camera->getCurrentSpeed();
	//}
	// player end

	// mouse picking start
	if (input()->getMouseState(DXGLInputManager::LMB_STATE)) {
		Point2f mouse = DXGLMain::input()->getMousePosition();
		POINT clientMouse = { mouse.x, mouse.y };
		ScreenToClient(getWindow(), &clientMouse);
		governor::EntityId id = mousePick()->getColorId(Point2f{ (float)clientMouse.x, (float)clientMouse.y });
		if (entities()->entityHasComponent<DestroyableComponent>(id)) {
			entities()->destroyEntity(id);
		}
	}
	// mouse picking end

	// update entities start
	std::list<OctTree<governor::EntityId>::ptr> searchedEntities = entities()->searchEntities(512);

	{ // physics manager
		for (auto entity: searchedEntities) {
			governor::EntityId id = entity->item;

			if (entities()->entityHasComponent<RigidBodyComponent>(id)) {
				auto& transform = entities()->getEntityComponent<TransformComponent>(id);
				auto& mesh = entities()->getEntityComponent<MeshComponent>(id);
				auto& rigidbody = entities()->getEntityComponent<RigidBodyComponent>(id);

				Mat4f mat{};
				mat.setIdentity();
				mat.setScale(transform.scale * 1.25f);
				//mat.setRotation(transform.rotation);
				mat.setTranslation(transform.translation);

				AABB aabb = mesh.mesh->getAABB();
				aabb.min = mat * aabb.min;
				aabb.max = mat * aabb.max;

				// collision check
				for (auto collider : searchedEntities) {
					governor::EntityId colliderId = collider->item;
					if (id != colliderId && entities()->entityHasComponent<RigidBodyComponent>(colliderId)) {
						auto& colliderTransform = entities()->getEntityComponent<TransformComponent>(colliderId);
						auto& colliderMesh = entities()->getEntityComponent<MeshComponent>(colliderId);
						auto& colliderRigidbody = entities()->getEntityComponent<RigidBodyComponent>(colliderId);

						Mat4f colliderMat{};
						colliderMat.setIdentity();
						colliderMat.setScale(colliderTransform.scale * 1.25f);
						//colliderMat.setRotation(colliderTransform.rotation);
						colliderMat.setTranslation(colliderTransform.translation);

						AABB colliderAABB = colliderMesh.mesh->getAABB();
						colliderAABB.min = colliderMat * colliderAABB.min;
						colliderAABB.max = colliderMat * colliderAABB.max;


						if (aabb.isCollided(colliderAABB)) {
							Vec3f intersection{};
							bool foundIntersection = false;

							// loop through faces to determine exact contact point
							const auto& faces = mesh.mesh->getFaces();
							const auto& colliderFaces = colliderMesh.mesh->getFaces();

							for (const auto& face : faces) {
								mat.setIdentity();
								mat.setScale(transform.scale * 1.25f);
								mat.setRotation(transform.rotation);
								mat.setTranslation(transform.translation);

								Vec3f f0 = mat * face.v0;
								Vec3f f1 = mat * face.v1;
								Vec3f f2 = mat * face.v2;
								for (const auto& colliderFace : colliderFaces) {
									colliderMat.setIdentity();
									colliderMat.setScale(colliderTransform.scale * 1.25f);
									colliderMat.setRotation(colliderTransform.rotation);
									colliderMat.setTranslation(colliderTransform.translation);

									Vec3f cf0 = colliderMat * colliderFace.v0;
									Vec3f cf1 = colliderMat * colliderFace.v1;
									Vec3f cf2 = colliderMat * colliderFace.v2;
									if (Math::trianglesIntersect(f0, f1, f2, cf0, cf1, cf2)) {
										intersection = Math::getTriangleIntersection(f0, f1, f2, cf0, cf1, cf2);
										foundIntersection = true;
										break;
									}
								}

								if (foundIntersection) {
									break;
								}
							}

							if (foundIntersection) {
								// linear velocity

								Vec3f v1f{};
								Vec3f v2f{};

								Vec3f relativeVelocity = rigidbody.linearVelocity - colliderRigidbody.linearVelocity;

								float impulseDenominator = 1.0f / (rigidbody.mass + colliderRigidbody.mass);

								v1f = rigidbody.linearVelocity - relativeVelocity * impulseDenominator * (1.0f + rigidbody.elasticity) * colliderRigidbody.mass;
								v2f = colliderRigidbody.linearVelocity + relativeVelocity * impulseDenominator * (1.0f + colliderRigidbody.elasticity) * rigidbody.mass;

								rigidbody.linearVelocity = v1f;
								colliderRigidbody.linearVelocity = v2f;

								transform.translation += v1f * delta;
								colliderTransform.translation += v2f * delta;

								// angular velocity

								Vec3f w1f{};
								Vec3f w2f{};

								// force calculation for angular velocity
								Vec3f force = (v1f * rigidbody.mass) + (v2f * colliderRigidbody.mass);

								Vec3f distance1 = intersection - transform.translation;
								Vec3f distance2 = intersection - colliderTransform.translation;

								rigidbody.moi = (2.0f / 5.0f) * rigidbody.mass * (transform.scale.x * transform.scale.x);
								Vec3f torque1 = Vec3f::cross(force, distance1);
								w1f = torque1 / rigidbody.moi;
								rigidbody.angularVelocity = w1f;

								colliderRigidbody.moi = (2.0f / 5.0f) * colliderRigidbody.mass * (colliderTransform.scale.x * colliderTransform.scale.x) * 5.0f;
								Vec3f torque2 = Vec3f::cross(force, distance2);
								w2f = torque2 / colliderRigidbody.moi;
								colliderRigidbody.angularVelocity = w2f;
							}
						}
					}
				}

				if (Vec3f::length(rigidbody.linearVelocity) >= 0.01f) {
					transform.translation += rigidbody.linearVelocity * delta;
					rigidbody.linearVelocity *= 0.998f;
					entities()->relocateEntity(id);
				}

				if (Vec3f::length(rigidbody.angularVelocity) >= 0.01f) {
					transform.rotation += rigidbody.angularVelocity * delta;
					rigidbody.angularVelocity *= 0.998f;
				}
			}
		}
	}

	m_queue.submit(searchedEntities);
	// update entities end
}

void DXGLApp::draw() {

	//renderer()->merger()->setDepthStencil("basic");
	//shadowTimer.execute();

	RECT dim = getWindowSize();
	UINT width = dim.right- dim.left;
	UINT height = dim.bottom - dim.top;
	renderer()->setViewport(width, height);

	float color[4] = { 1, 1, 1, 1 };
	renderer()->setRenderTarget(m_backBufferRTV, color, m_backBufferDSV);

	m_queue.draw();

	// m_postProcessor.draw();

	if (input()->getKeyTapState('X')) {
		std::cout << renderer()->getDrawCallCount() << "\n";
	}

	renderer()->resetDrawCallCount();

	return;

	// render start

	//float color[4] = { 1, 1, 1, 1 };
	renderer()->setRenderTarget(m_backBufferRTV, color, m_backBufferDSV);

	// set cascaded shadow textures
	renderer()->shader()->PS_setResources(6, 4, renderer()->shadow()->getSRVs());

	// Skybox
	m_skybox.getCube()->bind(1);
	resource()->get<SP_Texture2D>("brdf")->bind(2);

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

	// terrain
	renderer()->terrain()->draw();

	// post process start

	m_postProcessor.draw();

	// post process end

}
