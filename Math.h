#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "Vec2f.h"
#include "Vec3f.h"

class Math {
public:

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
			Vec2f point = begin + (end / density) * i;
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
			outlineEdge(A + (AtoCenter / density) * i, AB, density, output);
			outlineEdge(B + (BtoCenter / density) * i, BC, density, output);
			outlineEdge(C + (CtoCenter / density) * i, CA, density, output);
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
};