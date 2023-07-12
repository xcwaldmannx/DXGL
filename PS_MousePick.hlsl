struct PS_Input {
	float4 position: SV_POSITION;
	float3 colorId:  COLOR_ID;
};

float4 main(PS_Input input) : SV_TARGET {
	return float4(input.colorId, 1);
}