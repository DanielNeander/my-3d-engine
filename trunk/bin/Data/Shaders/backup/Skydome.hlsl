
#define STARFIELD
#define LDR

struct VS_IN {
	float4 iPosition	        : POSITION;
	float3 iNPosition           : TEXCOORD0;
	float2 iUV                  : TEXCOORD1;
	float  iOpacity             : TEXCOORD2;
};

struct VS_OUT {

	float4 oPosition		: SV_Position;
	float2 oUV              : TEXCOORD0;
	float3 oRayleighColor   : TEXCOORD1;
	float3 oMieColor        : TEXCOORD2;
	float3 oDirection       : TEXCOORD3;
	float  oOpacity         : TEXCOORD4;
	float  oHeight          : TEXCOORD5;
};

struct PS_IN {

	float4 oPosition		: SV_Position;
	float2 iUV              : TEXCOORD0;
	float3 iRayleighColor   : TEXCOORD1;
	float3 iMieColor        : TEXCOORD2;
	float3 iDirection       : TEXCOORD3;
	float  iOpacity         : TEXCOORD4;
	float  iHeight          : TEXCOORD5;
};

float scale(float cos, float uScaleDepth)
{
	float x = 1.0 - cos;
	return uScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

[Vertex shader]

void main(
    // IN
	float4 iPosition	        : POSITION,
	float3 iNPosition           : TEXCOORD0,
	float2 iUV                  : TEXCOORD1,
	float  iOpacity             : TEXCOORD2,
	// OUT
	out VS_OUT o,
	// UNIFORM
	uniform float4x4 uWorldViewProj,
	// Global information
	uniform float3 uLightDir,
	// Position information
	uniform float3 uCameraPos,
	uniform float3 uInvWaveLength,
	uniform float  uInnerRadius,
	// Scattering parameters
	uniform float  uKrESun, // Kr * ESun
	uniform float  uKmESun, // Km * ESun
	uniform float  uKr4PI,  // Kr * 4 * PI
	uniform float  uKm4PI,  // Km * 4 * PI
	// Atmosphere properties
	uniform float uScale,               // 1 / (outerRadius - innerRadius)
	uniform float uScaleDepth,          // Where the average atmosphere density is found
	uniform float uScaleOverScaleDepth, // Scale / ScaleDepth
	// Number of samples
	uniform int   uNumberOfSamples,
	uniform float uSamples)
{	
    // Clip space position
	o.oPosition   = mul(uWorldViewProj, iPosition);

	float3 v3Pos = iNPosition;
	v3Pos.y += uInnerRadius;

    float3 v3Ray = v3Pos - uCameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;
	
	// Calculate the ray's starting position, then calculate its scattering offset
	float3 v3Start = uCameraPos;
	float fHeight = uCameraPos.y;
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	// NOTE: fDepth is not pased as parameter(like a constant) to avoid the little precission issue (Apreciable)
	float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius - uCameraPos.y));
	float fStartOffset = fDepth * scale(fStartAngle, uScaleDepth);

    // Init loop variables
	float fSampleLength = fFar / uSamples;
	float fScaledLength = fSampleLength * uScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	float3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;

	// Loop the ray
	float3 color = float3(0,0,0);
	for (int i = 0; i < uNumberOfSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius-fHeight));
		
		float fLightAngle = dot(uLightDir, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		
		float fScatter = (fStartOffset + fDepth*(scale(fLightAngle, uScaleDepth) - scale(fCameraAngle, uScaleDepth)));
		float3 v3Attenuate = exp(-fScatter * (uInvWaveLength * uKr4PI + uKm4PI)); // <<< TODO
		
		// Accumulate color
		v3Attenuate *= (fDepth * fScaledLength);
		color += v3Attenuate;

		// Next sample point
		v3SamplePoint += v3SampleRay;
	}

    // Outputs
    o.oRayleighColor = color * (uInvWaveLength * uKrESun); // TODO <--- parameter
    o.oMieColor      = color * uKmESun; // TODO <--- *uInvMieWaveLength
    o.oDirection     = uCameraPos - v3Pos;
    o.oUV = iUV;
    o.oOpacity = iOpacity;
    o.oHeight = 1-iNPosition.y;
}

[Fragment shader]

#ifdef STARFIELD
   
Texture2D StarTex;
SamplerState uStarfield : register(s0);
#endif // STARFIELD


void main(
    // IN
	PS_IN input,
	// OUT 
	out float4 oColor		: SV_Target,
	// UNIFORM
#ifdef STARFIELD
	uniform float  uTime,
#endif // STARFIELD
	uniform float3 uLightDir,
    // Phase function
	uniform float  uG,
	uniform float  uG2,
	uniform float  uExposure
	)
{
    float cos = dot(uLightDir, input.iDirection) / length(input.iDirection);
	float cos2 = cos*cos;
	
	float rayleighPhase = 0.75 * (1.0 + 0.5*cos2);
	
	float miePhase = 1.5f * ((1.0f - uG2) / (2.0f + uG2)) * // <<< TODO
					 (1.0f + cos2) / pow(1.0f + uG2 - 2.0f * uG * cos, 1.5f);

#ifdef LDR
	oColor = float4((1 - exp(-uExposure * (rayleighPhase * input.iRayleighColor + miePhase * input.iMieColor))), input.iOpacity);
#else // HDR
    oColor = float4(uExposure * (rayleighPhase * input.iRayleighColor + miePhase * input.iMieColor), input.iOpacity);
#endif // LDR
	
	// For night rendering
	float nightmult = saturate(1 - max(oColor.x, max(oColor.y, oColor.z))*10);
	
#ifdef STARFIELD
	#ifdef LDR
		oColor.xyz += nightmult *(float3(0.05, 0.05, 0.1)*(2-0.75*saturate(-uLightDir.y))*pow(input.iHeight,3) + StarTex.Sample(uStarfield, input.iUV+uTime)*(0.35f + saturate(-uLightDir.y*0.45f))); 
	#else // HDR (Linear pipeline -> Gamma correction)
		oColor.xyz += nightmult *(pow(float3(0.05, 0.05, 0.1)*(2-0.75*saturate(-uLightDir.y))*pow(input.iHeight,3),2.2) + StarTex.Sample(uStarfield, input.iUV+uTime)*(0.35f + saturate(-uLightDir.y*0.45f))); 
	#endif // LDR
#else // NO STARFIELD
	#ifdef LDR
		oColor.xyz += nightmult *(float3(0.05, 0.05, 0.1)*(2-0.75*saturate(-uLightDir.y))*pow(input.iHeight,3)); 
	#else // HDR (Linear pipeline -> Gamma correction)
		oColor.xyz += nightmult * pow(float3(0.05, 0.05, 0.1)*(2-0.75*saturate(-uLightDir.y))*pow(input.iHeight,3), 2.2); 
	#endif // LDR
#endif // STARFIELD	
}
