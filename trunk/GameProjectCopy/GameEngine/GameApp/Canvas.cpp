#include "stdafx.h"
#include "Canvas.h"
#include "Client.h"

FCanvas::FCanvas( class FRenderTarget* InRenderTarget )
	:	RenderTarget(InRenderTarget)
	,	bEnableDepthTest(FALSE)
	,	bRenderTargetDirty(FALSE)
{
	assert(RenderTarget);
	// Push the viewport transform onto the stack.  Default to using a 2D projection. 
	new(TransformStack) FTransformEntry( 
		FMatrix( CalcBaseTransform2D(RenderTarget->GetSizeX(),RenderTarget->GetSizeY()) ) 
		);
	// init alpha to 1
	AlphaModulate=1.0;

	// init sort key to 0
	PushDepthSortKey(0);
}


void FCanvas::Flush()
{

}

void FCanvas::PushRelativeTransform( const FMatrix& Transform )
{
	INT PreviousTopIndex = TransformStack.Num() - 1;
	TransformStack.AddItem( FTransformEntry(Transform * TransformStack(PreviousTopIndex).GetMatrix()) );

}

void FCanvas::PushAbsoluteTransform( const FMatrix& Transform )
{
	TransformStack.AddItem( FTransformEntry(Transform * TransformStack(0).GetMatrix()) );

}

/** Pops the topmost matrix from the canvas transform stack. */
void FCanvas::PopTransform()
{
	TransformStack.Pop();
}

void FCanvas::SetBaseTransform( const FMatrix& Transform )
{
	// set the base transform
	if( TransformStack.Num() > 0 )
	{
		TransformStack(0).SetMatrix(Transform);
	}
	else
	{
		new(TransformStack) FTransformEntry(Transform);
	}
}

/**
* Generate a 2D projection for the canvas. Use this if you only want to transform in 2D on the XY plane
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @return Matrix for canvas projection
*/
FMatrix FCanvas::CalcBaseTransform2D( UINT ViewSizeX, UINT ViewSizeY )
{
	return 
		FTranslationMatrix(FVector(-GPixelCenterOffset,-GPixelCenterOffset,0)) *
		FMatrix(
		FPlane(	1.0f / (ViewSizeX / 2.0f),	0.0,										0.0f,	0.0f	).ToVec4(),
		FPlane(	0.0f,						-1.0f / (ViewSizeY / 2.0f),					0.0f,	0.0f	).ToVec4(),
		FPlane(	0.0f,						0.0f,										1.0f,	0.0f	).ToVec4(),
		FPlane(	-1.0f,						1.0f,										0.0f,	1.0f	).ToVec4()
		);
}

/**
* Generate a 3D projection for the canvas. Use this if you want to transform in 3D 
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @param fFOV - Field of view for the projection
* @param NearPlane - Distance to the near clip plane
* @return Matrix for canvas projection
*/
FMatrix FCanvas::CalcBaseTransform3D( UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane )
{
	FMatrix ViewMat(CalcViewMatrix(ViewSizeX,ViewSizeY,fFOV));
	FMatrix ProjMat(CalcProjectionMatrix(ViewSizeX,ViewSizeY,fFOV,NearPlane));
	return ViewMat * ProjMat;
}

/**
* Generate a view matrix for the canvas. Used for CalcBaseTransform3D
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @param fFOV - Field of view for the projection
* @return Matrix for canvas view orientation
*/
FMatrix FCanvas::CalcViewMatrix( UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV )
{
	// convert FOV to randians
	FLOAT FOVRad = fFOV * (FLOAT)noMath::PI / 360.0f;
	// move camera back enough so that the canvas items being rendered are at the same screen extents as regular canvas 2d rendering	
	FTranslationMatrix CamOffsetMat(-FVector(0,0,-appTan(FOVRad)*ViewSizeX/2));
	// adjust so that canvas items render as if they start at [0,0] upper left corner of screen 
	// and extend to the lower right corner [ViewSizeX,ViewSizeY]. 
	FMatrix OrientCanvasMat(
		FPlane(	1.0f,				0.0f,				0.0f,	0.0f	).ToVec4(),
		FPlane(	0.0f,				-1.0f,				0.0f,	0.0f	).ToVec4(),
		FPlane(	0.0f,				0.0f,				1.0f,	0.0f	).ToVec4(),
		FPlane(	ViewSizeX * -0.5f,	ViewSizeY * 0.5f,	0.0f, 1.0f		).ToVec4()
		);
	return 
		// also apply screen offset to align to pixel centers
		FTranslationMatrix(FVector(-GPixelCenterOffset,-GPixelCenterOffset,0)) * 
		OrientCanvasMat * 
		CamOffsetMat;
}

