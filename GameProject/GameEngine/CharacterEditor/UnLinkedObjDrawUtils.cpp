#include "EditorPCH.h"
#include "UnLinkedObjDrawUtils.h"

/** Minimum viewport zoom at which text will be rendered. */
static const FLOAT	TextMinZoom(0.3f);

/** Minimum viewport zoom at which arrowheads will be rendered. */
static const FLOAT	ArrowheadMinZoom(0.3f);

/** Minimum viewport zoom at which connectors will be rendered. */
static const FLOAT	ConnectorMinZoom(0.2f);

/** Minimum viewport zoom at which connectors will be rendered. */
static const FLOAT	SliderMinZoom(0.2f);

static const FLOAT	MaxPixelsPerStep(15.f);

static const INT	ArrowheadLength(14);
static const INT	ArrowheadWidth(4);

static const FColor SliderHandleColor(0, 0, 0);



void FLinkedObjDrawUtils::DrawNGon( FCanvas* Canvas, const FVector2D& Center, const FColor& Color, INT NumSides, FLOAT Radius )
{
	if ( AABBLiesWithinViewport( Canvas, Center.x-Radius, Center.y-Radius, Radius*2, Radius*2) )
	{
		FVector2D Verts[256];
		NumSides = Clamp(NumSides, 3, 255);

		for(INT i=0; i<NumSides+1; i++)
		{
			const FLOAT Angle = (2 * (FLOAT)noMath::PI) * (FLOAT)i/(FLOAT)NumSides;
			Verts[i] = Center + FVector2D( Radius*appCos(Angle), Radius*appSin(Angle) );
		}

		for(INT i=0; i<NumSides; i++)
		{
			DrawTriangle2D(
				Canvas,
				FVector2D(Center), FVector2D(0,0),
				FVector2D(Verts[i+0]), FVector2D(0,0),
				FVector2D(Verts[i+1]), FVector2D(0,0),
				Color
				);
		}
	}
}


void FLinkedObjDrawUtils::DrawSpline( FCanvas* Canvas, const FIntPoint& Start, const FVector2D& StartDir, const FIntPoint& End, const FVector2D& EndDir, const FColor& LineColor, UBOOL bArrowhead, UBOOL bInterpolateArrowDirection/*=FALSE*/ )
{
	const INT MinX = Min( Start.X, End.X );
	const INT MaxX = Max( Start.X, End.X );
	const INT MinY = Min( Start.Y, End.Y );
	const INT MaxY = Max( Start.Y, End.Y );

	if ( AABBLiesWithinViewport( Canvas, MinX, MinY, MaxX - MinX, MaxY - MinY ) )
	{
		// Don't draw the arrowhead if the editor is zoomed out most of the way.
		const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
		if ( Zoom2D < ArrowheadMinZoom )
		{
			bArrowhead = FALSE;
		}

		const FVector2D StartVec( Start.X, Start.Y );
		const FVector2D EndVec( End.X, End.Y );

		// Rough estimate of length of curve. Use direct length and distance between 'handles'. Sure we can do better.
		const FLOAT DirectLength = (EndVec - StartVec).Length();
		const FLOAT HandleLength = ((EndVec - EndDir) - (StartVec + StartDir)).Length();

		const INT NumSteps = appCeil(Max(DirectLength,HandleLength)/MaxPixelsPerStep);

		FVector2D OldPos = StartVec;

		for(INT i=0; i<NumSteps; i++)
		{
			const FLOAT Alpha = ((FLOAT)i+1.f)/(FLOAT)NumSteps;
			const FVector2D NewPos = CubicInterp(StartVec, StartDir, EndVec, EndDir, Alpha);

			const FIntPoint OldIntPos = FIntPoint( appRound(OldPos.x), appRound(OldPos.y) );
			const FIntPoint NewIntPos = FIntPoint( appRound(NewPos.x), appRound(NewPos.y) );

			DrawLine2D( Canvas, OldIntPos, NewIntPos, LineColor );

			// If this is the last section, use its direction to draw the arrowhead.
			if( (i == NumSteps-1) && (i >= 2) && bArrowhead )
			{
				// Go back 3 steps to give us decent direction for arrowhead
				FVector2D ArrowStartPos;

				if(bInterpolateArrowDirection)
				{
					const FLOAT ArrowStartAlpha = ((FLOAT)i-2.f)/(FLOAT)NumSteps;
					ArrowStartPos = CubicInterp(StartVec, StartDir, EndVec, EndDir, ArrowStartAlpha);
				}
				else
				{
					ArrowStartPos = OldPos;
				}

				const FVector2D StepDir = (NewPos - ArrowStartPos).SafeNormal();
				DrawArrowhead( Canvas, NewIntPos, StepDir, LineColor );
			}

			OldPos = NewPos;
		}
	}
}

