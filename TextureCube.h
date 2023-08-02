#pragma once

#include <string>

#include "DXGLMain.h"
#include "Bindable.h"

namespace dxgl {
	class TextureCube : public Bindable {
	public:
		TextureCube(const std::string& filepath);
		~TextureCube();

		void bind(int slot) override;

		ID3D11ShaderResourceView* get();

	private:
		ID3D11ShaderResourceView* m_srv = nullptr;
	};
};