/**
* Generate a projection matrix for the canvas. Used for CalcBaseTransform3D
*
* @param ViewSizeX - Viewport width
* @param ViewSizeY - Viewport height
* @param fFOV - Field of view for the projection
* @param NearPlane - Distance to the near clip plane
* @return Matrix for canvas projection
*/
FMatrix FCanvas::CalcProjectionMatrix( UINT ViewSizeX, UINT ViewSizeY, FLOAT fFOV, FLOAT NearPlane )
{
	// convert FOV to randians
	FLOAT FOVRad = fFOV * (FLOAT)noMath::PI / 360.0f;
	// project based on the FOV and near plane given
	return FPerspectiveMatrix(
		FOVRad,
		ViewSizeX,
		ViewSizeY,
		NearPlane
		);
}

/**
* Toggles current depth testing state for the canvas. All batches
* will render with depth testing against the depth buffer if enabled.
*
* @param bEnabled - if TRUE then depth testing is enabled
*/
void FCanvas::SetDepthTestingEnabled( UBOOL bEnabled )
{
	if( bEnableDepthTest != bEnabled )
	{
		Flush();
		bEnableDepthTest = bEnabled;
	}
}

/** 
 * Set the current masked region on the canvas
 * All rendering from this point on will be masked to this region.
 * The region being masked uses the current canvas transform
 *
 * @param X - x offset in canvas coords
 * @param Y - y offset in canvas coords
 * @param SizeX - x size in canvas coords
 * @param SizeY - y size in canvas coords
 */
void FCanvas::PushMaskRegion( FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY )
{
	FMaskRegion NewMask(X, Y, SizeX, SizeY, TransformStack.Top().GetMatrix());
	if ( !NewMask.IsEqual(GetCurrentMaskRegion()) )
	{
		Flush();
	}

	MaskRegionStack.Push(NewMask);
}

void FCanvas::ReplaceMaskRegion( FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY )
{
	if ( MaskRegionStack.Num() > 0 )
	{
		const INT CurrentMaskIdx = MaskRegionStack.Num() - 1;

		FMaskRegion NewMask(X, Y, SizeX, SizeY, TransformStack.Top().GetMatrix());
		if ( !NewMask.IsEqual(MaskRegionStack(CurrentMaskIdx)) )
		{
			Flush();
			MaskRegionStack(CurrentMaskIdx) = NewMask;
		}
	}
	else
	{
		PushMaskRegion(X, Y, SizeX, SizeY);
	}
}

void FCanvas::PopMaskRegion()
{
	FMaskRegion NextMaskRegion = MaskRegionStack.Num() > 1 
		? MaskRegionStack(MaskRegionStack.Num() - 2)
		: FMaskRegion();

	if ( !NextMaskRegion.IsEqual(GetCurrentMaskRegion()) )
	{
		Flush();
	}

	if ( MaskRegionStack.Num() > 0 )
	{
		MaskRegionStack.Pop();
	}
}

void FCanvas::SetRenderTarget( FRenderTarget* NewRenderTarget )
{

}

FCanvas::FMaskRegion FCanvas::GetCurrentMaskRegion() const
{
	if ( MaskRegionStack.Num() > 0 )
	{
		return MaskRegionStack(MaskRegionStack.Num() - 1);
	}

	return FMaskRegion();
}

