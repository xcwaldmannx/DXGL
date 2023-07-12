Texture2D<float4> InputTexture1 : register(t0);
Texture2D<float4> InputTexture2 : register(t1);
RWTexture2D<float4> OutputTexture1 : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) {
    OutputTexture1[DTid.xy] = InputTexture1[DTid.xy] + InputTexture2[DTid.xy];
}
