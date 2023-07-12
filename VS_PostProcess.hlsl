#include "SharedPostProcess.hlsli"

struct VS_Input {
	float4 position: POSITION;
	float2 texcoord: TEXCOORD;
};

struct PS_Input {
	float4 position: SV_POSITION;
	float2 texcoord: TEXCOORD;
};

PS_Input main(VS_Input input) {
	PS_Input output = (PS_Input)0;

	output.position = input.position;
	output.texcoord = input.texcoord;

	return output;
}