//----------------------------------------------------------------------------------
// File:  Lighting.fx
// Author: Mathias Schott
// Email:  sdkfeedback@nvidia.com
// 
// Copyright (c) 2007 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA OR ITS SUPPLIERS
// BE  LIABLE  FOR  ANY  SPECIAL,  INCIDENTAL,  INDIRECT,  OR  CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//
//----------------------------------------------------------------------------------

// note that the order of the variables HAS to match the C++ struct/

cbuffer LightningAppearance
{
	float3 ColorInside;					// color of the inside of the beam
	float ColorFallOffExponent;			// determines how quickly the color changes from
										// inside to outside
	
	float3 ColorOutside;				// color of the outside of the beam
	float2 BoltWidth;					// size in world space of the beam
	

};

cbuffer LightningStructure
{
	// for ZigZag pattern
	float2 ZigZagFraction;				// determines at which percentage the segment will be broken
	float2 ZigZagDeviationRight;		// min and max of deviation in segments local frame
	float2 ZigZagDeviationUp;			// min and max of deviation in segments local frame
	float  ZigZagDeviationDecay;		// controls how fast the deviation get smaller
	
	// for Fork pattern
	float2 ForkFraction;				// similiar above, but for fork pattern
	float2 ForkZigZagDeviationRight;
	float2 ForkZigZagDeviationUp;
	float  ForkZigZagDeviationDecay;
	
	float2 ForkDeviationRight;
	float2 ForkDeviationUp;
	float2 ForkDeviationForward;
	float  ForkDeviationDecay;

	float2	ForkLength;					// min and max of length of fork segments, in world space
	float	ForkLengthDecay;			// decay of length
};

#define MaxTargets 8
cbuffer LightningChain
{

	float3	ChainSource;

	float4 ChainTargetPositions[MaxTargets];
	
	int			NumTargets;
};

cbuffer PerSubdivision
{
	bool	Fork;
	uint	SubdivisionLevel;
};

cbuffer Lightning
{
	float	AnimationSpeed;
};

// decay based on global subdivision level
float Decay(float amount)
{
	return exp(-amount * SubdivisionLevel);
}

// decay based in explicit level
float Decay(float2 amount, uint level)
{
	return  amount.x * exp(-amount.y * level);
}

// Random number generation
// found in numerical recipes
// http://www.library.cornell.edu/nr/bookcpdf/c7-1.pdf

// per shader global variable to keep track of the last random number 
int random_x;

#define RANDOM_IA 16807
#define RANDOM_IM 2147483647
#define RANDOM_AM (1.0f/float(RANDOM_IM))
#define RANDOM_IQ 127773
#define RANDOM_IR 2836
#define RANDOM_MASK 123459876

float Random()
{
	int k;
	float ans;
	
	random_x ^= RANDOM_MASK;								//XORing with MASK allows use of zero and other
	k = random_x / RANDOM_IQ;								//simple bit patterns for idum.
	random_x = RANDOM_IA * (random_x - k * RANDOM_IQ ) - RANDOM_IR * k;	//Compute idum=(IA*idum) % IM without overif
	if (random_x < 0) 
		random_x += RANDOM_IM;					//flows by Schrage’s method.
	
	ans = RANDOM_AM * random_x;					//Convert idum to a floating result.
	random_x ^= RANDOM_MASK;					//Unmask before return.
	
	return ans;
}

void RandomSeed (int value)
{
	random_x = value;
	Random();
}

float Random(float low, float high)
{
	float v = Random();
	return low * (1.0f - v) + high * v;
}

float3 Random(float3 low, float3 high)
{
	float3 v = float3(Random(),Random(),Random());
	return low * (1.0f - v) + high * v;
}

