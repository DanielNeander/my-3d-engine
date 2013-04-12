
struct PsIn {
	float4 position: SV_Position;
	float2 texCoord: TexCoord;
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
	Out.texCoord = position.xy * float2(0.5, -0.5) + 0.5;

	return Out;
}


[Fragment shader]

Texture2D BackBuffer;
SamplerState filter;

float4 main(PsIn In) : SV_Target {
	clip(BackBuffer.Sample(filter, In.texCoord).a - 0.5);

	return 0;
}
