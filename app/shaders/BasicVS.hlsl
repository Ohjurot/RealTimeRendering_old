#include "Basic.hlsli"

[RootSignature(BasicRootSignature)]
Vertex BasicVS(in Vertex vtx)
{
    vtx.position.w = 1.0f;
    return vtx;
}