// little debug helpers
float3 colors[] =
{
	float3(1.0f,0.0f,0.0f),
	float3(0.0f,1.0f,0.0f),
	float3(0.0f,0.0f,1.0f),
	float3(0.0f,1.0f,1.0f),
	float3(1.0f,0.0f,1.0f),
	float3(1.0f,1.0f,0.0f),
	float3(0.0f,0.0f,0.0f),
	float3(1.0f,1.0f,1.0f),

	0.5f * float3(1.0f,0.0f,0.0f),
	0.5f * float3(0.0f,1.0f,0.0f),
	0.5f * float3(0.0f,0.0f,1.0f),
	0.5f * float3(0.0f,1.0f,1.0f),
	0.5f * float3(1.0f,0.0f,1.0f),
	0.5f * float3(1.0f,1.0f,0.0f),
	0.5f * float3(1.0f,1.0f,1.0f),
};

// Subdivision
struct SubdivideVertex
{
	float3	Start		: Start;		// start of segment
	float3	End			: End;			// end of segment
	float3	Up			: Up;			// up vector, specifying frame of orientation for deviation parameters
	uint	Level		: Level;		// n + 1 for forked segment, n for jittered segments
};

void SubdivideVS( in SubdivideVertex input, out SubdivideVertex output)
{
	output = input;
}

// helper data structure for passing stuff around
struct Segment
{
	float3 Start;
	float3 End;

	float3 Center;
	
	float3 Right;
	float3 Up;
	float3 Forward;
	
	uint Level;
	uint Primitive;
};

// make coordinate frame
float3 GetUp(float3 start, float3 end, float3 right)
{
	float3 forward = normalize(end - start);
	return cross(right, forward);
}

// appends vertex representing a segment to stream
void DrawLineRight
(
	float3 start, 
	float3 end, 
	float3 right, 
	uint level, 
	inout PointStream<SubdivideVertex> output)
{
	SubdivideVertex v = {start, end, GetUp(start,end,right), level};
	output.Append(v);
}

// subdivision by splitting segment into two and randomly moving split point
void PatternZigZag(in Segment segment , inout PointStream<SubdivideVertex> output)
{
	float2 delta = Decay(ZigZagDeviationDecay) * float2(Random(ZigZagDeviationRight.x,ZigZagDeviationRight.y), Random(ZigZagDeviationUp.x,ZigZagDeviationUp.y));
	
	float3 	jittered = lerp(segment.Start,segment.End, Random(ZigZagFraction.x, ZigZagFraction.y)) + 
			delta.x * segment.Right + delta.y * segment.Up;
	
	DrawLineRight(segment.Start, jittered, segment.Right, segment.Level,   output);
	DrawLineRight(jittered, segment.End, segment.Right, segment.Level,  output);

}

// subdivision by splitting segment into two and randomly moving split point
// and adding a branch segment between the split position and the random end point
void PatternFork(in Segment segment , inout PointStream<SubdivideVertex> output)
{
	float2 delta = Decay(ForkZigZagDeviationDecay) * float2(Random(ForkZigZagDeviationRight.x,ForkZigZagDeviationRight.y), Random(ForkZigZagDeviationUp.x, ForkZigZagDeviationUp.y));
	
	float3 	jittered = lerp(segment.Start,segment.End, Random(ForkFraction.x, ForkFraction.y)) + 
			delta.x * segment.Right + delta.y * segment.Up;
	
	DrawLineRight(segment.Start, jittered, segment.Right, segment.Level, output);
	DrawLineRight(jittered, segment.End, segment.Right, segment.Level, output);
	

	float3 fork_dir = normalize(segment.Right);

	
	float3 f_delta = Decay(ForkDeviationDecay) * float3(Random(ForkDeviationRight.x,ForkDeviationRight.y), Random(ForkDeviationUp.x, ForkDeviationUp.y),  Random(ForkDeviationForward.x, ForkDeviationForward.y));
	float  f_length = Random(ForkLength.x, ForkLength.y) * Decay(ForkLengthDecay);
	float3 f_jittered = jittered + f_length * normalize(f_delta.x * segment.Right + f_delta.y  * segment.Up + f_delta.z * segment.Forward);
	
	DrawLineRight(jittered, f_jittered, segment.Forward,  segment.Level + 1, output);
	
}

