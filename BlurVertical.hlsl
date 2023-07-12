Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture1 : register(u0);

cbuffer Constants : register(b0) {
    uint2 ScreenSize;   // The size of the screen or texture
}

SamplerState sam;

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    float4 finalColor = float4(0, 0, 0, 0);

    int radius = 16;

    for (int i = 0; i < radius; i++) {
        float2 coord = ((float2) DTid.xy + float2(0, i)) / (float2) ScreenSize;
        float4 color = InputTexture.SampleLevel(sam, coord, 0);
        finalColor += color / pow(i + 1, 2.3f);
    }

    OutputTexture1[DTid.xy] = finalColor;
}
