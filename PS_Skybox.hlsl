SamplerState skyboxSampler: register(s0);
TextureCube cubemap:        register(t0);

struct PS_Input {
	float3 worldPosition: POSITION;
	float4 position:      SV_POSITION;
};

float4 main(PS_Input input) : SV_TARGET {
	return pow(cubemap.SampleLevel(skyboxSampler, input.worldPosition, 0), 2.2333f);
}