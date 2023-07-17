//////////////////////
// Constant Values  //
//////////////////////

static const float PI = 3.14159265359f;

static const float TEX_NORM_DISP = 0;
static const float TEX_ALBEDO    = 1;
static const float TEX_METALLIC  = 2;
static const float TEX_ROUGHNESS = 3;
static const float TEX_AMB_OCC   = 4;

static const int BONE_ID_MAX =   4;
static const int BONE_MAX    = 100;

//////////////////////
// Constant Buffers //
//////////////////////

cbuffer entity: register(b0) {
	row_major float4x4 world;
	row_major float4x4 view;
	row_major float4x4 proj;

	float4x4 boneSpace[BONE_MAX];

	float3 camPosition;
	float1 pad0;
	float3 camDirection;
	float1 pad1;

	float1 screenWidth;
	float1 screenHeight;
	float2 pad2;

	int1 materialFlags;
	int1 globalFlags;
	float2 pad3;
};

struct ShadowCascade {
	row_major float4x4 shadowView;
	row_major float4x4 shadowProj;
	float shadowMapResolution;
	float shadowPcfSampleCount;
};

cbuffer shadow: register(b1) {
	ShadowCascade cascades[4];
};

struct Material {
	bool useNormal;
	bool useHeight;
	bool useAlbedo;
	bool useMetallic;
	bool useRoughness;
	bool useAmbOcc;
};

Material getMaterial() {
	Material mat = (Material)0;
	mat.useNormal    = ((materialFlags & (1 << 0)) != 0);
	mat.useHeight    = ((materialFlags & (1 << 1)) != 0);
	mat.useAlbedo    = ((materialFlags & (1 << 2)) != 0);
	mat.useMetallic  = ((materialFlags & (1 << 3)) != 0);
	mat.useRoughness = ((materialFlags & (1 << 4)) != 0);
	mat.useAmbOcc    = ((materialFlags & (1 << 5)) != 0);
	return mat;
}

//////////////////////
// Input Structures //
//////////////////////

struct VS_Input {
	float3 position: POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	uint4  boneIds: BONE_ID;
	float4 weights: BONE_WEIGHT;

	int1   instanceId:          INSTANCE_ID;
	float3 instanceScale:       INSTANCE_SCALE;
	float3 instanceRotation:    INSTANCE_ROTATION;
	float3 instanceTranslation: INSTANCE_TRANSLATION;
	int1   instanceFlags:       INSTANCE_FLAGS;
};

struct HS_Input {
	float3 position:   POSITION;
	float2 texcoord:   TEXCOORD;
	float3 normal:     NORMAL;
	float3 tangent:    TANGENT;

	int1 instanceFlags: INSTANCE_FLAGS;

	float1 tessFactor: TESS;

	row_major float4x4 viewProj: SCREEN_SPACE;

	float4 shadowPositions[4]: SHADOW_POSITIONS;
};

struct DS_Input {
	float3 position: POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	int1 instanceFlags: INSTANCE_FLAGS;

	row_major float4x4 viewProj: SCREEN_SPACE;

	float4 shadowPositions[4]: SHADOW_POSITIONS;
};

struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
	float3 normal:   NORMAL;
	float3 tangent:  TANGENT;

	uint4  boneIds: BONE_ID;
	float4 weights: BONE_WEIGHT;

	int1 instanceFlags: INSTANCE_FLAGS;

	float3 worldPosition: WORLD_POSITION;
	float4 shadowPositions[4]: SHADOW_POSITIONS;
};

//////////////////////
//      Flags       //
//////////////////////

struct GlobalFlags {
	bool useFullscreen;
};

GlobalFlags getGlobalFlags() {
	GlobalFlags flags = (GlobalFlags)0;
	flags.useFullscreen = ((globalFlags & (1 << 0)) != 0);
	return flags;
}

struct InstanceFlags {
	bool useLighting;
	bool useShadowing;
	bool useFog;
	bool isSelected;
};

InstanceFlags getInstanceFlags(int instanceFlags) {
	InstanceFlags flags = (InstanceFlags)0;
	flags.useLighting  = ((instanceFlags & (1 << 0)) != 0);
	flags.useShadowing = ((instanceFlags & (1 << 1)) != 0);
	flags.useFog       = ((instanceFlags & (1 << 2)) != 0);

	flags.isSelected   = ((instanceFlags & (1 << 3)) != 0);
	return flags;
}

//////////////////////
//  Helper Methods  //
//////////////////////

float rand(float2 xy) {
	float2 noise = (frac(sin(dot(xy, float2(12.9898f, 78.233f) * 2.0f)) * 43758.5453f));
	return abs(noise.x + noise.y) * 0.5f;
}

float4x4 getWorldMatrix(float3 scale, float3 rotation, float3 translation) {
	float4x4 scaleMatrix = float4x4(
		float4(scale.x, 0.0, 0.0, 0.0),
		float4(0.0, scale.y, 0.0, 0.0),
		float4(0.0, 0.0, scale.z, 0.0),
		float4(0.0, 0.0, 0.0, 1.0)
		);

	float4x4 rotationMatrix = float4x4(
		float4(cos(rotation.y) * cos(rotation.z), cos(rotation.y) * sin(rotation.z), -sin(rotation.y), 0.0),
		float4(sin(rotation.x) * sin(rotation.y) * cos(rotation.z) - cos(rotation.x) * sin(rotation.z),
			sin(rotation.x) * sin(rotation.y) * sin(rotation.z) + cos(rotation.x) * cos(rotation.z),
			sin(rotation.x) * cos(rotation.y), 0.0),
		float4(cos(rotation.x) * sin(rotation.y) * cos(rotation.z) + sin(rotation.x) * sin(rotation.z),
			cos(rotation.x) * sin(rotation.y) * sin(rotation.z) - sin(rotation.x) * cos(rotation.z),
			cos(rotation.x) * cos(rotation.y), 0.0),
		float4(0.0, 0.0, 0.0, 1.0)
		);

	float4x4 translationMatrix = float4x4(
		float4(1.0, 0.0, 0.0, 0.0),
		float4(0.0, 1.0, 0.0, 0.0),
		float4(0.0, 0.0, 1.0, 0.0),
		float4(translation.x, translation.y, translation.z, 1.0)
		);

	return mul(mul(scaleMatrix, rotationMatrix), translationMatrix);
}
