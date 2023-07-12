SamplerState textureSampler: register(s0);

Texture2D tex: register(t0);

struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;
};

float4 main(PS_Input input) : SV_TARGET {
	float3 color = tex.Sample(textureSampler, input.texcoord).rgb;

	// HDR tone mapping and gamma correction
	float exposure = 3.0f;
	float gamma = 1.0f;// 2.2f;
	float3 toneMap = float3(1.0f, 1.0f, 1.0f) - exp((-color) * exposure);
	float3 toneMappedColor = pow(toneMap, float3(1.0f / gamma, 1.0f / gamma, 1.0f / gamma));

	return float4(toneMappedColor, 1.0f);
}