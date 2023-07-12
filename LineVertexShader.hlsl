struct VS_Input {
	float4 position: POSITION0;
	float3 color: COLOR0;
};

struct VS_Output {
	float4 position: SV_POSITION;
	float3 color: COLOR0;
};

cbuffer transform: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;
};

VS_Output main(VS_Input input) {
	VS_Output output = (VS_Output)0;

	// world space coordinates
	output.position = mul(input.position, world);

	// view space coordinates
	output.position = mul(output.position, view);

	// screen space coordinates
	output.position = mul(output.position, proj);

	output.color = input.color;

	return output;
}