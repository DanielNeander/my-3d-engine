[Vertex shader]

void main(
    // IN
	float4 iPosition	        : POSITION,
	float3 iColor               : COLOR,
	float2 iUV                  : TEXCOORD0,
	// OUT
	out float4 oPosition		: POSITION,
	out float3 oUV              : TEXCOORD0,
	out float4 oData            : TEXCOORD1,
	// UNIFORM
	uniform float4x4 uWorldViewProj,
	uniform float3   uData)
{
    oPosition = mul(uWorldViewProj, iPosition);
    oUV.xy = iUV;
   
    // Alpha
    oUV.z = uData.x; 
   
    if (iColor.x > 0.5)
    {
		// Reverse y coord and mark
		oUV.y = 2+(1-oUV.y);
    }
   
    oData.xy = iColor.yz;
    oData.zw = uData.yz;
}

[Fragment shader]

void main(
    // IN
    float3 iUV        : TEXCOORD0,
	float4 iData      : TEXCOORD1,
	// OUT 
	out float4 oColor : SV_Target,
	// UNIFORM
	uniform float3 uColor)
{    
    float intensity = 0;
	float mult = 1;
	float smoothAvance = 16;

	if (iData.x+iUV.y*(iData.y-iData.x) > iData.z)
	{
		iUV.z *= 1-saturate(length(iData.x+iUV.y*(iData.y-iData.x) - iData.z)*smoothAvance);
	}
	
	if (iUV.y > 2)
	{
		iUV.y-=2; // Get back y coord
		intensity = saturate((1-2*length(float2(0.5,0.0)-iUV.xy)))*mult;
	}
	else
	{
		intensity = (1-2*length(0.5-iUV.x))*mult;
	}
	
	intensity = pow(intensity,1/(0.1f+iUV.z));
	
	// Falling effect
	smoothAvance = 6;
	iUV.z *= 1-iData.w*saturate(length(iData.x+iUV.y*(iData.y-iData.x) - iData.z)*smoothAvance);
	
	// Final color
	oColor = float4(uColor*iUV.z*intensity,1);
}