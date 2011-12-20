/**************************************************************************************************
module	:	U2MaxOptions
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_MAXOPTIONS_H
#define U2_MAXOPTIONS_H


enum MeshType {
	None      = 0,  /// 0,  none of selected in radiobutton of max script ui
	Shape     = 1,  /// static or skinned mesh 
	Swing     = 2,  /// swing shape node
	Shadow    = 3,  /// shadow mesh
	Collision = 4,  /// collision mesh only
	Particle2 = 5,  /// nparticleshapenode2
	Sky       = 6,  /// sky node, this will contain some sky elements
};

enum ExportMode 
{
	EXPMODE_MESH, 
	EXPMODE_ANIM,
	EXPMODE_BOTH,
	NUM_EXPMODE,
};

class U2MaxOptions 
{
public:	
		

	static bool ms_bNormal;
	static bool ms_bVertexColor;
	static bool ms_bUVs;
	static bool ms_bTangent;
	static bool ms_bHiddenNodes;
	//static bool ms_bAnim;

	static float ms_fWeightThreshold;
	static int ms_iSampleRate;
	static float ms_fGeomScale;

	static MeshType ms_eMeshType;

	static int ms_iMaxBonePaletteSize;
	
	static ExportMode ms_eMode;
	
};

#endif