struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;
};

SamplerState textureSampler: register(s0);

Texture2D texDiffuseMap: register(t0);
Texture2D texNormalMap:  register(t1);


float3 calculateNormals(PS_Input input) {

	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent);
	tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	float3 bitangent = normalize(cross(normal, tangent));

	const float3x3 TBN = float3x3(tangent, bitangent, normal);

	// issue with sRGB color space

	float3 normalSample = texNormalMap.Sample(textureSampler, input.texcoord).rgb;
	normalSample.x = 2.0f * normalSample.r - 1.0f;
	normalSample.y = -2.0f * normalSample.g + 1.0f;
	normalSample.z = normalSample.b;
	normal = mul(normalSample, TBN);

	return normal;
}

float4 main(PS_Input input) : SV_TARGET {
	input.normal = calculateNormals(input);

	float3 ambientLight = float3(0.0f, 0.0f, 0.0f);

	float3 diffuseLight = float3(0.0f, 0.0f, 0.0f);

	// normal of the pixel
	float3 N = normalize(input.normal);

	// the direction of the light
	float3 dir = float3(0, -1, 0);
	float3 L = normalize(-dir);

	float3 diffuseAmount = max(0.0f, dot(L, N));
	diffuseLight = texDiffuseMap.Sample(textureSampler, input.texcoord).rgb;
	diffuseLight *= diffuseAmount;

	float3 color = ambientLight + diffuseLight;

	// HDR tone mapping and gamma correction
	float exposure = 5.0f;
	float gamma = 1.0f;// 2.2f;
	float3 toneMap = float3(1.0f, 1.0f, 1.0f) - exp((-color) * exposure);
	float3 toneMappedColor = pow(toneMap, float3(1.0f / gamma, 1.0f / gamma, 1.0f / gamma));

	return float4(toneMappedColor, 1.0f);
}
