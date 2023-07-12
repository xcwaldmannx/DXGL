#pragma once

#include "Vec2f.h"
#include "Vec3f.h"

class Vertex {
public:
	Vertex() : position(), texcoord(), normal(), tangent() {
	}


	Vertex(Vec3f position, Vec2f texcoord, Vec3f normal, Vec3f tangent) : position(position), texcoord(texcoord), normal(normal), tangent(tangent) {
	}

	Vertex(const Vertex& mesh) : position(mesh.position), texcoord(mesh.texcoord), normal(mesh.normal), tangent(mesh.tangent) {
	}

	~Vertex() {
	}

	Vec3f position{};
	Vec2f texcoord{};
	Vec3f normal{};
	Vec3f tangent{};

	bool operator==(const Vertex& other) {
		return position == other.position && texcoord == other.texcoord && normal == other.normal && tangent == other.tangent;
	}
};