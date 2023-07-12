SamplerState textureSampler: register(s0);
Texture2D backBuffer: register(t0);

struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
};

cbuffer screen: register(b0) {
	float1 screenWidth;
	float1 screenHeight;
	float2 pad;
}

cbuffer nightvision: register(b1) {
	float time;
	float amplitude;
	float speed;
};

float rand(float2 n) {
	return frac(sin(dot(n, float2(12.9898, 78.233))) * 43758.5453);
}

float2 rand2(float2 n) {
	return float2(rand(n), rand(n + float2(5.2, 1.3)));
}

float4 main(PS_Input input) : SV_TARGET {
	float2 noise = rand2(input.texcoord * time * speed);
	float2 offset = (noise * 2.0 - 1.0) * amplitude;
	float3 noiseColor = backBuffer.Sample(textureSampler, input.texcoord + offset).rgb;
	float3 color = backBuffer.Sample(textureSampler, input.texcoord).rgb + noiseColor;

	float intensity = (color.r + color.g + color.b) / 3.0f;
	color.g = intensity;
	color.r = intensity;
	color.b = intensity;

	float3 finalColor = color;

	// HDR tone mapping and gamma correction
	float exposure = 5.0f;
	float gamma = 1.0f;

	float3 toneMap = 1.0f - exp((-finalColor) * exposure);
	float3 toneMappedColor = pow(toneMap, float3(gamma, gamma, gamma));

	return float4(toneMappedColor, 1);
}