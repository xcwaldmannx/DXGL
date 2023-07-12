struct VS_Input {
	float4 position: POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;
};

struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	float3 pixelPosition: POSITION;
};

cbuffer transform: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;
};

PS_Input main(VS_Input input) {
	PS_Input output = (PS_Input)0;

	output.position = mul(input.position, world);

	output.pixelPosition = output.position;

	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);

	output.texcoord = input.texcoord;

	output.normal = mul(input.normal, world);
	output.tangent = mul(input.tangent, world);

	return output;
}
