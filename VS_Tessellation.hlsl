#include "SharedShader.hlsli"

HS_Input main(VS_Input input) {
	HS_Input output = (HS_Input)0;

	float4x4 worldMatrix = getWorldMatrix(input.instanceScale, input.instanceRotation, input.instanceTranslation);

	output.position = mul(float4(input.position, 1.0f), worldMatrix);

	output.texcoord = input.texcoord * input.instanceScale.xz;
	output.normal = mul(input.normal, worldMatrix);
	output.tangent = mul(input.tangent, worldMatrix);

	output.instanceFlags = input.instanceFlags;

	output.viewProj = mul(view, proj);

	// Compute tessellation factor
	float MAX_TESSELLATION = 64.0f;

	//float pixelDirection = input.position.xyz - cameraPosition.xyz;
	float distCamToPixel = distance(output.position.xyz, camPosition.xyz);

	const float e = 2.71828183f;
	float decay = 0.3f;
	float offset = 64.0f;
	float tess = ceil(MAX_TESSELLATION / (1.0f + pow(e, decay * (distCamToPixel - offset))));
	output.tessFactor = max(1.0f, tess);

	// Compute shadows (Move to DS)
	float4 shadowWorldPos = mul(float4(input.position, 1.0f), worldMatrix);

	float4x4 shadowViewProj_0 = mul(cascades[0].shadowView, cascades[0].shadowProj);
	float4x4 shadowViewProj_1 = mul(cascades[1].shadowView, cascades[1].shadowProj);
	float4x4 shadowViewProj_2 = mul(cascades[2].shadowView, cascades[2].shadowProj);
	float4x4 shadowViewProj_3 = mul(cascades[3].shadowView, cascades[3].shadowProj);

	float4 shadowHomo_0 = mul(float4(shadowWorldPos.xyz, 1.0f), shadowViewProj_0);
	float4 shadowHomo_1 = mul(float4(shadowWorldPos.xyz, 1.0f), shadowViewProj_1);
	float4 shadowHomo_2 = mul(float4(shadowWorldPos.xyz, 1.0f), shadowViewProj_2);
	float4 shadowHomo_3 = mul(float4(shadowWorldPos.xyz, 1.0f), shadowViewProj_3);

	output.shadowPositions[0] = shadowHomo_0 * float4(0.5f, -0.5f, 1.0f, 1.0f) + (float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowHomo_0.w);
	output.shadowPositions[1] = shadowHomo_1 * float4(0.5f, -0.5f, 1.0f, 1.0f) + (float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowHomo_1.w);
	output.shadowPositions[2] = shadowHomo_2 * float4(0.5f, -0.5f, 1.0f, 1.0f) + (float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowHomo_2.w);
	output.shadowPositions[3] = shadowHomo_3 * float4(0.5f, -0.5f, 1.0f, 1.0f) + (float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowHomo_3.w);

	return output;
}
