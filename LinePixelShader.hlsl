struct VS_Output {
	float4 position: SV_POSITION;
	float3 color: COLOR0;
};

float4 main(VS_Output input) : SV_TARGET {
	return float4(input.color, 1.0f);
}