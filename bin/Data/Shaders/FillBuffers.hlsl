
struct VsIn {
	float4 position : Position;
	float2 texCoord : TexCoord;
	float3 tangent  : Tangent;
	float3 binormal : Binormal;
	float3 normal   : Normal;
};

struct PsIn {
	centroid float4 position : SV_Position;
	float2 texCoord : TexCoord;
	float3 tangent  : Tangent;
	float3 binormal : Binormal;
	float3 normal   : Normal;
};

[Vertex shader]

float4x4 viewProj;
float3 camPos;

PsIn main(VsIn In){
	PsIn Out;

	Out.position = mul(viewProj, In.position);
	Out.texCoord = In.texCoord;
	// Pass tangent space
	Out.tangent  = In.tangent;
	Out.binormal = In.binormal;
	Out.normal   = In.normal;

	return Out;
}


[Fragment shader]

struct PsOut {
	float4 base   : SV_Target0;
	float3 normal : SV_Target1;
};

Texture2D Base;
Texture2D Bump;
SamplerState baseFilter;

PsOut main(PsIn In){
	PsOut Out;

	float3 base = Base.Sample(baseFilter, In.texCoord).rgb;
	float3 bump = Bump.Sample(baseFilter, In.texCoord).xyz;

	// Compute world-space normal
	float3 normal = In.tangent * bump.x + In.binormal * bump.y + In.normal * bump.z;
	normal = normalize(normal);

	// Fill the buffers
	Out.base.rgb = base;
	// Detect multisample edge pixels by checking whether the sample position has been shifted from center of pixel by centroid sampling
	Out.base.a = dot(abs(frac(In.position.xy) - 0.5), 1000.0);
	Out.normal = normal;

	return Out;
}
