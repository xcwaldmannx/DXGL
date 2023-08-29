#pragma once

#include "Engine.h"
#include "Bindable.h"

namespace dxgl {
	class DSConstantBuffer : public Bindable {
	public:
		DSConstantBuffer(UINT bytes);
		~DSConstantBuffer();

		void bind(int slot) override;

		void update(void* buffer);
		ID3D11Buffer* get();

	private:
		ID3D11Buffer* m_cbuffer = nullptr;
	};
}
