#include "Game01.h"

#include "EntityComponentTypes.h"
#include "ResourceManager.h"
#include "EntityManager.h"
#include "CameraManager.h"
#include "ControllerManager.h"
#include "MousePickManager.h"

#include "InputManager.h"
#include "InputSystem.h"

Game01::Game01() {

}

Game01::~Game01() {

}

void Game01::create() {
	// create buffers (Make this automatic)
	RESOURCE_VIEW_DESC rtvDesc = {
	FLOAT16,
		4,
		D3D11_STANDARD_MULTISAMPLE_PATTERN,
	};
	renderer()->createRenderTargetView(&rtvDesc, RESOURCE_VIEW_SLOT_BACK_BUFFER, &m_backbufferRTV);

	RESOURCE_VIEW_DESC dsvDesc = {
		D24_S8,
		4,
		D3D11_STANDARD_MULTISAMPLE_PATTERN,
	};
	renderer()->createDepthStencilView(&dsvDesc, RESOURCE_VIEW_SLOT_BACK_BUFFER, &m_backbufferDSV);

	// initialize ECS components
	entities()->registerComponent<TransformComponent>(); // Sets the entities position, rotation, and scale
	entities()->registerComponent<MeshComponent>(); // Sets the entities 3d model and stuff

	entities()->registerComponent<CameraComponent>(); // Allows an entity to have a viewable camera
	entities()->registerComponent<ControllerComponent>(); // Allows an entity to be controlled

	entities()->registerComponent<PickableComponent>(); // Allows an entity to be clickable
	entities()->registerComponent<DescriptionComponent>(); // Sets a description for an entity

	// load assets (Make this easier)
	{ // box
		dxgl::MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_ALL;
		desc.amountMetallic = 1;
		desc.amountRoughness = 1;
		resource()->storeMesh(desc, "Assets/Meshes/box.fbx", "box");
	}

	{ // player
		dxgl::MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_ALL;
		desc.amountMetallic = 0;
		desc.amountRoughness = 1;
		resource()->storeMesh(desc, "Game01/Meshes/player.fbx", "player");
	}

	{ // flipped cube model (for skybox - add default for skybox rather than loading a model)
		dxgl::MeshDesc desc{};
		desc.vertexAttributes = VERTEX_ALL;
		desc.miscAttributes = MISC_INDEX;
		desc.amountMetallic = 1;
		desc.amountRoughness = 1;
		resource()->storeMesh(desc, "Assets/Meshes/cubeFlipped.fbx", "cubeFlipped");
	}

	{ // font
		textrender()->createFont("Assets/Fonts/heavy heap.ttf", "hh");
	}

	// create some entities
	{ // player
		TransformComponent transform{};
		transform.translation = { 0, 0, 0 };
		transform.rotation = { 0, 0, 0 };
		transform.scale = { 1, 1, 1 };

		MeshComponent mesh{};
		mesh.mesh = resource()->get<SP_Mesh>("player");

		m_player = entities()->createEntity(transform, mesh);

		CameraComponent camera{};
		camera.translation = transform.translation;
		camera.trackEntity = false;
		camera.trackMouse = true;
		camera.enableThirdPerson = true;
		entities()->addEntityComponent<CameraComponent>(camera, m_player);

		ControllerComponent controller{};
		controller.addAction('W', ControllerActionFlag::ON_PRESS, [&](dxgl::governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& cam = entities()->getEntityComponent<CameraComponent>(id);
			trans.translation += Vec3f(cam.forward().x, 0, cam.forward().z).normalize() * m_playerSpeed * delta;
			cam.translation = trans.translation + Vec3f(0, 0.75f, 0);
		});
		controller.addAction('S', ControllerActionFlag::ON_PRESS, [&](dxgl::governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& cam = entities()->getEntityComponent<CameraComponent>(id);
			trans.translation -= Vec3f(cam.forward().x, 0, cam.forward().z).normalize() * m_playerSpeed * delta;
			cam.translation = trans.translation + Vec3f(0, 0.75f, 0);
		});
		controller.addAction('D', ControllerActionFlag::ON_PRESS, [&](dxgl::governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& cam = entities()->getEntityComponent<CameraComponent>(id);
			trans.translation += Vec3f(cam.right().x, 0, cam.right().z).normalize() * m_playerSpeed * delta;
			cam.translation = trans.translation + Vec3f(0, 0.75f, 0);
		});
		controller.addAction('A', ControllerActionFlag::ON_PRESS, [&](dxgl::governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& cam = entities()->getEntityComponent<CameraComponent>(id);
			trans.translation -= Vec3f(cam.right().x, 0, cam.right().z).normalize() * m_playerSpeed * delta;
			cam.translation = trans.translation + Vec3f(0, 0.75f, 0);
		});
		controller.addAction(VK_SPACE, ControllerActionFlag::ON_PRESS, [&](dxgl::governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& cam = entities()->getEntityComponent<CameraComponent>(id);
			trans.translation += Vec3f(0, 1, 0) * m_playerSpeed * delta;
			cam.translation = trans.translation + Vec3f(0, 0.75f, 0);
		});
		controller.addAction(VK_SHIFT, ControllerActionFlag::ON_PRESS, [&](dxgl::governor::EntityId id, long double delta) {
			auto& trans = entities()->getEntityComponent<TransformComponent>(id);
			auto& cam = entities()->getEntityComponent<CameraComponent>(id);
			trans.translation -= Vec3f(0, 1, 0) * m_playerSpeed * delta;
			cam.translation = trans.translation + Vec3f(0, 0.75f, 0);
		});
		entities()->addEntityComponent<ControllerComponent>(controller, m_player);
	}

	// set player as active camera and controller
	camera()->setActiveCamera(m_player);
	controller()->setActiveController(m_player);

	{ // some boxes
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				TransformComponent transform{};
				transform.translation = Vec3f(i * 5.0f, 0, j * 5.0f);
				transform.rotation = { 0, 0, 0 };
				transform.scale = { 1, 1, 1 };

				MeshComponent mesh{};
				mesh.mesh = resource()->get<SP_Mesh>("box");

				auto id = entities()->createEntity(transform, mesh);

				PickableComponent pickable{};
				entities()->addEntityComponent<PickableComponent>(pickable, id);

				DescriptionComponent description{};
				description.name = "Box Number " + std::to_string(id);
				entities()->addEntityComponent<DescriptionComponent>(description, id);
			}
		}
	}

	// create some lighting
	{ // lights
		Light l{};
		l.direction = { 1, -0.2f, 0 };
		l.color = { 2, 2, 2 };
		renderer()->light()->addLight(l);
	}
}

