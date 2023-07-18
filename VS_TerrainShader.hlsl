struct VS_TerrainInput {
	float3 position: POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;
};

struct PS_TerrainInput {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	float3 worldPosition: WORLD_POS;
};

cbuffer terrainbuffer: register(b4) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;

	int materialsFlags;
}

PS_TerrainInput main(VS_TerrainInput input) {
	PS_TerrainInput output = (PS_TerrainInput)0;

	output.position = mul(float4(input.position, 1.0f), world);
	output.worldPosition = output.position;
	output.position = mul(output.position, view);
	output.position = mul(output.position, proj);

	output.texcoord = input.texcoord;
	output.normal = mul(input.normal, world);
	output.tangent = mul(input.tangent, world);

	return output;
}