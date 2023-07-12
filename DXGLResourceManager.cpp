#include "DXGLResourceManager.h"

using namespace dxgl;

DXGLResourceManager::DXGLResourceManager() {

}

DXGLResourceManager::~DXGLResourceManager() {
}

template<typename T>
T DXGLResourceManager::get(const std::string& alias) {
	std::unordered_map<std::string, std::any> res;

	if (find<T>(alias)) {
		res = m_resources[typeid(T)];
		return std::any_cast<T>(res.at(alias));
	}

	throw std::runtime_error("Resource was not found or has an incompatible type: '" + alias + "'.");
}

template<typename T>
bool DXGLResourceManager::find(const std::string& alias) {
	auto it1 = m_resources.find(typeid(T));

	if (it1 != m_resources.end()) {
		std::unordered_map<std::string, std::any> res = m_resources[typeid(T)];
		auto it2 = res.find(alias);
		if (it2 != res.end()) {
			return true;
		}
	}

	return false;
}

// get
template SP_DXGLVertexBuffer   DXGLResourceManager::get<SP_DXGLVertexBuffer>(const std::string&);
template SP_DXGLIndexBuffer    DXGLResourceManager::get<SP_DXGLIndexBuffer>(const std::string&);
template SP_DXGLInstanceBuffer DXGLResourceManager::get<SP_DXGLInstanceBuffer>(const std::string&);
template SP_DXGLCBuffer        DXGLResourceManager::get<SP_DXGLCBuffer>(const std::string&);
template SP_DXGLInputLayout    DXGLResourceManager::get<SP_DXGLInputLayout>(const std::string&);

template SP_DXGLSamplerState   DXGLResourceManager::get<SP_DXGLSamplerState>(const std::string&);

template SP_DXGLTexture2D      DXGLResourceManager::get<SP_DXGLTexture2D>(const std::string&);
template SP_DXGLTextureCube    DXGLResourceManager::get<SP_DXGLTextureCube>(const std::string&);

template SP_DXGLMaterial       DXGLResourceManager::get<SP_DXGLMaterial>(const std::string&);
template SP_DXGLMesh           DXGLResourceManager::get<SP_DXGLMesh>(const std::string&);
template SP_DXGLBasicMesh      DXGLResourceManager::get<SP_DXGLBasicMesh>(const std::string&);

// find
template bool DXGLResourceManager::find<SP_DXGLVertexBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_DXGLIndexBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_DXGLInstanceBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_DXGLCBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_DXGLInputLayout>(const std::string&);

template bool DXGLResourceManager::find<SP_DXGLSamplerState>(const std::string&);

template bool DXGLResourceManager::find<SP_DXGLTexture2D>(const std::string&);
template bool DXGLResourceManager::find<SP_DXGLTextureCube>(const std::string&);

template bool DXGLResourceManager::find<SP_DXGLMaterial>(const std::string&);
template bool DXGLResourceManager::find<SP_DXGLMesh>(const std::string&);
template bool DXGLResourceManager::find<SP_DXGLBasicMesh>(const std::string&);

