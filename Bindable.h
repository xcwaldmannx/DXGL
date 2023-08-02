#pragma once

namespace dxgl {
	class Bindable {
	public:
		virtual void bind(int slot = -1) = 0;
	};
}