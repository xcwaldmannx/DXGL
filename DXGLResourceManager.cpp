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
template SP_VertexBuffer     DXGLResourceManager::get<SP_VertexBuffer>(const std::string&);
template SP_IndexBuffer      DXGLResourceManager::get<SP_IndexBuffer>(const std::string&);
template SP_InstanceBuffer   DXGLResourceManager::get<SP_InstanceBuffer>(const std::string&);
template SP_VSConstantBuffer DXGLResourceManager::get<SP_VSConstantBuffer>(const std::string&);
template SP_HSConstantBuffer DXGLResourceManager::get<SP_HSConstantBuffer>(const std::string&);
template SP_DSConstantBuffer DXGLResourceManager::get<SP_DSConstantBuffer>(const std::string&);
template SP_PSConstantBuffer DXGLResourceManager::get<SP_PSConstantBuffer>(const std::string&);
template SP_InputLayout      DXGLResourceManager::get<SP_InputLayout>(const std::string&);

template SP_DXGLSamplerState   DXGLResourceManager::get<SP_DXGLSamplerState>(const std::string&);

template SP_Texture2D      DXGLResourceManager::get<SP_Texture2D>(const std::string&);
template SP_TextureCube    DXGLResourceManager::get<SP_TextureCube>(const std::string&);

template SP_Material       DXGLResourceManager::get<SP_Material>(const std::string&);
template SP_Mesh           DXGLResourceManager::get<SP_Mesh>(const std::string&);

// find
template bool DXGLResourceManager::find<SP_VertexBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_IndexBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_InstanceBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_VSConstantBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_HSConstantBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_DSConstantBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_PSConstantBuffer>(const std::string&);
template bool DXGLResourceManager::find<SP_InputLayout>(const std::string&);

template bool DXGLResourceManager::find<SP_DXGLSamplerState>(const std::string&);

template bool DXGLResourceManager::find<SP_Texture2D>(const std::string&);
template bool DXGLResourceManager::find<SP_TextureCube>(const std::string&);

template bool DXGLResourceManager::find<SP_Material>(const std::string&);
template bool DXGLResourceManager::find<SP_Mesh>(const std::string&);

