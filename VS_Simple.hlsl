struct VS_Input {
	float3 position: POSITION;

	int1   instanceId:          INSTANCE_ID;
	float3 instanceScale:       INSTANCE_SCALE;
	float3 instanceRotation:    INSTANCE_ROTATION;
	float3 instanceTranslation: INSTANCE_TRANSLATION;
	int1   instanceFlags:       INSTANCE_FLAGS;
};

struct PS_Input {
	float4 position: SV_POSITION;
};

cbuffer transform: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;
};

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

PS_Input main(VS_Input input) {
	PS_Input output = (PS_Input)0;

	float4x4 worldMatrix = getWorldMatrix(input.instanceScale, input.instanceRotation, input.instanceTranslation);

	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);

	return output;
}