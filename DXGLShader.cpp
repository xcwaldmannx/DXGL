#include "DXGLShader.h"

#include <fstream>
#include <vector>

using namespace dxgl;

DXGLShader::DXGLShader(SP_DXGLGraphics graphics) : m_graphics(graphics) {
}

DXGLShader::~DXGLShader() {
}

template<class T>
std::shared_ptr<T> DXGLShader::create(const std::string& filename) {

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
        shader = std::make_shared<T>(byteCode.data(), length, m_graphics);
    }
    catch (...) {
        throw std::exception("Shader could not be created.");
    }

    return shader;
}

void DXGLShader::addSet(const DXGLShaderSet& set, const std::string& alias) {
    SP_DXGLShaderSet shaderSet = nullptr;
    try {
        shaderSet = std::make_shared<DXGLShaderSet>(set);
        m_shaderSets[alias] = shaderSet;
    } catch (...) {
        throw std::exception("DXGLShaderSet could not be created.");
    }
}

SP_DXGLShaderSet DXGLShader::getSet(const std::string& alias) {
    return m_shaderSets[alias];
}

void DXGLShader::VS_setShader(const SP_DXGLVertexShader& vs) {
    if (vs) {
        m_graphics->context()->VSSetShader(vs->get(), nullptr, 0);
    } else {
        m_graphics->context()->VSSetShader(0, nullptr, 0);
    }
}

void DXGLShader::HS_setShader(const SP_DXGLHullShader& hs) {
    if (hs) {
        m_graphics->context()->HSSetShader(hs->get(), nullptr, 0);
    }
    else {
        m_graphics->context()->HSSetShader(0, nullptr, 0);
    }
}

void DXGLShader::DS_setShader(const SP_DXGLDomainShader& ds) {
    if (ds) {
        m_graphics->context()->DSSetShader(ds->get(), nullptr, 0);
    }
    else {
        m_graphics->context()->DSSetShader(0, nullptr, 0);
    }
}

void DXGLShader::PS_setShader(const SP_DXGLPixelShader& ps) {
    if (ps) {
        m_graphics->context()->PSSetShader(ps->get(), nullptr, 0);
    } else {
        m_graphics->context()->PSSetShader(0, nullptr, 0);
    }
}

void DXGLShader::setShaderSet(const DXGLShaderSet& set) {
    VS_setShader(set.vs);
    HS_setShader(set.hs);
    DS_setShader(set.ds);
    PS_setShader(set.ps);
}

void DXGLShader::setShaderSet(const std::string& alias) {
    const SP_DXGLShaderSet& set = m_shaderSets[alias];
    VS_setShader(set->vs);
    HS_setShader(set->hs);
    DS_setShader(set->ds);
    PS_setShader(set->ps);
}

void DXGLShader::VS_setResource(UINT slot, ID3D11ShaderResourceView* srv) {
    ID3D11ShaderResourceView* srvv = (srv == 0) ? nullptr : srv;
    m_graphics->context()->VSSetShaderResources(slot, 1, &srvv);
    srvv = nullptr;
}

void DXGLShader::VS_setResources(UINT slot, UINT count, ID3D11ShaderResourceView** srv) {
    m_graphics->context()->VSSetShaderResources(slot, count, srv);
}

void DXGLShader::PS_setResource(UINT slot, ID3D11ShaderResourceView* srv) {
    m_graphics->context()->PSSetShaderResources(slot, 1, &srv);
}

void DXGLShader::PS_setResources(UINT slot, UINT count, ID3D11ShaderResourceView** srv) {
    m_graphics->context()->PSSetShaderResources(slot, count, srv);
}

void DXGLShader::VS_setMaterial(UINT slot, UINT count, const SP_Material& material) {
    ID3D11ShaderResourceView* srv = material->get();
    m_graphics->context()->VSSetShaderResources(slot, count, &srv);
}

void DXGLShader::DS_setMaterial(UINT slot, UINT count, const SP_Material& material) {
    ID3D11ShaderResourceView* srv = material->get();
    m_graphics->context()->DSSetShaderResources(slot, count, &srv);
}

void DXGLShader::PS_setMaterial(UINT slot, UINT count, const SP_Material& material) {
    ID3D11ShaderResourceView* srv = material->get();
    m_graphics->context()->PSSetShaderResources(slot, count, &srv);
}

void DXGLShader::VS_setCBuffer(UINT slot, UINT count, ID3D11Buffer* buffer) {
    //ID3D11Buffer* cbuffer = (buffer == 0) ? nullptr : buffer->get();
    m_graphics->context()->VSSetConstantBuffers(slot, count, &buffer);
    //cbuffer = nullptr;
}

void DXGLShader::HS_setCBuffer(UINT slot, UINT count, ID3D11Buffer* buffer) {
    //ID3D11Buffer* cbuffer = (buffer == 0) ? nullptr : buffer->get();
    m_graphics->context()->HSSetConstantBuffers(slot, count, &buffer);
    //cbuffer = nullptr;
}

void DXGLShader::DS_setCBuffer(UINT slot, UINT count, ID3D11Buffer* buffer) {
    //ID3D11Buffer* cbuffer = (buffer == 0) ? nullptr : buffer->get();
    m_graphics->context()->DSSetConstantBuffers(slot, count, &buffer);
    //cbuffer = nullptr;
}

void DXGLShader::PS_setCBuffer(UINT slot, UINT count, ID3D11Buffer* buffer) {
    //ID3D11Buffer* cbuffer = (buffer == 0) ? nullptr : buffer->get();
    m_graphics->context()->PSSetConstantBuffers(slot, count, &buffer);
    //cbuffer = nullptr;
}

void DXGLShader::DS_setSampler(UINT slot, UINT count, SP_DXGLSamplerState sampler) {
    ID3D11SamplerState* samp = sampler->get();
    m_graphics->context()->DSSetSamplers(slot, count, &samp);
    samp = nullptr;
}

void DXGLShader::PS_setSampler(UINT slot, UINT count, SP_DXGLSamplerState sampler) {
    ID3D11SamplerState* samp = sampler->get();
    m_graphics->context()->PSSetSamplers(slot, count, &samp);
    samp = nullptr;
}

template SP_DXGLVertexShader DXGLShader::create<DXGLVertexShader>(const std::string&);
template SP_DXGLHullShader DXGLShader::create<DXGLHullShader>(const std::string&);
template SP_DXGLDomainShader DXGLShader::create<DXGLDomainShader>(const std::string&);
template SP_DXGLPixelShader DXGLShader::create<DXGLPixelShader>(const std::string&);
//template SP_DXGLComputeShader DXGLStageShader::createShader<DXGLComputeShader>(const std::string&);
