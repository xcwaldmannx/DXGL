#pragma once

#include "DXGLMain.h"

#include "Vec3f.h"
#include "Mat4f.h"

namespace dxgl {

	struct FoliageInstance {
		Vec3f scale{};
		Vec3f rotation{};
		Vec3f translation{};
		Vec3f color0{};
		Vec3f color1{};
		Vec3f color2{};
		Vec3f color3{};
		float timeOffset = 0;
	};

	struct FoliageBuffer {
		Mat4f model{};
		Mat4f view{};
		Mat4f proj{};
		float time = 0;
		float pad[3];
	};

	const float GRASS_DENSITY = 64;
	const float GRASS_TILE_SIZE = 8;
	const float GRASS_TILE_LENGTH = 16;
	const float GRASS_TOTAL_LENGTH = GRASS_TILE_SIZE * GRASS_TILE_LENGTH;
	const float GRASS_BLADES_MAX = (GRASS_DENSITY * GRASS_DENSITY) * (GRASS_TILE_LENGTH * GRASS_TILE_LENGTH);

	struct FoliageChunk {
		float lastX = 0;
		float lastZ = 0;
		float curX = 0;
		float curZ = 0;
		float LOD = 0;
		int bufferLocation = 0;
	};

	class DXGLFoliageManager {
	public:
		DXGLFoliageManager();
		~DXGLFoliageManager();

		void generateFoliage();

		void update(long double delta);
		void draw();
	private:
		void cull();

	private:

		template <class _InIt>
		void copyPercentage(_InIt _First, _InIt _Last, _InIt _Dest, float percentage) {
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
				if (_UFirst + index < _ULast) {
					_UDest[copiedCount] = _UFirst[index];
				}
			}
		}

		template <class _InIt, class _OutIt>
		_OutIt copySkipEveryOtherN(_InIt _First, _InIt _Last, _OutIt _Dest, int n) {
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
		_OutIt copyEveryNth(_InIt _First, _InIt _Last, _OutIt _Dest, int n) {

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

		std::vector<FoliageInstance> m_foliage{};
		std::vector<FoliageInstance> m_culledFoliage{};

		std::vector<FoliageChunk> m_foliageChunks{};
		std::vector<int> m_visibleBufferLocations{};
		std::vector<int> m_bufferAdvance{};
		std::vector<float> m_chunkLODs{};

		SP_DXGLInputLayout m_layout = nullptr;
		SP_DXGLVertexShader m_vs = nullptr;
		SP_DXGLPixelShader m_ps = nullptr;

		SP_DXGLBasicMesh m_mesh = nullptr;

		SP_DXGLCBuffer m_cb = nullptr;
		SP_DXGLVertexBuffer m_vbInstance = nullptr;

		float m_timePassed = 0;
	};
}