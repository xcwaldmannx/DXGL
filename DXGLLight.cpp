#include "DXGLLight.h"

#include "Engine.h"
#include "ResourceManager.h"

using namespace dxgl;

DXGLLight::DXGLLight() {
	m_pscbLight = Engine::resource()->createPSConstantBuffer(sizeof(LightBuffer));
}

DXGLLight::~DXGLLight() {
}

void DXGLLight::addLight(Light& light) {
	if (m_lightCount < 16) {

		m_lightBuffer.lights[m_lightCount] = light;
		m_lightCount++;
		m_lightBuffer.lightCount = m_lightCount;

		m_pscbLight->update(&m_lightBuffer);
	}

}

SP_PSConstantBuffer DXGLLight::getBuffer() {
	return m_pscbLight;
}
