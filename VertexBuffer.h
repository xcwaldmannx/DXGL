#pragma once

#include <d3d11.h>

#include "Bindable.h"

namespace dxgl {

	class VertexBuffer :public Bindable {
	public:
		VertexBuffer(void* vertices, int vertexCount, int vertexSize);
		~VertexBuffer();

		void bind(int slot) override;

		ID3D11Buffer* get();
		int count();
		int size();

	private:
		ID3D11Buffer* m_vbuffer = nullptr;
		int m_count = 0;
		int m_size = 0;

	};

}
