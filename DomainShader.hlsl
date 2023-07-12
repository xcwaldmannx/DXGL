#include "SharedShader.hlsli"

#define NUM_CONTROL_POINTS 3

SamplerState textureSampler: register(s0);

Texture2DArray<float4> tex_materials: register(t0);

struct HS_CONSTANT_DATA_OUTPUT {
	float EdgeTessFactor[3]: SV_TessFactor;
	float InsideTessFactor:  SV_InsideTessFactor;
};

[domain("tri")]
PS_Input main(HS_CONSTANT_DATA_OUTPUT input, float3 domain : SV_DomainLocation,
    const OutputPatch<DS_Input, NUM_CONTROL_POINTS> patch) {
    PS_Input output = (PS_Input)0;

    // Calculate the position of the vertex using barycentric coordinates
    output.position = float4(patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z, 1.0f);
    output.worldPosition = output.position;

    output.texcoord = patch[0].texcoord * domain.x + patch[1].texcoord * domain.y + patch[2].texcoord * domain.z;
    output.normal = patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z;
    output.tangent = patch[0].tangent * domain.x + patch[1].tangent * domain.y + patch[2].tangent * domain.z;

    // Calculate the displacement based on the height map
    float displacement = tex_materials.SampleLevel(textureSampler, float3(output.texcoord, TEX_NORM_DISP), 0).a;
    output.position += float4(output.normal * displacement * 0.1f, 0.0f);

    output.position = mul(output.position, patch[0].viewProj * domain.x + patch[1].viewProj * domain.y + patch[2].viewProj * domain.z);

    output.instanceFlags = patch[0].instanceFlags;

    for (int i = 0; i < 4; i++) {
        output.shadowPositions[i] = patch[0].shadowPositions[i] * domain.x + patch[1].shadowPositions[i] * domain.y + patch[2].shadowPositions[i] * domain.z;
    }

    return output;
}

