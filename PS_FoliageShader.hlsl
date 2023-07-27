struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 instColor0: INSTANCE_COLOR_ZERO;
	float3 instColor1: INSTANCE_COLOR_ONE;
	float3 instColor2: INSTANCE_COLOR_TWO;
	float3 instColor3: INSTANCE_COLOR_THREE;
};

float4 main(PS_Input input) : SV_TARGET{

	float height = 1.0f - input.texcoord.y;

	float3 outputColor = float3(0, 0, 0);

	float transitionHeight = 0.25f;

	if (height > 0.75f) {
		float blend = (height - 0.75f) / transitionHeight;
		outputColor = lerp(input.instColor1, input.instColor0, blend);
	} else if (height > 0.5f) {
		float blend = (height - 0.5f) / transitionHeight;
		outputColor = lerp(input.instColor2, input.instColor1, blend);
	} else if (height > 0.25f) {
		float blend = (height - 0.25f) / transitionHeight;
		outputColor = lerp(input.instColor3, input.instColor2, blend);
	} else {
		outputColor = input.instColor3;
	}

	outputColor = pow(outputColor, 2.2333f);

    return float4(outputColor, 1.0f);
}