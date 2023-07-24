// Defaults for number of lights.
#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#include "LightingUtil.hlsl"
#include "Common.hlsl"

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
    float4 ShadowPosH : POSITION1;
};

// Normal Distribution function
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float pi = 3.1415926f;
    float alpha = roughness;
    float a2 = alpha * alpha;
    float NdotH = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH * (a2 - 1.0f) + 1.0f);
    denom = pi * denom * denom;

    return num / denom;
}

float GeometrySchilicGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0f);
    float k = (r * r) / 2.0f;

    float num = NdotV;
    float denom = NdotV * (1.0f - k) + k;

    return num / denom;
}

// Geometry Function G
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0f);
    float NdotL = max(dot(N, L), 0.0f);
    float ggx1 = GeometrySchilicGGX(NdotV, roughness);
    float ggx2 = GeometrySchilicGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel-Schlick
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
}

// PBR for Point Light
void ComputePointLight(float3 albedo, float roughness, float metallic,
    Light light, float3 pos, float3 N, float3 V, float3 F0,
    out float3 lo)
{
    float3 PosToLight = light.Position - pos;
    float distance = length(PosToLight);

    PosToLight /= distance;
    // half-vec
    float3 H = normalize(V + PosToLight);

    float attenuation = 1.0 / (distance * distance);
    float3 radiance = light.Strength * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, PosToLight, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= (1.0f - metallic);

    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, PosToLight), 0.0f);
    float3 specular = numerator / max(denominator, 0.001f);

    float pi = 3.1415926;
    float NdotPosToLight = max(dot(N, PosToLight), 0.0f);
    lo = (kD * albedo / pi + specular) * radiance * NdotPosToLight;
}

// PBR for Directional Light
void ComputeDirectionalLight(float3 albedo, float roughness, float metallic,
    Light light, float3 pos, float3 N, float3 V, float3 F0,
    out float3 lo)
{
    float3 PosToLight = -light.Direction;
    PosToLight = normalize(PosToLight);

    // half-vec
    float3 H = normalize(V + PosToLight);
    
    float3 radiance = light.Strength;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, PosToLight, roughness);
    float3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= (1.0f - metallic);

    float3 numerator = NDF * G * F;
    float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, PosToLight), 0.0f);
    float3 specular = numerator / max(denominator, 0.001f);

    float pi = 3.1415926;
    float NdotPosToLight = max(dot(N, PosToLight), 0.0f);
    lo = (kD * albedo / pi + specular) * radiance * NdotPosToLight;
}

// PBR for all light
float4 ComputeAllLights(float3 albedo, float roughness, float metallic,
    Light gLights[MaxLights], float3 pos, float3 N, float3 V, float3 F0)
{
    float3 result = 0.0f;

    int i = 0;

#if (NUM_DIR_LIGHTS > 0)
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        float3 lo = float3(0.0f, 0.0f, 0.0f);
        ComputeDirectionalLight(albedo, roughness, metallic,
            gLights[i], pos, N, V, F0, 
            lo);
        result += lo;
    }
#endif

#if (NUM_POINT_LIGHTS > 0)
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; ++i)
    {
        float3 lo = float3(0.0f, 0.0f, 0.0f);
        ComputePointLight(albedo, roughness, metallic,
            gLights[i], pos, N, V, F0, 
            lo);
        result += lo;
    }
#endif

#if (NUM_SPOT_LIGHTS > 0)
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        result += ComputeSpotLight(gLights[i], mat, pos, normal, toEye);
    }
#endif 

    return float4(result, 0.0f);
}


float CalcShadowFactor(float4 shadowPosH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float)width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }

    return percentLit / 9.0f;
}

float3 aces(float3 rgb)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((rgb * (rgb * a + b)) / (rgb * (c * rgb + d) + e));
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform to homogeneous clip space
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    vout.PosW = float3(posW.x, posW.y, posW.z);
	vout.PosH = mul(posW, gViewProj);
    vout.PosH.x = -vout.PosH.x;
    vout.NormalW = mul(vin.Normal, (float3x3)gWorld);

    float4 texC = mul(float4(vin.UV, 0.0f, 1.0f), gTexTransform);
    vout.TexC = mul(texC, gMatTransform).xy;

    vout.ShadowPosH = mul(posW, gShadowTransform);
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    // Normalize the World Normal
    pin.NormalW = normalize(pin.NormalW);

    // View Vector
    float3 toEyeW = normalize(gEyePosW - pin.PosW);

    // DiffuseAlbedo based on Base Color
    float4 diffuseAlbedo = gDiffuseMap.Sample( gsamAnisotropicWrap, pin.TexC) * gDiffuseAlbedo;

    // ********** PBRMaterials ***********
    float4 PBRInfo = gPDRInfo.Sample(gsamAnisotropicWrap, pin.TexC);

    float occlution = PBRInfo.x;
    float roughness = PBRInfo.y;
    float metal = PBRInfo.z;
    float alpha = PBRInfo.w;

    // calculate F0 by metallic
    float3 F0 = float3(0.04f, 0.04f, 0.04f);
    F0 = lerp(F0, float3(gDiffuseAlbedo.x, gDiffuseAlbedo.y, gDiffuseAlbedo.z), metal);
    float3 lo = float3(0.0f, 0.0f, 0.0f);
    /*ComputePointLight(gDiffuseAlbedo, roughness, metal,
        gLights[1], pin.PosW, pin.NormalW, toEyeW, F0,
        lo);

    float3 lo1 = float3(0.0f, 0.0f, 0.0f);
    ComputeDirectionalLight(gDiffuseAlbedo, roughness, metal,
        gLights[0], pin.PosW, pin.NormalW, toEyeW, F0,
        lo1);*/
    float4 PBRLight = float4(0.0f, 0.0f, 0.0f, 0.0f);
    PBRLight = ComputeAllLights(float3(gDiffuseAlbedo.x, gDiffuseAlbedo.y, gDiffuseAlbedo.z),
        roughness, metal,
        gLights, pin.PosW, pin.NormalW, toEyeW, F0);
    PBRLight = CalcShadowFactor(pin.ShadowPosH) * PBRLight;
    // ************** End ******************


    // Ambiend Light based on Base Color
    float4 ambient = gAmbientLight * gDiffuseAlbedo;

    // Parameter for AO
    float3 shadowFactor = 1.0f;

    const float shininess = 1.0f - gRoughness;
    Material mat = { diffuseAlbedo, gFresnelR0, shininess };

    // Compute Light
    /*float4 directLight = ComputeLighting(gLights, mat, pin.PosW,
        pin.NormalW, toEyeW, shadowFactor);

    float4 finalCol = directLight + ambient;*/
    float4 finalCol = (PBRLight + ambient) * diffuseAlbedo;

    float3 clorgb = finalCol.xyz;
    clorgb = aces(clorgb);
    finalCol = float4(clorgb, finalCol.w);

	return finalCol;
}