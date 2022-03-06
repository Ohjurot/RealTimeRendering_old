#define RS "\
RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)\
"

[RootSignature(RS)]
float4 VsMain(float4 pos : SV_POSITION) : SV_POSITION
{
    return float4(pos.xy, 0.0f, 1.0f);
}

[RootSignature(RS)]
float4 PsMain(float4 pos : SV_POSITION) : SV_TARGET
{
    return float4(0.3f, 0.525f, 0.651f, 1.0f);
}
