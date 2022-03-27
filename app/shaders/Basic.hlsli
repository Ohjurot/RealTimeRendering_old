#define BasicRootSignature "" \
"RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
"DescriptorTable(" \
"   CBV(b0)" /* Matricies */ \
")"

// Matrix constant buffer
struct
{
    float4x4 Projection;
    float4x4 View;
    float4x4 Model;
} Matrix : register(b0);

// Vertex layout
struct Vertex
{
    float4 position : SV_POSITION;
};
