#include "DXGLTerrainManager.h"

#include "Engine.h"
#include "ResourceManager.h"
#include "CameraManager.h"

using namespace dxgl;

DXGLTerrainManager::DXGLTerrainManager() {
	InputLayoutDesc descLayout{};
	descLayout.add("POSITION", 0, FLOAT3, false);
	descLayout.add("TEXCOORD", 0, FLOAT2, false);
	descLayout.add("NORMAL",   0, FLOAT3, false);
	descLayout.add("TANGENT",  0, FLOAT3, false);
	m_layout = Engine::resource()->createInputLayout(descLayout, "Assets/Shaders/VS_TerrainShader.cso");

	m_vs = Engine::resource()->createShader<DXGLVertexShader>("Assets/Shaders/VS_TerrainShader.cso");
	m_ps = Engine::resource()->createShader<DXGLPixelShader>("Assets/Shaders/PS_TerrainShader.cso");

	m_vscb = Engine::resource()->createVSConstantBuffer(sizeof(TerrainBuffer));
	m_pscb = Engine::resource()->createPSConstantBuffer(sizeof(TerrainBuffer));
}

DXGLTerrainManager::~DXGLTerrainManager() {
}

void DXGLTerrainManager::load(const MeshDesc& desc, const std::string& filename) {
	m_mesh = Engine::resource()->createMesh(desc, filename);

	AABB aabb = m_mesh->getAABB();

	float scale = 50.0f;

	float minX = aabb.min.x * scale;
	float minY = aabb.min.y * scale;
	float minZ = aabb.min.z * scale;

	float maxX = aabb.max.x * scale;
	float maxY = aabb.max.y * scale;
	float maxZ = aabb.max.z * scale;

	chunkSize = std::ceil((std::abs(minX) + maxX) / 100.0f);

	m_area = { Vec2f{ minX, minZ }, Vec2f{ maxX - minX, maxZ - minZ } };
	m_chunkTree.resize(m_area);

	int idx = 0;
	int idy = 0;
	for (int x = minX; x < maxX; x += chunkSize) {
		for (int z = minZ; z < maxZ; z += chunkSize) {
			TerrainChunk chunk{};
			chunk.id = idx + idy;
			chunk.minVertex = Vec3f(x, 0, z);
			chunk.maxVertex = Vec3f(x + chunkSize, 0, z + chunkSize);
			QuadTreeRect rect = { Vec2f(x, z), Vec2f(chunkSize, chunkSize) };
			m_chunkTree.insert(chunk, rect);

			idy += 4;
		}
		idx += 4;
	}

	std::vector<Face> faces = m_mesh->getFaces();
	std::vector<Vec3f> faceCenters{};

	for (int i = 0; i < faces.size(); i++) {
		const Face& face = faces[i];
		float cx = (face.v0.x * scale + face.v1.x * scale + face.v2.x * scale) / 3.0f;
		float cy = (face.v0.y * scale + face.v1.y * scale + face.v2.y * scale) / 3.0f;
		float cz = (face.v0.z * scale + face.v1.z * scale + face.v2.z * scale) / 3.0f;
		Vec3f faceCenter = { cx, cy, cz };
		faceCenters.push_back(faceCenter);
	}

	for (int faceCenterIndex = 0; faceCenterIndex < faceCenters.size(); faceCenterIndex++) {
		Vec3f& faceCenter = faceCenters[faceCenterIndex];
		QuadTreeRect searchFace{ Vec2f{ faceCenter.x, faceCenter.z }, Vec2f{ 0.001f, 0.001f } };

		QuadTree<TerrainChunk>::list searchedChunks = m_chunkTree.search(searchFace);
		for (auto chunkIt = searchedChunks.begin(); chunkIt != searchedChunks.end(); chunkIt++) {
			auto chunk = *chunkIt;
			chunk->faceIndices.push_back(faces[faceCenterIndex].indices.x);
			chunk->faceIndices.push_back(faces[faceCenterIndex].indices.y);
			chunk->faceIndices.push_back(faces[faceCenterIndex].indices.z);
			chunk->indexCount += 3;

			// set chunk bounds based on extent of faces
			TerrainFace face{};
			face.v0 = faces[faceCenterIndex].v0 * scale;
			face.v1 = faces[faceCenterIndex].v1 * scale;
			face.v2 = faces[faceCenterIndex].v2 * scale;

			// x
			if (face.v0.x < chunk->minVertex.x) chunk->minVertex.x = face.v0.x;
			if (face.v1.x < chunk->minVertex.x) chunk->minVertex.x = face.v1.x;
			if (face.v2.x < chunk->minVertex.x) chunk->minVertex.x = face.v2.x;
										 					 
			if (face.v0.x > chunk->maxVertex.x) chunk->maxVertex.x = face.v0.x;
			if (face.v1.x > chunk->maxVertex.x) chunk->maxVertex.x = face.v1.x;
			if (face.v2.x > chunk->maxVertex.x) chunk->maxVertex.x = face.v2.x;
										 					 
			// y				 					 
			if (face.v0.y < chunk->minVertex.y) chunk->minVertex.y = face.v0.y;
			if (face.v1.y < chunk->minVertex.y) chunk->minVertex.y = face.v1.y;
			if (face.v2.y < chunk->minVertex.y) chunk->minVertex.y = face.v2.y;
										 					 
			if (face.v0.y > chunk->maxVertex.y) chunk->maxVertex.y = face.v0.y;
			if (face.v1.y > chunk->maxVertex.y) chunk->maxVertex.y = face.v1.y;
			if (face.v2.y > chunk->maxVertex.y) chunk->maxVertex.y = face.v2.y;
										 					 
			// z				 					 
			if (face.v0.z < chunk->minVertex.z) chunk->minVertex.z = face.v0.z;
			if (face.v1.z < chunk->minVertex.z) chunk->minVertex.z = face.v1.z;
			if (face.v2.z < chunk->minVertex.z) chunk->minVertex.z = face.v2.z;
										 					 
			if (face.v0.z > chunk->maxVertex.z) chunk->maxVertex.z = face.v0.z;
			if (face.v1.z > chunk->maxVertex.z) chunk->maxVertex.z = face.v1.z;
			if (face.v2.z > chunk->maxVertex.z) chunk->maxVertex.z = face.v2.z;

			// calc face normal
			Vec3f U = face.v1 - face.v0;
			Vec3f V = face.v2 - face.v0;

			face.normal.x = U.y * V.z - U.z * V.y;
			face.normal.y = U.z * V.x - U.x * V.z;
			face.normal.z = U.x * V.y - U.y * V.x;

			face.normal = face.normal.normalize();

			face.center = faceCenter;

			chunk->faces.push_back(face);

			break;
		}
	}
}

