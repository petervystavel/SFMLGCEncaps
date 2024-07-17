#include "Utils.hlsl"

Texture2D g_buffer_albedo : register(t0);
Texture2D g_buffer_worldPosition : register(t1);
Texture2D g_buffer_normal : register(t2);

SamplerState g_sampler : register(s0);

cbuffer cbPerCamera : register(b0)
{
    float4x4 gView;
    float4x4 gProj;
};

struct Light
{
    float3 cbPerLight_position;
    float3 cbPerLight_direction;
    float3 cbPerLight_color;
    float cbPerLight_spotAngle;
    int cbPerLight_lightType;
    float cbPerLight_lightIntensity;
};

cbuffer cbPerLights : register(b1)
{
    Light lights[10];
};

static float4 ambientLightColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
static float4 ambient = float4(0.2f, 0.2f, 0.2f, 1.0f);
static float4 diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);
static float4 specular = float4(0.8f, 0.8f, 0.8f, 1.0f);
static float shininess = 5.0f;

struct VSOutput
{
    float4 PosH : SV_POSITION;
    float2 UV : TEXCOORD;
};

VSOutput VS(float3 posL : POSITION, float2 uv : TEXCOORD)
{
    VSOutput output;
    output.PosH = float4(posL, 1.0f);
    output.UV = uv;
    return output;
}

float4 PS(VSOutput pin) : SV_Target
{
    // R�cup�rer les donn�es des G-Buffers
    float4 albedo = g_buffer_albedo.Sample(g_sampler, pin.UV);
    float4 worldPos = g_buffer_worldPosition.Sample(g_sampler, pin.UV);
    float4 remappedNormal = g_buffer_normal.Sample(g_sampler, pin.UV);
    
    if (albedo.r == 0.0f && albedo.g == 0.0f && albedo.b == 0.0f)
    {
        return float4(0.678431392f, 0.847058892f, 0.901960850f, 1.0f);
    }
        

    // Remap des normales de [0, 1] � [-1, 1]
    float3 normal = remappedNormal.xyz * 2.0 - 1.0;

    float3x3 invViewMatrix = (float3x3) transpose(gView); // Transpose of view matrix
    float3 cameraPosition = -mul(invViewMatrix, gView[3].xyz); // Camera position in world space

    float3 viewDirection = normalize(cameraPosition - worldPos.xyz); // View direction

    float4 finalColor = ComputeAmbient(ambientLightColor, ambient, albedo);

    float3 diffuseColor = float3(0.0f, 0.0f, 0.0f);
    float3 specularColor = float3(0.0f, 0.0f, 0.0f);

    for (int i = 0; i < 10; ++i)
    {
        Light currentLight = lights[i];

        if (currentLight.cbPerLight_lightType == 0)
        {
            // Directional light calculations
            float3 lightColorDirectional = currentLight.cbPerLight_color * currentLight.cbPerLight_lightIntensity;
            float3 lightDirectionDirectional = -normalize(currentLight.cbPerLight_direction); // Invert direction for directional light

            float diffuseIntensity = dot(normal, lightDirectionDirectional);
            diffuseIntensity = saturate(diffuseIntensity);

            diffuseColor += ComputeDiffuse(lightDirectionDirectional, normal, lightColorDirectional, diffuse) * diffuseIntensity;
            specularColor += ComputePhongSpecular(lightDirectionDirectional, normal, viewDirection, lightColorDirectional, specular, shininess);
        }
        else if (currentLight.cbPerLight_lightType == 1)
        {
            // Spot light calculations
            float3 lightColorSpot = currentLight.cbPerLight_color * currentLight.cbPerLight_lightIntensity;
            float3 lightPositionSpot = currentLight.cbPerLight_position;
            float3 lightDirectionSpot = normalize(currentLight.cbPerLight_direction);
            float spotAngle = radians(currentLight.cbPerLight_direction);

            float spotIntensity = ComputeSpotIntensity(lightPositionSpot, lightDirectionSpot, worldPos.xyz, spotAngle);

            diffuseColor += ComputeDiffuse(lightDirectionSpot, normal, lightColorSpot, diffuse) * spotIntensity;
            specularColor += ComputePhongSpecular(lightDirectionSpot, normal, viewDirection, lightColorSpot, specular, shininess) * spotIntensity;
        }
        else if (currentLight.cbPerLight_lightType == 2)
        {
            // Point light calculations
            float3 lightColorPoint = currentLight.cbPerLight_color;
            float3 lightPositionPoint = currentLight.cbPerLight_position;

            float pointLightIntensity = ComputePointLightIntensity(lightPositionPoint, worldPos.xyz, currentLight.cbPerLight_lightIntensity);

            float3 lightDirectionPoint = normalize(lightPositionPoint - worldPos.xyz);

            diffuseColor += ComputeDiffuse(lightDirectionPoint, normal, lightColorPoint, diffuse) * pointLightIntensity;
            specularColor += ComputePhongSpecular(lightDirectionPoint, normal, viewDirection, lightColorPoint, specular, shininess) * pointLightIntensity;
        }
    }

    finalColor.rgb += diffuseColor * albedo.rgb;
    finalColor.rgb += specularColor * albedo.rgb;
    finalColor.a = albedo.a;
   
    return finalColor;
}


