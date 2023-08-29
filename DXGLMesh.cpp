#include "DXGLMesh.h"

#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION 
// define this in only *one* .cc
// Optional. define TINYOBJLOADER_USE_MAPBOX_EARCUT gives robust trinagulation. Requires C++11
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "tiny_obj_loader.h"

using namespace dxgl;

DXGLMesh::DXGLMesh(const std::string& filepath) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = filepath.substr(0, filepath.find_last_of("/")); // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    size_t sizeVertexIndexLists = 0;
    for (size_t s = 0; s < shapes.size(); s++) {
        sizeVertexIndexLists += shapes[s].mesh.indices.size();
    }
    m_vertices.reserve(sizeVertexIndexLists);
    m_indices.reserve(sizeVertexIndexLists);
    m_materialSlots.resize(materials.size());

    size_t indexGlobalOffset = 0;

    for (size_t m = 0; m < materials.size(); m++) {
        m_materialSlots[m].startIndex = indexGlobalOffset;
        m_materialSlots[m].material = materials[m].name;

        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            // Shape name

            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                if (shapes[s].mesh.material_ids[f] != m) {
                    continue;
                }
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {

                    Vec3f position{};
                    Vec2f texcoord{};
                    Vec3f normal{};

                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                    position = Vec3f(vx, vy, vz);

                    // Check if `normal_index` is zero or positive. negative = no normal data
                    if (idx.normal_index >= 0) {
                        tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                        tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                        tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                        normal = Vec3f(nx, ny, nz);
                    }

                    // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                    if (idx.texcoord_index >= 0) {
                        tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                        tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                        texcoord = Vec2f(tx, ty);
                    }

                    computeAxialMinAndMax(position);

                    Vertex vertex(position, texcoord, normal, Vec3f{});
                    m_vertices.push_back(vertex);
                    m_indices.push_back((unsigned int)(indexGlobalOffset + v));

                    // Optional: vertex colors
                    // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
                    // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
                    // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
                }
                index_offset += fv;
                indexGlobalOffset += fv;

                // per-face material
                // shapes[s].mesh.material_ids[f];
            }
        }
        m_materialSlots[m].indexCount = indexGlobalOffset - m_materialSlots[m].startIndex;
    }

    computeTangents();
    m_vertexBuffer = Engine::resource()->createVertexBuffer(&m_vertices[0], (UINT)m_vertices.size(), sizeof(Vertex));
    m_indexBuffer = Engine::resource()->createIndexBuffer(&m_indices[0], (UINT)m_indices.size());
}

DXGLMesh::~DXGLMesh() {
}

const Vec3f& DXGLMesh::getAABBMin() {
    return m_axialMin;
}

const Vec3f& DXGLMesh::getAABBMax() {
    return m_axialMax;
}

void DXGLMesh::computeTangents() {

    for (int i = 0; i < m_indices.size(); i += 3) {
        unsigned int i0 = m_indices[i + 0];
        unsigned int i1 = m_indices[i + 1];
        unsigned int i2 = m_indices[i + 2];

        Vec3f edge1 = m_vertices[i1].position - m_vertices[i0].position;
        Vec3f edge2 = m_vertices[i2].position - m_vertices[i0].position;

        float deltaU1 = m_vertices[i1].texcoord.x - m_vertices[i0].texcoord.x;
        float deltaV1 = m_vertices[i1].texcoord.y - m_vertices[i0].texcoord.y;

        float deltaU2 = m_vertices[i2].texcoord.x - m_vertices[i0].texcoord.x;
        float deltaV2 = m_vertices[i2].texcoord.y - m_vertices[i0].texcoord.y;

        float dividend = (deltaU1 * deltaV2 - deltaU2 * deltaV1);
        float fc = 1.0f / dividend;

        Vec3f tangent = {
            fc * (deltaV2 * edge1.x - deltaV1 * edge2.x),
            fc * (deltaV2 * edge1.y - deltaV1 * edge2.y),
            fc * (deltaV2 * edge1.z - deltaV1 * edge2.z),
        };

        tangent = tangent.normalize();

        float sx = deltaU1;
        float sy = deltaU2;
        float tx = deltaV1;
        float ty = deltaV2;
        // handedness ignored for now
        float handedness = ((tx * sy - ty * sx) < 0.0f) ? -1.0f : 1.0f;

        m_vertices[i0].tangent = tangent;
        m_vertices[i1].tangent = tangent;
        m_vertices[i2].tangent = tangent;

    }
}

void DXGLMesh::computeAxialMinAndMax(const Vec3f& vec) {
    m_axialMin.x = vec.x < m_axialMin.x ? vec.x : m_axialMin.x;
    m_axialMin.y = vec.y < m_axialMin.y ? vec.y : m_axialMin.y;
    m_axialMin.z = vec.z < m_axialMin.z ? vec.z : m_axialMin.z;

    m_axialMax.x = vec.x > m_axialMax.x ? vec.x : m_axialMax.x;
    m_axialMax.y = vec.y > m_axialMax.y ? vec.y : m_axialMax.y;
    m_axialMax.z = vec.z > m_axialMax.z ? vec.z : m_axialMax.z;
}

void DXGLMesh::computeAABB() {
    std::vector<Vec3f>verticesBB = {
        // FRONT FACE VERTICES
        // top left
        Vec3f(m_axialMin.x, m_axialMax.y, m_axialMin.z),
        // bottom left
        Vec3f(m_axialMin.x, m_axialMin.y, m_axialMin.z),
        // bottom right
        Vec3f(m_axialMax.x, m_axialMin.y, m_axialMin.z),
        // top right
        Vec3f(m_axialMax.x, m_axialMax.y, m_axialMin.z),

        // BACK FACE VERTICES
        // top left
        Vec3f(m_axialMin.x, m_axialMax.y, m_axialMax.z),
        // bottom left
        Vec3f(m_axialMin.x, m_axialMin.y, m_axialMax.z),
        // bottom right
        Vec3f(m_axialMax.x, m_axialMin.y, m_axialMax.z),
        // top right
        Vec3f(m_axialMax.x, m_axialMax.y, m_axialMax.z),
    };

    std::vector<unsigned int>indicesBB = {
        // FRONT
        0, 1,
        1, 2,
        2, 3,
        3, 0,

        // BACK
        4, 5,
        5, 6,
        6, 7,
        7, 4,

        // LEFT
        0, 4,
        4, 5,
        5, 1,
        1, 0,

        // RIGHT
        3, 7,
        7, 6,
        6, 2,
        2, 3,

        // TOP
        0, 3,
        3, 7,
        7, 4,
        4, 0,

        // BOTTOM
        1, 2,
        2, 6,
        6, 5,
        5, 1,
    };
}

const SP_VertexBuffer& DXGLMesh::getVertexBuffer() {
    return m_vertexBuffer;
}

const SP_IndexBuffer& DXGLMesh::getIndexBuffer() {
    return m_indexBuffer;
}

std::vector<Vertex> DXGLMesh::getVertices() {
    return m_vertices;
}

std::vector<unsigned int> DXGLMesh::getIndices() {
    return m_indices;
}

std::vector<MeshMaterialSlot> DXGLMesh::getMaterials() {
    return m_materialSlots;
}
