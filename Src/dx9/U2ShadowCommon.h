#pragma once 

#include <U2_3D/src/collision/U2Aabb.h>

const float ZFAR_MAX = 800.0f;
const float ZNEAR_MIN = 1.0f;
const int TEXDEPTH_HEIGHT_20 = 1536;
const int TEXDEPTH_WIDTH_20 = 1536;
const int TEXDEPTH_SIZE_11 = 1024;
const float W_EPSILON = 0.001f;
const float SMQUAD_SIZE = 800.0f;
const int NUM_OBJECTS = 40;
const float VIEW_ANGLE = 60.f;

void GetTerrainBoundingBox( std::vector<U2Aabb>* shadowReceivers, const D3DXMATRIX* modelView, const U2Culler* sceneFrustum );

