#pragma once

#include <d3d11.h>

#include "Bindable.h"

namespace dxgl {
	class DSConstantBuffer : public Bindable {
	public:
		DSConstantBuffer(unsigned int bytes);
		~DSConstantBuffer();

		void bind(int slot) override;

		void update(void* buffer);
		ID3D11Buffer* get();

	private:
		ID3D11Buffer* m_cbuffer = nullptr;
	};
}
