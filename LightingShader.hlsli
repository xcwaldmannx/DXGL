#ifndef INCLUDE_SHARED
#include "SharedShader.hlsli"
#define INCLUDE_SHARED
#endif

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

float1 distributionGGX(float3 N, float3 H, float1 roughness) {
    float1 a = roughness * roughness;
    float1 a2 = a * a;
    float1 NdotH = max(dot(N, H), 0.0f);
    float1 NdotH2 = NdotH * NdotH;

    float1 nom = a2;
    float1 denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;

    return nom / denom;
}

float1 geometrySchlickGGX(float1 NdotV, float1 roughness) {
    float1 r = (roughness + 1.0f);
    float1 k = (r * r) / 8.0f;

    float1 nom = NdotV;
    float1 denom = NdotV * (1.0f - k) + k;

    return nom / denom;
}

float1 geometrySmith(float3 N, float3 V, float3 L, float1 roughness) {
    float1 NdotV = max(dot(N, V), 0.0f);
    float1 NdotL = max(dot(N, L), 0.0f);
    float1 ggx2 = geometrySchlickGGX(NdotV, roughness);
    float1 ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float1 cosTheta, float3 F0) {
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float3 fresnelSchlickRoughness(float1 cosTheta, float3 F0, float1 roughness) {
    return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float3 calcLighting(Texture2DArray materials, TextureCube skybox, Texture2D brdf, SamplerState textureSampler, PS_Input input) {
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);

    Material mat = getMaterial();

    float3 albedoSample = mat.useAlbedo ? pow(materials.Sample(textureSampler, float3(input.texcoord, TEX_ALBEDO)).rgb, float3(2.2333f, 2.2333f, 2.2333f)) : float3(0.25f, 0.25f, 0.25f);
    float1 metallicSample = mat.useMetallic ? materials.Sample(textureSampler, float3(input.texcoord, TEX_METALLIC)).r : 0;
    float1 roughnessSample = mat.useRoughness ? materials.Sample(textureSampler, float3(input.texcoord, TEX_ROUGHNESS)).r : 1;
    float1 amboccSample = mat.useAmbOcc ? materials.Sample(textureSampler, float3(input.texcoord, TEX_AMB_OCC)).r : 1;

    float3 N = input.normal;
    float3 V = normalize(camPosition - input.worldPosition);
    float3 R = reflect(-V, N);

    float3 irradianceSample = pow(skybox.SampleLevel(textureSampler, N, 8).rgb, 2.2333f);
    float2 envBRDF = brdf.Sample(textureSampler, float2(max(dot(N, V), 0.0f), roughnessSample)).rg;

    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, albedoSample, metallicSample);

    // reflectance equation
    float3 Lo = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < lightCount; i++) {

        // calculate per-light radiance
        float3 L = float3(0, 0, 0);
        float3 H = float3(0, 0, 0);
        float3 radiance = float3(0, 0, 0);

        if (length(lights[i].direction) == 0) { // point light

            L = normalize(lights[i].position - input.worldPosition);
            H = normalize(V + L);
            float1 distance = length(lights[i].position - input.worldPosition);
            float1 attenuation = 1.0f / (distance * distance);
            radiance = lights[i].color * attenuation;

        } else if (length(lights[i].angles) == 0) { // directional light
            
            L = normalize(-lights[i].direction);
            H = normalize(V + L);
            radiance = lights[i].color;

        } else { // spot light
        }

        // Cook-Torrance BRDF
        float1 NDF = distributionGGX(N, H, roughnessSample);
        float1 G = geometrySmith(N, V, L, roughnessSample);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

        float3 numerator = NDF * G * F;
        float1 denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001f; // + 0.0001 to prevent divide by zero
        float3 specular = (numerator / denominator) * (1.0f - roughnessSample);

        // kS is equal to Fresnel
        float3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0f - metallicSample;

        // scale light by NdotL
        float1 NdotL = max(dot(N, L), 0.0f);

        // add to outgoing radiance Lo
        Lo += (kD * albedoSample / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (we now use IBL as the ambient term)
    float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0f), F0, roughnessSample);

    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metallicSample;

    float3 diffuse = irradianceSample * albedoSample;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float1 MIN_REFLECT_LOD =  5.0f;
    const float1 MAX_REFLECT_LOD = 10.0f;
    float3 prefilteredColor = pow(skybox.SampleLevel(textureSampler, R, MIN_REFLECT_LOD + roughnessSample * (MAX_REFLECT_LOD - MIN_REFLECT_LOD)).rgb, 2.2333f);
    float3 specular = prefilteredColor * (F * envBRDF.x + envBRDF.y);

    float3 ambient = (kD * diffuse + (specular * (1.0f - roughnessSample))) * amboccSample;

    outputColor = ambient + Lo;

    return outputColor;
}
