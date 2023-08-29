#pragma once

#include "Engine.h"
#include "Bindable.h"

namespace dxgl {
	class HSConstantBuffer : public Bindable {
	public:
		HSConstantBuffer(UINT bytes);
		~HSConstantBuffer();

		void bind(int slot) override;

		void update(void* buffer);
		ID3D11Buffer* get();

	private:
		ID3D11Buffer* m_cbuffer = nullptr;
	};
}