void FLinkedObjDrawUtils::DrawArrowhead( FCanvas* Canvas, const FIntPoint& Pos, const FVector2D& Dir, const FColor& Color )
{
	// Don't draw the arrowhead if the editor is zoomed out most of the way.
	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
	if ( Zoom2D > ArrowheadMinZoom )
	{
		const FVector2D Orth(Dir.y, -Dir.x);
		const FVector2D PosVec(Pos.X, Pos.Y);
		const FVector2D pt2 = PosVec - (Dir * ArrowheadLength) - (Orth * ArrowheadWidth);
		const FVector2D pt1 = PosVec;
		const FVector2D pt3 = PosVec - (Dir * ArrowheadLength) + (Orth * ArrowheadWidth);
		DrawTriangle2D(Canvas,
			pt1,FVector2D(0,0),
			pt2,FVector2D(0,0),
			pt3,FVector2D(0,0),
			Color,0);
	}
}

FIntPoint FLinkedObjDrawUtils::GetTitleBarSize( FCanvas* Canvas, const TCHAR* Name )
{
	INT XL, YL;
	//StringSize( GEngine->SmallFont, XL, YL, Name );
	// Get Draw String Size
	const INT LabelWidth = XL + (LO_TEXT_BORDER*2) + 4;

	return FIntPoint( Max(LabelWidth, LO_MIN_SHAPE_SIZE), LO_CAPTION_HEIGHT );
}

void FLinkedObjDrawUtils::DrawTitleBar( FCanvas* Canvas, const FIntPoint& Pos, const FIntPoint& Size, const FColor& BorderColor, const FColor& BkgColor, const TCHAR* Name, const TCHAR* Comment/*=NULL*/, const TCHAR* Comment2/*=NULL*/ )
{

	// Draw label at top
	if ( AABBLiesWithinViewport( Canvas, Pos.X, Pos.Y, Size.X, Size.Y ) )
	{
		DrawTile( Canvas, Pos.X,		Pos.Y,		Size.X,		Size.Y,		0.0f,0.0f,0.0f,0.0f, BorderColor );
		DrawTile( Canvas, Pos.X+1,	Pos.Y+1,	Size.X-2,	Size.Y-2,	0.0f,0.0f,0.0f,0.0f, BkgColor );
	}

	if ( Name )
	{
		INT XL, YL;
		//StringSize( GEngine->SmallFont, XL, YL, Name );

		const FIntPoint StringPos( Pos.X+((Size.X-XL)/2), Pos.Y+((Size.Y-YL)/2)+1 );
		if ( AABBLiesWithinViewport( Canvas, StringPos.X, StringPos.Y, XL, YL ) )
		{
			//DrawShadowedString( Canvas, StringPos.X, StringPos.Y, Name, GEngine->SmallFont, FColor(255,255,128) );
		}
	}

	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
	INT CommentY = Pos.Y - 2;
}

FIntPoint FLinkedObjDrawUtils::GetLogicConnectorsSize( FCanvas* Canvas, const FLinkedObjDrawInfo& ObjInfo, INT* InputY/*=NULL*/, INT* OutputY/*=NULL*/ )
{

}

void FLinkedObjDrawUtils::DrawLogicConnectors( FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const FIntPoint& Pos, const FIntPoint& Size, const FLinearColor* ConnectorTileBackgroundColor/*=NULL*/ )
{

}

FIntPoint FLinkedObjDrawUtils::GetVariableConnectorsSize( FCanvas* Canvas, const FLinkedObjDrawInfo& ObjInfo )
{

}

void FLinkedObjDrawUtils::DrawVariableConnectors( FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const FIntPoint& Pos, const FIntPoint& Size, const INT VarWidth )
{

}

