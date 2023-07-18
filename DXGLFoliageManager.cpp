#include "DXGLFoliageManager.h"

using namespace dxgl;

DXGLFoliageManager::DXGLFoliageManager() {
	InputLayoutDesc desc{};
	desc.add("POSITION", 0, FLOAT3, false);
	desc.add("TEXCOORD", 0, FLOAT2, false);
	desc.add("NORMAL",   0, FLOAT3, false);
	desc.add("INSTANCE_SCALE",        1, FLOAT3, true);
	desc.add("INSTANCE_ROTATION",     1, FLOAT3, true);
	desc.add("INSTANCE_TRANSLATION",  1, FLOAT3, true);

	desc.add("INSTANCE_COLOR_ZERO",  1, FLOAT3, true);
	desc.add("INSTANCE_COLOR_ONE",   1, FLOAT3, true);
	desc.add("INSTANCE_COLOR_TWO",   1, FLOAT3, true);
	desc.add("INSTANCE_COLOR_THREE", 1, FLOAT3, true);

	desc.add("INSTANCE_TIME_OFFSET", 1, FLOAT1, true);
	m_layout = DXGLMain::resource()->createInputLayout(desc, "Assets/Shaders/VS_FoliageShader.cso");
	m_vs = DXGLMain::resource()->createShader<DXGLVertexShader>("Assets/Shaders/VS_FoliageShader.cso");
	m_ps = DXGLMain::resource()->createShader<DXGLPixelShader>("Assets/Shaders/PS_FoliageShader.cso");

	m_cb = DXGLMain::resource()->createCBuffer(sizeof(FoliageBuffer));

	MeshDesc meshDesc{};
	meshDesc.vertexAttributes = VERTEX_POSITION | VERTEX_TEXCOORD | VERTEX_NORMAL;
	meshDesc.miscAttributes = MISC_INDEX;
	m_mesh = DXGLMain::resource()->createBasicMesh(meshDesc, "Assets/Meshes/landscapes/grass.fbx");

	generateFoliage();

	// init spaces
	m_foliageChunks.resize(GRASS_TILE_LENGTH * GRASS_TILE_LENGTH);

	for (int x = 0; x < GRASS_TILE_LENGTH; x++) {
		for (int z = 0; z < GRASS_TILE_LENGTH; z++) {
			FoliageChunk& chunk = m_foliageChunks[x * GRASS_TILE_LENGTH + z];
			chunk.bufferLocation = (x * GRASS_TILE_LENGTH) * (GRASS_DENSITY * GRASS_DENSITY) + z * (GRASS_DENSITY * GRASS_DENSITY);

			chunk.curX = x * GRASS_TILE_SIZE;
			chunk.curZ = z * GRASS_TILE_SIZE;

			chunk.lastX = x * GRASS_TILE_SIZE;
			chunk.lastZ = z * GRASS_TILE_SIZE;
		}
	}
}

DXGLFoliageManager::~DXGLFoliageManager() {
}

void DXGLFoliageManager::generateFoliage() {
	m_foliage.reserve(GRASS_BLADES_MAX);
	m_culledFoliage.reserve(GRASS_BLADES_MAX);

	for (int chunkX = 0; chunkX < GRASS_TILE_LENGTH; chunkX++) {
		for (int chunkZ = 0; chunkZ < GRASS_TILE_LENGTH; chunkZ++) {
			for (int i = 0; i < GRASS_DENSITY; i++) {
				for (int j = 0; j < GRASS_DENSITY; j++) {
					float x = (chunkX * GRASS_TILE_SIZE) + ((float) i / GRASS_DENSITY) * GRASS_TILE_SIZE;
					float y = 0;
					float z = (chunkZ * GRASS_TILE_SIZE) + ((float) j / GRASS_DENSITY) * GRASS_TILE_SIZE;

					FoliageInstance grass{};

					float scale = 0.25f + (std::rand() % 25) * 0.01f;
					grass.scale = Vec3f{ 0.1f, scale, 0.1f };

					float rotation = (std::rand() % 314) * 0.01f;
					grass.rotation = Vec3f{ 0, rotation, 0 };

					float offX = (std::rand() % ((int) GRASS_TILE_SIZE * 10)) * 0.01f;
					float offZ = (std::rand() % ((int) GRASS_TILE_SIZE * 10)) * 0.01f;
					grass.translation = { x + offX, y, z + offZ };

					grass.color0 = Vec3f{ 0.025f, 0.3f, 0.0f };
					grass.color1 = Vec3f{ 0.0125f, 0.2f, 0.0f };
					grass.color2 = Vec3f{ 0.0f, 0.1f, 0.0f };
					grass.color3 = Vec3f{ 0.0f, 0.05f, 0.0f };

					grass.timeOffset = x * (6.28f) / GRASS_DENSITY + z * (6.28f) / GRASS_DENSITY;

					m_foliage.push_back(grass);
				}
			}
		}
	}
}