// decides whether to fork or to jitter bases upon uniform parameter
[MaxVertexCount(3)]
void SubdivideGS
(
	in point SubdivideVertex input[1],
	in uint primitive_id : SV_PrimitiveID,
	
	inout PointStream<SubdivideVertex> output
)
{
	RandomSeed(primitive_id + 1 + time * AnimationSpeed);

	float3 center = 0.5f * ( input[0].Start + input[0].End );
	
	Segment segment;
	
	segment.Start = input[0].Start;
	segment.End = input[0].End;
	
	segment.Center = 0.5f * (segment.Start + segment.End);
	
	segment.Up = input[0].Up;
	
	segment.Forward = normalize(segment.End - segment.Start);
	segment.Right = normalize(cross(segment.Forward, segment.Up));
	segment.Up = normalize(cross(segment.Right, segment.Forward));
	

	segment.Level = input[0].Level;
	
	segment.Primitive = primitive_id;

	if(Fork)
		PatternFork(segment, output);	
	else
		PatternZigZag(segment, output);	

}

GeometryShader gs_subdivide = ConstructGSWithSO
( 
	CompileShader
	( 
		gs_4_0, 
		SubdivideGS() 
	),
	"Start.xyz; End.xyz; Up.xyz; Level.x" 
);

technique10 Subdivide
{
    pass P0
    {       
		SetVertexShader  (CompileShader(vs_4_0, SubdivideVS()));
        SetGeometryShader(gs_subdivide);
        SetPixelShader(0);
        
        SetDepthStencilState(DisableDepth,0);
    }
}


// as there are no vertex buffers bound, we use the vertex ID to index into the constant buffer
// to connect the segments
void ChainLightningVS( uint VertexId: SV_VertexID, out SubdivideVertex output)
{
	if(0 == VertexId)
	{
		output.Start = ChainSource;
		output.End = ChainTargetPositions[0].xyz;
	}
	else
	{
		output.Start = ChainTargetPositions[VertexId-1].xyz;
		output.End = ChainTargetPositions[VertexId].xyz;
	
	}

	output.Up = float3(0,1,0);
	output.Level = 0;
}
 
technique10 ChainLightning
{
    pass P0
    {       
		SetVertexShader  (CompileShader(vs_4_0, ChainLightningVS()));
        SetGeometryShader(gs_subdivide);
        SetPixelShader(0);
        
        SetDepthStencilState(DisableDepth,0);
    }
}

// for debugging
struct LinesOutVertexGS2PS
{
	float4	Position : SV_Position;
	uint    Level : Level;
};

void LinesOutVS( in SubdivideVertex input, in uint id: SV_VertexID,out SubdivideVertex output)
{
	output = input;
}

[MaxVertexCount(3)]
void LinesOutGS
(
	in point SubdivideVertex input[1],
	in uint primitive_id : SV_PrimitiveID,
	
	inout LineStream<LinesOutVertexGS2PS> output
)
{
	LinesOutVertexGS2PS v1 = { mul(float4(input[0].Start,1.0f), world_view_projection), input[0].Level };
	output.Append(v1);
	
	LinesOutVertexGS2PS v2 = { mul(float4(input[0].End,1.0f), world_view_projection), input[0].Level };
	output.Append(v2);
	
	output.RestartStrip();

}
void LinesOutPS( in LinesOutVertexGS2PS input, out float4 output : SV_Target) 
{
	output = float4(colors[input.Level], 1.0f);
}

technique10 LinesOut
{
    pass P0
    {       
		SetVertexShader  (CompileShader(vs_4_0, LinesOutVS()));
        SetGeometryShader(CompileShader(gs_4_0, LinesOutGS()));
        SetPixelShader   (CompileShader(ps_4_0, LinesOutPS()));
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
    }
}
 