SP_DXGLVertexBuffer DXGLResourceManager::createVertexBuffer(void* vertices, int vertexCount, int vertexSize) {
	SP_DXGLVertexBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLVertexBuffer>(vertices, vertexCount, vertexSize, DXGLMain::graphics());
	} catch (...) {
		throw std::runtime_error("DXGLVertexBuffer could ont be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeVertexBuffer(void* vertices, int vertexCount, int vertexSize, const std::string& alias) {
	SP_DXGLVertexBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLVertexBuffer>(vertices, vertexCount, vertexSize, DXGLMain::graphics());
		m_resources[typeid(SP_DXGLVertexBuffer)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("DXGLVertexBuffer could ont be stored.");
	}
}

SP_DXGLIndexBuffer DXGLResourceManager::createIndexBuffer(void* indices, int indexCount) {
	SP_DXGLIndexBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLIndexBuffer>(indices, indexCount, DXGLMain::graphics());
	} catch (...) {
		throw std::runtime_error("DXGLIndexBuffer could ont be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeIndexBuffer(void* indices, int indexCount, const std::string& alias) {
	SP_DXGLIndexBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLIndexBuffer>(indices, indexCount, DXGLMain::graphics());
		m_resources[typeid(SP_DXGLIndexBuffer)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("DXGLIndexBuffer could ont be stored.");
	}
}

SP_DXGLInstanceBuffer DXGLResourceManager::createInstanceBuffer(void* instances, int instanceCount, int instanceSize) {
	SP_DXGLInstanceBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLInstanceBuffer>(instances, instanceCount, instanceSize, DXGLMain::graphics());
	} catch (...) {
		throw std::runtime_error("DXGLInstanceBuffer could ont be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeInstanceBuffer(void* instances, int instanceCount, int instanceSize, const std::string& alias) {
	SP_DXGLInstanceBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLInstanceBuffer>(instances, instanceCount, instanceSize, DXGLMain::graphics());
		m_resources[typeid(SP_DXGLInstanceBuffer)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("DXGLInstanceBuffer could ont be stored.");
	}
}

SP_DXGLCBuffer DXGLResourceManager::createCBuffer(UINT bytes) {
	SP_DXGLCBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLCBuffer>(bytes, DXGLMain::graphics());
	} catch (...) {
		throw std::runtime_error("DXGLCBuffer could not be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeCBuffer(UINT bytes, std::string alias) {
	SP_DXGLCBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLCBuffer>(bytes, DXGLMain::graphics());
		m_resources[typeid(SP_DXGLCBuffer)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("DXGLCBuffer could not be stored.");
	}
}

SP_DXGLInputLayout DXGLResourceManager::createInputLayout(const InputLayoutDesc& desc, const std::string& filename) {
	SP_DXGLInputLayout buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLInputLayout>(desc, filename, DXGLMain::graphics());
	} catch (...) {
		throw std::runtime_error("DXGLInputLayout could ont be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeInputLayout(const InputLayoutDesc& desc, const std::string& filename, const std::string& alias) {
	SP_DXGLInputLayout buffer = nullptr;
	try {
		buffer = std::make_shared<DXGLInputLayout>(desc, filename, DXGLMain::graphics());
		m_resources[typeid(SP_DXGLInputLayout)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("DXGLInputLayout could ont be stored.");
	}
}

template<class T>
std::shared_ptr<T> DXGLResourceManager::createShader(const std::string& filename) {
	std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
	ifs.seekg(0, std::ios::end);
	std::vector<std::byte> byteCode;
	size_t length = ifs.tellg();
	byteCode.resize(length);
	ifs.seekg(0, std::ios::beg);
	ifs.read(reinterpret_cast<char*>(&byteCode[0]), length);
	ifs.close();

	std::shared_ptr<T> shader = nullptr;
	try {
		shader = std::make_shared<T>(byteCode.data(), length, DXGLMain::graphics());
	} catch (...) {
		throw std::runtime_error("DXGLShader could not be created.");
	}

	return shader;
}

template<class T>
void DXGLResourceManager::storeShader(const std::string& filename, const std::string& alias) {
	std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
	ifs.seekg(0, std::ios::end);
	std::vector<std::byte> byteCode;
	size_t length = ifs.tellg();
	byteCode.resize(length);
	ifs.seekg(0, std::ios::beg);
	ifs.read(reinterpret_cast<char*>(&byteCode[0]), length);
	ifs.close();

	std::shared_ptr<T> shader = nullptr;
	try {
		shader = std::make_shared<T>(byteCode.data(), length, DXGLMain::graphics());
		m_resources[typeid(T)][alias] = shader;
	} catch (...) {
		throw std::runtime_error("DXGLShader could not be stored.");
	}
}

template SP_DXGLVertexShader DXGLResourceManager::createShader<DXGLVertexShader>(const std::string&);
template SP_DXGLHullShader   DXGLResourceManager::createShader<DXGLHullShader>(const std::string&);
template SP_DXGLDomainShader DXGLResourceManager::createShader<DXGLDomainShader>(const std::string&);
template SP_DXGLPixelShader  DXGLResourceManager::createShader<DXGLPixelShader>(const std::string&);

template void DXGLResourceManager::storeShader<DXGLVertexShader>(const std::string&, const std::string&);
template void DXGLResourceManager::storeShader<DXGLHullShader>(const std::string&, const std::string&);
template void DXGLResourceManager::storeShader<DXGLDomainShader>(const std::string&, const std::string&);
template void DXGLResourceManager::storeShader<DXGLPixelShader>(const std::string&, const std::string&);

SP_DXGLSamplerState DXGLResourceManager::createSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressUVW,
	D3D11_COMPARISON_FUNC comparison, FLOAT borderColor) {
	SP_DXGLSamplerState sampler = nullptr;
	try {
		sampler = std::make_shared<DXGLSamplerState>(filter, addressUVW, comparison, borderColor, DXGLMain::graphics());
	} catch(...) {
		throw std::runtime_error("DXGLSamplerState could not be created.");
	}

	return sampler;
}

void DXGLResourceManager::storeSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressUVW,
	D3D11_COMPARISON_FUNC comparison, FLOAT borderColor, const std::string& alias) {
	SP_DXGLSamplerState sampler = nullptr;
	try {
		sampler = std::make_shared<DXGLSamplerState>(filter, addressUVW, comparison, borderColor, DXGLMain::graphics());
		m_resources[typeid(SP_DXGLSamplerState)][alias] = sampler;
	} catch (...) {
		throw std::runtime_error("DXGLSamplerState could not be stored.");
	}
}

void createRasterState();
void storeRasterState();

void createDepthStencilState();
void storeDepthStencilState();

void createBlendState();
void storeBlendState();

////////////
// Assets //
////////////

SP_DXGLTexture2D DXGLResourceManager::createTexture2D(const std::string& filename) {
	SP_DXGLTexture2D texture = nullptr;
	try {
		texture = std::make_shared<DXGLTexture2D>(filename, DXGLMain::graphics());
	} catch (...) {
		throw std::exception("DXGLTexture2D could not be created.");
	}
	return texture;
}

SP_DXGLTexture2D DXGLResourceManager::createTexture2D(unsigned int width, unsigned int height, unsigned char* data) {
	SP_DXGLTexture2D texture = nullptr;
	try {
		texture = std::make_shared<DXGLTexture2D>(width, height, data, DXGLMain::graphics());
	} catch (...) {
		throw std::exception("DXGLTexture2D could not be created.");
	}
	return texture;
}

void DXGLResourceManager::storeTexture2D(const std::string& filename, const std::string& alias) {
	SP_DXGLTexture2D texture = nullptr;
	try {
		if (!find<SP_DXGLTexture2D>(alias)) {
			texture = std::make_shared<DXGLTexture2D>(filename, DXGLMain::graphics());
			m_resources[typeid(SP_DXGLTexture2D)][alias] = texture;
		}
	} catch (...) {
		throw std::exception("DXGLTextureManager could not be stored.");
	}
}

void DXGLResourceManager::storeTexture2D(unsigned int width, unsigned int height, unsigned char* data, const std::string& alias) {
	SP_DXGLTexture2D texture = nullptr;
	try {
		if (!find<SP_DXGLTexture2D>(alias)) {
			texture = std::make_shared<DXGLTexture2D>(width, height, data, DXGLMain::graphics());
			m_resources[typeid(SP_DXGLTexture2D)][alias] = texture;
		}
	} catch (...) {
		throw std::exception("DXGLTextureManager could not be stored.");
	}
}

SP_DXGLTextureCube DXGLResourceManager::createTextureCube(const std::string& filename) {
	SP_DXGLTextureCube texture = nullptr;
	try {
		texture = std::make_shared<DXGLTextureCube>(filename, DXGLMain::graphics());
	}
	catch (...) {
		throw std::exception("DXGLTextureCube could not be created.");
	}
	return texture;
}

void DXGLResourceManager::storeTextureCube(const std::string& filename, const std::string& alias) {
	SP_DXGLTextureCube texture = nullptr;
	try {
		if (!find<SP_DXGLTextureCube>(alias)) {
			texture = std::make_shared<DXGLTextureCube>(filename, DXGLMain::graphics());
			m_resources[typeid(SP_DXGLTextureCube)][alias] = texture;
		}
	}
	catch (...) {
		throw std::exception("DXGLTextureCube could not be stored.");
	}
}

SP_DXGLMaterial DXGLResourceManager::createMaterial(const std::string& filepath) {
	SP_DXGLMaterial material = nullptr;
	try {
		material = std::make_shared<DXGLMaterial>(filepath, DXGLMain::graphics());
	} catch (...) {
		throw std::runtime_error("DXGLMaterial could not be created.");
	}
	return material;
}

SP_DXGLMaterial DXGLResourceManager::createMaterial(const MaterialData& data) {
	SP_DXGLMaterial material = nullptr;
	try {
		material = std::make_shared<DXGLMaterial>(data, DXGLMain::graphics());
	} catch (...) {
		throw std::runtime_error("DXGLMaterial could not be created.");
	}
	return material;
}

void DXGLResourceManager::storeMaterial(const std::string& filepath, const std::string& alias) {
	SP_DXGLMaterial material = nullptr;
	try {
		if (!find<SP_DXGLMaterial>(alias)) {
			material = std::make_shared<DXGLMaterial>(filepath, DXGLMain::graphics());
			m_resources[typeid(SP_DXGLMaterial)][alias] = material;
		}
	} catch (...) {
		throw std::runtime_error("DXGLMaterial could not be stored.");
	}
}

void DXGLResourceManager::storeMaterial(const MaterialData& data, const std::string& alias) {
	SP_DXGLMaterial material = nullptr;
	try {
		if (!find<SP_DXGLMaterial>(alias)) {
			material = std::make_shared<DXGLMaterial>(data, DXGLMain::graphics());
			m_resources[typeid(SP_DXGLMaterial)][alias] = material;
		}
	} catch (...) {
		throw std::runtime_error("DXGLMaterial could not be stored.");
	}
}

SP_DXGLMesh DXGLResourceManager::createMesh(const std::string& filepath) {
	SP_DXGLMesh mesh = nullptr;
	try {
		mesh = std::make_shared<DXGLMesh>(filepath);
	} catch (...) {
		throw std::runtime_error("DXGLMesh could ont be created.");
	}
	return mesh;
}

SP_DXGLBasicMesh DXGLResourceManager::createBasicMesh(const std::string& filepath) {
	SP_DXGLBasicMesh mesh = nullptr;
	try {
		mesh = std::make_shared<DXGLBasicMesh>(filepath);
	} catch (...) {
		throw std::runtime_error("DXGLBasicMesh could ont be created.");
	}
	return mesh;
}

void DXGLResourceManager::storeMesh(const std::string& filepath, const std::string& alias) {
	SP_DXGLMesh mesh = nullptr;
	try {
		if (!find<SP_DXGLMesh>(alias)) {
			mesh = std::make_shared<DXGLMesh>(filepath);
			m_resources[typeid(SP_DXGLMesh)][alias] = mesh;
		}
	} catch (...) {
		throw std::runtime_error("DXGLMesh could ont be stored.");
	}
}

void DXGLResourceManager::storeBasicMesh(const std::string& filepath, const std::string& alias) {
	SP_DXGLBasicMesh mesh = nullptr;
	try {
		if (!find<SP_DXGLBasicMesh>(alias)) {
			mesh = std::make_shared<DXGLBasicMesh>(filepath);
			m_resources[typeid(SP_DXGLBasicMesh)][alias] = mesh;
		}
	}
	catch (...) {
		throw std::runtime_error("DXGLBasicMesh could ont be stored.");
	}
}