void DXGLFoliageManager::update(long double delta) {

	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");
	Vec3f camPos = cam->getPosition();

	// update spaces
	m_visibleBufferLocations.clear();

	for (int x = 0; x < GRASS_TILE_LENGTH; x++) {
		for (int z = 0; z < GRASS_TILE_LENGTH; z++) {
			FoliageChunk& chunk = m_foliageChunks[x * GRASS_TILE_LENGTH + z];
			chunk.curX = camPos.x;
			chunk.curZ = camPos.z;

			bool isVisible = !cam->cull(Vec3f{ chunk.lastX, 0, chunk.lastZ }, Vec3f{ 1, 1, 1 }, Vec3f{ 0, 0, 0 }, Vec3f{ GRASS_TILE_SIZE, 1, GRASS_TILE_SIZE });

			if (isVisible) {
				m_visibleBufferLocations.push_back(chunk.bufferLocation);
			}

			if (chunk.lastX + GRASS_TOTAL_LENGTH / 2 <= chunk.curX) {
				chunk.lastX += GRASS_TOTAL_LENGTH;
				for (int i = 0; i < (GRASS_DENSITY * GRASS_DENSITY); i++) {
					FoliageInstance& foliage = m_foliage[chunk.bufferLocation + i];
					foliage.translation.x += GRASS_TOTAL_LENGTH;
				}
			} else if (chunk.lastX - GRASS_TOTAL_LENGTH / 2 >= chunk.curX) {
				chunk.lastX -= GRASS_TOTAL_LENGTH;
				for (int i = 0; i < (GRASS_DENSITY * GRASS_DENSITY); i++) {
					FoliageInstance& foliage = m_foliage[chunk.bufferLocation + i];
					foliage.translation.x -= GRASS_TOTAL_LENGTH;
				}
			}

			if (chunk.lastZ + GRASS_TOTAL_LENGTH / 2 <= chunk.curZ) {
				chunk.lastZ += GRASS_TOTAL_LENGTH;
				for (int i = 0; i < (GRASS_DENSITY * GRASS_DENSITY); i++) {
					FoliageInstance& foliage = m_foliage[chunk.bufferLocation + i];
					foliage.translation.z += GRASS_TOTAL_LENGTH;
				}
			} else if (chunk.lastZ - GRASS_TOTAL_LENGTH / 2 >= chunk.curZ) {
				chunk.lastZ -= GRASS_TOTAL_LENGTH;
				for (int i = 0; i < (GRASS_DENSITY * GRASS_DENSITY); i++) {
					FoliageInstance& foliage = m_foliage[chunk.bufferLocation + i];
					foliage.translation.z -= GRASS_TOTAL_LENGTH;
				}
			}
		}

		m_culledFoliage.resize(m_visibleBufferLocations.size() * (GRASS_DENSITY * GRASS_DENSITY));

		for (int i = 0; i < m_visibleBufferLocations.size(); i++) {
			int loc = m_visibleBufferLocations[i];
			auto start = m_foliage.begin() + loc;
			auto end = start + (GRASS_DENSITY * GRASS_DENSITY);
			std::copy(start, end, m_culledFoliage.begin() + (GRASS_DENSITY * GRASS_DENSITY) * i);
		}
	}

	if (!m_culledFoliage.empty()) {
		m_vbInstance = DXGLMain::resource()->createVertexBuffer(&m_culledFoliage[0], m_culledFoliage.size(), sizeof(FoliageInstance));
	}

	m_timePassed += delta;

	FoliageBuffer buffer{};
	buffer.model.setIdentity();
	buffer.view = cam->view();
	buffer.proj = cam->proj();
	buffer.time = m_timePassed;
	m_cb->update(&buffer);
}

void DXGLFoliageManager::draw() {
	DXGLMain::renderer()->input()->setInputLayout(m_layout);
	DXGLMain::renderer()->input()->setVertexBuffer(0, 1, &m_mesh->getMeshVertexBuffer());
	DXGLMain::renderer()->input()->setIndexBuffer(m_mesh->getIndexBuffer());

	if (!m_culledFoliage.empty()) {
		DXGLMain::renderer()->input()->setVertexBuffer(1, 1, &m_vbInstance);
	}

	DXGLMain::renderer()->shader()->VS_setShader(m_vs);
	DXGLMain::renderer()->shader()->PS_setShader(m_ps);

	DXGLMain::renderer()->shader()->VS_setCBuffer(0, 1, m_cb->get());

	for (auto& mesh : m_mesh->getMeshes()) {
		DXGLMain::renderer()->drawIndexedTriangleListInstanced(mesh.indexCount, m_culledFoliage.size(), mesh.baseIndex, mesh.baseVertex, 0);
	}
}

void DXGLFoliageManager::cull() {
}
