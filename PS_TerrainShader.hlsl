#include "LightingShader.hlsli"

struct PS_TerrainInput {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	float3 worldPosition: WORLD_POS;
};

SamplerState textureSampler: register(s0);
SamplerState shadowSampler:  register(s1);

Texture2DArray<float4> tex_materials: register(t0);
TextureCube            tex_skybox:    register(t1);
Texture2D              tex_brdf:      register(t2);

cbuffer terrainbuffer: register(b4) {
	row_major float4x4 worldd;
	row_major float4x4 vieww;
	row_major float4x4 projj;

	int materialsFlags;
}

float3 calculateNormals(PS_TerrainInput input) {

	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent);
	tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	float3 bitangent = normalize(cross(normal, tangent));

	const float3x3 TBN = float3x3(tangent, bitangent, normal);

	Material mat = getMaterial(materialsFlags);

	float3 normalSample = mat.useNormal ? tex_materials.Sample(textureSampler, float3(input.texcoord, TEX_NORM_DISP)).rgb : float3(0, 1, 0);
	normalSample.x = 2.0f * normalSample.r - 1.0f;
	normalSample.y = -2.0f * normalSample.g + 1.0f;
	normalSample.z = normalSample.b;
	normal = mul(normalSample, TBN);

	return normal;
}

float4 main(PS_TerrainInput input) : SV_TARGET {
	// transform normals
	input.normal = calculateNormals(input);

	float visibility = 1.0f;

	// lighting
	float3 lightColor = float3(0, 0, 0);
	float3 fogColor = float3(0, 0, 0);
	float3 outputColor = float3(0, 0, 0);

	//if (instanceFlags.useLighting) {
		lightColor = calcLighting(tex_materials, tex_skybox, tex_brdf, textureSampler, input.texcoord, input.normal, input.worldPosition, materialsFlags);
		outputColor = (visibility * lightColor) + (1.0f - visibility) * lightColor;
		// outputColor = (visibility * direcLightColor) + ((1.0f - visibility) * pointLightColor) + (visibility * pointLightColor);
	//} else {
	//	outputColor = visibility * pow(tex_materials.Sample(textureSampler, float3(input.texcoord, TEX_ALBEDO)).rgb, 2.2333f);
	//}

	return float4(outputColor, 1.0f);
}