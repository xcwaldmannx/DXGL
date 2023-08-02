#pragma once

#include "DXGLMain.h"
#include "Bindable.h"

namespace dxgl {
	class VSConstantBuffer : public Bindable {
	public:
		VSConstantBuffer(UINT bytes);
		~VSConstantBuffer();

		void bind(int slot) override;

		void update(void* buffer);
		ID3D11Buffer* get();

	private:
		ID3D11Buffer* m_cbuffer = nullptr;
	};
}
