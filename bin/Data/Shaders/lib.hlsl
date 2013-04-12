#define DIRLIGHTS_ENABLEOPPOSITECOLOR 1
#define DIRLIGHTS_OPPOSITECOLOR float3(0.45f, 0.52f, 0.608f)
#define DEBUG_LIGHTCOMPLEXITY 0

#define DECLARE_SCREENPOS(X) float4 screenPos : X;
#define PS_SCREENPOS psComputeScreenCoord(psIn.screenPos)
#define VS_SETSCREENPOS(X) vsOut.screenPos = vsComputeScreenCoord(X);

typedef float3 color3;
typedef float4 color4;

float4   HalfPixelSize;
float BumpScale = 0.0f;
float CubeLightScale;

cbuffer LightParams {
	
	float3   LightPos;	
	float4   LightDiffuse;
	float4   LightSpecular;
	float4   LightAmbient;
	//float4   ShadowIndex;
	float    LightRange;
	int      LightType;
	
};

cbuffer MaterialParams {
	//float4 Color0;
	float4 MatDiffuse;
	float4 MatAmbient;
	float4 MatSpecular;
	float4 MatEmissive = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float  MatEmissiveIntensity = 0.0f;
	float  MatSpecularPower;
};



color4 EncodeHDR(in color4 rgba)
{
    return rgba * color4(0.5, 0.5, 0.5, 1.0);
}

//------------------------------------------------------------------------------
/**
    Vertex shader part to compute texture coordinate shadow texture lookups.
*/
float4 vsComputeScreenCoord(float4 pos)
{
    return pos;
}

//------------------------------------------------------------------------------
/**
    Pixel shader part to compute texture coordinate shadow texture lookups.
*/
float2 psComputeScreenCoord(float4 pos)
{
    float2 screenCoord = ((pos.xy / pos.ww) * float2(0.5, -0.5)) + float2(0.5f, 0.5f);
    screenCoord += HalfPixelSize.xy;
    return screenCoord;
}

//------------------------------------------------------------------------------
/**
	transformStatic()

	Transform position into modelview-projection space without deformations.

	@param	pos		a position in model space
	@param	mvp		the modelViewProjection matrix
	@return         transformed position
*/
float4
transformStatic(const float3 pos, const float4x4 mvp)
{
	return mul(float4(pos, 1.0), mvp);
}

//------------------------------------------------------------------------------
/**
    skinnedPosition()

    Compute a skinned position.

    @param  inPos           input vertex position
    @param  weights         4 weights
    @param  indices         4 joint indices into the joint palette
    @param  jointPalette    joint palette as vector4 array
    @return                 the skinned position
*/
float4
skinnedPosition(const float4 inPos, const float4 weights, const float4 indices, const matrix<float,4,3> jointPalette[164])
{
    float3 pos[4];

    int i;
    for (i = 0; i < 4; i++)
    {
        pos[i] = (mul(inPos, jointPalette[indices[i]])) * weights[i];
    }
    return float4(pos[0] + pos[1] + pos[2] + pos[3], 1.0f);
}

//------------------------------------------------------------------------------
/**
    skinnedNormal()

    Compute a skinned normal vector (without renormalization).

    @param  inNormal        input normal vector
    @param  weights         4 weights
    @param  indices         4 joint indices into the joint palette
    @param  jointPalette    joint palette as vector4 array
    @return                 the skinned normal
*/
float3
skinnedNormal(const float3 inNormal, const float4 weights, const float4 indices, const matrix<float,4,3> jointPalette[164])
{
    float3 normal[4];
    int i;
    for (i = 0; i < 4; i++)
    {
        normal[i] = mul(inNormal, (matrix<float,3,3>)jointPalette[indices[i]]) * weights[i];
    }
    return float3(normal[0] + normal[1] + normal[2] + normal[3]);
}

