// ------------------------------------------------------
//
//	ANDREW CONNER SKATZES (c) 2025
//
//	MIT License
// 
//	Description:
//		Fog shader (fragment/pixel DirectX version)
// ------------------------------------------------------

cbuffer UMO : register(b1)
{
    float4 uColor;
};

Texture2D uSample0 : register(t2);
Texture2D uSample1 : register(t3);
SamplerState uSampleState0 : register(s0);
SamplerState uSampleState1 : register(s1);

/* Metal Shader Input structure*/
struct MSInput
{
    float2 uTexCoord0 : TEXCOORD0;
    float2 uTexCoord1 : TEXCOORD1;
};

float4 main(MSInput input) : SV_Target
{
    float4 tex0 = uSample0.Sample(uSampleState0, input.uTexCoord0);
    float4 tex1 = uSample1.Sample(uSampleState1, input.uTexCoord1);
    return tex0 * tex1 * uColor;
}