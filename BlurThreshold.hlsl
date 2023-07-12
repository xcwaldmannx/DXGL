Texture2D<float4> InputTexture : register(t0);
RWTexture2D<float4> OutputTexture : register(u0);

cbuffer Constants : register(b0) {
    uint2 ScreenSize;   // The size of the screen or texture
}

SamplerState sam;

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {

    float2 coord = (float2) DTid.xy / (float2) ScreenSize;
    float4 color = InputTexture.SampleLevel(sam, coord, 0);

    float threshold = 0.5f;

    float brightness = max(max(color.r, color.g), color.b) * 0.25f;

    if (color.r < threshold && color.g < threshold && color.b < threshold) {
        OutputTexture[DTid.xy] = float4(0, 0, 0, 0);
    }  else {
        OutputTexture[DTid.xy] = color + float4(brightness, brightness, brightness, 1);
    }
}
