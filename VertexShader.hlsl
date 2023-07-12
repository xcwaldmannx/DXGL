#include "ShaderStructs.hlsli"

Texture2D texAmbient: register(t0);
Texture2D texDiffuse: register(t1);
Texture2D texSpecular: register(t2);
Texture2D texNormal: register(t3);
Texture2D depthBuffer: register(t5);

cbuffer transform: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;
};

struct Cascade {
	row_major float4x4 shadowView;
	row_major float4x4 shadowProj;
};

cbuffer shadow: register(b3) {
	Cascade cascade[4];
};

VS_Output main(VS_Input input) {
	VS_Output output = (VS_Output)0;

	// world space coordinates
	output.position = mul(input.position, world);
	output.pixelPos = output.position;

	// view space coordinates
	output.position = mul(output.position, view);

	// projection space coordinates
	output.position = mul(output.position, proj);

	output.texcoord = input.texcoord;

	output.normal = mul(input.normal, (float3x3) world);
	output.tangent = mul(input.tangent, (float3x3) world);

	float4 shadowWorldPos = mul(float4(input.position.xyz, 1.0f), world);
	float4x4 shadowViewProj = mul(cascade[0].shadowView, cascade[0].shadowProj);
	float4 shadowHomo = mul(float4(shadowWorldPos.xyz, 1.0f), shadowViewProj);
	output.shadowPos = shadowHomo * float4(0.5f, -0.5f, 1.0f, 1.0f) + (float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowHomo.w);

	return output;
}