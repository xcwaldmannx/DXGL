#include "DXGLApp.h"

#include <iostream>
#include <chrono>

#include "ResourceManager.h"
#include "TextRenderManager.h"
#include "MousePickManager.h"
#include "PhysicsManager.h"
#include "EntityManager.h"
#include "ControllerManager.h"
#include "CameraManager.h"

#include "InputSystem.h"
#include "InputManager.h"

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

	m_vscbEntityBuffer = resource()->createVSConstantBuffer(sizeof(EntityBuffer));
	m_pscbEntityBuffer = resource()->createPSConstantBuffer(sizeof(EntityBuffer));

	// Texture SamplerState
	resource()->storeSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0, "textureSampler");

	renderer()->shader()->DS_setSampler(0, 1, resource()->get<SP_DXGLSamplerState>("textureSampler"));
	renderer()->shader()->PS_setSampler(0, 1, resource()->get<SP_DXGLSamplerState>("textureSampler"));

	// Shadow SamplerState
	resource()->storeSamplerState(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_ALWAYS, 1, "shadowSampler");
	renderer()->setSamplerState(1, 1, resource()->get<SP_DXGLSamplerState>("shadowSampler")->get());

	// setup shadow updates
	/*shadowTimer.every(25, [&]() {
		float camX = m_camera->getPosition().x;
		float camY = m_camera->getPosition().y;
		float camZ = m_camera->getPosition().z;

		float x = camX;
		float y = 512 * sin(m_timePassed / 100.0f);
		float z = camZ + 512 * cos(m_timePassed / 100.0f);

		renderer()->shadow()->update(Vec3f{ x, y, z }, Vec3f{ camX, 0, camZ });
		renderer()->shadow()->draw();
		});*/
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
	entities()->registerComponent<DescriptionComponent>();
	entities()->registerComponent<RigidBodyComponent>();
	entities()->registerComponent<MovementComponent>();
	entities()->registerComponent<ControllerComponent>();
	entities()->registerComponent<CameraComponent>();

	// standard meshes
	{
		MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_ALL;
		desc.amountMetallic = 1;
		desc.amountRoughness = 1;
		resource()->storeMesh(desc, "Assets/Meshes/box.fbx", "box");
		resource()->storeMesh(desc, "Assets/Meshes/sphere.fbx", "sphere");
		resource()->storeMesh(desc, "Assets/Meshes/torus.fbx", "torus");
	}

	{ // house

		{
			MeshDesc desc{};
			desc.vertexAttributes = VERTEX_ALL;
			desc.miscAttributes = MISC_ALL;
			desc.amountMetallic = 1;
			desc.amountRoughness = 1;
			resource()->storeMesh(desc, "Assets/Meshes/house.fbx", "house");
		}

		TransformComponent transform{};
		transform.scale = { 1, 1, 1 };
		transform.rotation = { 0, 0, 0 };
		transform.translation = { 0, 0, 32 };

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_Mesh>("house");
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;

		auto id = entities()->createEntity(transform, mesh);

		RigidBodyComponent rigidbody{};
		rigidbody.shape = new RigidBodyTriangleMesh();
		rigidbody.mass = 100.0f;
		rigidbody.staticFriction = 0.75f;
		rigidbody.dynamicFriction = 0.75f;
		rigidbody.restitution = 0.1f;
		rigidbody.isStatic = true;
		entities()->addEntityComponent<RigidBodyComponent>(rigidbody, id);
	}

	{ // player
		TransformComponent transform{};
		transform.scale = { 1, 1, 1 };
		transform.rotation = { 0, 0, 0 };
		transform.translation = { 0, 1, 0 };

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_Mesh>("box");
		mesh.hideMesh = true;
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;

		m_player = entities()->createEntity(transform, mesh);

		RigidBodyComponent rigidbody{};
		rigidbody.shape = new RigidBodyCapsule(0.75f, 0.75f);
		rigidbody.lockFlags = RigidBodyLockFlag::LOCK_ANGULAR_X | RigidBodyLockFlag::LOCK_ANGULAR_Y | RigidBodyLockFlag::LOCK_ANGULAR_Z;
		rigidbody.mass = 100.0f;
		rigidbody.staticFriction = 0.999f;
		rigidbody.dynamicFriction = 0.999f;
		rigidbody.restitution = 0.001f;
		rigidbody.isStatic = false;
		entities()->addEntityComponent<RigidBodyComponent>(rigidbody, m_player);

		CameraComponent camera{};
		camera.translation = transform.translation;
		camera.rotation = { 0, 0, 0 };
		camera.trackMouse = true;
		camera.trackEntity = true;
		entities()->addEntityComponent<CameraComponent>(camera, m_player);

		MovementComponent movement{};
		entities()->addEntityComponent<MovementComponent>(movement, m_player);

		ControllerComponent controller{};
		controller.speed = 6.0f;
		controller.addAction('W', ControllerActionFlag::ON_PRESS, [&](governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& rigidbody = entities()->getEntityComponent<RigidBodyComponent>(id);
			auto& controller = entities()->getEntityComponent<ControllerComponent>(id);
			auto& camera = entities()->getEntityComponent<CameraComponent>(id);

			Vec3f direction = Vec3f(camera.forward().x, 0, camera.forward().z).normalize();

			rigidbody.applyTransform(direction * controller.speed * delta);
			camera.translation = trans.translation;
		});

		controller.addAction('S', ControllerActionFlag::ON_PRESS, [&](governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& rigidbody = entities()->getEntityComponent<RigidBodyComponent>(id);
			auto& controller = entities()->getEntityComponent<ControllerComponent>(id);
			auto& camera = entities()->getEntityComponent<CameraComponent>(id);

			Vec3f direction = Vec3f(camera.forward().x, 0, camera.forward().z).normalize();

			rigidbody.applyTransform(direction * -controller.speed * delta);
			camera.translation = trans.translation;
		});

		controller.addAction('D', ControllerActionFlag::ON_PRESS, [&](governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& rigidbody = entities()->getEntityComponent<RigidBodyComponent>(id);
			auto& controller = entities()->getEntityComponent<ControllerComponent>(id);
			auto& camera = entities()->getEntityComponent<CameraComponent>(id);

			Vec3f direction = Vec3f(camera.right().x, 0, camera.right().z).normalize();

			rigidbody.applyTransform(direction * controller.speed * delta);
			camera.translation = trans.translation;
		});

		controller.addAction('A', ControllerActionFlag::ON_PRESS, [&](governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& rigidbody = entities()->getEntityComponent<RigidBodyComponent>(id);
			auto& controller = entities()->getEntityComponent<ControllerComponent>(id);
			auto& camera = entities()->getEntityComponent<CameraComponent>(id);

			Vec3f direction = Vec3f(camera.right().x, 0, camera.right().z).normalize();

			rigidbody.applyTransform(direction * -controller.speed * delta);
			camera.translation = trans.translation;
		});
		controller.addAction(VK_SPACE, ControllerActionFlag::ON_PRESS, [&](governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& rigidbody = entities()->getEntityComponent<RigidBodyComponent>(id);
			auto& controller = entities()->getEntityComponent<ControllerComponent>(id);
			auto& camera = entities()->getEntityComponent<CameraComponent>(id);

			if (rigidbody.isStationary) {
				Vec3f direction = Vec3f(0, camera.up().y, 0).normalize();
				rigidbody.applyVelocity(direction * controller.speed);
				camera.translation = trans.translation;
			}
		});
		controller.addAction(VK_SHIFT, ControllerActionFlag::ON_PRESS, [&](governor::EntityId id, long double delta) {
			auto& control = entities()->getEntityComponent<ControllerComponent>(id);
			control.speed = 10.0f;
		});
		controller.addAction(VK_SHIFT, ControllerActionFlag::ON_RELEASE, [&](governor::EntityId id, long double delta) {
			auto& control = entities()->getEntityComponent<ControllerComponent>(id);
			control.speed = 6.0f;
		});
		entities()->addEntityComponent<ControllerComponent>(controller, m_player);

	}

	controller()->setActiveController(m_player);
	camera()->setActiveCamera(m_player);

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
		transform.translation = { 0, 5, 0 };

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_Mesh>("guitar");
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;

		m_guitar = entities()->createEntity(transform, mesh);

		PickableComponent pickable{};
		pickable.isSelected = false;
		entities()->addEntityComponent<PickableComponent>(pickable, m_guitar);

		RigidBodyComponent rigidbody{};
		rigidbody.shape = new RigidBodyConvexMesh();
		rigidbody.mass = 25.0f;
		rigidbody.staticFriction = 0.75f;
		rigidbody.dynamicFriction = 0.75f;
		rigidbody.restitution = 0.1f;
		rigidbody.isStatic = true;
		entities()->addEntityComponent<RigidBodyComponent>(rigidbody, m_guitar);

		CameraComponent camera{};
		camera.translation = transform.translation;
		camera.rotation = { 0, 0, 0 };
		camera.trackMouse = true;
		camera.trackEntity = true;
		entities()->addEntityComponent<CameraComponent>(camera, m_guitar);
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
		desc.miscAttributes = MISC_ALL;
		desc.amountMetallic = 1;
		desc.amountRoughness = 1;
		resource()->storeMesh(desc, "Assets/Meshes/rock1.fbx", "rock1");
	}

	{ // floor
		TransformComponent transform{};
		transform.scale = { 128, 1, 128 };
		transform.rotation = { 0, 0, 0 };
		transform.translation = { 0, -0.5f, 0 };

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_Mesh>("box");
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;

		m_floor = entities()->createEntity(transform, mesh);

		RigidBodyComponent rigidbody{};
		rigidbody.shape = new RigidBodyBox(64, 0.5f, 64);
		rigidbody.staticFriction = 0.75f;
		rigidbody.dynamicFriction = 0.75f;
		rigidbody.restitution = 0.1f;
		rigidbody.isStatic = true;
		entities()->addEntityComponent<RigidBodyComponent>(rigidbody, m_floor);
	}

	{ // torus
		TransformComponent transform{};
		transform.scale = { 3, 3, 3 };
		transform.rotation = { 0, 3.14159f / 2.0f, 0 };
		transform.translation = { -20, -6, 0 };

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_Mesh>("torus");
		mesh.useTessellation = false;
		mesh.instanceFlags = INSTANCE_USE_LIGHTING | INSTANCE_USE_SHADOWING;

		auto id = entities()->createEntity(transform, mesh);

		RigidBodyComponent rigidbody{};
		rigidbody.shape = new RigidBodyTriangleMesh();
		rigidbody.staticFriction = 0.5f;
		rigidbody.dynamicFriction = 0.5f;
		rigidbody.restitution = 0.5f;
		rigidbody.isStatic = true;
		entities()->addEntityComponent<RigidBodyComponent>(rigidbody, id);
	}

	// Depth Stencils
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	renderer()->merger()->createDepthStencil(depthStencilDesc, "basic");

	// load font
	Engine::textrender()->createFont("Assets/Fonts/heavy heap.ttf", "hh");
 }

