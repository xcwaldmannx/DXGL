#include "SharedShader.hlsli"

PS_Input main(VS_Input input) {
	PS_Input output = (PS_Input)0;

	bool hasBones = false;

	// bone space
	float4x4 boneTransform = getWorldMatrix(float3(1, 1, 1), float3(0, 0, 0), float3(0, 0, 0));

	if (hasBones) {
		boneTransform = boneSpace[input.boneIds[0]] * input.weights[0];
		boneTransform += boneSpace[input.boneIds[1]] * input.weights[1];
		boneTransform += boneSpace[input.boneIds[2]] * input.weights[2];
		boneTransform += boneSpace[input.boneIds[3]] * input.weights[3];
	}

	float4 bonePosition = mul(float4(input.position, 1.0f), boneTransform);

	// world space
	float4x4 worldMatrix = getWorldMatrix(input.instanceScale, input.instanceRotation, input.instanceTranslation);

	worldMatrix = mul(boneTransform, worldMatrix);

	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	output.worldPosition = output.position;

	// view + projection space
	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);

	output.texcoord = input.texcoord;

	output.normal = mul(input.normal, worldMatrix);
	output.tangent = mul(input.tangent, worldMatrix);

	output.boneIds = input.boneIds;
	output.weights = input.weights;

	output.instanceFlags = input.instanceFlags;

	// shadows start

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

	// shadows end

	return output;
}
