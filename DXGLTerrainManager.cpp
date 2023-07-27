#include "DXGLTerrainManager.h"

using namespace dxgl;

DXGLTerrainManager::DXGLTerrainManager() {
	InputLayoutDesc descLayout{};
	descLayout.add("POSITION", 0, FLOAT3, false);
	descLayout.add("TEXCOORD", 0, FLOAT2, false);
	descLayout.add("NORMAL",   0, FLOAT3, false);
	descLayout.add("TANGENT",  0, FLOAT3, false);
	m_layout = DXGLMain::resource()->createInputLayout(descLayout, "Assets/Shaders/VS_TerrainShader.cso");

	m_vs = DXGLMain::resource()->createShader<DXGLVertexShader>("Assets/Shaders/VS_TerrainShader.cso");
	m_ps = DXGLMain::resource()->createShader<DXGLPixelShader>("Assets/Shaders/PS_TerrainShader.cso");

	m_cb = DXGLMain::resource()->createCBuffer(sizeof(TerrainBuffer));
}

DXGLTerrainManager::~DXGLTerrainManager() {
}

void DXGLTerrainManager::load(const MeshDesc& desc, const std::string& filename) {
	m_mesh = DXGLMain::resource()->createBasicMesh(desc, filename);

	AABB aabb = m_mesh->getAABB();

	float scale = 20.0f;

	float minX = aabb.min.x * scale;
	float minY = aabb.min.y * scale;
	float minZ = aabb.min.z * scale;

	float maxX = aabb.max.x * scale;
	float maxY = aabb.max.y * scale;
	float maxZ = aabb.max.z * scale;

	chunkSize = std::ceil((std::abs(minX) + maxX) / 50.0f);

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
	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

	m_indices.clear();

	auto chunk = m_chunkTree.begin();
	while (chunk != m_chunkTree.end()) {
		if (!cam->cull(chunk->minVertex, Vec3f{ 1, 1, 1 }, Vec3f{ 0, 0, 0 }, chunk->maxVertex - chunk->minVertex)) {
			m_indices.insert(m_indices.end(), chunk->faceIndices.begin(), chunk->faceIndices.end());
		}
		chunk++;
	}

	if (m_indices.size() > 0) {
		m_ib = DXGLMain::resource()->createIndexBuffer(&m_indices[0], m_indices.size());
	}

	float searchSize = 128.0f;
	m_searchArea = { Vec2f(cam->getPosition().x - (searchSize / 2.0f), cam->getPosition().z - (searchSize / 2.0f)),
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

		DXGLMain::renderer()->foliage()->unloadTerrain(diffUnload);
		DXGLMain::renderer()->foliage()->loadTerrain(diffLoad);

		m_lastSearch = searched;
	}
}

void DXGLTerrainManager::draw() {
	DXGLMain::renderer()->input()->setInputLayout(m_layout);
	DXGLMain::renderer()->input()->setVertexBuffer(0, 1, &m_mesh->getMeshVertexBuffer());
	if (m_ib)
	DXGLMain::renderer()->input()->setIndexBuffer(m_ib);

	DXGLMain::renderer()->shader()->VS_setShader(m_vs);
	DXGLMain::renderer()->shader()->PS_setShader(m_ps);

	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

	float height = 1.7f + getTerrainHeight(cam->getPosition().x, cam->getPosition().z);
	cam->world().setTranslation(Vec3f{cam->getPosition().x, height, cam->getPosition().z});

	float scale = 20.0f;

	TerrainBuffer tbuff{};
	tbuff.world.setIdentity();
	tbuff.world.setScale(Vec3f{ scale, scale, scale });
	tbuff.view = cam->view();
	tbuff.proj = cam->proj();
	tbuff.materialFlags = m_mesh->getUsedMaterials();
	m_cb->update(&tbuff);

	DXGLMain::renderer()->shader()->VS_setCBuffer(4, 1, m_cb->get());
	DXGLMain::renderer()->shader()->PS_setCBuffer(4, 1, m_cb->get());

	for (auto& mesh : m_mesh->getMeshes()) {
		SP_DXGLMaterial material = DXGLMain::resource()->get<SP_DXGLMaterial>(mesh.materialName);
		DXGLMain::renderer()->shader()->PS_setMaterial(0, 1, material);
		DXGLMain::renderer()->drawIndexedTriangleList(m_indices.size(), mesh.baseIndex, mesh.baseVertex);
	}
}

float DXGLTerrainManager::getTerrainHeight(float x, float z) {
	for (auto c : m_lastSearch) {
		if (x >= c->minVertex.x && x < c->maxVertex.x) {
			if (z >= c->minVertex.z && z < c->maxVertex.z) {
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
		}
	}

	return 0;
}
