struct VS_Input {
	float3 position: POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 instScale:       INSTANCE_SCALE;
	float3 instRotation:    INSTANCE_ROTATION;
	float3 instTranslation: INSTANCE_TRANSLATION;
	float3 instColor0: INSTANCE_COLOR_ZERO;
	float3 instColor1: INSTANCE_COLOR_ONE;
	float3 instColor2: INSTANCE_COLOR_TWO;
	float3 instColor3: INSTANCE_COLOR_THREE;
	float1 instTimeOffset: INSTANCE_TIME_OFFSET;
};

struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 instColor0: INSTANCE_COLOR_ZERO;
	float3 instColor1: INSTANCE_COLOR_ONE;
	float3 instColor2: INSTANCE_COLOR_TWO;
	float3 instColor3: INSTANCE_COLOR_THREE;
};

cbuffer transform: register(b0) {
	row_major float4x4 model;
	row_major float4x4 view;
	row_major float4x4 proj;
	float time;
	float3 pad;
}

static const float PI = 3.14159265359f;

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

	// sway grass
	float y = (PI / 3.0f) * input.texcoord.y - (PI / 3.0f);
	float finalTime = time + input.instTimeOffset;
	float offsetX = sin(y) * cos(finalTime);
	float offsetZ = sin(y) * cos(2* finalTime);

	offsetX *= input.instScale;
	offsetZ *= input.instScale;

	float4x4 worldMatrix = getWorldMatrix(input.instScale, input.instRotation, input.instTranslation + float3(offsetX, 0, offsetZ));

	output.position = mul(float4(input.position, 1.0f), worldMatrix);
	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);

	output.texcoord = input.texcoord;

	output.instColor0 = input.instColor0;
	output.instColor1 = input.instColor1;
	output.instColor2 = input.instColor2;
	output.instColor3 = input.instColor3;

	return output;
}