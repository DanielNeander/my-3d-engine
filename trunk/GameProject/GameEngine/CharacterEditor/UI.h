#pragma once 

enum EUIWidgetFace
{
	UIFACE_Left             =0,
	UIFACE_Top              =1,
	UIFACE_Right            =2,
	UIFACE_Bottom           =3,
	UIFACE_MAX              =4,
};

enum EPositionEvalType
{
	EVALPOS_None            =0,
	EVALPOS_PixelViewport   =1,
	EVALPOS_PixelScene      =2,
	EVALPOS_PixelOwner      =3,
	EVALPOS_PercentageViewport=4,
	EVALPOS_PercentageOwner =5,
	EVALPOS_PercentageScene =6,
	EVALPOS_MAX             =7,
};