/**
* Get the sort element for the given sort key. Allocates a new entry if one does not exist
*
* @param DepthSortKey - the key used to find the sort element entry
* @return sort element entry
*/
FCanvas::FCanvasSortElement& FCanvas::GetSortElement( INT DepthSortKey )
{
	// find the FCanvasSortElement array entry based on the sortkey
	INT ElementIdx = INDEX_NONE;	
	std::map<INT, INT>::iterator iter = SortedElementLookupMap.find(DepthSortKey);
	if( iter != SortedElementLookupMap.end() )
	{
		ElementIdx  = iter->second;		
		assert( SortedElements.IsValidIndex(ElementIdx) );
	}	
	// if it doesn't exist then add a new entry (no duplicates allowed)
	else
	{
		new(SortedElements) FCanvasSortElement(DepthSortKey);
		ElementIdx = SortedElements.Num()-1;
		// keep track of newly added array index for later lookup
		SortedElementLookupMap.insert(std::make_pair(DepthSortKey, ElementIdx) );
	}
	return SortedElements(ElementIdx);
}

void FCanvas::FlushSetMaskRegion()
{

}

void FCanvas::FlushResetMaskRegion()
{

}

FCanvas::~FCanvas()
{
	Flush();
}


extern void DrawTriangle2D( FCanvas* Canvas, const FIntPoint& Position0, const FIntPoint& TexCoord0, const FIntPoint& Position1, const FIntPoint& TexCoord1, const FIntPoint& Position2, const FIntPoint& TexCoord2, const FLinearColor& Color, UBOOL AlphaBlend /*= 1 */ )
{

}

/**
 *	Draws a 2D line.
 *
 * @param	Canvas		Drawing canvas.
 * @param	StartPos	Starting position for the line.
 * @param	EndPos		Ending position for the line.
 * @param	Color		Color for the line.
 */
extern void DrawLine2D( FCanvas* Canvas, const FIntPoint& StartPos, const FIntPoint& EndPos, const FLinearColor& Color )
{
	/*FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Line);
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	BatchedElements->AddLine(FVector(StartPos.X,StartPos.Y,0),FVector(EndPos.X,EndPos.Y,0),Color,HitProxyId);*/
}

extern void DrawBox2D( FCanvas* Canvas,const FIntPoint& StartPos,const FIntPoint& EndPos,const FLinearColor& Color )
{
	DrawLine2D(Canvas,FIntPoint(StartPos.X,StartPos.Y),FIntPoint(StartPos.X,EndPos.Y),Color);
	DrawLine2D(Canvas,FIntPoint(StartPos.X,EndPos.Y),FIntPoint(EndPos.X,EndPos.Y),Color);
	DrawLine2D(Canvas,FIntPoint(EndPos.X,EndPos.Y),FIntPoint(EndPos.X,StartPos.Y),Color);
	DrawLine2D(Canvas,FIntPoint(EndPos.X,StartPos.Y),FIntPoint(StartPos.X,StartPos.Y),Color);
}

void DrawTile(
	FCanvas* Canvas,
	FLOAT X,
	FLOAT Y,
	FLOAT SizeX,
	FLOAT SizeY,
	FLOAT U,
	FLOAT V,
	FLOAT SizeU,
	FLOAT SizeV,
	const FLinearColor& Color,	
	UBOOL AlphaBlend
	)
{
	FLinearColor ActualColor = Color;
	ActualColor.A *= Canvas->AlphaModulate;

	/*const FTexture* FinalTexture = Texture ? Texture : GWhiteTexture;
	const EBlendMode BlendMode = AlphaBlend ? BLEND_Translucent : BLEND_Opaque;
	FBatchedElements* BatchedElements = Canvas->GetBatchedElements(FCanvas::ET_Triangle, FinalTexture, BlendMode);	
	FHitProxyId HitProxyId = Canvas->GetHitProxyId();

	INT V00 = BatchedElements->AddVertex(FVector4(X,		Y,			0,1),FVector2D(U,			V),			ActualColor,HitProxyId);
	INT V10 = BatchedElements->AddVertex(FVector4(X + SizeX,Y,			0,1),FVector2D(U + SizeU,	V),			ActualColor,HitProxyId);
	INT V01 = BatchedElements->AddVertex(FVector4(X,		Y + SizeY,	0,1),FVector2D(U,			V + SizeV),	ActualColor,HitProxyId);
	INT V11 = BatchedElements->AddVertex(FVector4(X + SizeX,Y + SizeY,	0,1),FVector2D(U + SizeU,	V + SizeV),	ActualColor,HitProxyId);

	BatchedElements->AddTriangle(V00,V10,V11,FinalTexture,BlendMode);
	BatchedElements->AddTriangle(V00,V11,V01,FinalTexture,BlendMode);*/
}