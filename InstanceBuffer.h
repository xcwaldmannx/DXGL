#pragma once

#include "Engine.h"
#include "Bindable.h"

namespace dxgl {

	class InstanceBuffer : public Bindable {
	public:
		InstanceBuffer(void* instances, int instanceCount, int instanceSize);
		~InstanceBuffer();

		void bind(int slot) override;

		void update(void* buffer);

		ID3D11Buffer* get();
		int count();
		int size();

	private:
		ID3D11Buffer* m_ibuffer = nullptr;
		int m_count = 0;
		int m_size = 0;

	};

}
