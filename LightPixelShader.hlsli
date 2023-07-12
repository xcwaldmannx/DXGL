struct TEXTURE_DATA {
	bool hasAmbientMap;
	bool hasDiffuseMap;
	bool hasSpecularMap;
	bool hasNormalMap;
	Texture2D ambient;
	Texture2D diffuse;
	Texture2D specular;
	Texture2D normal;
};

struct DIREC_LIGHT_ELEMENT {
	float4 direction;
	float4 color;
	float4 surface;
};

struct POINT_LIGHT_ELEMENT {
	float4 position;
	float4 color;
	float4 surface;
	float4 attenuation;
};

struct SPOT_LIGHT_ELEMENT {
	float4 position;
	float4 direction;
	float4 angles;
	float4 color;
	float4 surface;
	float4 attenuation;
};

float dist(float3 v1, float3 v2) {
	return sqrt((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y) + (v2.z - v1.z) * (v2.z - v1.z));
}

float3 calcDirecLightColor(DIREC_LIGHT_ELEMENT light, float3 pixelNormal, float3 pixelPosition, float3 cameraPosition, TEXTURE_DATA textures, float2 textureCoords, sampler textureSampler) {
	// normal of the pixel
	float3 N = normalize(pixelNormal);

	// the direction of the light
	float3 L = normalize(-light.direction.xyz);

	// pixel to camera view
	float3 V = normalize(cameraPosition.xyz - pixelPosition);

	// half-way vector between view and light
	float3 H = normalize(L + V);

	// calculate ambient color
	float3 ambientColor = float3(light.surface.x, light.surface.x, light.surface.x);
	if (textures.hasAmbientMap) {
		//ambientColor *= textures.ambient.Sample(textureSampler, textureCoords);
	}

	// calculate diffuse color
	float3 diffuseColor = max(0.0f, dot(L, N)) * light.color.rgb * light.surface.y;
	if (textures.hasDiffuseMap) {
		float3 color = textures.diffuse.Sample(textureSampler, textureCoords);
		diffuseColor *= light.surface.y * color;
		ambientColor *= color;
	}

	// calulate specular color
	float3 specularColor = pow(dot(N, H), max(1.0f, light.surface.w)) * light.surface.z;
	if (textures.hasSpecularMap) {
		float specularAmount = textures.specular.Sample(textureSampler, textureCoords).x;
		specularColor = pow(dot(N, H), max(1.0f, light.surface.w)) * light.surface.z * specularAmount;
	}

	// combine colors
	float3 outColor = (ambientColor + diffuseColor);
	if (length(specularColor) > 0.0f) {
		outColor += specularColor;
	}

	return outColor;
}

float3 calcPointLightColor(POINT_LIGHT_ELEMENT light, float3 pixelNormal, float3 pixelPosition, float3 cameraPosition, TEXTURE_DATA textures, float2 textureCoords, sampler textureSampler) {

	// normal of the pixel
	float3 N = normalize(pixelNormal);

	// the direction of the light
	float3 L = normalize(light.position.xyz - pixelPosition);

	// pixel to camera view
	float3 V = normalize(cameraPosition.xyz - pixelPosition);

	// half-way vector between view and light
	float3 H = normalize(L + V);

	// calculate ambient color
	float3 ambientColor = float3(light.surface.x, light.surface.x, light.surface.x);
	if (textures.hasAmbientMap) {
		//ambientColor *= textures.ambient.Sample(textureSampler, textureCoords);
	}

	// calculate diffuse color
	float3 diffuseColor = max(0.0f, dot(L, N)) * light.color.rgb * light.surface.y;
	if (textures.hasDiffuseMap) {
		float3 color = textures.diffuse.Sample(textureSampler, textureCoords);
		diffuseColor *= color;
		ambientColor *= color;
	}

	// calulate specular color
	float3 specularColor = pow(dot(N, H), max(1.0f, light.surface.w)) * light.color.rgb * light.surface.z;
	if (textures.hasSpecularMap) {
		float specularAmount = textures.specular.Sample(textureSampler, textureCoords).x;
		specularColor = pow(dot(N, H), max(1.0f, light.surface.w)) * light.color.rgb * specularAmount;
	}

	// combine colors
	float3 combinedColor = (ambientColor + diffuseColor);
	if (length(specularColor) > 0.0f) {
		combinedColor += specularColor;
	}

	// calculate attenuation
	float distLightToPixel = dist(light.position.xyz, pixelPosition);
	float attenuation = 1.0f / (light.attenuation.x + light.attenuation.y * distLightToPixel + light.attenuation.z * (distLightToPixel * distLightToPixel));
	float3 outColor = combinedColor * attenuation;

	return outColor;
}

float3 calcSpotLightColor(SPOT_LIGHT_ELEMENT light, float3 pixelNormal, float3 pixelPosition, float3 cameraPosition, TEXTURE_DATA textures, float2 textureCoords, sampler textureSampler) {

	// normal of the pixel
	float3 N = normalize(pixelNormal);

	// the direction of the light
	float3 L = normalize(light.position.xyz - pixelPosition);

	// pixel to camera view
	float3 V = normalize(cameraPosition.xyz - pixelPosition);

	// half-way vector between view and light
	float3 H = normalize(L + V);

	// spot light stuff
	float innerAngle = cos((light.angles.x * 3.1415926f) / 180.0f);
	float outerAngle = cos((light.angles.y * 3.1415926f) / 180.0f);
	float theta = dot(L, normalize(-light.direction.xyz));
	float epsilon = innerAngle - outerAngle;
	float intensity = clamp((theta - outerAngle) / epsilon, 0.0, 1.0);

	// calculate ambient color
	float3 ambientColor = float3(light.surface.x, light.surface.x, light.surface.x);
	if (textures.hasAmbientMap) {
		//ambientColor *= textures.ambient.Sample(textureSampler, textureCoords);
	}


	// calculate diffuse color
	float3 diffuseColor = max(0.0f, dot(L, N)) * light.color.rgb * light.surface.y;
	if (textures.hasDiffuseMap) {
		float3 color = textures.diffuse.Sample(textureSampler, textureCoords);
		diffuseColor *= color;
		ambientColor *= color;
	}
	diffuseColor *= intensity;

	// calulate specular color
	float3 specularColor = pow(dot(N, H), max(1.0f, light.surface.w)) * light.surface.z;
	if (textures.hasSpecularMap) {
		float specularAmount = textures.specular.Sample(textureSampler, textureCoords).x;
		specularColor = pow(dot(N, H), max(1.0f, light.surface.w)) * specularAmount;
	}
	specularColor *= intensity;

	// combine colors
	float3 combinedColor = (ambientColor + diffuseColor);
	if (length(specularColor) > 0.0f) {
		combinedColor += specularColor;
	}

	// calculate attenuation
	float distLightToPixel = dist(light.position.xyz, pixelPosition);
	float attenuation = 1.0f / (light.attenuation.x + light.attenuation.y * distLightToPixel + light.attenuation.z * (distLightToPixel * distLightToPixel));
	float3 outColor = combinedColor * attenuation;

	return outColor;
}