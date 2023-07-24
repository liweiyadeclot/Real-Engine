#include "LightingUtil.hlsl"

Texture2D<float> shadowMap : register(t0);

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

    Light gLights[MaxLights];
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
    float4 position  : SV_POSITION;
    float4 shadowTexCoord  : TEXCOORD0;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    float3 sceneMin = float3(-10.0f, -10.0f, -10.0f);
    float3 sceneMax = float3(10.0f, 10.0f, 10.0f);
    float3 sceneCenter = float3(0.0f, 0.0f, 0.0f);
    float3 targetPosition = sceneCenter;
    float3 upDirection = float3(0.0f, 1.0f, 0.0f);
    const int PI = 3.1415926f;
    // PointLight viewProjection matrix;
    float4x4 lightViewProjection = mul(
        mul(
            float4x4::LookAt(gLights[0].Position, targetPosition, upDirection),
            float4x4::PerspectiveForLH(PI / 2.0f, 1.0f, 0.1f, 100.0f)
        ),
        float4x4::Scale(1.0f, -1.0f, 1.0f)
    );

    vout.position = mul(float4(input.PosL, 1.0f), lightViewProjection);
    // TO-DO: Finish point light 
    vout.shadowTexCoord = vout.position;

}

float PS(VertexOut pin)
{
    float4 shadowCoord = pin.PosS / input.PosS.w;
    float depth = shadowCoord.z;
    depth = (depth + 1.0f) * 0.5f;

    shadowMap[uint2(pin.shadowTexCoord.xy)] = depth;

    return float4(0.0f, 0.0f, 0.0f, 0.0f)
}