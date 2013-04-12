[Vertex shader]

void main(
    // IN
	float4 iPosition	        : POSITION,
	float2 iUV                  : TEXCOORD0,
	// OUT
	out float4 oPosition		: SV_Position,
	out float4 oUVYLength       : TEXCOORD0,
	// UNIFORM
	uniform float4x4 uWorldViewProj,
	uniform float4x4 uWorld,
	uniform float3   uSkydomeCenter)
{
    // Clip space position
	oPosition   = mul(uWorldViewProj, iPosition);
	// World position
	float3 ObjectSpacePosition = mul(uWorld, iPosition) - uSkydomeCenter;

    // UV
    oUVYLength.xy = iUV;
    // Y
    oUVYLength.z  = ObjectSpacePosition.y;
    // Length
    oUVYLength.w  = length(ObjectSpacePosition);
}

[Fragment shader]

Texture2D MoonTex;
Texture2D MoonHaloTex;
SamplerState uMoon : register(s0);
SamplerState uMoonHalo : register(s1);

void main(
    // IN	
	float4 oPosition		: SV_Position,
    float4 iUVYLength       : TEXCOORD0,
	// OUT
	out float4 oColor		: SV_Target,
	// UNIFORM
	uniform float3    uMoonPhase,
	uniform float3    uMoonHalo1,
	uniform float3    uMoonHalo2,
	uniform float     uMoonHaloFlip)
{
    // Output color
    oColor = MoonTex.Sample(uMoon, iUVYLength.xy);

	// Moon phase + halo
	float radius = abs(uMoonPhase.x);
	float2 center = float2(uMoonPhase.y, 0.5);
	float dist = length(iUVYLength.xy - center);
	float att = saturate((radius-dist+0.015)*40);

	if (uMoonHaloFlip > 0.5)
	{
		iUVYLength.x = 1-iUVYLength.x;
	}

	float2 haloUV = float2(iUVYLength.x/4, iUVYLength.y/2);
	float2 halo1UV = float2(uMoonHalo1.x + haloUV.x, uMoonHalo1.y + haloUV.y);
	float2 halo2UV = float2(uMoonHalo2.x + haloUV.x, uMoonHalo2.y + haloUV.y);

	float haloIntensity = MoonHaloTex.Sample(uMoonHalo, halo1UV).w*uMoonHalo1.z + MoonHaloTex.Sample(uMoonHalo, halo2UV).w*uMoonHalo2.z;

	haloIntensity = pow(haloIntensity, uMoonPhase.z);

	if (uMoonPhase.x > 0)
	{
		oColor.rgb *= 0.16 + (1-0.16)*(1-att);
		oColor.rgb += saturate(haloIntensity-oColor.r)*(1-(1-att)*oColor.a);
	}
	else
	{
		oColor.rgb *= 0.16 + (1-0.16)*att;
		oColor.rgb += saturate(haloIntensity-oColor.r)*(1-att*oColor.a);
	}

	oColor.rgb += (1-oColor.a)*1.4*(1-pow(oColor.a,2*haloIntensity)); // Anti-alias at moon edges hack
	oColor.rgb = saturate(oColor.rgb);

	oColor.a = max(oColor.a, haloIntensity);

	// Transparency at horizon
	oColor.w *= saturate((iUVYLength.z/iUVYLength.w)*10);
}