#include "DXGLFoliageManager.h"

#include "Engine.h"
#include "ResourceManager.h"
#include "CameraManager.h"

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
	m_layout = Engine::resource()->createInputLayout(desc, "Assets/Shaders/VS_FoliageShader.cso");
	m_vs = Engine::resource()->createShader<DXGLVertexShader>("Assets/Shaders/VS_FoliageShader.cso");
	m_ps = Engine::resource()->createShader<DXGLPixelShader>("Assets/Shaders/PS_FoliageShader.cso");

	m_cb = Engine::resource()->createVSConstantBuffer(sizeof(FoliageBuffer));

	MeshDesc meshDesc{};
	meshDesc.vertexAttributes = VERTEX_POSITION | VERTEX_TEXCOORD | VERTEX_NORMAL;
	meshDesc.miscAttributes = MISC_INDEX;
	m_mesh = Engine::resource()->createMesh(meshDesc, "Assets/Meshes/landscapes/grass.fbx");

	m_foliage.reserve(2000000);
}

DXGLFoliageManager::~DXGLFoliageManager() {
}

void DXGLFoliageManager::update(long double delta) {
	auto& cam = Engine::camera()->getActiveCamera();

	Vec3f camPos = cam.translation;

	// check futures
	for (auto it = m_futures.begin(); it != m_futures.end();) {
		auto& f = it->second;
		auto status = f.wait_for(std::chrono::seconds(0));

		if (status == std::future_status::ready) {
			std::vector<FoliageChunk> chunks = std::move(f.get());
			for (auto& chunk : chunks) {
				m_chunks.emplace(chunk.id, std::move(chunk));
			}
			it = m_futures.erase(it); // Erase the element and get the next iterator.
		} else {
			it++; // Move to the next element.
		}
	}

	m_foliage.clear();

	int count = 0;
	for (auto it = m_chunks.begin(); it != m_chunks.end(); it++) {
		FoliageChunk& chunk = it->second;

		// cull check
		if (Engine::camera()->cullActiveCamera(chunk.minVertex, Vec3f{ 1, 1, 1 }, Vec3f{ 0, 0, 0 }, chunk.maxVertex - chunk.minVertex)) {
			continue;
		}

		// update chunk LOD
		float chunkLength = Vec3f::length(chunk.maxVertex - chunk.minVertex);
		float distToCam = Vec3f::dist(camPos, chunk.minVertex + (chunk.maxVertex - chunk.minVertex) / 2.0f);

		// Define the desired increment step
		float incrementStep = 0.025f;

		if (distToCam <= chunkLength) {
			chunk.LOD = 1.0f;
		} else {
			float rawLOD = Math::smoothstep(1.0f / (1.0f + (distToCam - chunkLength) * 0.1f), 0, 1);

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
		m_vbInstance = Engine::resource()->createVertexBuffer(&m_foliage[0], m_foliage.size(), sizeof(FoliageInstance));
	}

	m_timePassed += delta;

	FoliageBuffer buffer{};
	buffer.model.setIdentity();
	buffer.view = cam.view();
	buffer.proj = cam.proj();
	buffer.camPos = cam.translation;
	buffer.time = m_timePassed;
	m_cb->update(&buffer);
}

void DXGLFoliageManager::draw() {
	m_layout->bind();
	m_mesh->getMeshVertexBuffer()->bind(0);
	m_mesh->getIndexBuffer()->bind();

	if (!m_foliage.empty()) {
		m_vbInstance->bind(1);
	}

	Engine::renderer()->shader()->VS_setShader(m_vs);
	Engine::renderer()->shader()->PS_setShader(m_ps);

	m_cb->bind(0);

	for (auto& mesh : m_mesh->getMeshes()) {
		Engine::renderer()->drawIndexedTriangleListInstanced(mesh.indexCount, m_foliage.size(), mesh.baseIndex, mesh.baseVertex, 0);
	}
}

void DXGLFoliageManager::loadTerrain(const QuadTree<TerrainChunk>::list& terrain) {
	for (const auto& t : terrain) {
		m_futures.insert(std::pair<uint32_t, std::future<std::vector<FoliageChunk>>>(t->id, std::async(std::launch::async, [this, t]() { return asyncLoadTerrain(t); })));
	}
}

std::vector<FoliageChunk> DXGLFoliageManager::asyncLoadTerrain(const QuadTree<TerrainChunk>::ptr& terrain) {
	return generateChunks(terrain);
}

void DXGLFoliageManager::unloadTerrain(const QuadTree<TerrainChunk>::list& terrain) {
	for (auto t : terrain) {
		m_chunks.erase(t->id);
		m_chunks.erase(t->id + 1);
		m_chunks.erase(t->id + 2);
		m_chunks.erase(t->id + 3);
	}
}

std::vector<FoliageChunk> DXGLFoliageManager::generateChunks(const QuadTree<TerrainChunk>::ptr& terrain) {
	Vec3f minVertex = terrain->minVertex;
	Vec3f maxVertex = terrain->maxVertex;

	float chunkLengthX = maxVertex.x - minVertex.x;
	float chunkLengthZ = maxVertex.z - minVertex.z;

	QuadTreeRect rect{};
	QuadTree<TerrainFace> faces{rect, 8};
	rect.pos = Vec2f{ minVertex.x, minVertex.z };
	rect.size = Vec2f{ chunkLengthX , chunkLengthZ };
	faces.resize(rect);

	Vec3f up = { 0, 1, 0 };
	for (const auto& f : terrain->faces) {
		if (Vec3f::dot(f.normal, up) < 0.75f) {
			continue;
		}

		QuadTreeRect faceRect{};
		faceRect.pos = { f.center.x, f.center.z };
		faceRect.size = { 0.01f, 0.01f };
		faces.insert(f, faceRect);
	}

	std::vector<FoliageChunk> chunks{};

	FoliageChunk topLeft{};
	topLeft.id = terrain->id;
	topLeft.minVertex.x = minVertex.x;
	topLeft.minVertex.y = minVertex.y;
	topLeft.minVertex.z = minVertex.z;
	topLeft.maxVertex.x = minVertex.x + chunkLengthX / 2.0f;
	topLeft.maxVertex.y = maxVertex.y;
	topLeft.maxVertex.z = minVertex.z + chunkLengthZ / 2.0f;
	chunks.push_back(topLeft);

	FoliageChunk topRight{};
	topRight.id = terrain->id + 1;
	topRight.minVertex.x = minVertex.x + chunkLengthX / 2.0f;
	topRight.minVertex.y = minVertex.y;
	topRight.minVertex.z = minVertex.z;
	topRight.maxVertex.x = minVertex.x + chunkLengthX;
	topRight.maxVertex.y = maxVertex.y;
	topRight.maxVertex.z = minVertex.z + chunkLengthZ / 2.0f;
	chunks.push_back(topRight);

	FoliageChunk bottomLeft{};
	bottomLeft.id = terrain->id + 2;
	bottomLeft.minVertex.x = minVertex.x;
	bottomLeft.minVertex.y = minVertex.y;
	bottomLeft.minVertex.z = minVertex.z + chunkLengthZ / 2.0f;
	bottomLeft.maxVertex.x = minVertex.x + chunkLengthX / 2.0f;
	bottomLeft.maxVertex.y = maxVertex.y;
	bottomLeft.maxVertex.z = minVertex.z + chunkLengthZ;
	chunks.push_back(bottomLeft);

	FoliageChunk bottomRight{};
	bottomRight.id = terrain->id + 3;
	bottomRight.minVertex.x = minVertex.x + chunkLengthX / 2.0f;
	bottomRight.minVertex.y = minVertex.y;
	bottomRight.minVertex.z = minVertex.z + chunkLengthZ / 2.0f;
	bottomRight.maxVertex.x = minVertex.x + chunkLengthX;
	bottomRight.maxVertex.y = maxVertex.y;
	bottomRight.maxVertex.z = minVertex.z + chunkLengthZ;
	chunks.push_back(bottomRight);

	for (auto& chunk : chunks) {
		QuadTreeRect search{};
		search.pos = Vec2f{ chunk.minVertex.x, chunk.minVertex.z };
		search.size = Vec2f{ chunk.maxVertex.x - chunk.minVertex.x, chunk.maxVertex.z - chunk.minVertex.z };
		QuadTree<TerrainFace>::list searchedFaces = faces.search(search);

		for (const QuadTree<TerrainFace>::ptr face : searchedFaces) {
			Vec2f A = { face->v0.x, face->v0.z };
			Vec2f B = { face->v1.x, face->v1.z };
			Vec2f C = { face->v2.x, face->v2.z };

			std::vector<Vec2f> foliagePositions = Math::fillTriangle(A, B, C, 30);

			for (Vec2f& p : foliagePositions) {
				FoliageInstance foliage{};

				std::random_device rd;
				std::uniform_real_distribution<float> dist(0.0f, 1.0f);

				float scale = 0.25f + dist(rd) * 0.25f;
				foliage.scale = Vec3f{ scale * 0.25f, scale, scale * 0.25f };

				float rotation = dist(rd) * 3.14f;
				foliage.rotation = Vec3f{ 0, rotation, 0 };

				p.x += dist(rd) * 0.25f;
				p.y += dist(rd) * 0.25f;
				foliage.translation = Vec3f{ p.x, Math::barycentricHeight(face->v0, face->v1, face->v2, p), p.y };

				// Modify the colors to create a brownish-green transition with added green and yellow
				foliage.color0 = Vec3f{ 0.2f, 0.25f, 0.15f };    // Lighter brownish-green with more green
				foliage.color1 = Vec3f{ 0.18f, 0.18f, 0.1f };   // Light brownish-green with more green
				foliage.color2 = Vec3f{ 0.15f, 0.16f, 0.08f };  // Dark brownish-green
				foliage.color3 = Vec3f{ 0.1f, 0.14f, 0.05f };   // Darker brownish-green with a little yellow

				foliage.timeOffset = p.x * (6.28f * 0.0075f) + p.y * (6.28f * 0.0075f);
				chunk.foliage.push_back(std::move(foliage));
			}
		}
	}

	return chunks;
}
