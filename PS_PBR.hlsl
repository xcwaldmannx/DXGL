struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	float3 pixelPosition: POSITION;
};

cbuffer camera: register(b0) {
	float3 cameraPosition;
	float3 cameraDirection;
}

cbuffer fullscreen: register(b1) {
	int fullscreenMode;
}

static const float PI = 3.14159265359f;

SamplerState textureSampler: register(s0);

Texture2D tex_normalMap:    register(t0);
Texture2D tex_heightMap:    register(t1);
Texture2D tex_albedoMap:    register(t2);
Texture2D tex_metallicMap:  register(t3);
Texture2D tex_roughnessMap: register(t4);
Texture2D tex_aoMap:        register(t5);

float3 calculateNormals(PS_Input input) {

	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent);
	tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	float3 bitangent = normalize(cross(normal, tangent));

	const float3x3 TBN = float3x3(tangent, bitangent, normal);

	// issue with sRGB color space MAYBE

	float3 normalSample = tex_normalMap.Sample(textureSampler, input.texcoord).rgb;
	normalSample.x = 2.0f * normalSample.r - 1.0f;
	normalSample.y = -2.0f * normalSample.g + 1.0f;
	normalSample.z = normalSample.b;
	normal = mul(normalSample, TBN);

	return normal;
}

float3 fresnelSchlick(float cosTheta, float3 F0) {
	if (cosTheta > 1.0f) {
		cosTheta = 1.0f;
	}
	float p = pow(1.0f - cosTheta, 5.0f);
	return F0 + (1.0f - F0) * p;
}

float DistributionGGX(float3 N, float3 H, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
	float r = (roughness + 1.0f);
	float k = (r * r) / 8.0f;

	float num = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
	float NdotV = max(dot(N, V), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 getPhongLighting(PS_Input input) {
	// normal of the pixel
	float3 N = normalize(input.normal);

	// the direction of the light
	float3 dir = float3(0, -1, 2);
	float3 L = normalize(-dir);

	// pixel to camera view
	float3 V = normalize(cameraPosition - input.pixelPosition);

	// half-way vector between view and light
	float3 H = normalize(L + V);

	float3 ambientLight = float3(0.0f, 0.0f, 0.0f);
	float3 diffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 specularLight = float3(0.0f, 0.0f, 0.0f);

	float3 diffuseAmount = max(0.0f, dot(L, N));
	diffuseLight = tex_albedoMap.Sample(textureSampler, input.texcoord).rgb;
	diffuseLight *= diffuseAmount;

	float intensity = 0.25f;
	float shininess = 50.0f;
	float specularAmount = pow(dot(N, H), max(1.0f, shininess)) * intensity;
	specularLight = float3(1.0f, 1.0f, 1.0f) * specularAmount;

	float3 color = ambientLight + diffuseLight;
	if (length(specularLight) > 0.0f) {
		color += specularLight;
	}

	return color;
}

float3 getPBRLighting(PS_Input input) {
	float3 cameraPosition1 = float3(0, 0, -5);

	float3 color = float3(0.0f, 0.0f, 0.0f);
	float3 albedoSample = pow(tex_albedoMap.Sample(textureSampler, input.texcoord).rgb, 2.2333f);
	float metallicSample = tex_metallicMap.Sample(textureSampler, input.texcoord).r;
	float roughnessSample = tex_roughnessMap.Sample(textureSampler, input.texcoord).r;
	float aoSample = tex_aoMap.Sample(textureSampler, input.texcoord).r;

	float3 N = normalize(input.normal);
	float3 V = normalize(cameraPosition - input.pixelPosition);

	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	F0 = lerp(F0, albedoSample, metallicSample);

	float3 lightPositions[7] = {
		float3(-18.0f, 0.0f, -3.0f),
		float3(-12.0f, 0.0f, -3.0f),
		float3( -6.0f, 0.0f, -3.0f),
		float3(  0.0f, 0.0f, -3.0f),
		float3(  6.0f, 0.0f, -3.0f),
		float3( 12.0f, 0.0f, -3.0f),
		float3( 18.0f, 0.0f, -3.0f),
	};
	float3 lightColor = float3(1.0f, 1.0f, 1.0f);

	// reflectance equation
	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 7; i++) {
		// calculate per-light radiance
		float3 L = normalize(lightPositions[i] - input.pixelPosition);
		float3 H = normalize(V + L);
		float distance = length(lightPositions[i] - input.pixelPosition);
		float attenuation = 1.0f / (distance * distance);
		float3 radiance = lightColor * attenuation;

		// cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughnessSample);
		float G = GeometrySmith(N, V, L, roughnessSample);
		float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

		float3 kS = F;
		float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
		kD *= 1.0f - metallicSample;

		float3 numerator = NDF * G * F;
		float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f;
		float3 specular = numerator / denominator;

		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0f);
		Lo += (((kD * albedoSample / PI) + specular) * radiance * NdotL);// (kD * albedoSample / 3.14f + specular)* radiance* NdotL;// (kD * albedoSample / PI + specular);
	}

	float3 ambient = float3(0.001f, 0.001f, 0.001f) * albedoSample * aoSample;
	color = ambient + Lo;
	return color;
}

float4 main(PS_Input input) : SV_TARGET {
	input.normal = calculateNormals(input);

	//float3 color = getPhongLighting(input);
	float3 color = getPBRLighting(input);

	// HDR tone mapping and gamma correction
	float exposure = 10.0f;
	float gamma = 1.0f;
	if (fullscreenMode == 1) {
		gamma = 1.0f / 2.2333f;
	};
	float3 toneMap = float3(1.0f, 1.0f, 1.0f) - exp((-color) * exposure);
	float3 toneMappedColor = pow(toneMap, float3(gamma, gamma, gamma));

	return float4(toneMappedColor, 1.0f);
}
