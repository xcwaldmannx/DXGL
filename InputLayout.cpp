#include "InputLayout.h"

#include <fstream>

using namespace dxgl;

InputLayout::InputLayout(const InputLayoutDesc& desc, const std::string& filename) {
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = createLayout(desc);

    std::ifstream ifs(filename, std::ifstream::in | std::ifstream::binary);
    ifs.seekg(0, std::ios::end);
    std::vector<std::byte> byteCode;
    size_t length = ifs.tellg();
    byteCode.resize(length);
    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char*>(&byteCode[0]), length);
    ifs.close();

    HRESULT result = Engine::graphics()->device()->CreateInputLayout(&layout[0], (UINT)layout.size(), byteCode.data(), length, &m_layout);

    if (FAILED(result)) {
        throw std::exception("InputLayout could not be created.");
    }
}

InputLayout::InputLayout(const InputLayoutDesc& desc, ID3DBlob* shaderBytecode) {
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = createLayout(desc);

    HRESULT result = Engine::graphics()->device()->CreateInputLayout(&layout[0], (UINT)layout.size(),
        shaderBytecode->GetBufferPointer(), shaderBytecode->GetBufferSize(), &m_layout);

    if (FAILED(result)) {
        throw std::exception("InputLayout could not be created.");
    }
}

InputLayout::~InputLayout() {
    if (m_layout) m_layout->Release();
}

void InputLayout::bind(int slot) {
    Engine::graphics()->context()->IASetInputLayout(m_layout);
}

ID3D11InputLayout* InputLayout::get() {
    return m_layout;
}

std::vector<D3D11_INPUT_ELEMENT_DESC> InputLayout::createLayout(const InputLayoutDesc& desc) {
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
