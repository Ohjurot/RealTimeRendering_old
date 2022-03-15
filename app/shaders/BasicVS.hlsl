#include "Basic.hlsli"

[RootSignature(BasicRootSignature)]
Vertex BasicVS(in Vertex vtx)
{
    return vtx;
}
