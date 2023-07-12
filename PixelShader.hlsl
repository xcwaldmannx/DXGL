#include "ShaderStructs.hlsli"

#ifndef LIGHT_PIXEL_SHADER
#define LIGHT_PIXEL_SHADER
#include "LightPixelShader.hlsli"
#endif

sampler textureSampler: register(s0);
sampler shadowSampler: register(s1);

Texture2D texAmbient: register(t0);
Texture2D texDiffuse: register(t1);
Texture2D texSpecular: register(t2);
Texture2D texNormal: register(t3);
Texture2D depthBuffer: register(t5);
Texture2D bloomBuffer: register(t6);

cbuffer transformBuffer: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;
};

cbuffer LightBuffer: register(b1) {
	float4 cameraPosition;
	DIREC_LIGHT_ELEMENT direcLights[16];
	POINT_LIGHT_ELEMENT pointLights[16];
	SPOT_LIGHT_ELEMENT spotLights[16];
	int direcLightCount;
	int pointLightCount;
	int spotLightCount;
};

cbuffer materialBuffer: register(b2) {
	Material material;
}

struct Cascade {
	row_major float4x4 shadowView;
	row_major float4x4 shadowProj;
};

cbuffer shadow: register(b3) {
	Cascade cascade[4];
};

cbuffer bloom: register(b4) {
	bool enableBloom;
	int bloomDirection;
	int width;
	int height;
}

float3 calcNormalsFromMap(VS_Output input) {

	float3 normal = normalize(input.normal);
	float3 tangent = normalize(input.tangent);
	tangent = normalize(input.tangent - dot(input.tangent, input.normal) * input.normal);
	float3 bitangent = normalize(cross(normal, tangent));

	const float3x3 TBN = float3x3(tangent, bitangent, normal);

	// issue with sRGB color space

	float3 normalSample = texNormal.Sample(textureSampler, input.texcoord).rgb;
	normalSample.x =  2.0f * normalSample.r - 1.0f;
	normalSample.y = -2.0f * normalSample.g + 1.0f;
	normalSample.z = normalSample.b;
	normal = mul(normalSample, TBN);

	return normal;
}

float3 bloomPass(float3 color, float threshold, float4 position) {
	//color.r *= 0.2126f;
	//color.g *= 0.7152f;
	//color.b *= 0.0722f;
	if (color.r < threshold && color.g < threshold && color.b < threshold) {
		return float3(0, 0, 0);
	}
	
	return color;

}

float3 addBloom(float4 position) {
	float3 result = float3(0, 0, 0);

	float x = position.x;
	float y = position.y;

	float samples = 8.0f;

	/*
	if (bloomDirection == 0) {
		for (int i = 0; i < samples; i++) {
			float tx = (x + i) / width;
			float ty = y / height;
			result += bloomBuffer.Sample(textureSampler, float2(tx, ty)).rgb / (i + 1);
		}
	}
	else if (bloomDirection == 1) {
		for (int i = 0; i < samples; i++) {
			float tx = x / width;
			float ty = (y + i) / height;
			result += bloomBuffer.Sample(textureSampler, float2(tx, ty)).rgb / (i + 1);
		}
	}
	*/
	float tx = x / width;
	float ty = y / height;
	return bloomBuffer.Sample(textureSampler, float2(tx, ty)).rgb;
}

float4 main(VS_Output input) : SV_TARGET {

	float4 outColor = float4(0, 0, 0, 0);

	const TEXTURE_DATA textureData = {
		true,
		true,
		true,
		true,
		texAmbient,
		texDiffuse,
		texSpecular,
		texNormal,
	};

	//if (textureData.hasNormalMap) {
		input.normal = calcNormalsFromMap(input);
	//}

	float3 lightColor = float3(0.0f, 0.0f, 0.0f);

	// shadowing
	float2 poissonDisk[4] = {
		float2(0.0005f,  0.0005f),
		float2(-0.0005f,  0.0005f),
		float2(0.0005f, -0.0005f),
		float2(-0.0005f, -0.0005f)
	};

	float visibility = 1.0f;
	float bias = 0.0001f;
	float samples = 8.0f;

	input.shadowPos.z = input.shadowPos.z / input.shadowPos.w;
	for (int i = 0; i < samples; i++) {
		float2 shadowCoord = input.shadowPos.xy + poissonDisk[i % 4] * ((i + 1) / 8.0f);
		if (depthBuffer.Sample(shadowSampler, shadowCoord).r + bias <= input.shadowPos.z) {
			visibility -= 1.0f / samples;
		}
	}
	// shadowing

	for (int i = 0; i < direcLightCount; i++) {
		lightColor += visibility * calcDirecLightColor(direcLights[i], input.normal.xyz, input.pixelPos.xyz, cameraPosition.xyz, textureData, input.texcoord, textureSampler);
	}

	for (int i = 0; i < pointLightCount; i++) {
		lightColor += calcPointLightColor(pointLights[i], input.normal.xyz, input.pixelPos.xyz, cameraPosition.xyz, textureData, input.texcoord, textureSampler);
	}

	for (int i = 0; i < spotLightCount; i++) {
		//lightColor += calcSpotLightColor(spotLights[i], input.normal.xyz, input.pixelPos.xyz, cameraPosition.xyz, textureData, input.texcoord, textureSampler);
	}

	// bloom
	if (enableBloom) {
		lightColor = bloomPass(lightColor, 0.0f, input.position);
	} else {
		lightColor += addBloom(input.position);
	}

	// HDR tone mapping and gamma correction
	float exposure = 3.0f;
	float gamma = 1.0f;// 2.2f;
	float3 toneMap = float3(1.0f, 1.0f, 1.0f) - exp((-lightColor) * exposure);
	toneMap = pow(toneMap, float3(1.0f / gamma, 1.0f / gamma, 1.0f / gamma));
	lightColor = toneMap;

	return float4(lightColor, 1.0f);

}