//------------------------------------------------------------------------------
/**
	tangentSpaceVector()

	Compute an unnormalized tangent space vector from a vertex position, reference
	position, normal and tangent (all in model space). This will compute
	an unnormalized light vector, and a binormal behind the scene.
*/
float3
tangentSpaceVector(const float3 pos, const float3 refPos, const float3 normal, const float3 tangent)
{
    // compute the light vector in model space
    float3 vec = refPos - pos;

    // compute the binormal
    float3 binormal = cross(normal, tangent);

	// transform with transpose of tangent matrix!
	float3 outVec = mul(float3x3(tangent, binormal, normal), vec);
	return outVec;
}

//------------------------------------------------------------------------------
/**
	tangentSpaceHalfVector()

	Compute the unnormalized tangent space half vector from a vertex position, light
	position, eye position, normal and tangent (all in model space). This
	will compute a normalized lightVec, a normalized eyeVec, an unnormalized
	half vector and a binormal behind the scenes.
*/
float3
tangentSpaceHalfVector(const float3 pos,
                       const float3 lightPos,
                       const float3 eyePos,
                       const float3 normal,
                       const float3 tangent)
{
    // compute the light vector, eye vector and half vector in model space
    float3 lightVec = normalize(lightPos - pos);
    float3 eyeVec   = normalize(eyePos - pos);
    float3 halfVec  = lightVec + eyeVec;

    // compute the binormal
    float3 binormal = cross(normal, tangent);

	// transform with transpose of tangent matrix!
	float3 outVec = mul(float3x3(tangent, binormal, normal), halfVec);
    return outVec;
}


//------------------------------------------------------------------------------
/**
	tangentSpaceEyeHalfVector()

	Compute tangent space eye and half vectors.
*/
void
tangentSpaceEyeHalfVector(in const float3 pos,
                          in const float3 lightPos,
                          in const float3 eyePos,
                          in const float3 normal,
                          in const float3 tangent,
                          out float3 eyeVec,
                          out float3 halfVec)
{
    // compute the light vector, eye vector and half vector in model space
    float3 lVec = normalize(lightPos - pos);
    float3 eVec = normalize(eyePos - pos);
    float3 hVec = lVec + eVec;

    // compute the binormal and tangent matrix
    float3 binormal = cross(normal, tangent);
    float3x3 tangentMatrix = float3x3(tangent, binormal, normal);

	// transform with transpose of tangent matrix!
    eyeVec = mul(tangentMatrix, eVec);
    halfVec = mul(tangentMatrix, hVec);
}

//------------------------------------------------------------------------------
/**
	tangentSpaceLightHalfEyeVector()

	Compute tangent space light and half vectors.
*/
void
tangentSpaceLightHalfEyeVector(in const float3 pos,
                               in const float3 lightPos,
                               in const float3 eyePos,
                               in const float3 normal,
                               in const float3 tangent,
                               out float3 lightVec,
                               out float3 halfVec,
                               out float3 eyeVec)
{
    // compute the light vector, eye vector and half vector in model space
    float3 lVec = normalize(lightPos - pos);
    float3 eVec = normalize(eyePos - pos);
    float3 hVec = lVec + eVec;

    // compute the binormal and tangent matrix
    float3 binormal = cross(normal, tangent);
    float3x3 tangentMatrix = float3x3(tangent, binormal, normal);

	// transform with transpose of tangent matrix!
    lightVec = mul(tangentMatrix, lVec);
    halfVec  = mul(tangentMatrix, hVec);
    eyeVec   = mul(tangentMatrix, eVec);
}

//------------------------------------------------------------------------------
/**
    reflectionVector()

    Returns the eye vector reflected around the surface normal in world space.
*/
float3
reflectionVector(const float3 pos,
                 const float3 eyePos,
                 const float3 normal,
                 const float4x4 model)
{
    float3 eyeVec = eyePos - pos;
    float3 reflVec = reflect(eyeVec, normal);
    float3 worldVec = mul(reflVec, (float3x3)model);
    return 0.5f * (1.0f + normalize(worldVec));
}