void FLinkedObjDrawUtils::DrawLinkedObj( FCanvas* Canvas, FLinkedObjDrawInfo& ObjInfo, const TCHAR* Name, const TCHAR* Comment, const FColor& BorderColor, const FColor& TitleBkgColor, const FIntPoint& Pos )
{

}

INT FLinkedObjDrawUtils::ComputeSliderHeight( INT SliderWidth )
{

}

INT FLinkedObjDrawUtils::Compute2DSliderHeight( INT SliderWidth )
{

}

INT FLinkedObjDrawUtils::DrawSlider( FCanvas* Canvas, const FIntPoint& SliderPos, INT SliderWidth, const FColor& BorderColor, const FColor& BackGroundColor, FLOAT SliderPosition, const FString& ValText, UObject* Obj, int SliderIndex/*=0*/, UBOOL bDrawTextOnSide/*=FALSE*/ )
{

}

INT FLinkedObjDrawUtils::Draw2DSlider( FCanvas* Canvas, const FIntPoint &SliderPos, INT SliderWidth, const FColor& BorderColor, const FColor& BackGroundColor, FLOAT SliderPositionX, FLOAT SliderPositionY, const FString &ValText, UObject *Obj, int SliderIndex, UBOOL bDrawTextOnSide )
{

}

UBOOL FLinkedObjDrawUtils::AABBLiesWithinViewport( FCanvas* Canvas, FLOAT X, FLOAT Y, FLOAT SizeX, FLOAT SizeY )
{
	const FMatrix TransformMatrix = Canvas->GetTransform();
	const FLOAT Zoom2D = GetUniformScaleFromMatrix(Canvas->GetTransform());
	FRenderTarget* RenderTarget = Canvas->GetRenderTarget();
	if ( !RenderTarget )
	{
		return FALSE;
	}

	// Transform the 2D point by the current transform matrix.
	FVector Point(X,Y, 0.0f);
	Point = TransformMatrix * Point;
	X = Point.x;
	Y = Point.y;

	// Check right side.
	if ( X > RenderTarget->GetSizeX() )
	{
		return FALSE;
	}

	// Check left side.
	if ( X+SizeX*Zoom2D < 0.f )
	{
		return FALSE;
	}

	// Check bottom side.
	if ( Y > RenderTarget->GetSizeY() )
	{
		return FALSE;
	}

	// Check top side.
	if ( Y+SizeY*Zoom2D < 0.f )
	{
		return FALSE;
	}

	return TRUE;
}

void FLinkedObjDrawUtils::DrawTile( FCanvas* Canvas,FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,const FLinearColor& Color, UBOOL AlphaBlend /*= 1*/ )
{
	if ( AABBLiesWithinViewport( Canvas, X, Y, SizeX, SizeY ) )
	{
		::DrawTile(Canvas,X,Y,SizeX,SizeY,U,V,SizeU,SizeV,Color, AlphaBlend);
	}
}
//
//void FLinkedObjDrawUtils::DrawTile( FCanvas* Canvas, FLOAT X,FLOAT Y,FLOAT SizeX,FLOAT SizeY,FLOAT U,FLOAT V,FLOAT SizeU,FLOAT SizeV,FMaterialRenderProxy* MaterialRenderProxy )
//{
//
//}

//INT FLinkedObjDrawUtils::DrawString( FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color )
//{
//
//}

//INT FLinkedObjDrawUtils::DrawShadowedString( FCanvas* Canvas,FLOAT StartX,FLOAT StartY,const TCHAR* Text,class UFont* Font,const FLinearColor& Color )
//{
//
//}

FLOAT FLinkedObjDrawUtils::GetUniformScaleFromMatrix( const FMatrix &Matrix )
{
	const FVector XAxis(Matrix[0].x, Matrix[0].y, Matrix[0].z);
	const FVector YAxis(Matrix[1].x, Matrix[1].y, Matrix[1].z);
	const FVector ZAxis(Matrix[2].x, Matrix[2].y, Matrix[2].z);

	FLOAT Scale = Max(XAxis.Length(), YAxis.Length());
	Scale = Max(Scale, ZAxis.Length());

	return Scale;
}
