#pragma once

#include <d3d11.h>

#include "Bindable.h"

namespace dxgl {

	class IndexBuffer : public Bindable {
	public:
		IndexBuffer(void* indices, int indexCount);
		~IndexBuffer();

		void bind(int slot = 0) override;

		ID3D11Buffer* get();

		int count();

	private:
		ID3D11Buffer* m_ibuffer = nullptr;
		int m_count = 0;
	};

}