//------------------------------------------------------------------------------
/**
    fog()

    Compute a distance/layer fog.

    @param  pos                     the current vertex position in model space
    @param  worldPos,               the current vertex position in world space
    @param  modelEyePos             the eye position in model space
    @param  fogDistances            fog plane distances, x=near, y=far, z=bottom, w=top
    @param  fogNearBottomColor      the color at the near bottom, rgb=color, a=intensity
    @param  fogNearTopColor         the color at the near top
    @param  fogFarBottomColor       the color at the far bottom
    @param  fogFarTopColor          the color at the far top
*/
float4
fog(const float3 pos,
    const float3 worldPos,
    const float3 modelEyePos,
    const float4 fogDistances,
    const float4 fogNearBottomColor,
    const float4 fogNearTopColor,
    const float4 fogFarBottomColor,
    const float4 fogFarTopColor)
{
    // get normalized vertical and horizontal distance
    float2 dist;
    dist.x = clamp(distance(pos.xz, modelEyePos.xz), fogDistances.x, fogDistances.y);
    dist.y = clamp(worldPos.y, fogDistances.z, fogDistances.w);
    dist.x = (dist.x - fogDistances.x) / (fogDistances.y - fogDistances.x);
    dist.y = (dist.y - fogDistances.z) / (fogDistances.w - fogDistances.z);

    // get 2 horizontal interpolated colors
    float4 topColor = lerp(fogNearTopColor, fogFarTopColor, dist.x);
    float4 bottomColor = lerp(fogNearBottomColor, fogFarBottomColor, dist.x);

    // get resulting fog color
    float4 fogColor = lerp(bottomColor, topColor, dist.y);
    return fogColor;
}

