struct VS_Input {
	float4 position: POSITION0;
	float2 texcoord: TEXCOORD0;
	float3 normal: NORMAL;
	float3 tangent: TANGENT;
};

struct VS_Output {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD0;
	float3 normal: NORMAL;
	float3 tangent: TANGENT;

	float3 pixelPos: POSITION1;
	float4 shadowPos: POSITION2;
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

struct PointLight {
	float4 position;
	float4 color;
	float ambient;
	float diffuse;
	float specular;
	float shininess;
	float attenuation0;
	float attenuation1;
	float attenuation2;
	float pad;
};

struct SpotLight {
	float4 position;
	float4 direction;
	float4 color;
	float innerAngleDeg;
	float outerAngleDeg;
	float ambient;
	float diffuse;
	float specular;
	float shininess;
	float attenuation0;
	float attenuation1;
	float attenuation2;
	float3 pad;
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
