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
}

DXGLFoliageManager::~DXGLFoliageManager() {
}

void DXGLFoliageManager::update(long double delta) {
	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");
	Vec3f camPos = cam->getPosition();

	int count = 0;
	for (auto it = m_chunks.begin(); it != m_chunks.end(); it++) {
		FoliageChunk& chunk = it->second;

		// update chunk LOD
		float chunkLength = chunk.maxVertex.x - chunk.minVertex.x;
		float distToCam = Vec3f::dist(camPos, chunk.minVertex + chunkLength / 2.0f);

		// Define the desired increment step
		float incrementStep = 0.025f;

		if (distToCam <= chunkLength / 4.0f) {
			chunk.LOD = 1.0f;
		} else {
			float rawLOD = Math::smoothstep(1.0f / (1.0f + (distToCam - chunkLength / 4.0f) * 0.1f), 0, 1);

			// Round the rawLOD to the nearest multiple of incrementStep
			chunk.LOD = std::round(rawLOD / incrementStep) * incrementStep;

			// Ensure LOD is not greater than 1 or less than 0
			chunk.LOD = std::clamp<float>(chunk.LOD, 0.05f, 1.0f);
		}

		// get next buffer location for updating instances
		auto location = m_foliage.begin() + count;
		count += chunk.foliage.size() * chunk.LOD;

		// resize to fit instances
		m_foliage.resize(count);

		// Copy the instances
		Math::copyPercentage(chunk.foliage.begin(), chunk.foliage.end(), location, chunk.LOD);
	}

	if (!m_foliage.empty()) {
		m_vbInstance = DXGLMain::resource()->createVertexBuffer(&m_foliage[0], m_foliage.size(), sizeof(FoliageInstance));
	}

	m_timePassed += delta;

	FoliageBuffer buffer{};
	buffer.model.setIdentity();
	buffer.view = cam->view();
	buffer.proj = cam->proj();
	buffer.camPos = cam->getPosition();
	buffer.time = m_timePassed;
	m_cb->update(&buffer);
}

void DXGLFoliageManager::loadTerrain(const QuadTree<TerrainChunk>::list& terrain) {
	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");
	Vec3f camPos = cam->getPosition();

	m_foliage.clear();

	int totalGrassBlades = 0;

	std::for_each(std::execution::par, terrain.begin(), terrain.end(), [&](auto t) {
		FoliageChunk chunk{};

		chunk.minVertex = t->minVertex;
		chunk.maxVertex = t->maxVertex;

		for (int i = 0; i < t->faces.size(); i++) {
			TerrainFace& face = t->faces[i];
			Vec2f A = { face.v0.x, face.v0.z };
			Vec2f B = { face.v1.x, face.v1.z };
			Vec2f C = { face.v2.x, face.v2.z };

			std::vector<Vec2f> foliagePositions = Math::fillTriangle(A, B, C, 40);

			for (Vec2f& p : foliagePositions) {
				FoliageInstance foliage{};

				float scale = 0.5f + (std::rand() % 25) * 0.01f;
				foliage.scale = Vec3f{ 0.1f, scale, 0.1f };

				float rotation = (std::rand() % 314) * 0.01f;
				foliage.rotation = Vec3f{ 0, rotation, 0 };

				float offX = (std::rand() % ((int)GRASS_TILE_SIZE * 10)) * 0.01f;
				float offZ = (std::rand() % ((int)GRASS_TILE_SIZE * 10)) * 0.01f;
				foliage.translation = Vec3f{ p.x + offX, Math::barycentricHeight(face.v0, face.v1, face.v2, p), p.y + offZ };

				foliage.color0 = Vec3f{ 0.25f, 0.25f, 0.2f };  // Dark brown
				foliage.color1 = Vec3f{ 0.3f, 0.25f, 0.1f };   // Dark yellowish brown
				foliage.color2 = Vec3f{ 0.25f, 0.2f, 0.1f };  // Slightly more yellowish greenish brown
				foliage.color3 = Vec3f{ 0.2f, 0.2f, 0.1f };  // Slightly more yellowish green

				foliage.timeOffset = p.x * (6.28f) / GRASS_DENSITY + p.y * (6.28f) / GRASS_DENSITY;
				chunk.foliage.push_back(std::move(foliage));
			}
		}
		totalGrassBlades += chunk.foliage.size();
		m_chunks.insert(std::pair<uint32_t, FoliageChunk>(t->id, chunk));
	});

	m_foliage.reserve(totalGrassBlades);
}

void DXGLFoliageManager::unloadTerrain(const QuadTree<TerrainChunk>::list& terrain) {
	for (auto t : terrain) {
		m_chunks.erase(t->id);
	}
}

void DXGLFoliageManager::draw() {
	DXGLMain::renderer()->input()->setInputLayout(m_layout);
	DXGLMain::renderer()->input()->setVertexBuffer(0, 1, &m_mesh->getMeshVertexBuffer());
	DXGLMain::renderer()->input()->setIndexBuffer(m_mesh->getIndexBuffer());

	if (!m_foliage.empty()) {
		DXGLMain::renderer()->input()->setVertexBuffer(1, 1, &m_vbInstance);
	}

	DXGLMain::renderer()->shader()->VS_setShader(m_vs);
	DXGLMain::renderer()->shader()->PS_setShader(m_ps);

	DXGLMain::renderer()->shader()->VS_setCBuffer(0, 1, m_cb->get());

	for (auto& mesh : m_mesh->getMeshes()) {
		DXGLMain::renderer()->drawIndexedTriangleListInstanced(mesh.indexCount, m_foliage.size(), mesh.baseIndex, mesh.baseVertex, 0);
	}
}

void DXGLFoliageManager::cull() {
}
