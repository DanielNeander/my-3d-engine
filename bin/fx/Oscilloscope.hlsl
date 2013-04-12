// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#ifndef NUMBER_OF_FRAMES_TO_DISPLAY
#   define NUMBER_OF_FRAMES_TO_DISPLAY 100
#endif

cbuffer cbOscilloscopeParams : register(b0)
{
	float4 g_OscilloscopePosPS = float4(-0.9,0.9,-0.7,0.7);
	float g_TimeScale = 1;
	int g_iFirstFrameNum = 0;
}

cbuffer cbFrameTimes : register(b1)
{
	float4 g_FrameTimes[NUMBER_OF_FRAMES_TO_DISPLAY];
}

struct RenderOscilloscopeVS_OUTPUT
{
    float4 m_ScreenPos_PS : SV_POSITION;
    float CurrTime : TIME;
    float FrameNum : FRAME_NUM;
};

RenderOscilloscopeVS_OUTPUT RenderOscilloscopeVS( in uint VertexId : SV_VertexID)
{
    RenderOscilloscopeVS_OUTPUT Verts[4] = 
    {
        {float4(g_OscilloscopePosPS.xy, 1.0, 1.0), g_TimeScale, 0}, 
        {float4(g_OscilloscopePosPS.xw, 1.0, 1.0), 0,           0},
        {float4(g_OscilloscopePosPS.zy, 1.0, 1.0), g_TimeScale, NUMBER_OF_FRAMES_TO_DISPLAY},
        {float4(g_OscilloscopePosPS.zw, 1.0, 1.0), 0,           NUMBER_OF_FRAMES_TO_DISPLAY}
    };
    return Verts[VertexId];
}

float4 RenderOscilloscopePS(in RenderOscilloscopeVS_OUTPUT In) : SV_Target
{
    clip( g_FrameTimes[ (In.FrameNum+g_iFirstFrameNum) % NUMBER_OF_FRAMES_TO_DISPLAY ].x - In.CurrTime );

    return float4(0,0,0.5,0.7);
}