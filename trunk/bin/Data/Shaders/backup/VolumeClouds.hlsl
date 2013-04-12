[Vertex shader]

void main(
    // IN
	float4 iPosition	        : POSITION,
	float3 i3DCoord             : TEXCOORD0,
	float2 iNoiseUV             : TEXCOORD1,
	float  iOpacity             : TEXCOORD2,
	// OUT
	out float4 oPosition		: SV_Position,
	out float3 o3DCoord         : TEXCOORD0,
	out float2 oNoiseUV         : TEXCOORD1,
	out float  oOpacity         : TEXCOORD2,
	out float3 oEyePixel        : TEXCOORD3,
	out float  oDistance        : TEXCOORD4,
	// UNIFORM
	uniform float4x4 uWorldViewProj,
	uniform float3   uCameraPosition,
	uniform float    uRadius)
{
    // Clip space position
	oPosition   = mul(uWorldViewProj, iPosition);

	// 3D coords
	o3DCoord = i3DCoord;
    // Noise coords
    oNoiseUV = iNoiseUV;
    // Opacity
    oOpacity = iOpacity;
    // Eye to pixel vector
    oEyePixel = normalize(iPosition.xyz - uCameraPosition);
    
    // Distance in [0,1] range
    oDistance = length(float2(iPosition.x, iPosition.z)) / uRadius;
}

[Fragment shader]

Texture3D DensityTex0, DensityTex1;
Texture2D NoiseTex;
SamplerState uDensity0 : register(s0);	// 3d
SamplerState uDensity1 : register(s1);	// 3d
SamplerState uNoise    : register(s2);

// UNIFORM
	uniform float     uInterpolation;
	uniform float3    uSunDirection;
	uniform float3    uAmbientColor;
	uniform float3    uSunColor;
	uniform float4    uLightResponse;
	uniform float4    uAmbientFactors;

void main(
    // IN
	float4 oPosition		: SV_Position,
    float3 i3DCoord  : TEXCOORD0,
    float2 iNoiseUV  : TEXCOORD1,
    float  iOpacity  : TEXCOORD2,
    float3 iEyePixel : TEXCOORD3,
    float  iDistance : TEXCOORD4,
	// OUT 
	out float4 oColor		: SV_Target)	
{    
    // x - Sun light power
    // y - Sun beta multiplier
    // z - Ambient color multiplier
    // w - Distance attenuation
	// uLightResponse = float4(0.25,0.2,1,0.1);
	
	// Ambient light factors
	// x - constant, y - linear, z - cuadratic, w - cubic
	// float4 uAmbientFactors = float4(0.4,1,1,1);

	float3 Noise = NoiseTex.Sample(uNoise, iNoiseUV*5);
	float3 Final3DCoord = i3DCoord+0.002575*(Noise-0.5f)*2;
	Final3DCoord.z = saturate(Final3DCoord.z);
	
	float3 Density0 = DensityTex0.Sample(uDensity0, Final3DCoord);
	float3 Density1 = DensityTex1.Sample(uDensity1, Final3DCoord);
	float3 Density  = Density0*(1-uInterpolation) + Density1*uInterpolation;
	
	float3 finalcolor = float3(0,0,0);
	float  Opacity    = 0;
	
	if (Density.x > 0)
	{
	    float cos0 = saturate(dot(uSunDirection,iEyePixel));
	    float c2=cos0*cos0;
	
		float Beta = c2*uLightResponse.y*(0.5f+2.5f*saturate(1-2*uSunDirection.y)*Density.y);

		float sunaccumulation = max(0.2, saturate(Beta+Density.y*uLightResponse.x+pow(iDistance,1.5)*uLightResponse.w));
		float ambientaccumulation = 
			  saturate(uAmbientFactors.x + uAmbientFactors.y*i3DCoord.z + uAmbientFactors.z*pow(i3DCoord.z,2) + uAmbientFactors.w*pow(i3DCoord.z,3))*uLightResponse.z;
	    
		finalcolor = uAmbientColor*ambientaccumulation + uSunColor*sunaccumulation;
		Opacity = (1 - exp(-Density.x*(7.5-6.5*i3DCoord.z)))*iOpacity;
	}
	
    oColor = float4(finalcolor, Opacity);

 //oColor.xyz*=0.0001;
 // oColor.a = saturate(oColor.a+1)*iOpacity;
 // oColor.xyz+=Noise;
//oColor.rgb*=0.0001;oColor.r = dot(uLightDirection,iEyePixel);
}