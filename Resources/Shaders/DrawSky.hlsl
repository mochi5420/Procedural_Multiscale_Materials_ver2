//------------------------------------------------------------------------------------------
//	Constant Buffer
//------------------------------------------------------------------------------------------
cbuffer CONSTANT_BUFFER : register(b0)
{
    matrix WVP : packoffset(c0);
    matrix W : packoffset(c4);
    float3 CamPos : packoffset(c8);
    float3 LightPos : packoffset(c9);
};

SamplerState ObjSamplerState;
TextureCube SkyMap;

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 texCoord : TEXCOORD;
};

VS_OUTPUT VS(float3 Pos : POSITION, float3 Normal : NORMAL, float2 Tex : TEXCOORD)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    output.Pos = mul(float4(Pos, 1.0f), WVP);

    output.texCoord = Pos;

    return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
    return SkyMap.Sample(ObjSamplerState, input.texCoord);
}
