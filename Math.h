#pragma once

#include <iostream>

#include <algorithm>
#include <cmath>
#include <vector>

#include "Vec2f.h"
#include "Vec3f.h"

class Math {
public:

	// Traingle shit start
	
	static bool lineIntersectsTriangle(const Vec3f& P1, const Vec3f& P2, const Vec3f& P3, const Vec3f& R1, const Vec3f& R2, Vec3f& output) {
		Vec3f Normal{}, IntersectPos{};

		// Find Triangle Normal
		Normal = Vec3f::cross(P2 - P1, P3 - P1).normalize();

		// Find distance from LP1 and LP2 to the plane defined by the triangle
		float Dist1 = Vec3f::dot(R1 - P1, Normal);
		float Dist2 = Vec3f::dot(R2 - P1, Normal);

		if ((Dist1 * Dist2) >= 0.0f) {
			//std::cout << "no cross\n";
			return false;
		} // line doesn't cross the triangle.

		if (Dist1 == Dist2) {
			//std::cout << "parallel\n";
			return false;
		} // line and plane are parallel

		// Find point on the line that intersects with the plane
		IntersectPos = R1 + (R2 - R1) * (-Dist1 / (Dist2 - Dist1));

		// Find if the interesection point lies inside the triangle by testing it against all edges
		Vec3f vTest;

		vTest = Vec3f::cross(Normal, P2 - P1);
		if (Vec3f::dot(vTest, IntersectPos - P1) < 0.0f) {
			return false;
		}

		vTest = Vec3f::cross(Normal, P3 - P2);
		if (Vec3f::dot(vTest, IntersectPos - P2) < 0.0f) {
			return false;
		}

		vTest = Vec3f::cross(Normal, P1 - P3);
		if (Vec3f::dot(vTest, IntersectPos - P1) < 0.0f) {
			return false;
		}

		//std::cout << "Intersects at (" << IntersectPos.x << ", " << IntersectPos.y << ")\n";

		output = IntersectPos;

		return true;
	}

	static bool rayIntersectsTriangle(const Vec3f& origin, const Vec3f& dir,
		const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, Vec3f& output) {
		const float epsilon = 0.00000001f;

		Vec3f edge1 = v1 - v0;
		Vec3f edge2 = v2 - v0;

		Vec3f pvec = Vec3f::cross(dir, edge2);

		float det = Vec3f::dot(edge1, pvec);

		bool testCull = true;
		if (testCull) {
			if (det < epsilon) return false;

			Vec3f tvec = origin - v0;

			output.y = Vec3f::dot(tvec, pvec);
			if (output.y < 0 || output.y > det) return false;

			Vec3f qvec = Vec3f::cross(tvec, edge1);

			output.z = Vec3f::dot(dir, qvec);
			if (output.z < 0 || output.y + output.z > det) return false;

			output.x = Vec3f::dot(edge2, qvec);
			float invDet = 1.0f / det;
			output *= invDet;
		}
		return true;
	}

	static bool getTriangleIntersection(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3,
		const Vec3f& q1, const Vec3f& q2, const Vec3f& q3,
		Vec3f& intersectionPoint) {
		Vec3f e1 = p2 - p1;
		Vec3f e2 = p3 - p1;
		Vec3f h = Vec3f::cross(q2 - q1, q3 - q1);
		float a = Vec3f::dot(e1, h);

		if (a > -1e-6 && a < 1e-6) {
			return false;  // Triangles are parallel or coincide
		}

		float f = 1.0f / a;
		Vec3f s = q1 - p1;
		float u = f * Vec3f::dot(s, h);

		if (u < 0.0f || u > 1.0f) {
			return false;
		}

		Vec3f q = Vec3f::cross(s, e1);
		float v = f * Vec3f::dot(q2 - p1, q);

		if (v < 0.0f || u + v > 1.0f) {
			return false;
		}

		float t = f * Vec3f::dot(e2, q);

		if (t > 1e-6) {
			intersectionPoint = p1 + e2 * t;
			return true;  // Triangles intersect
		}

		return false;  // No intersection
	}

	// triangle shit end

	static float barycentricHeight(Vec3f p1, Vec3f p2, Vec3f p3, Vec2f pos) {
		// Calculate the barycentric coordinates for the first triangle (p1, p2, p3)
		float detT = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
		float detU = (p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z);
		float detV = (p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z);

		float l1 = detU / detT;
		float l2 = detV / detT;
		float l3 = 1.0f - l1 - l2;

		// Check if the point is inside the first triangle (p1, p2, p3)
		if (l1 >= 0 && l1 <= 1 && l2 >= 0 && l2 <= 1 && l3 >= 0 && l3 <= 1) {
			// The point is inside the first triangle, so return the interpolated value for that triangle.
			return l1 * p1.y + l2 * p2.y + l3 * p3.y;
		}

		// Assume the point is inside the second triangle (p3, p2, p1)
		// Calculate the barycentric coordinates for the second triangle (p3, p2, p1)
		float detT2 = (p2.z - p3.z) * (p3.x - p2.x) + (p3.x - p2.x) * (p3.z - p2.z);
		float detU2 = (p2.z - p3.z) * (pos.x - p2.x) + (p3.x - p2.x) * (pos.y - p2.z);
		float detV2 = (p3.z - p2.z) * (pos.x - p2.x) + (p2.x - p3.x) * (pos.y - p2.z);

		float l12 = detU2 / detT2;
		float l22 = detV2 / detT2;
		float l32 = 1.0f - l12 - l22;

		// Return the interpolated value for the second triangle (p3, p2, p1)
		return l12 * p3.y + l22 * p2.y + l32 * p1.y;
	}