struct BoltOutVertexGS2PS
{
	float4	Position : SV_Position;
	float2	Gradient : Gradient;
	int		Level	: Level;
};

void BoltOutVS( in SubdivideVertex input, in uint id: SV_VertexID,out SubdivideVertex output)
{
	output = input;
}


// generate camera and segment aligned quads with inter segment gaps filled
[MaxVertexCount(8)]
void BoltOutGS
(
	in point SubdivideVertex input[1],
	in uint primitive_id : SV_PrimitiveID,
	
	inout TriangleStream<BoltOutVertexGS2PS> output
)
{
	// vs stands for view space
	float3 vs_start = mul(float4(input[0].Start, 1.0f), world_view);
	float3 vs_end = mul(float4(input[0].End, 1.0f), world_view);
	float3 vs_forward = normalize(vs_end - vs_start);
	
	float width =  Decay(BoltWidth, input[0].Level);
	
	float3 right = width * normalize(cross(vs_forward,float3(0,0,1)));
	
	float x = 1;
	float y = 1;

	static const bool close_gaps = true;


	if(close_gaps)
	{
		BoltOutVertexGS2PS v0 = { mul(float4(vs_start - right - width * vs_forward, 1.0f), projection), float2(-x,1), input[0].Level};
		output.Append(v0);
	
		BoltOutVertexGS2PS v1 = { mul(float4(vs_start + right - width * vs_forward, 1.0f), projection), float2(x,1), input[0].Level};
		output.Append(v1);
	
	}

	BoltOutVertexGS2PS v2 = { mul(float4(vs_start - right, 1.0f), projection), float2(-x,0), input[0].Level };
	output.Append(v2);
	
	BoltOutVertexGS2PS v3 = { mul(float4(vs_start + right, 1.0f), projection), float2(x,0), input[0].Level};
	output.Append(v3);

	BoltOutVertexGS2PS v4 = { mul(float4(vs_end - right, 1.0f), projection), float2(-x,0), input[0].Level};
	output.Append(v4);
	
	BoltOutVertexGS2PS v5 = { mul(float4(vs_end + right, 1.0f), projection), float2(x,0), input[0].Level};
	output.Append(v5);
	

	if(close_gaps)
	{	
		BoltOutVertexGS2PS v6 = { mul(float4(vs_end - right + width * vs_forward, 1.0f), projection), float2(-x,1), input[0].Level };
		output.Append(v6);
	
		BoltOutVertexGS2PS v7 = { mul(float4(vs_end + right + width * vs_forward, 1.0f), projection), float2(x,1), input[0].Level};
		output.Append(v7);
	 }
	 
	output.RestartStrip();
}


void BoltOutPS(in BoltOutVertexGS2PS input, out float4 output : SV_Target) 
{
	float f = saturate(length(input.Gradient));
	float brightness = 1-f;
	float color_shift = saturate(pow(1-f, ColorFallOffExponent));
	output = brightness * float4(lerp(ColorOutside,ColorInside,color_shift), 1.0f);
}

technique10 BoltOut
{
    pass P0
    {       
		SetVertexShader  (CompileShader(vs_4_0, BoltOutVS()));
        SetGeometryShader(CompileShader(gs_4_0, BoltOutGS()));
        SetPixelShader   (CompileShader(ps_4_0, BoltOutPS()));
     
        SetDepthStencilState(DisableDepthWrite, 0);
        SetRasterizerState(NoCull);
        
        SetBlendState(AdditiveBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
    }
}

technique10 ShowLines
{
    pass P0
    {       
		SetVertexShader  (CompileShader(vs_4_0, BoltOutVS()));
        SetGeometryShader(CompileShader(gs_4_0, LinesOutGS()));
        SetPixelShader   (CompileShader(ps_4_0, LinesOutPS()));
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(1.0f, 1.0f, 1.0f, 1.0f), ~0);
    }
}