void DXGLTerrainManager::update(long double delta) {
	m_indices.clear();

	auto& cam = Engine::camera()->getActiveCamera();

	auto chunk = m_chunkTree.begin();
	while (chunk != m_chunkTree.end()) {
		if (!Engine::camera()->cullActiveCamera(chunk->minVertex, Vec3f{ 1, 1, 1 }, Vec3f{ 0, 0, 0 }, chunk->maxVertex - chunk->minVertex)) {
			m_indices.insert(m_indices.end(), chunk->faceIndices.begin(), chunk->faceIndices.end());
		}
		chunk++;
	}

	if (m_indices.size() > 0) {
		m_ib = Engine::resource()->createIndexBuffer(&m_indices[0], m_indices.size());
	}

	float searchSize = 128.0f;
	m_searchArea = { Vec2f(cam.translation.x - (searchSize / 2.0f), cam.translation.z - (searchSize / 2.0f)),
		Vec2f(searchSize, searchSize) };

	std::list<Vec3f> foliagePositions{};

	auto searched = m_chunkTree.search(m_searchArea);
	if (searched != m_lastSearch) {
		// get chunks to load/unload
		QuadTree<TerrainChunk>::list diffLoad{};
		QuadTree<TerrainChunk>::list diffUnload{};
		for (auto it = m_lastSearch.begin(); it != m_lastSearch.end(); it++) {
			if (std::find(searched.begin(), searched.end(), *it) == searched.end()) {
				diffUnload.push_back(*it);
			}
			else {
				diffLoad.push_back(*it);
			}
		}

		Engine::renderer()->foliage()->unloadTerrain(diffUnload);
		Engine::renderer()->foliage()->loadTerrain(diffLoad);

		m_lastSearch = searched;
	}
}

void DXGLTerrainManager::draw() {
	m_layout->bind();
	m_mesh->getMeshVertexBuffer()->bind(0);
	if (m_ib)
	m_ib->bind();

	Engine::renderer()->shader()->VS_setShader(m_vs);
	Engine::renderer()->shader()->PS_setShader(m_ps);

	auto& cam = Engine::camera()->getActiveCamera();

	float scale = 50.0f;

	TerrainBuffer tbuff{};
	tbuff.world.setIdentity();
	tbuff.world.setScale(Vec3f{ scale, scale, scale });
	tbuff.view = cam.view();
	tbuff.proj = cam.proj();
	tbuff.materialFlags = m_mesh->getTextureIndex();
	m_vscb->update(&tbuff);
	m_pscb->update(&tbuff);

	m_vscb->bind(4);
	m_pscb->bind(4);

	for (auto& mesh : m_mesh->getMeshes()) {
		SP_Material material = Engine::resource()->get<SP_Material>(mesh.materialName);
		Engine::renderer()->shader()->PS_setMaterial(0, 1, material);
		Engine::renderer()->drawIndexedTriangleList(m_indices.size(), mesh.baseIndex, mesh.baseVertex);
	}
}

float DXGLTerrainManager::getTerrainHeight(float x, float z) {
	QuadTreeRect rect{};
	rect.pos = Vec2f{x, z};
	rect.size = { 1.0f, 1.0f };
	auto search = m_chunkTree.search(rect);
	for (auto c : search) {
		TerrainFace face{};
		for (TerrainFace& f : c->faces) {
			Vec2f p0 = { f.v0.x, f.v0.z };
			Vec2f p1 = { f.v1.x, f.v1.z };
			Vec2f p2 = { f.v2.x, f.v2.z };
			if (Math::isPointInsideTriangle(p0, p1, p2, Vec2f{ x, z })) {
				return Math::barycentricHeight(f.v0, f.v1, f.v2, Vec2f(x, z));
			}
		}
	}

	return 0;
}
