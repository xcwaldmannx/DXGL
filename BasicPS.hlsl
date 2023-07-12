sampler textureSampler: register(s0);

Texture2D texAmbient: register(t0);
Texture2D texDiffuse: register(t1);
Texture2D texSpecular: register(t2);
Texture2D texNormal: register(t3);

cbuffer transform: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;
};

struct Material {
	float ambient;
	float diffuse;
	float specular;
	float shininess;
	bool useAmbient;
	bool useDiffuse;
	bool useSpecular;
	bool useNormal;
};

cbuffer materialBuffer: register(b2) {
	Material material;
}

struct VS_Output {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD0;
	float3 normal: NORMAL0;
	float3 tangent: NORMAL1;
	float3 bitangent: NORMAL2;

	float3 pixelPos: POSITION1;
};

struct DirecLight {
	float4 position;
	float4 direction;
	float4 color;
	float ambient;
	float diffuse;
	float specular;
	float shininess;
};

float3 calcNormalsFromMap(VS_Output input) {

	const float3x3 TBN = float3x3(
		normalize(input.tangent),
		normalize(input.bitangent),
		normalize(input.normal)
		);

	float3 normalSample = texNormal.Sample(textureSampler, input.texcoord).xyz;
	float3 newNormal = float3(0, 0, 0);
	newNormal.x = normalSample.x * 2.0f - 1.0f;
	newNormal.y = -normalSample.y * 2.0f + 1.0f;
	newNormal.z = normalSample.z * 2.0f - 1.0f;

	newNormal = mul(newNormal, TBN);

	return newNormal;
}

float4 main(VS_Output input) : SV_TARGET{

	DirecLight light;
	light.position = float4(0, 10, 0, 1);
	light.direction = float4(0, -1, 1, 1);
	light.color = float4(1, 1, 1, 1);
	light.ambient = 0.25f;
	light.diffuse = 1.0f;
	light.specular = 0.0f;
	light.shininess = 0.0f;

	if (material.useNormal) {
		input.normal = calcNormalsFromMap(input);
	}

	// normal of the pixel
	float3 N = normalize(input.normal);

	// the direction of the light
	float3 L = normalize(-light.direction.xyz);

	// pixel to viewing camera
	float3 cameraPosition = float3(0, 0, 0);
	float3 V = normalize(cameraPosition - input.pixelPos.xyz);

	// half-way vector between view and light
	float3 H = normalize(L + V);

	// ambient lighting
	float3 ambientFinal = float3(light.ambient, light.ambient, light.ambient);
	if (material.useAmbient) {
		ambientFinal = min(1.0f, ambientFinal + material.ambient * texAmbient.Sample(textureSampler, input.texcoord));
	}

	// diffuse lighting
	float3 diffuseFinal = max(0.0f, dot(L, N)) * normalize(light.color) * light.diffuse;
	if (material.useDiffuse) {
		float3 color = texDiffuse.Sample(textureSampler, input.texcoord);
		diffuseFinal *= material.diffuse * color;
		ambientFinal *= color;
	}

	// specular lighting
	float3 specularFinal = pow(dot(N, H), max(1.0f, light.shininess)) * light.specular;
	if (material.useSpecular) {
		float texSpecularAmount = texSpecular.Sample(textureSampler, input.texcoord).x * material.specular;
		specularFinal = pow(dot(N, H), max(1.0f, material.shininess)) * texSpecularAmount;
	}

	// output color of pixel
	float3 outColor = (ambientFinal + diffuseFinal);
	if (length(specularFinal) > 0.0f) {
		outColor += specularFinal;
	}

	return float4(outColor, 1.0f);

}