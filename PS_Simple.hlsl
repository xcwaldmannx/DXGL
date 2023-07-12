struct PS_Input {
	float4 position: SV_POSITION;
};

float4 main(PS_Input input) : SV_TARGET {
	float color = input.position.z;
	return float4(color, color, color, 1);
}