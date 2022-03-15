#include "Basic.hlsli"

[RootSignature(BasicRootSignature)]
float4 BasicPS(in Vertex vtx) : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
