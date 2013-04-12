#include "stdafx.h"
#include "DebugDraw.h"
#include "GameApp/AI/NavDebugDraw.h"
#include "GameApp/Util/DrawUtil.h"
#include "CurveLines.h"


void CurveLines::Init( class NavDebugDraw* drawtool )
{
	Draw_ = drawtool;
	Index_ = 0;
	m_NumVerts = 0; //MAX_DRAWLINE;
	AddPoint(vec3_zero);

	CalculateLine();
}

void CurveLines::Update( float deltaTime )
{
	FSMObject::TickState();
}

void CurveLines::Draw( float deltaTime )
{
	Draw_->Draw();
}

void CurveLines::CalculateLine()
{
	m_NumVerts = CoordQueue_.size();
	
	int i=0;
	DebugVertex* ptr = (DebugVertex*)Draw_->LockVB(0);
	while (!CoordQueue_.empty()) {
		noVec3 p = CoordQueue_.front();
		ptr[Index_+i].pos =  noVec3(p.x, p.y, p.z);
		ptr[Index_+i].color = colorCyan;
		CoordQueue_.pop();
		++i;
	}
	Draw_->UnlockVB();
	

	//WORD* pIndices = Draw_->LockIB16();
	//for (int j = 0; j<m_NumVerts; j++)
	//	pIndices[j] = j;
	//Draw_->UnlockIB16();
}

void CurveLines::ClearQueue() {
	while( !CoordQueue_.empty() )
		CoordQueue_.pop();
}

void CurveLines::SetCoordQueue( std::queue<noVec3>* q ) {
	CoordQueue_ = *q;
}

std::queue<noVec3>* CurveLines::GetLastQueue() {
	return &LastQueue_;
}

CurveLines::~CurveLines()
{
	CleanUp();
}

void CurveLines::CleanUp() {
	Draw_ = NULL;
}

void CurveLines::AddPoint( noVec3 p ) {
	CoordQueue_.push(p);
}
