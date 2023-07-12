struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	float3 pixelPosition:  POSITION0;
	float4 shadowPositions[4]: SHADOW_POSITIONS;
};

cbuffer entity: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;

	int1 useInstancing;
	int1 flags;
	float2 pad0;

	float3 camPosition;
	float1 pad1;
	float3 camDirection;
	float1 pad2;
}

struct ShadowCascade {
	row_major float4x4 shadowView;
	row_major float4x4 shadowProj;
	float shadowMapResolution;
	float shadowPcfSampleCount;
};

cbuffer shadow: register(b2) {
	ShadowCascade cascades[4];
};

struct Light {
	float3 position;
	float pad0;
	float3 direction;
	float pad1;
	float3 color;
	float pad2;
	float2 angles;
	float2 pad3;
};

cbuffer light: register(b3) {
	Light lights[16];
	int lightCount;
	float3 pad4;
};

static const float PI = 3.14159265359f;

SamplerState textureSampler: register(s0);
SamplerState shadowSampler:  register(s1);

Texture2D tex_normalMap:     register(t0);
Texture2D tex_heightMap:     register(t1);
Texture2D tex_albedoMap:     register(t2);
Texture2D tex_metallicMap:   register(t3);
Texture2D tex_roughnessMap:  register(t4);
Texture2D tex_aoMap:         register(t5);

Texture2D tex_shadowMaps[4]: register(t6);

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
	float3 V = normalize(camPosition - input.pixelPosition);

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

float3 getPBRDirecLighting(PS_Input input) {
	float3 color = float3(0.0f, 0.0f, 0.0f);
	float3 albedoSample = pow(tex_albedoMap.Sample(textureSampler, input.texcoord).rgb, 2.2333f);
	float metallicSample = tex_metallicMap.Sample(textureSampler, input.texcoord).r;
	float roughnessSample = tex_roughnessMap.Sample(textureSampler, input.texcoord).r;
	float aoSample = tex_aoMap.Sample(textureSampler, input.texcoord).r;

	float3 N = normalize(input.normal);
	float3 V = normalize(camPosition - input.pixelPosition);

	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	F0 = lerp(F0, albedoSample, metallicSample);

	float3 lightDirections[2] = {
		normalize(float3(0.0f, -1.0f, 1.0f)),  // Direction of light 1
		normalize(float3(-1.0f, -1.0f, 0.0f))  // Direction of light 2
	};

	float3 lightColors[2] = {
		float3(1.0f, 1.0f, 1.0f),   // Color of light 1
		float3(0.7f, 0.7f, 1.0f)    // Color of light 2
	};

	// reflectance equation
	float3 Lo = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 2; i++) {
		// calculate per-light radiance
		float3 L = -lightDirections[i];  // Invert the light direction for a directional light
		float3 H = normalize(V + L);
		float3 radiance = lightColors[i];

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
		Lo += (((kD * albedoSample / PI) + specular) * radiance * NdotL);
	}

	float3 ambient = float3(0.001f, 0.001f, 0.001f) * albedoSample * aoSample;
	color = ambient + Lo;
	return color;
}


