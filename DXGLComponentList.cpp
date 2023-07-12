#include "DXGLComponentList.h"

using namespace dxgl::governor;

DXGLComponentList::DXGLComponentList() {
	for (int i = 0; i < MAX_COMPONENTS; i++) {
		m_components[i] = NULL_COMPONENT;
	}
}

DXGLComponentList::~DXGLComponentList() {

}
