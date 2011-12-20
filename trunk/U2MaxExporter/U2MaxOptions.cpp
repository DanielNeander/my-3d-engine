#include "U2MaxOptions.h"

bool U2MaxOptions::ms_bNormal = true;
bool U2MaxOptions::ms_bVertexColor = true;
bool U2MaxOptions::ms_bUVs = true;
bool U2MaxOptions::ms_bTangent = true;
bool U2MaxOptions::ms_bHiddenNodes = false;
//bool U2MaxOptions::ms_bAnim = false;

float U2MaxOptions::ms_fWeightThreshold = 0.0001f;
int U2MaxOptions::ms_iSampleRate = 2;
float U2MaxOptions::ms_fGeomScale = 1.f;

MeshType U2MaxOptions::ms_eMeshType = Shape;

int U2MaxOptions::ms_iMaxBonePaletteSize = 24;

 ExportMode U2MaxOptions::ms_eMode = EXPMODE_BOTH;