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

	float scale = 50.0f;

	float minX = aabb.min.x * scale;
	float minY = aabb.min.y * scale;
	float minZ = aabb.min.z * scale;

	float maxX = aabb.max.x * scale;
	float maxY = aabb.max.y * scale;
	float maxZ = aabb.max.z * scale;

	chunkSize = std::ceil((std::abs(minX) + maxX) / 25.0f);

	for (int x = minX; x < maxX; x += chunkSize) {
		for (int z = minZ; z < maxZ; z += chunkSize) {
			TerrainChunk* chunk = new TerrainChunk();
			chunk->minVertex = Vec3f(x, minY, z);
			chunk->maxVertex = Vec3f(x + chunkSize, maxY, z + chunkSize);
			m_chunks.push_back(chunk);
		}
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
		for (int chunkIndex = 0; chunkIndex < m_chunks.size(); chunkIndex++) {
			Vec3f& faceCenter = faceCenters[faceCenterIndex];
			TerrainChunk* chunk = m_chunks[chunkIndex];
			if (faceCenter.x >= chunk->minVertex.x && faceCenter.x < chunk->maxVertex.x) {
				if (faceCenter.z >= chunk->minVertex.z && faceCenter.z < chunk->maxVertex.z) {
					chunk->faceIndices.push_back(faces[faceCenterIndex].indices.x);
					chunk->faceIndices.push_back(faces[faceCenterIndex].indices.y);
					chunk->faceIndices.push_back(faces[faceCenterIndex].indices.z);
					chunk->indexCount += 3;

					TerrainFace* face = new TerrainFace();
					face->v0 = faces[faceCenterIndex].v0 * scale;
					face->v1 = faces[faceCenterIndex].v1 * scale;
					face->v2 = faces[faceCenterIndex].v2 * scale;

					// calc normal
					Vec3f U = face->v1 - face->v0;
					Vec3f V = face->v2 - face->v0;

					face->normal.x = U.y * V.z - U.z * V.y;
					face->normal.y = U.z * V.x - U.x * V.z;
					face->normal.z = U.x * V.y - U.y * V.x;

					face->center = faceCenter;

					chunk->faces.push_back(face);

					break;
				}
			}
		}
	}
}

void DXGLTerrainManager::update(long double delta) {
	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

	std::vector<TerrainChunk> culledTerrainChunks{};

	int indexCount = 0;

	for (int i = 0; i < m_chunks.size(); i++) {
		TerrainChunk* chunk = m_chunks[i];
		if (!cam->cull(chunk->minVertex, Vec3f{ 1, 1, 1 }, Vec3f{ 0, 0, 0 }, chunk->maxVertex - chunk->minVertex)) {
			indexCount += chunk->indexCount;
			culledTerrainChunks.push_back(*chunk);
		}
	}

	m_indices.resize(indexCount);

	int lastIndex = 0;
	for (int i = 0; i < culledTerrainChunks.size(); i++) {
		TerrainChunk& chunk = culledTerrainChunks[i];
		std::copy(chunk.faceIndices.begin(), chunk.faceIndices.end(), m_indices.begin() + lastIndex);
		lastIndex += chunk.indexCount;
	}

	if (m_indices.size() > 0) {
		m_ib = DXGLMain::resource()->createIndexBuffer(&m_indices[0], m_indices.size());
	}
}

void DXGLTerrainManager::draw() {
	DXGLMain::renderer()->input()->setInputLayout(m_layout);
	DXGLMain::renderer()->input()->setVertexBuffer(0, 1, &m_mesh->getMeshVertexBuffer());
	DXGLMain::renderer()->input()->setIndexBuffer(m_ib);

	DXGLMain::renderer()->shader()->VS_setShader(m_vs);
	DXGLMain::renderer()->shader()->PS_setShader(m_ps);

	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

	float height = 1.7f + getTerrainHeight(cam->getPosition().x, cam->getPosition().z);
	cam->world().setTranslation(Vec3f{cam->getPosition().x, height, cam->getPosition().z});

	TerrainBuffer tbuff{};
	tbuff.world.setIdentity();
	tbuff.world.setScale(Vec3f{ 50, 50, 50 });
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
	// get corresponding chunk
	TerrainChunk* chunk = nullptr;

	for (TerrainChunk* c : m_chunks) {
		if (x >= c->minVertex.x && x < c->maxVertex.x) {
			if (z >= c->minVertex.z && z < c->maxVertex.z) {
				chunk = c;
				break;
			}
		}
	}

	TerrainFace* face = nullptr;
	if (chunk) {
		for (TerrainFace* f : chunk->faces) {
			Vec2f p0 = { f->v0.x, f->v0.z };
			Vec2f p1 = { f->v1.x, f->v1.z };
			Vec2f p2 = { f->v2.x, f->v2.z };
			if (Math::isPointInsideTriangle(p0, p1, p2, Vec2f{ x, z })) {
				face = f;
				break;
			}
		}
	} else {
		//std::cout << "NO CHUNK\n";
		return 0;
	}


	float result = 0;
	if (face) {
		result = Math::barryCentric(face->v0, face->v1, face->v2, Vec2f(x, z));
	} else {
		std::cout << "NO FACE\n";
	}

	return result;
}
