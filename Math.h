#pragma once

#include <algorithm>

#include "Vec2f.h"
#include "Vec3f.h"

class Math {
public:

	static float barryCentric(Vec3f p1, Vec3f p2, Vec3f p3, Vec2f pos) {
		float detT = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
		float detU = (p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z);
		float detV = (p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z);

		float l1 = detU / detT;
		float l2 = detV / detT;
		float l3 = 1.0f - l1 - l2;

		return l1 * p1.y + l2 * p2.y + l3 * p3.y;
	}

	static bool isPointInsideTriangle(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
		// Calculate the barycentric coordinates
		// of point P with respect to triangle ABC
		float denominator = (B.y - C.y) * (A.x - C.x) + (C.x - B.x) * (A.y - C.y);
		float a = ((B.y - C.y) * (P.x - C.x) + (C.x - B.x) * (P.y - C.y)) / denominator;
		float b = ((C.y - A.y) * (P.x - C.x) + (A.x - C.x) * (P.y - C.y)) / denominator;
		float c = 1.0f - a - b;

		// Check if all barycentric coordinates are non-negative and not greater than 1
		return (a >= 0 && b >= 0 && c >= 0 && a <= 1 && b <= 1 && c <= 1);
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