float3 getPBRPointLighting(PS_Input input) {
	float3 color = float3(0.0f, 0.0f, 0.0f);
	float3 albedoSample = pow(tex_albedoMap.Sample(textureSampler, input.texcoord).rgb, 2.2333f);
	float metallicSample = tex_metallicMap.Sample(textureSampler, input.texcoord).r;
	float roughnessSample = tex_roughnessMap.Sample(textureSampler, input.texcoord).r;
	float aoSample = tex_aoMap.Sample(textureSampler, input.texcoord).r;

	float3 N = normalize(input.normal);
	float3 V = normalize(camPosition - input.pixelPosition);

	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	F0 = lerp(F0, albedoSample, metallicSample);

	// reflectance equation
	float3 Lo = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < lightCount; i++) {
		// calculate per-light radiance
		float3 L = normalize(lights[i].position - input.pixelPosition);
		float3 H = normalize(V + L);
		float distance = length(lights[i].position - input.pixelPosition);
		float attenuation = 1.0f / (distance * distance);
		float3 radiance = lights[i].color * attenuation;

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

float rand(float2 xy) {
	float2 noise = (frac(sin(dot(xy, float2(12.9898, 78.233) * 2.0)) * 43758.5453));
	return abs(noise.x + noise.y) * 0.5;
}

float getVisibility(PS_Input input, int cascade) {
	float visibility = 1.0f;
	const float bias = 0.005f;
	const int distCamToPixel = floor(length(camPosition.xyz - input.pixelPosition.xyz));

	const int pcf = cascades[cascade].shadowPcfSampleCount;
	const int resolution = cascades[cascade].shadowMapResolution;

	const float texelSize = 1.0f / resolution;
	const float totalTexels = pow(pcf * 2.0f + 1.0f, 2);
	float total = 0.0f;

	input.shadowPositions[cascade].z = input.shadowPositions[cascade].z / input.shadowPositions[cascade].w;

	[loop]
	for (int x = -pcf; x <= pcf; x++) {
		[loop]
		for (int y = -pcf; y <= pcf; y++) {
			float2 pcfOffset = float2(x, y);// *max(1.0f, distCamToPixel / 100.0f);
			float2 shadowCoord = input.shadowPositions[cascade].xy + pcfOffset * texelSize;

			// Apply noise to the pixel
			float2 noiseOffset = rand(input.texcoord.xy) / (resolution + distCamToPixel * 10.0f);
			shadowCoord += noiseOffset;

			float pixelSample = tex_shadowMaps[cascade].Sample(shadowSampler, shadowCoord).r;
			if (pixelSample + bias <= input.shadowPositions[cascade].z) {
				total++;
			}
		}
	}

	total /= totalTexels;
	visibility = max(0.05f, 1.0f - total);

	return visibility;
}

float4 main(PS_Input input) : SV_TARGET {

	// transform normals
	input.normal = calculateNormals(input);

	const float distCamToPixel = length(camPosition.xyz - input.pixelPosition.xyz);

	// fog
	float fog = 1.0f;
	float fogDistNear = 128.0f;
	float fogDistFar = 384.0f;
	if (distCamToPixel > fogDistNear) {
		fog = 1.0f - saturate((distCamToPixel - fogDistNear) / (fogDistFar - fogDistNear));
	}
	// fog

	// shadowing
	float visibility = 1.0f;
	float distThresholdMax = 32.0f + (2.0f * pow(3, 3)) + (4.0f * pow(3, 2)) + (16.0f * 3);

	if (distCamToPixel >= distThresholdMax && distCamToPixel < fogDistFar) {
		visibility = getVisibility(input, 3);
	} else {
		int cascadeOffset = 0;
		for (int cascade = 0; cascade < 4; cascade++) {
			float distThreshold = 32.0f + (2.0f * pow(cascade, 3)) + (4.0f * pow(cascade, 2)) + (16.0f * cascade);
			if (distCamToPixel < distThreshold) {
				visibility = getVisibility(input, min(cascade + cascadeOffset, 3));
				break;
			}
		}
	}
	// shadowing

	// lighting
	// calculate point lghting if within certain distance
	float3 pointLightColor = (distCamToPixel < 50.0f) ? getPBRPointLighting(input) : float3(0, 0, 0);
	float3 direcLightColor = getPBRDirecLighting(input);

	float3 color = (visibility * direcLightColor) + ((1.0f - visibility) * pointLightColor) + (visibility * pointLightColor);

	// skybox + fog
	color = lerp(float3(0, 0.25f, 1.0f), color, fog);

	// HDR tone mapping and gamma correction
	float exposure = 10.0f;
	float gamma = 1.0f;
	if (flags == 1) {
		gamma = 1.0f / 2.2333f;
	}

	float3 toneMap = float3(1.0f, 1.0f, 1.0f) - exp((-color) * exposure);
	float3 toneMappedColor = pow(toneMap, float3(gamma, gamma, gamma));

	return float4(toneMappedColor, 1.0f);
}