void DXGLApp::update(long double delta) {
	m_timeDelta = delta;
	m_timePassed += m_timeDelta;

	// active camera

	if (input()->getKeyTapState('1')) {
		camera()->setActiveCamera(m_player);
	}

	if (input()->getKeyTapState('2')) {
		camera()->setActiveCamera(m_guitar);
	}

	auto& cam = camera()->getActiveCamera();

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
		dxgl::InputSystem::get()->showCursor(!m_playState);
	}

	if (m_playState) {
		POINT center = getWindowCenter();
		dxgl::InputSystem::get()->setCursorPosition(Point2f{ (float) center.x, (float) center.y });
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
		//float rx = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
		//float ry = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
		//float rz = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) * (float)(std::rand() % 6);
		//transform.rotation = { rx, ry, rz };
		transform.translation = cam.translation + cam.forward() * 2.0f;

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_Mesh>("guitar");

		dxgl::governor::EntityId id = entities()->createEntity(transform, mesh);

		DestroyableComponent destroyable{};
		entities()->addEntityComponent<DestroyableComponent>(destroyable, id);

		DescriptionComponent description{};
		description.name = "Guitar " + std::to_string(id);
		description.description = "This guitar's ID is " + std::to_string(id);
		entities()->addEntityComponent<DescriptionComponent>(description, id);

		RigidBodyComponent rigidbody{};
		rigidbody.shape = new RigidBodyConvexMesh();
		rigidbody.linearVelocity = cam.forward() * 25.0f;
		rigidbody.angularVelocity = Vec3f{ 1, 1, 1 };
		rigidbody.mass = 25.0f;
		rigidbody.staticFriction = 0.75f;
		rigidbody.dynamicFriction = 0.75f;
		rigidbody.restitution = 0.1f;
		entities()->addEntityComponent<RigidBodyComponent>(rigidbody, id);
	}

	// add entity end

	// point gun start
	{
		Vec3f aim(-0.75f, -0.25f, 1.0f);
		if (input()->getMouseState(InputManager::RMB_STATE)) {
			aim = Vec3f(0.0f, -0.185f, 1.0f);
		}

		auto& transform = entities()->getEntityComponent<TransformComponent>(m_gun);
		transform.rotation = cam.world().getRotation();
		Vec3f offsetX = cam.world().getXDirection() * aim.x;
		Vec3f offsetY = cam.world().getYDirection() * aim.y;
		Vec3f offsetZ = cam.world().getZDirection() * aim.z;
		Vec3f offset = offsetX + offsetY + offsetZ;
		transform.translation = cam.world().getTranslation() + offset;
		entities()->relocateEntity(m_gun);
	}
	// point gun end

	// rotate guitar start
	
	{
		auto& transform = entities()->getEntityComponent<TransformComponent>(m_guitar);
		transform.rotation = Vec3f(0, m_timePassed, 0);
	}

	// rotate guitar end

	// mouse picking start
	if (input()->getMouseState(InputManager::LMB_STATE)) {
		Point2f mouse = Engine::input()->getMousePosition();
		POINT clientMouse = { mouse.x, mouse.y };
		ScreenToClient(getWindow(), &clientMouse);
		governor::EntityId id = mousePick()->getColorId(Point2f{ (float)clientMouse.x, (float)clientMouse.y });
		if (entities()->entityHasComponent<DestroyableComponent>(id)) {
			entities()->destroyEntity(id);
		}
	}

	if (input()->getMouseState(InputManager::RMB_STATE)) {
		Point2f mouse = Engine::input()->getMousePosition();
		POINT clientMouse = { mouse.x, mouse.y };
		ScreenToClient(getWindow(), &clientMouse);
		governor::EntityId id = mousePick()->getColorId(Point2f{ (float)clientMouse.x, (float)clientMouse.y });
		if (entities()->entityHasComponent<DescriptionComponent>(id)) {
			auto& description = entities()->getEntityComponent<DescriptionComponent>(id);
			dxgl::Text name{};
			name.text = description.name;
			name.size = 16.0f;
			name.bounds = { 0, 0, 0.5f, 0.25f };
			name.color = { 1, 1, 1 };
			name.alias = "hh";
			m_queue.submit(name);

			dxgl::Text desc{};
			desc.text = description.description;
			desc.size = 16.0f;
			desc.bounds = { 0, 0.05f, 0.5f, 0.25f };
			desc.color = { 1, 1, 1 };
			desc.alias = "hh";
			m_queue.submit(desc);
		}
	}

	// mouse picking end

	// draw FPS
	Text t{};
	t.text = "FPS: " + std::to_string(Engine::getAverageFPS());
	t.size = 8.0f;
	t.bounds = { 0, 0, 0.1f, 0.1f };
	t.color = { 1, 1, 1 };
	t.alias = "hh";
	m_queue.submit(t);

	// update entities start

	std::list<OctTree<governor::EntityId>::ptr> searchedEntities = entities()->searchEntities(512);

	physics()->update(searchedEntities, delta);

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
