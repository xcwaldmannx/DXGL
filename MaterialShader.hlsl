//SamplerState textureSampler: register(s0);
//
//Texture2DArray<float4> tex_materials: register(t0);
//TextureCube            tex_skybox:    register(t1);
//Texture2D              tex_brdf:      register(t2);
//
//// Material texture indices
//static const float TEX_NORM_DISP = 0;
//static const float TEX_ALBEDO    = 1;
//static const float TEX_METALLIC  = 2;
//static const float TEX_ROUGHNESS = 3;
//static const float TEX_AMB_OCC   = 4;
//
struct PSInput {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;
};
//
//float3 calculateNormals(float3 normal, float3 tangent, float2 texcoord, SamplerState textureSampler, Texture2D tex) {
//	normal = normalize(normal);
//	tangent = normalize(tangent);
//	tangent = normalize(tangent - dot(tangent, normal) * normal);
//	float3 bitangent = normalize(cross(normal, tangent));
//
//	const float3x3 TBN = float3x3(tangent, bitangent, normal);
//
//	Material mat = getMaterial(materialFlags);
//
//	float3 normalSample = mat.useNormal ? tex.Sample(textureSampler, input.texcoord).rgb : float3(0, 1, 0);
//	normalSample.x = 2.0f * normalSample.r - 1.0f;
//	normalSample.y = -2.0f * normalSample.g + 1.0f;
//	normalSample.z = normalSample.b;
//	normal = mul(normalSample, TBN);
//
//	return normal;
//}
//
float4 main(PSInput input) : SV_TARGET {
	//float3 normal = calculateNormals(input.normal, input.tangent, input.texcoord, textureSampler, tex_materials[TEX_NORM_DISP]);
	return float4(1, 1, 1, 1);
}