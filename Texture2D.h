#pragma once

#include <string>

#include "Engine.h"
#include "Bindable.h"

namespace dxgl {

	class Texture2D : public Bindable {
	public:
		Texture2D(const std::string& filename);
		Texture2D(unsigned int width, unsigned int height, unsigned char* data);
		~Texture2D();

		void bind(int slot) override;

		ID3D11ShaderResourceView* get();

	private:
		ID3D11ShaderResourceView* m_srv = nullptr;
	};

}
