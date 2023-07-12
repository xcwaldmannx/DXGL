#include "SharedPostProcess.hlsli"

SamplerState textureSampler: register(s0);
Texture2D    backBuffer:     register(t0);

struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
};

cbuffer screen: register(b0) {
	float1 screenWidth;
	float1 screenHeight;
	float2 pad;
}

float4 main(PS_Input input) : SV_TARGET{

	float2 resolution = float2(screenWidth, screenHeight);
	float dx = 4.0 * (1.0 / resolution.x);
	float dy = 4.0 * (1.0 / resolution.y);
	float2 coord = float2(dx * floor(input.texcoord.x / dx), dy * floor(input.texcoord.y / dy));
	float3 color = backBuffer.Sample(textureSampler, coord).rgb;

	// HDR tone mapping and gamma correction
	float exposure = 10.0f;
	float gamma = 1.0f;

	float3 toneMap = float3(1.0f, 1.0f, 1.0f) - exp((-color) * exposure);
	float3 toneMappedColor = pow(toneMap, float3(gamma, gamma, gamma));

	return float4(toneMappedColor, 1.0f);
}