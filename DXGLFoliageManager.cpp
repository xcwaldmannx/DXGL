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

			chunk.curX = 0;
			chunk.curZ = 0;

			chunk.lastX = x * GRASS_TILE_SIZE + GRASS_TILE_SIZE / 2;
			chunk.lastZ = z * GRASS_TILE_SIZE + GRASS_TILE_SIZE / 2;
			// chunk.lastY = DXGLMain::renderer()->terrain()->getTerrainHeight(chunk.lastX, chunk.lastZ);

			chunk.LOD = 0;
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

					grass.color0 = Vec3f{ 0.025f, 0.1f, 0.0f };
					grass.color1 = Vec3f{ 0.0125f, 0.05f, 0.0f };
					grass.color2 = Vec3f{ 0.0f, 0.01f, 0.0f };
					grass.color3 = Vec3f{ 0.0f, 0.005f, 0.0f };

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
	m_bufferAdvance.clear();
	m_chunkLODs.clear();

	unsigned int totalGrassBlades = 0;

	for (int x = 0; x < GRASS_TILE_LENGTH; x++) {
		for (int z = 0; z < GRASS_TILE_LENGTH; z++) {
			FoliageChunk& chunk = m_foliageChunks[x * GRASS_TILE_LENGTH + z];
			chunk.curX = camPos.x;
			chunk.curZ = camPos.z;

			float distToCam = Vec3f::dist(camPos, Vec3f{ chunk.lastX, chunk.lastY, chunk.lastZ });
			if (distToCam < GRASS_TILE_SIZE * 2) {
				chunk.LOD = 1.0f;
			} else {
				// Define the desired increment step
				float incrementStep = 0.025f;
				
				float rawLOD = Math::smoothstep(1.0f / (1.0f + (distToCam - GRASS_TILE_SIZE * 2) * 0.1f), 0, 1);

				// Round the rawLOD to the nearest multiple of incrementStep
				chunk.LOD = std::round(rawLOD / incrementStep) * incrementStep;

				// Ensure chunk.LOD is not greater than 1 or less than 0
				chunk.LOD = std::clamp<float>(chunk.LOD, 0.0f, 1.0f);
			}

			bool isVisible = !cam->cull(Vec3f{ chunk.lastX, chunk.lastY, chunk.lastZ }, Vec3f{ 1, 1, 1 },
				Vec3f{ -GRASS_TILE_SIZE / 2, -1, -GRASS_TILE_SIZE / 2 }, Vec3f{ GRASS_TILE_SIZE / 2, 5, GRASS_TILE_SIZE / 2 });

			if (isVisible) {
				int grassBladeCount = (GRASS_DENSITY * GRASS_DENSITY) * chunk.LOD;
				totalGrassBlades += grassBladeCount;

				m_visibleBufferLocations.push_back(chunk.bufferLocation);
				m_bufferAdvance.push_back(grassBladeCount);
				m_chunkLODs.push_back(chunk.LOD);
			}

			if (chunk.lastX + GRASS_TOTAL_LENGTH / 2 <= chunk.curX) {
				chunk.lastX += GRASS_TOTAL_LENGTH;
				chunk.lastY = DXGLMain::renderer()->terrain()->getTerrainHeight(chunk.lastX, chunk.lastZ);;

				for (int i = 0; i < (GRASS_DENSITY * GRASS_DENSITY); i++) {
					FoliageInstance& foliage = m_foliage[chunk.bufferLocation + i];
					foliage.translation.x += GRASS_TOTAL_LENGTH;
					foliage.translation.y = DXGLMain::renderer()->terrain()->getTerrainHeight(foliage.translation.x, foliage.translation.z);
				}
			} else if (chunk.lastX - GRASS_TOTAL_LENGTH / 2 >= chunk.curX) {
				chunk.lastX -= GRASS_TOTAL_LENGTH;
				chunk.lastY = DXGLMain::renderer()->terrain()->getTerrainHeight(chunk.lastX, chunk.lastZ);;

				for (int i = 0; i < (GRASS_DENSITY * GRASS_DENSITY); i++) {
					FoliageInstance& foliage = m_foliage[chunk.bufferLocation + i];
					foliage.translation.x -= GRASS_TOTAL_LENGTH;
					foliage.translation.y = DXGLMain::renderer()->terrain()->getTerrainHeight(foliage.translation.x, foliage.translation.z);
				}
			}

			if (chunk.lastZ + GRASS_TOTAL_LENGTH / 2 <= chunk.curZ) {
				chunk.lastZ += GRASS_TOTAL_LENGTH;
				chunk.lastY = DXGLMain::renderer()->terrain()->getTerrainHeight(chunk.lastX, chunk.lastZ);;

				for (int i = 0; i < (GRASS_DENSITY * GRASS_DENSITY); i++) {
					FoliageInstance& foliage = m_foliage[chunk.bufferLocation + i];
					foliage.translation.z += GRASS_TOTAL_LENGTH;
					foliage.translation.y = DXGLMain::renderer()->terrain()->getTerrainHeight(foliage.translation.x, foliage.translation.z);
				}
			} else if (chunk.lastZ - GRASS_TOTAL_LENGTH / 2 >= chunk.curZ) {
				chunk.lastZ -= GRASS_TOTAL_LENGTH;
				chunk.lastY = DXGLMain::renderer()->terrain()->getTerrainHeight(chunk.lastX, chunk.lastZ);;

				for (int i = 0; i < (GRASS_DENSITY * GRASS_DENSITY); i++) {
					FoliageInstance& foliage = m_foliage[chunk.bufferLocation + i];
					foliage.translation.z -= GRASS_TOTAL_LENGTH;
					foliage.translation.y = DXGLMain::renderer()->terrain()->getTerrainHeight(foliage.translation.x, foliage.translation.z);
				}
			}
		}
	}

	m_culledFoliage.clear();
	m_culledFoliage.resize(totalGrassBlades);

	auto dest = m_culledFoliage.begin();
	for (int i = 0; i < m_visibleBufferLocations.size(); i++) {
		auto start = m_foliage.begin() + m_visibleBufferLocations[i];
		auto end = start + (GRASS_DENSITY * GRASS_DENSITY);
		Math::copyPercentage(start, end, dest, m_chunkLODs[i]);
		dest += m_bufferAdvance[i];
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
