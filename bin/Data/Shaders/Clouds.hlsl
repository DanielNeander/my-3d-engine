
[Vertex shader]

void main(
    // IN
	float4 iPosition	        : POSITION,
	float3 iNPosition           : TEXCOORD0,
	// OUT
	out float4 oPosition		: SV_Position,
	out float3 oPosition_       : TEXCOORD0,
	// 
	uniform float4x4 uWorldViewProj)
{
    // Clip space position
	oPosition   = mul(uWorldViewProj, iPosition);

    oPosition_  = iNPosition;
}

[Fragment shader]

Texture2D CloudTex;
Texture2D CloudNormalTex;
Texture2D CloudTileTex;
SamplerState uClouds : register(s0);
SamplerState uCloudsNormal : register(s1);
SamplerState uCloudsTile : register(s2);


// 
float     uExposure;
// Sun information
float3    uSunColor;
// Main cloud layer parameters
 float     uHeight;
 float     uTime;
 float     uScale;
 float2    uWindDirection;
	// Advanced cloud layer parameters
	 float     uCloudLayerHeightVolume; // 0.25
	 float     uCloudLayerVolumetricDisplacement; // 0.01
	 float3    uAmbientLuminosity; // 0.55 0.55 0.55
	 float     uDetailAttenuation; // 0.45
	 float     uDistanceAttenuation; // 0.05

void main(
    // IN
	float4 oPosition		: SV_Position,
    float3 iPosition       : TEXCOORD0,
	// OUT 
	out float4 oColor		: SV_Target	)
{
    // Get the cloud pixel lenght on the projected plane
    float vh = uHeight / iPosition.y;
    // Get the 3D position of the cloud pixel
    float3 CloudPosition = iPosition * vh;
    
    // Get texture coords
    float2 TexCoord = CloudPosition.xz*uScale;
    float Density   = CloudTex.Sample(uClouds, TexCoord+uTime*uWindDirection*0.25f).r;
    float3 Normal    = -(2* CloudNormalTex.Sample(uCloudsNormal, TexCoord+uTime*uWindDirection*0.25f)-1);
    Normal.zy = Normal.yz;
 
    ///------------ Volumetric effect:
    float CloudLayerHeightVolume = uCloudLayerHeightVolume*iPosition.y;
    float CloudLayerVolumetricDisplacement = uCloudLayerVolumetricDisplacement*iPosition.y;
    float3 iNewPosition = normalize(iPosition + CloudLayerVolumetricDisplacement*float3(Normal.x,0,Normal.z));
    vh = (uHeight+uHeight*(1-Density)*CloudLayerHeightVolume) / iNewPosition.y;
    CloudPosition = iNewPosition * vh;
    TexCoord = CloudPosition.xz*uScale;                               // Little offset
    Density    = CloudTex.Sample(uClouds, TexCoord+uTime*uWindDirection*0.25f + float2(0.2,0.6)).r;
    ///------------
    
    float  CloudTile     = CloudTileTex.Sample(uCloudsTile, TexCoord-uTime*uWindDirection*0.25).r;

    float3 PixelColor = uAmbientLuminosity + uSunColor*Density;
    
    // AMBIENT addition
    // PixelColor += uAmbientLuminosity;
    
    // SUN addition 
    // PixelColor  += uSunColor*saturate(dot(-normalize(Normal), normalize(uSunPosition)));
    
    // FINAL colour
    float Alpha = Density * saturate(10*saturate(-uDistanceAttenuation+iPosition.y));
    
    oColor = float4(PixelColor*(1-Density*0.35), Alpha*saturate(1-CloudTile*uDetailAttenuation));
    
#ifdef LDR
    oColor.xyz = float3(1 - exp(-uExposure * oColor.xyz));
#else // HDR
    oColor.xyz *= pow(uExposure, 0.5);
#endif
}