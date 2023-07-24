Texture2D    gDiffuseMap : register(t0);

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
};

cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;

    float4 gAmbientLight;

};

cbuffer cbMaterial : register(b2)
{
    float4   gDiffuseAlbedo;
    float3   gFresnelR0;
    float    gRoughness;
    float4x4 gMatTransform;
};

struct VertexIn
{
    float3 PosL  : POSITION;
    float4 Tangent  : TANGENT;
    float3 Normal  : NORMAL;
    float2 UV  : TEXCOORD0;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
    float3 PosW  : POSITION;
    float2 TexC  : TEXCOORD0;
    float3 NormalW : NORMAL;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to homogeneous clip space
    float4 posW = mul(float4(5 * vin.PosL, 1.0f), gWorld);
    vout.PosW = float3(posW.r, posW.g, posW.b);
    vout.PosH = mul(posW, gViewProj);

    vout.NormalW = mul(vin.Normal, (float3x3)gWorld);

    float4 texC = mul(float4(vin.UV, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;
    vout.PosW.x = -vout.PosW.x;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{

    float4 diffuseAlbedo = gDiffuseMap.Sample(gsamAnisotropicWrap, pin.TexC);

    return diffuseAlbedo;
}