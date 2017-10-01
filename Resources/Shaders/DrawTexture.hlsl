//------------------------------------------------------------------------------------------
//	Constant Buffer
//------------------------------------------------------------------------------------------
cbuffer CONSTANT_BUFFER :register(b0)
{
    matrix WVP              : packoffset(c0);
    float3 LightPos         : packoffset(c4);
    //float  Time           : packoffset(c4);
    //float2 Mouse          : packoffset(c5);
    //float2 Roughness      : packoffset(c6);
    //float2 MicroRoughness : packoffset(c7);
    //float Variation       : packoffset(c8);
    //float Density         : packoffset(c9);
};

//------------------------------------------------------------------------------------------
//	Structure
//------------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : TEXCOORD0;
    float2 Tex : TEXCOORD1;
};

//------------------------------------------------------------------------------------------
//  Vertex Shader
//------------------------------------------------------------------------------------------
VS_OUTPUT VS(float4 Pos : POSITION, float3 Normal : NORMAL, float2 Tex : TEXCOORD)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    output.Pos = mul(Pos, WVP);
    output.Normal = mul(Normal, (float3x3) WVP);
    output.Tex = Tex;

    return output;
}

//------------------------------------------------------------------------------------------
//  Pixel Shader
//------------------------------------------------------------------------------------------
float4 PS(VS_OUTPUT input) : SV_Target
{
    float4 pos = input.Pos;
    float3 N = normalize(input.Normal);
    float2 uv = input.Tex;
    float3 L = normalize(LightPos);

    float lambert = saturate(dot(N, L));
    
    //float3 col = float3(lambert, lambert, lambert);
    float3 col = float3(uv, 0);

    return float4(col, 1);
}