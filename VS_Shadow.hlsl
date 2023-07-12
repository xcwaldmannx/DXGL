struct VS_Input {
	float3 position: POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;
};

struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	float3 pixelPosition:  POSITION0;
	float4 shadowPositions[4]: SHADOW_POSITIONS;
};

cbuffer entity: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;

	int1 useInstancing;
	int1 flags;
	float2 pad0;

	float3 camPosition;
	float1 pad1;
	float3 camDirection;
	float1 pad2;
}

struct ShadowCascade {
	row_major float4x4 shadowView;
	row_major float4x4 shadowProj;
	float shadowMapResolution;
	float shadowPcfSampleCount;
};

cbuffer shadow: register(b1) {
	ShadowCascade cascades[4];
};

PS_Input main(VS_Input input) {
	PS_Input output = (PS_Input)0;

	output.position = mul(float4(input.position, 1.0f), world);

	output.pixelPosition = output.position;

	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);

	output.texcoord = input.texcoord;

	output.normal = mul(input.normal, world);
	output.tangent = mul(input.tangent, world);

	// shadows start

	float4 shadowWorldPos = mul(float4(input.position.xyz, 1.0f), world);

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
