struct VS_Input {
	float4 position: POSITION0;
	float2 texcoord: TEXCOORD0;
	float3 normal: NORMAL0;
	float3 tangent: NORMAL1;
	float3 bitangent: NORMAL2;
};

struct VS_Output {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD0;
	float3 normal: NORMAL0;
	float3 tangent: NORMAL1;
	float3 bitangent: NORMAL2;

	float3 pixelPos: POSITION1;
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

	// projection space coordinates
	output.position = mul(output.position, proj);

	output.texcoord = input.texcoord;

	output.pixelPos = mul(input.position.xyz, world);

	output.normal = mul(input.normal, (float3x3) world);
	output.tangent = mul(input.tangent, (float3x3) world);
	output.bitangent = mul(input.bitangent, (float3x3) world);

	return output;
}