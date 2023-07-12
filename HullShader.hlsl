#include "SharedShader.hlsli"

#define NUM_CONTROL_POINTS 3

struct HS_CONSTANT_DATA_OUTPUT {
	float EdgeTessFactor[3]: SV_TessFactor;
	float InsideTessFactor:  SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(InputPatch<HS_Input,
	NUM_CONTROL_POINTS> input, uint PatchID : SV_PrimitiveID) {
	HS_CONSTANT_DATA_OUTPUT output = (HS_CONSTANT_DATA_OUTPUT)0;

	// Calculate the face normal
	float3 faceNormal = normalize(cross(input[1].position - input[0].position, input[2].position - input[0].position));

	// facing away
	float facingAway = dot(faceNormal, normalize(camPosition.xyz - input[0].position.xyz));

	// Check if the face is facing away from the camera
	if (facingAway <= -0.25f) {
		// Set edge tessellation factors to zero
		output.EdgeTessFactor[0] = 0;
		output.EdgeTessFactor[1] = 0;
		output.EdgeTessFactor[2] = 0;
		output.InsideTessFactor = 0;
		return output;
	}

	output.EdgeTessFactor[0] = 0.5f * (input[1].tessFactor + input[2].tessFactor);
	output.EdgeTessFactor[1] = 0.5f * (input[2].tessFactor + input[0].tessFactor);
	output.EdgeTessFactor[2] = 0.5f * (input[0].tessFactor + input[1].tessFactor);
	output.InsideTessFactor = output.EdgeTessFactor[0];

	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
DS_Input main(InputPatch<HS_Input, NUM_CONTROL_POINTS> input,
	uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID ) {
	DS_Input output = (DS_Input)0;

	output.position = input[i].position;
	output.texcoord = input[i].texcoord;
	output.normal   = input[i].normal;
	output.tangent  = input[i].tangent;

	output.instanceFlags = input[i].instanceFlags;

	output.viewProj = input[i].viewProj;

	output.shadowPositions = input[i].shadowPositions;

	return output;
}