//------------------------------------------------------------------------------
/**
    shadow()

    Compute the shadow modulation color.

    @param  shadowPos           position in shadow space
    @param  noiseSampler        sampler with a noise texture
    @param  shadowMapSampler    sampler with shadow map
    @param  shadowModSampler    shadow modulation sampler to fade shadow color in/out
    @return                     a shadow modulation color
*/
float4
shadow(const float4 shadowPos, float distOffset, sampler shadowMapSampler)
{
    // get projected position in shadow space
    float3 projShadowPos = shadowPos.xyz / shadowPos.w;

    // jitter shadow map position using noise texture lookup
//    projShadowPos.xy += tex2D(noiseSampler, projShadowPos.xy * 1234.5f).xy * 0.0005f;

    // sample shadow depth from shadow map
    float4 shadowDepth = tex2D(shadowMapSampler, projShadowPos.xy) + distOffset;

    // in/out test
    float4 shadowModulate;
    if ((projShadowPos.x < 0.0f) ||
        (projShadowPos.x > 1.0f) ||
        (projShadowPos.y < 0.0f) ||
        (projShadowPos.y > 1.0f))
    {
        // outside shadow projection
        shadowModulate = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else if ((shadowDepth.x > projShadowPos.z) || (shadowPos.z > shadowPos.w))
    {
        // not in shadow
        shadowModulate = float4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        shadowModulate = float4(0.5f, 0.5f, 0.5f, 1.0f);

        // in shadow
        //shadowModulate = tex2D(shadowModSampler, projShadowPos.xy);
        //float4 shadowColor = tex2D(shadowModSampler, projShadowPos.xy);
        //float4 blendColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
        //float relDist = saturate((projShadowPos.z - shadowDepth.x) * 20.0f);
        //shadowModulate = lerp(shadowColor, blendColor, relDist);
    }
    return shadowModulate;
}

//------------------------------------------------------------------------------
//  vsLighting
//
//  Vertex shader support function for simpler per-pixel lighting.
//
//  @param  pos                 [in] vertex position
//  @param  normal              [in] vertex normal
//  @param  tangent             [in] vertex tangent
//  @param  primLightVec        [out] xyz: primary light vector in tangent space, w: distance to light
//  @param  primHalfVec         [out] primary half vector in tangent space
//------------------------------------------------------------------------------
void
vsLight(in const float4 pos,
           in const float3 normal,
           in const float3 tangent,
           in const float3 lightPos,
           in const float3 eyePos,
           out float3 lightVec,
           out float3 halfVec)
{
    // compute the light vector, eye vector and half vector in model space
    float3 lVec = lightPos - pos.xyz;
    float3 eVec = eyePos - pos.xyz;
    float3 hVec = normalize(normalize(lVec) + normalize(eVec));

    // compute the binormal and tangent matrix
    float3 binormal = cross(normal, tangent);
    float3x3 tangentMatrix = float3x3(tangent, binormal, normal);

	// transform with transpose of tangent matrix!
    lightVec.xyz = mul(tangentMatrix, lVec);
    halfVec      = mul(tangentMatrix, hVec);
}

//------------------------------------------------------------------------------
/**
    Vertex shader part for per-pixel-lighting. Computes the light
    and half vector in tangent space which are then passed to the
    interpolators.
*/
void vsLight2(in float3 position,
             in float3 normal,
             in float3 tangent,
             in float3 binormal,
             in float3 modelEyePos,
             in float3 modelLightPos,
             out float3 tangentLightVec,
             out float3 modelLightVec,
             out float3 halfVec,
             out float3 tangentEyePos)
{
    float3 eVec = normalize(modelEyePos - position);
    if (LightType == 0)
    {
        // point light
        modelLightVec = modelLightPos - position;
    }
    else
    {
        // directional light
        modelLightVec = modelLightPos;
    }
    float3 hVec = normalize(normalize(modelLightVec) + eVec);
    float3x3 tangentMatrix = float3x3(tangent, binormal, normal);
    tangentLightVec = mul(tangentMatrix, modelLightVec);
    halfVec = mul(tangentMatrix, hVec);
    tangentEyePos = normalize(mul(tangentMatrix, eVec));
}

//------------------------------------------------------------------------------
/**
    Compute per-pixel lighting.

    NOTE: lightVec.w contains the distance to the light source
*/
color4 psLight(in color4 mapColor, in float3 tangentSurfaceNormal, in float3 lightVec, in float3 modelLightVec, in float3 halfVec, in half shadowValue)
{
    color4 color = mapColor * color4(LightAmbient.rgb + MatEmissive.rgb * MatEmissiveIntensity, MatDiffuse.a);

    // light intensities
    float specIntensity = pow(saturate(dot(tangentSurfaceNormal, normalize(halfVec))), MatSpecularPower); // Specular-Modulation * mapColor.a;
    float diffIntensity = dot(tangentSurfaceNormal, normalize(lightVec));

    color3 diffColor = mapColor.rgb * LightDiffuse.rgb * MatDiffuse.rgb;
    color3 specColor = specIntensity * LightSpecular.rgb * MatSpecular.rgb;

    // attenuation
    if (LightType == 0)
    {
        // point light source
        diffIntensity *= shadowValue * (1.0f - saturate(length(modelLightVec) / LightRange));
    }
    else
    {
        #if DIRLIGHTS_ENABLEOPPOSITECOLOR
        color.rgb += saturate(-diffIntensity) * DIRLIGHTS_OPPOSITECOLOR * mapColor.rgb * MatDiffuse.rgb;
        #endif
        diffIntensity *= shadowValue;
    }
    color.rgb += saturate(diffIntensity) * (diffColor.rgb + specColor.rgb);
    return color;
}


//float3 GetDeferredLightColor(float4  Pos_PS, , float3 diffuseAlbedo, float3 normalWS)
//{	  
//
//	float3 specularAlbedo = SpecularAlbedo;
//
//    diffuseAlbedo *= 1.0f - Balance;
//    specularAlbedo *= Balance;
//
//	float3 lighting = 0.0f;
//
//    // Add in the point Lights using the per-tile list
//    uint2 tileIdx = uint2(Pos_PS.xy) / LightTileSize;
//    uint bufferIdx = (tileIdx.y * NumTiles.x + tileIdx.x) * MaxLights;
//
//    [loop]
//    for(uint tileLightIdx = 0; tileLightIdx < MaxLights; ++tileLightIdx)
//    {
//        uint lightIndex = TileLights[bufferIdx + tileLightIdx];
//
//        [branch]
//        if(lightIndex >= MaxLights)
//            break;
//
//        Light light = Lights[lightIndex];
//        lighting += CalcPointLight(normalWS, light.Color, diffuseAlbedo,
//                                   specularAlbedo, Roughness, input.PositionWS,
//                                   light.Position, light.Falloff, CameraPosWS);
//    }
//
//    return lighting;
//}