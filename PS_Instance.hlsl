#ifndef INCLUDE_SHARED
#include "SharedShader.hlsli"
#define INCLUDE_SHARED
#endif

#include "LightingShader.hlsli"

SamplerState textureSampler: register(s0);
SamplerState shadowSampler:  register(s1);

Texture2DArray<float4> tex_materials: register(t0);
TextureCube            tex_skybox:    register(t1);
Texture2D              tex_brdf:      register(t2);

Texture2D tex_shadowMaps[4]: register(t6);

float3 calculateNormals(PS_Input input) {

	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent);
	tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	float3 bitangent = normalize(cross(normal, tangent));

	const float3x3 TBN = float3x3(tangent, bitangent, normal);

	Material mat = getMaterial(materialFlags);

	float3 normalSample = mat.useNormal ? tex_materials.Sample(textureSampler, float3(input.texcoord, TEX_NORM_DISP)).rgb : float3(0, 1, 0);
	normalSample.x =  2.0f * normalSample.r - 1.0f;
	normalSample.y = -2.0f * normalSample.g + 1.0f;
	normalSample.z = normalSample.b;
	normal = mul(normalSample, TBN);

	return normal;
}

float getVisibility(PS_Input input, int cascade) {
	float visibility = 1.0f;
	const float bias = 0.005f;
	const int distCamToPixel = floor(length(camPosition - input.worldPosition));

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

	// get flags
	GlobalFlags globalFlags = getGlobalFlags();
	InstanceFlags instanceFlags = getInstanceFlags(input.instanceFlags);

	// transform normals
	input.normal = calculateNormals(input);

	const float distCamToPixel = length(camPosition - input.worldPosition);

	// fog
	float fog = 1.0f;
	float fogDistNear = 256.0f;
	float fogDistFar = 384.0f;
	if (instanceFlags.useFog) {
		if (distCamToPixel > fogDistNear) {
			fog = 1.0f - saturate((distCamToPixel - fogDistNear) / (fogDistFar - fogDistNear));
		}
	}
	// fog

	// shadowing
	float visibility = 1.0f;
	if (instanceFlags.useShadowing) {
		float distThresholdMax = 32.0f + (2.0f * pow(3, 3)) + (4.0f * pow(3, 2)) + (16.0f * 3);

		if (distCamToPixel >= distThresholdMax && distCamToPixel < fogDistFar) {
			visibility = getVisibility(input, 3);
		}
		else {
			int cascadeOffset = 0;
			for (int cascade = 0; cascade < 4; cascade++) {
				float distThreshold = 32.0f + (2.0f * pow(cascade, 3)) + (4.0f * pow(cascade, 2)) + (16.0f * cascade);
				if (distCamToPixel < distThreshold) {
					visibility = getVisibility(input, min(cascade + cascadeOffset, 3));
					break;
				}
			}
		}
	}
	// shadowing

	// lighting
	float3 lightColor  = float3(0, 0, 0);
	float3 fogColor    = float3(0, 0, 0);
	float3 outputColor = float3(0, 0, 0);

	if (instanceFlags.useLighting) {
		lightColor = calcLighting(tex_materials, tex_skybox, tex_brdf, textureSampler, input.texcoord, input.normal, input.worldPosition, materialFlags);
		outputColor = (visibility * lightColor);// +(1.0f - visibility) * lightColor;
		// outputColor = (visibility * direcLightColor) + ((1.0f - visibility) * pointLightColor) + (visibility * pointLightColor);
	} else {
		outputColor = visibility * pow(tex_materials.Sample(textureSampler, float3(input.texcoord, TEX_ALBEDO)).rgb, 2.2333f);
	}

	// is selected
	if (instanceFlags.isSelected) {
		outputColor += float3(0.15f, 0.15f, 0);
	}

	// fog
	if (instanceFlags.useFog) {
		fogColor = float3(0.5f, 0.5f, 0.5f);
		outputColor = lerp(fogColor, outputColor, fog);
	}

	return float4(outputColor, 1.0f);
}
