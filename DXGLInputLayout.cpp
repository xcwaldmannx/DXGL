#include "DXGLInputLayout.h"

#include <fstream>

using namespace dxgl;

DXGLInputLayout::DXGLInputLayout(const InputLayoutDesc& desc, const std::string& filename, SP_DXGLGraphics graphics) {
    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs.seekg(0, std::ios::end);
    std::vector<std::byte> byteCode;
    size_t length = ifs.tellg();
    byteCode.resize(length);
    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char*>(&byteCode[0]), length);
    ifs.close();

    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = createLayout(desc);

    HRESULT result = graphics->device()->CreateInputLayout(&layout[0], (UINT)layout.size(), byteCode.data(), length, &m_layout);

    if (FAILED(result)) {
        throw std::exception("DXGLInputLayout could not be created.");
    }
}

DXGLInputLayout::~DXGLInputLayout() {
    if (m_layout) m_layout->Release();
}

ID3D11InputLayout* DXGLInputLayout::getLayout() {
    return m_layout;
}

std::vector<D3D11_INPUT_ELEMENT_DESC> DXGLInputLayout::createLayout(const InputLayoutDesc& desc) {
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout{};

    for (int i = 0; i < desc.inputs.size(); i++) {
        InputLayoutElement element = desc.inputs[i];

        D3D11_INPUT_CLASSIFICATION classification = element.isInstanced ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
        int useInstance = element.isInstanced ? 1 : 0;

        D3D11_INPUT_ELEMENT_DESC input = { element.semantic, 0, element.format, element.slot, D3D11_APPEND_ALIGNED_ELEMENT, classification, useInstance };
        layout.push_back(input);
    }

    return layout;
}