	static bool isPointInsideTriangle(Vec2f p1, Vec2f p2, Vec2f p3, Vec2f pos) {
		// Calculate the barycentric coordinates for the first triangle (p1, p2, p3)
		float detT = (p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y);
		float detU = (p2.y - p3.y) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.y);
		float detV = (p3.y - p1.y) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.y);

		float l1 = detU / detT;
		float l2 = detV / detT;
		float l3 = 1.0f - l1 - l2;

		// Check if the point is inside the first triangle (p1, p2, p3)
		if (l1 >= 0 && l1 <= 1 && l2 >= 0 && l2 <= 1 && l3 >= 0 && l3 <= 1) {
			return true;
		}

		// Assume the point is inside the second triangle (p3, p2, p1)
		// Calculate the barycentric coordinates for the second triangle (p3, p2, p1)
		float detT2 = (p2.y - p3.y) * (p3.x - p2.x) + (p3.x - p2.x) * (p3.y - p2.y);
		float detU2 = (p2.y - p3.y) * (pos.x - p2.x) + (p3.x - p2.x) * (pos.y - p2.y);
		float detV2 = (p3.y - p2.y) * (pos.x - p2.x) + (p2.x - p3.x) * (pos.y - p2.y);

		float l12 = detU2 / detT2;
		float l22 = detV2 / detT2;
		float l32 = 1.0f - l12 - l22;

		// Check if the point is inside the second triangle (p3, p2, p1)
		return (l12 >= 0 && l12 <= 1 && l22 >= 0 && l22 <= 1 && l32 >= 0 && l32 <= 1);
	}

	// creates points along an edge, forming an outline
	static void outlineEdge(Vec2f begin, Vec2f end, float density, std::vector<Vec2f>& output) {
		for (int i = 0; i < density; i++) {
			Vec2f point = begin + (end / density) * (float) i;
			output.push_back(point);
		}
	}

	// fills a triangle with points
	static std::vector<Vec2f> fillTriangle(Vec2f A, Vec2f B, Vec2f C, float density) {
		Vec2f AB = B - A;
		Vec2f BC = C - B;
		Vec2f CA = A - C;

		Vec2f center {
			(A.x + B.x + C.x) / 3.0f,
			(A.y + B.y + C.y) / 3.0f,
		};

		Vec2f AtoCenter = center - A;
		Vec2f BtoCenter = center - B;
		Vec2f CtoCenter = center - C;

		std::vector<Vec2f> output{};

		for (int i = 1; i < density; i++) {
			outlineEdge(A + (AtoCenter / density) * (float) i, AB, density, output);
			outlineEdge(B + (BtoCenter / density) * (float) i, BC, density, output);
			outlineEdge(C + (CtoCenter / density) * (float) i, CA, density, output);
		}


		return output;
	}

	static float smoothstep(float x, float edge0 = 0.0f, float edge1 = 1.0f) {
		// Scale, and clamp x to 0..1 range
		x = std::clamp<float>((x - edge0) / (edge1 - edge0), 0, 1);

		return x * x * (3.0f - 2.0f * x);
	}

	template <class _InIt>
	static void copyPercentage(_InIt _First, _InIt _Last, _InIt _Dest, float percentage) {
		if (percentage <= 0.0) {
			return;
		}

		auto _UFirst = std::_Get_unwrapped(_First);
		auto _ULast = std::_Get_unwrapped(_Last);
		auto _UDest = std::_Get_unwrapped(_Dest);

		size_t totalSize = std::distance(_UFirst, _ULast);
		size_t copyCount = static_cast<size_t>(percentage * totalSize);

		if (copyCount == 0) {
			return;
		}

		// Calculate the step size to evenly distribute elements
		double stepSize = static_cast<double>(totalSize) / static_cast<double>(copyCount);
		double stepIndex = 0.0;

		for (size_t copiedCount = 0; copiedCount < copyCount; ++copiedCount) {
			// Calculate the current index to copy from
			size_t index = static_cast<size_t>(stepIndex);
			stepIndex += stepSize;

			//if (index > totalSize) {
			//	return;
			//}

			// Copy the element to the destination range
			if (_UFirst + index < _ULast && index > 0) {
				_UDest[copiedCount] = _UFirst[index];
			}
		}
	}

	template <class _InIt, class _OutIt>
	static _OutIt copySkipEveryOtherN(_InIt _First, _InIt _Last, _OutIt _Dest, int n) {
		if (n < 1) {
			return _Dest;
		}

		auto _UFirst = std::_Get_unwrapped(_First);
		auto _ULast = std::_Get_unwrapped(_Last);
		auto _UDest = std::_Get_unwrapped(_Dest);

		int nCount = 0;

		while (_UFirst <= _ULast) {
			*_UDest = *_UFirst;

			std::advance(_UFirst, 1);
			++_UDest;

			nCount++;

			if (nCount >= n) {
				std::advance(_UFirst, 1);
				nCount = 0;
			}
		}

		return _Dest;
	}

	template <class _InIt, class _OutIt>
	static _OutIt copyEveryNth(_InIt _First, _InIt _Last, _OutIt _Dest, int n) {

		if (n < 1) {
			return _Dest;
		}

		auto _UFirst = std::_Get_unwrapped(_First);
		auto _ULast = std::_Get_unwrapped(_Last);
		auto _UDest = std::_Get_unwrapped(_Dest);

		while (_UFirst <= _ULast) {
			*_UDest = *_UFirst;

			std::advance(_UFirst, n);
			++_UDest;
		}

		return _Dest;
	}

	static uint32_t pcgHash(uint32_t input) {
		uint32_t state = input * 747796405u + 2891336453u;
		uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return word;
	}

	static float randf(uint32_t& seed) {
		seed = pcgHash(seed);
		return (float) seed / (float) UINT32_MAX;
	}
};