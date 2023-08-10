float4x4 getWorldMatrix(float3 scale, float3 rotation, float3 translation) {
	float4x4 scaleMatrix = float4x4(
		float4(scale.x, 0.0, 0.0, 0.0),
		float4(0.0, scale.y, 0.0, 0.0),
		float4(0.0, 0.0, scale.z, 0.0),
		float4(0.0, 0.0, 0.0, 1.0)
		);

	float4x4 rotationMatrix = float4x4(
		float4(cos(rotation.y) * cos(rotation.z), cos(rotation.y) * sin(rotation.z), -sin(rotation.y), 0.0),
		float4(sin(rotation.x) * sin(rotation.y) * cos(rotation.z) - cos(rotation.x) * sin(rotation.z),
			sin(rotation.x) * sin(rotation.y) * sin(rotation.z) + cos(rotation.x) * cos(rotation.z),
			sin(rotation.x) * cos(rotation.y), 0.0),
		float4(cos(rotation.x) * sin(rotation.y) * cos(rotation.z) + sin(rotation.x) * sin(rotation.z),
			cos(rotation.x) * sin(rotation.y) * sin(rotation.z) - sin(rotation.x) * cos(rotation.z),
			cos(rotation.x) * cos(rotation.y), 0.0),
		float4(0.0, 0.0, 0.0, 1.0)
		);

	float4x4 translationMatrix = float4x4(
		float4(1.0, 0.0, 0.0, 0.0),
		float4(0.0, 1.0, 0.0, 0.0),
		float4(0.0, 0.0, 1.0, 0.0),
		float4(translation.x, translation.y, translation.z, 1.0)
		);

	return mul(mul(scaleMatrix, rotationMatrix), translationMatrix);
}

cbuffer transform: register(b0) {
	row_major matrix view;
	row_major matrix proj;
}

struct VSInput {
	float3 position: POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	float3 iS : INSTANCE_S;
	float3 iR : INSTANCE_R;
	float3 iT : INSTANCE_T;
};

struct PSInput {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	float3 worldPosition: WORLD_POSITION;
};

PSInput main(VSInput input) {
	PSInput output = (PSInput)0;

	bool hasBones = false;

	// bone space
	matrix boneTransform = 
		matrix(1.0f, 0.0f, 0.0f, 0.0f,
		         0.0f, 1.0f, 0.0f, 0.0f,
		         0.0f, 0.0f, 1.0f, 0.0f,
		         0.0f, 0.0f, 0.0f, 1.0f);

	/*
	if (hasBones) {
		boneTransform = boneSpace[input.boneIds[0]] * input.weights[0];
		boneTransform += boneSpace[input.boneIds[1]] * input.weights[1];
		boneTransform += boneSpace[input.boneIds[2]] * input.weights[2];
		boneTransform += boneSpace[input.boneIds[3]] * input.weights[3];
	}
	*/

	float4 bonePosition = mul(float4(input.position, 1.0f), boneTransform);

	// world space
	float4x4 worldMatrix = getWorldMatrix(input.iS, input.iR, input.iT);

	worldMatrix = mul(boneTransform, worldMatrix);

	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	output.worldPosition = output.position;

	// view + projection space
	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);

	output.texcoord = input.texcoord;

	output.normal = mul(input.normal, worldMatrix);
	output.tangent = mul(input.tangent, worldMatrix);

	// output.boneIds = input.boneIds;
	// output.weights = input.weights;

	// output.instanceFlags = input.instanceFlags;

	// shadows start
	/*
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
	*/
	// shadows end

	return output;
}
