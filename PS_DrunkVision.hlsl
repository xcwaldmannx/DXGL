SamplerState textureSampler: register(s0);
Texture2D backBuffer: register(t0);

static const float PI = 3.14159265359f;

struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
};

cbuffer screen: register(b0) {
	float1 screenWidth;
	float1 screenHeight;
	float2 pad;
}

cbuffer drunkvision: register(b1) {
	float time;
	float period;
	float amplitude;
};

float4 main(PS_Input input) : SV_TARGET{

	float x1 = (sin(time * 2 * PI / period) * amplitude) * (1 - abs(2 * input.texcoord.x - 1));
	float y1 = (sin(time * 4 * PI / period) * amplitude) * (1 - abs(2 * input.texcoord.y - 1));

	float x2 = -(sin(time * 2 * (PI / 2.0f) / period) * amplitude) * (1 - abs(2 * input.texcoord.x - 1));
	float y2 = (sin(time * 4 * (PI / 2.0f) / period) * amplitude) * (1 - abs(2 * input.texcoord.y - 1));

	float3 color1 = backBuffer.Sample(textureSampler, input.texcoord + float2(x1, y1) / 3.0f).rgb;
	float3 color2 = (backBuffer.Sample(textureSampler, input.texcoord + float2(x1, y1)).rgb / 3.0f);
	float3 color3 = (backBuffer.Sample(textureSampler, input.texcoord + float2(x2, y2)).rgb / 3.0f);
	float3 finalColor = color1 + color2 + color3;

	// HDR tone mapping and gamma correction
	float exposure = 10.0f;
	float gamma = 1.0f;

	float3 toneMap = float3(1.0f, 1.0f, 1.0f) - exp((-finalColor) * exposure);
	float3 toneMappedColor = pow(toneMap, float3(gamma, gamma, gamma));

	return float4(toneMappedColor, 1);
}