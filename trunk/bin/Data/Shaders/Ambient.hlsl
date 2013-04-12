
struct PsIn {
	float4 position: SV_Position;
#if (SAMPLE_COUNT == 1)
	float2 texCoord: TexCoord;
#endif
};

[Vertex shader]

PsIn main(uint VertexID: SV_VertexID){
	PsIn Out;

	// Produce a fullscreen triangle
	float4 position;
	position.x = (VertexID == 2)?  3.0 : -1.0;
	position.y = (VertexID == 0)? -3.0 :  1.0;
	position.zw = 1.0;

	Out.position = position;
#if (SAMPLE_COUNT == 1)
	Out.texCoord = position.xy * float2(0.5, -0.5) + 0.5;
#endif

	return Out;
}


[Fragment shader]

#if (SAMPLE_COUNT > 1)
Texture2DMS <float4, SAMPLE_COUNT> Base;
#else
Texture2D <float4> Base;
SamplerState filter;
#endif

float2 factors;

float4 main(PsIn In) : SV_Target {

#if (SAMPLE_COUNT > 1)
	int3 texCoord = int3(int2(In.position.xy), 0);

	// Average all samples
	float4 base = 0;
	for (int i = 0; i < SAMPLE_COUNT; i++){
		base += (1.0 / SAMPLE_COUNT) * Base.Load(texCoord, i);
	}

	// Return 1 if any sample was non-zero.
	base.a = (base.a > 0.0);
#else
	float4 base = Base.Sample(filter, In.texCoord);
#endif
	
	base.rgb *= factors.x;
	// For stencil visualizations
	base.rgb += factors.y * base.a;

	return base;
}
