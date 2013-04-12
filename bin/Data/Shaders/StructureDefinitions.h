//=================================================================================================
//
//  Light Indexed Deferred Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

static const uint LightTileSize = 16;

struct Light
{
    float3 Position;
    float3 Color;
    float Falloff;
};