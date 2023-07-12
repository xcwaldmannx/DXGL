struct VS_Input {
	float3 position: POSITION;
};

struct PS_Input {
	float3 worldPosition: POSITION;
	float4 position:      SV_POSITION;
};

cbuffer transform: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;
}

PS_Input main(VS_Input input) {
	PS_Input output = (PS_Input)0;

	output.position = mul(float4(input.position, 0.0f), world);
	output.worldPosition = output.position;
	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);
	output.position.z = output.position.w;

	return output;
}