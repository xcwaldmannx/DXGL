#include "DXGLTerrainSystem.h"

using namespace dxgl;

DXGLTerrainSystem::DXGLTerrainSystem(const MeshDesc& desc, const std::string& filename) {
	m_mesh = DXGLMain::resource()->createBasicMesh(desc, filename);

	AABB aabb = m_mesh->getAABB();

	float scale = 50.0f;

	float minX = aabb.min.x * scale;
	float minY = aabb.min.y * scale;
	float minZ = aabb.min.z * scale;

	float maxX = aabb.max.x * scale;
	float maxY = aabb.max.y * scale;
	float maxZ = aabb.max.z * scale;

	float chunkSize = 128;

	for (int x = minX; x < maxX; x += chunkSize) {
		for (int z = minZ; z < maxZ; z += chunkSize) {
			TerrainChunk chunk = {x, minY, z, x + chunkSize, maxY, z + chunkSize};
			m_chunks.push_back(chunk);
		}
	}

	std::vector<Face> faces = m_mesh->getFaces();
	std::vector<Vec3f> faceCenters{};

	for (int i = 0; i < faces.size(); i++) {
		Face face = faces[i];
		float cx = (face.v0.x * scale + face.v1.x * scale + face.v2.x * scale) / 3.0f;
		float cy = (face.v0.y * scale + face.v1.y * scale + face.v2.y * scale) / 3.0f;
		float cz = (face.v0.z * scale + face.v1.z * scale + face.v2.z * scale) / 3.0f;
		Vec3f faceCenter = { cx, cy, cz };
		faceCenters.push_back(faceCenter);
	}

	for (int faceCenterIndex = 0; faceCenterIndex < faceCenters.size(); faceCenterIndex++) {
		for (int chunkIndex = 0; chunkIndex < m_chunks.size(); chunkIndex++) {
			Vec3f& faceCenter = faceCenters[faceCenterIndex];
			TerrainChunk& chunk = m_chunks[chunkIndex];
			if (faceCenter.x > chunk.x1 && faceCenter.x < chunk.x2) {
				if (faceCenter.z > chunk.z1 && faceCenter.z < chunk.z2) {
					chunk.faceIndices.push_back(faces[faceCenterIndex].indices.x);
					chunk.faceIndices.push_back(faces[faceCenterIndex].indices.y);
					chunk.faceIndices.push_back(faces[faceCenterIndex].indices.z);
					chunk.indexCount += 3;
				}
			}
		}
	}

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

DXGLTerrainSystem::~DXGLTerrainSystem() {

}

void DXGLTerrainSystem::update(long double delta) {
	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

	std::vector<TerrainChunk> culledTerrainChunks{};

	int indexCount = 0;

	for (int i = 0; i < m_chunks.size(); i++) {
		TerrainChunk& chunk = m_chunks[i];
		if (!cam->cull(Vec3f{ chunk.x1, chunk.y1, chunk.z1 }, Vec3f{ 10, 10, 10 }, Vec3f{ 0, 0, 0 }, Vec3f{ chunk.x2 - chunk.x1, chunk.y2, chunk.z2 - chunk.z1 })) {
			indexCount += chunk.indexCount;
			culledTerrainChunks.push_back(chunk);
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

void DXGLTerrainSystem::draw() {
	DXGLMain::renderer()->input()->setInputLayout(m_layout);
	DXGLMain::renderer()->input()->setVertexBuffer(0, 1, &m_mesh->getMeshVertexBuffer());
	DXGLMain::renderer()->input()->setIndexBuffer(m_ib);

	DXGLMain::renderer()->shader()->VS_setShader(m_vs);
	DXGLMain::renderer()->shader()->PS_setShader(m_ps);

	SP_DXGLCamera cam = DXGLMain::renderer()->camera()->get("primary");

	TerrainBuffer tbuff{};
	tbuff.world.setIdentity();
	tbuff.world.setScale(Vec3f{50, 50, 50});
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
