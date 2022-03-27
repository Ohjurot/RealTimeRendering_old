#include "Basic.hlsli"

[RootSignature(BasicRootSignature)]
Vertex BasicVS(in Vertex vtx)
{
    float4x4 mvpMatrix = mul(Matrix.Model, Matrix.View);
    mvpMatrix = mul(mvpMatrix, Matrix.Projection);
    vtx.position = mul(vtx.position, mvpMatrix);
    return vtx;
}