SP_VertexBuffer DXGLResourceManager::createVertexBuffer(void* vertices, int vertexCount, int vertexSize) {
	SP_VertexBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<VertexBuffer>(vertices, vertexCount, vertexSize);
	} catch (...) {
		throw std::runtime_error("VertexBuffer could ont be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeVertexBuffer(void* vertices, int vertexCount, int vertexSize, const std::string& alias) {
	SP_VertexBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<VertexBuffer>(vertices, vertexCount, vertexSize);
		m_resources[typeid(SP_VertexBuffer)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("VertexBuffer could ont be stored.");
	}
}

SP_IndexBuffer DXGLResourceManager::createIndexBuffer(void* indices, int indexCount) {
	SP_IndexBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<IndexBuffer>(indices, indexCount);
	} catch (...) {
		throw std::runtime_error("IndexBuffer could ont be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeIndexBuffer(void* indices, int indexCount, const std::string& alias) {
	SP_IndexBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<IndexBuffer>(indices, indexCount);
		m_resources[typeid(SP_IndexBuffer)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("IndexBuffer could ont be stored.");
	}
}

SP_InstanceBuffer DXGLResourceManager::createInstanceBuffer(void* instances, int instanceCount, int instanceSize) {
	SP_InstanceBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<InstanceBuffer>(instances, instanceCount, instanceSize);
	} catch (...) {
		throw std::runtime_error("InstanceBuffer could ont be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeInstanceBuffer(void* instances, int instanceCount, int instanceSize, const std::string& alias) {
	SP_InstanceBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<InstanceBuffer>(instances, instanceCount, instanceSize);
		m_resources[typeid(SP_InstanceBuffer)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("InstanceBuffer could ont be stored.");
	}
}

SP_VSConstantBuffer DXGLResourceManager::createVSConstantBuffer(UINT bytes) {
	SP_VSConstantBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<VSConstantBuffer>(bytes);
	} catch (...) {
		throw std::runtime_error("VSConstantBuffer could not be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeVSConstantBuffer(UINT bytes, std::string alias) {
	SP_VSConstantBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<VSConstantBuffer>(bytes);
		m_resources[typeid(SP_VSConstantBuffer)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("VSConstantBuffer could not be stored.");
	}
}

SP_HSConstantBuffer DXGLResourceManager::createHSConstantBuffer(UINT bytes) {
	SP_HSConstantBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<HSConstantBuffer>(bytes);
	}
	catch (...) {
		throw std::runtime_error("HSConstantBuffer could not be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeHSConstantBuffer(UINT bytes, std::string alias) {
	SP_HSConstantBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<HSConstantBuffer>(bytes);
		m_resources[typeid(SP_HSConstantBuffer)][alias] = buffer;
	}
	catch (...) {
		throw std::runtime_error("HSConstantBuffer could not be stored.");
	}
}

SP_DSConstantBuffer DXGLResourceManager::createDSConstantBuffer(UINT bytes) {
	SP_DSConstantBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DSConstantBuffer>(bytes);
	}
	catch (...) {
		throw std::runtime_error("DSConstantBuffer could not be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeDSConstantBuffer(UINT bytes, std::string alias) {
	SP_DSConstantBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<DSConstantBuffer>(bytes);
		m_resources[typeid(SP_DSConstantBuffer)][alias] = buffer;
	}
	catch (...) {
		throw std::runtime_error("DSConstantBuffer could not be stored.");
	}
}

SP_PSConstantBuffer DXGLResourceManager::createPSConstantBuffer(UINT bytes) {
	SP_PSConstantBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<PSConstantBuffer>(bytes);
	}
	catch (...) {
		throw std::runtime_error("PSConstantBuffer could not be created.");
	}
	return buffer;
}

void DXGLResourceManager::storePSConstantBuffer(UINT bytes, std::string alias) {
	SP_PSConstantBuffer buffer = nullptr;
	try {
		buffer = std::make_shared<PSConstantBuffer>(bytes);
		m_resources[typeid(SP_PSConstantBuffer)][alias] = buffer;
	}
	catch (...) {
		throw std::runtime_error("PSConstantBuffer could not be stored.");
	}
}

SP_InputLayout DXGLResourceManager::createInputLayout(const InputLayoutDesc& desc, const std::string& filename) {
	SP_InputLayout buffer = nullptr;
	try {
		buffer = std::make_shared<InputLayout>(desc, filename);
	} catch (...) {
		throw std::runtime_error("InputLayout could ont be created.");
	}
	return buffer;
}

SP_InputLayout DXGLResourceManager::createInputLayout(const InputLayoutDesc& desc, ID3DBlob* shaderBytecode) {
	SP_InputLayout buffer = nullptr;
	try {
		buffer = std::make_shared<InputLayout>(desc, shaderBytecode);
	}
	catch (...) {
		throw std::runtime_error("InputLayout could ont be created.");
	}
	return buffer;
}

void DXGLResourceManager::storeInputLayout(const InputLayoutDesc& desc, const std::string& filename, const std::string& alias) {
	SP_InputLayout buffer = nullptr;
	try {
		buffer = std::make_shared<InputLayout>(desc, filename);
		m_resources[typeid(SP_InputLayout)][alias] = buffer;
	} catch (...) {
		throw std::runtime_error("InputLayout could ont be stored.");
	}
}

void DXGLResourceManager::storeInputLayout(const InputLayoutDesc& desc, ID3DBlob* shaderBytecode, const std::string& alias) {
	SP_InputLayout buffer = nullptr;
	try {
		buffer = std::make_shared<InputLayout>(desc, shaderBytecode);
		m_resources[typeid(SP_InputLayout)][alias] = buffer;
	}
	catch (...) {
		throw std::runtime_error("InputLayout could ont be stored.");
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

SP_Texture2D DXGLResourceManager::createTexture2D(const std::string& filename) {
	SP_Texture2D texture = nullptr;
	try {
		texture = std::make_shared<Texture2D>(filename);
	} catch (...) {
		throw std::exception("Texture2D could not be created.");
	}
	return texture;
}

SP_Texture2D DXGLResourceManager::createTexture2D(unsigned int width, unsigned int height, unsigned char* data) {
	SP_Texture2D texture = nullptr;
	try {
		texture = std::make_shared<Texture2D>(width, height, data);
	} catch (...) {
		throw std::exception("Texture2D could not be created.");
	}
	return texture;
}

void DXGLResourceManager::storeTexture2D(const std::string& filename, const std::string& alias) {
	SP_Texture2D texture = nullptr;
	try {
		if (!find<SP_Texture2D>(alias)) {
			texture = std::make_shared<Texture2D>(filename);
			m_resources[typeid(SP_Texture2D)][alias] = texture;
		}
	} catch (...) {
		throw std::exception("Texture2D could not be stored.");
	}
}

void DXGLResourceManager::storeTexture2D(unsigned int width, unsigned int height, unsigned char* data, const std::string& alias) {
	SP_Texture2D texture = nullptr;
	try {
		if (!find<SP_Texture2D>(alias)) {
			texture = std::make_shared<Texture2D>(width, height, data);
			m_resources[typeid(SP_Texture2D)][alias] = texture;
		}
	} catch (...) {
		throw std::exception("Texture2D could not be stored.");
	}
}

SP_TextureCube DXGLResourceManager::createTextureCube(const std::string& filename) {
	SP_TextureCube texture = nullptr;
	try {
		texture = std::make_shared<TextureCube>(filename);
	}
	catch (...) {
		throw std::exception("TextureCube could not be created.");
	}
	return texture;
}

void DXGLResourceManager::storeTextureCube(const std::string& filename, const std::string& alias) {
	SP_TextureCube texture = nullptr;
	try {
		if (!find<SP_TextureCube>(alias)) {
			texture = std::make_shared<TextureCube>(filename);
			m_resources[typeid(SP_TextureCube)][alias] = texture;
		}
	}
	catch (...) {
		throw std::exception("TextureCube could not be stored.");
	}
}

SP_Material DXGLResourceManager::createMaterial(const std::string& filepath) {
	SP_Material material = nullptr;
	try {
		material = std::make_shared<Material>(filepath);
	} catch (...) {
		throw std::runtime_error("Material could not be created.");
	}
	return material;
}

SP_Material DXGLResourceManager::createMaterial(const MaterialData& data) {
	SP_Material material = nullptr;
	try {
		material = std::make_shared<Material>(data);
	} catch (...) {
		throw std::runtime_error("Material could not be created.");
	}
	return material;
}

void DXGLResourceManager::storeMaterial(const std::string& filepath, const std::string& alias) {
	SP_Material material = nullptr;
	try {
		if (!find<SP_Material>(alias)) {
			material = std::make_shared<Material>(filepath);
			m_resources[typeid(SP_Material)][alias] = material;
		}
	} catch (...) {
		throw std::runtime_error("Material could not be stored.");
	}
}

void DXGLResourceManager::storeMaterial(const MaterialData& data, const std::string& alias) {
	SP_Material material = nullptr;
	try {
		if (!find<SP_Material>(alias)) {
			material = std::make_shared<Material>(data);
			m_resources[typeid(SP_Material)][alias] = material;
		}
	} catch (...) {
		throw std::runtime_error("Material could not be stored.");
	}
}

SP_Mesh DXGLResourceManager::createMesh(const MeshDesc& desc, const std::string& filepath) {
	SP_Mesh mesh = nullptr;
	try {
		mesh = std::make_shared<Mesh>(desc, filepath);
	} catch (...) {
		throw std::runtime_error("Mesh could ont be created.");
	}
	return mesh;
}

void DXGLResourceManager::storeMesh(const MeshDesc& desc, const std::string& filepath, const std::string& alias) {
	SP_Mesh mesh = nullptr;
	try {
		if (!find<SP_Mesh>(alias)) {
			mesh = std::make_shared<Mesh>(desc, filepath);
			m_resources[typeid(SP_Mesh)][alias] = mesh;
		}
	}
	catch (...) {
		throw std::runtime_error("Mesh could ont be stored.");
	}
}