// game loop
void Game01::update(long double delta) {
	// quit game with ESC
	if (input()->getKeyPressState(VK_ESCAPE)) {
		quit();
	}

	// set fullscreen mode
	if (input()->getKeyTapState('F')) {
		m_fullscreen = !m_fullscreen;
		std::cout << "FULLSCREEN: " << m_fullscreen << "\n";
		renderer()->setFullscreen(m_fullscreen);
	}

	// lock mouse to center
	if (input()->getKeyTapState('P')) {
		m_playstate = !m_playstate;
		dxgl::InputSystem::get()->showCursor(!m_playstate);
	}

	if (m_playstate) {
		POINT center = getWindowCenter();
		dxgl::InputSystem::get()->setCursorPosition(Point2f{ (float)center.x, (float)center.y });
	}

	
	{ // adjust player rotation and camera
		auto& transform = entities()->getEntityComponent<TransformComponent>(m_player);
		auto& camera = entities()->getEntityComponent<CameraComponent>(m_player);

		if (!input()->getMouseState(InputManager::RMB_STATE) 
			|| (input()->getKeyPressState('W') || input()->getKeyPressState('S') || input()->getKeyPressState('D') || input()->getKeyPressState('A'))) {
			transform.rotation.y = camera.rotation.y + 3.14159265359f;
		}

		if (input()->getKeyPressState('Q') && camera.thirdPersonDistance <= 10.0f) {
			camera.thirdPersonDistance += 8.0f * delta;
		}

		if (input()->getKeyPressState('E') && camera.thirdPersonDistance >= 2.0f) {
			camera.thirdPersonDistance -= 8.0f * delta;
		}
	}

	// draw entity descriptions to screen by submitting text to render queue
	if (input()->getMouseState(InputManager::LMB_STATE)) {
		POINT center = getWindowCenter();
		auto id = mousePick()->getColorId(Point2f{ (float)center.x, (float)center.y });
		if (entities()->entityHasComponent<PickableComponent>(id) && entities()->entityHasComponent<DescriptionComponent>(id)) {
			auto& description = entities()->getEntityComponent<DescriptionComponent>(id);
			Text t{};
			t.text = description.name;
			t.color = { 1, 0.5f, 0 };
			t.bounds = { 0.75f, 0.05f, 0.25f, 0.1f };
			t.alias = "hh";
			m_renderQueue.submit(t);
		}
	}

	// search 3D world for nearby entities
	auto search = entities()->searchEntities(512);

	// submit nearby entities for rendering
	m_renderQueue.submit(search);
}

// rendering
void Game01::draw() {
	// set viewport (Make this automatic)
	RECT dim = getWindowSize();
	UINT width = dim.right - dim.left;
	UINT height = dim.bottom - dim.top;
	renderer()->setViewport(width, height);

	// render queue drawing
	m_renderQueue.draw();
}
