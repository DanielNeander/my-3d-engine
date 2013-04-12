#include "stdafx.h"
#include "DebugDraw.h"
#include "GameApp/AI/NavDebugDraw.h"
#include "CurveEditor.h"
#include "Animation/IvCurve.h"
#include "Font/Dx11TextHelper.h"
#include "GameApp/GameApp.h"




void InterpControl::SetupLinear()
{
	noVec3 samplePositions[4];
	samplePositions[0].Set( -6.0f, 3.0f, 0.0f );
	samplePositions[1].Set( 0.0f, 0.0f, 0.0f );
	samplePositions[2].Set( -3.0f, -3.0f, 0.0f );
	samplePositions[3].Set( 6.0f, 0.0f, 0.0f );

	float sampleTimes[4];
	sampleTimes[0] = 0.0f;
	sampleTimes[1] = 2.0f;
	sampleTimes[2] = 6.0f;
	sampleTimes[3] = 9.0f;

	mLinearInterp.Initialize( samplePositions, sampleTimes, 4 );
}

void InterpControl::SetupBezier()
{
	noVec3* samplePositions = new noVec3[4];
	samplePositions[0].Set( -6.0f, 3.0f, 0.0f );
	samplePositions[1].Set( 0.0f, 0.0f, 0.0f );
	samplePositions[2].Set( -3.0f, -3.0f, 0.0f );
	samplePositions[3].Set( 6.0f, 0.0f, 0.0f );

	noVec3* controls = new noVec3[6];
	controls[0].Set( -3.0f, 3.0f, 0.0f );
	controls[1].Set( -0.5f, 1.5f, 0.0f );
	controls[2].Set( 0.5f, -1.5f, 0.0f );
	controls[3].Set( -2.0f, -1.5f, 0.0f );
	controls[4].Set( 0.0f, -5.0f, 0.0f );
	controls[5].Set( 4.0f, -3.0f, 0.0f );

	float* sampleTimes = new float[4];
	sampleTimes[0] = 0.0f;
	sampleTimes[1] = 2.0f;
	sampleTimes[2] = 6.0f;
	sampleTimes[3] = 9.0f;

	mBezierCurve.Initialize( samplePositions, controls, sampleTimes, 4 );

	delete [] samplePositions;
	delete [] sampleTimes;
}

void InterpControl::SetupLagrange()
{
	noVec3* samplePositions = new noVec3[4];
	samplePositions[0].Set( -6.0f, 3.0f, 0.0f );
	samplePositions[1].Set( 0.0f, 0.0f, 0.0f );
	samplePositions[2].Set( -3.0f, -3.0f, 0.0f );
	samplePositions[3].Set( 6.0f, 0.0f, 0.0f );

	float* sampleTimes = new float[4];
	sampleTimes[0] = 0.0f;
	sampleTimes[1] = 2.0f;
	sampleTimes[2] = 6.0f;
	sampleTimes[3] = 9.0f;

	mLagrangeCurve.Initialize( samplePositions, sampleTimes, 4 );

	delete [] samplePositions;
	delete [] sampleTimes;
}

void InterpControl::SetupHermite()
{
	noVec3* samplePositions = new noVec3[4];
	samplePositions[0].Set( -6.0f, 3.0f, 0.0f );
	samplePositions[1].Set( 0.0f, 0.0f, 0.0f );
	samplePositions[2].Set( -3.0f, -3.0f, 0.0f );
	samplePositions[3].Set( 6.0f, 0.0f, 0.0f );

	noVec3* sampleTangents = new noVec3[4];
	sampleTangents[0].Set( 2.0f, -1.0f, 0.0f );
	sampleTangents[1].Set( 1.0f, -2.0f, 0.0f );
	sampleTangents[2].Set( 2.0f, 0.0f, 0.0f );
	sampleTangents[3].Set( 0.0f, 5.0f, 0.0f );

	float* sampleTimes = new float[4];
	sampleTimes[0] = 0.0f;
	sampleTimes[1] = 2.0f;
	sampleTimes[2] = 6.0f;
	sampleTimes[3] = 9.0f;

	mHermiteCurve.Initialize( samplePositions, sampleTangents, sampleTangents+1, 
		sampleTimes, 4 );

	delete [] samplePositions;
	delete [] sampleTangents;
	delete [] sampleTimes;
}

void InterpControl::SetupUniformBSpline()
{
	noVec3* samplePositions = new noVec3[4];
	samplePositions[0].Set( -6.0f, 3.0f, 0.0f );
	samplePositions[1].Set( 0.0f, 0.0f, 0.0f );
	samplePositions[2].Set( -3.0f, -3.0f, 0.0f );
	samplePositions[3].Set( 6.0f, 0.0f, 0.0f );

	mUBSpline.Initialize( samplePositions, 4, 0.0f, 9.0f );

	delete [] samplePositions;

}

void InterpControl::SetupCatmullRom()
{
	noVec3* samplePositions = new noVec3[4];
	samplePositions[0].Set( -6.0f, 3.0f, 0.0f );
	samplePositions[1].Set( 0.0f, 0.0f, 0.0f );
	samplePositions[2].Set( -3.0f, -3.0f, 0.0f );
	samplePositions[3].Set( 6.0f, 0.0f, 0.0f );

	float* sampleTimes = new float[4];
	sampleTimes[0] = 0.0f;
	sampleTimes[1] = 2.0f;
	sampleTimes[2] = 6.0f;
	sampleTimes[3] = 9.0f;

	mCatmullRomCurve.Initialize( samplePositions, sampleTimes, 4 );

	delete [] samplePositions;
	delete [] sampleTimes;
}

float InterpControl::DistanceSinusoid( float t, float k1, float k2 )
{
	// make sure ranges are correct
	assert( 0.0f <= t && t <= 1.0f );
	assert( 0.0f <= k1 && k1 <= k2 && k1 <= 1.0f );
	assert( 0.0f <= k2 && k2 <= 1.0f );

	const float HALF_PIrecip = 1.0f/noMath::HALF_PI;
	float f = k1*HALF_PIrecip + k2 - k1 + (1.0f - k2)*HALF_PIrecip;
	float result;
	if ( t < k1 )
	{ 
		result = k1*HALF_PIrecip*(noMath::Sin(t/k1*noMath::HALF_PI-noMath::HALF_PI) + 1.0f);
	}
	else if (t > k2 )
	{
		result = k1*HALF_PIrecip + k2 - k1 +
			(1.0f - k2)*HALF_PIrecip*noMath::Sin(((t-k2)/(1.0f-k2))*noMath::HALF_PI);
	}
	else
	{
		result = k1*HALF_PIrecip + t - k1;
	}

	return result/f;

}   // End of Player::DistanceSinusoid()


//-------------------------------------------------------------------------------
// @ Player::DistanceParabolic()
//-------------------------------------------------------------------------------
// Parabolic speed control
//-------------------------------------------------------------------------------
float InterpControl::DistanceParabolic( float t, float k1, float k2 )
{
	// make sure ranges are correct
	assert( 0.0f <= t && t <= 1.0f );
	assert( 0.0f <= k1 && k1 <= k2 && k1 <= 1.0f );
	assert( 0.0f <= k2 && k2 <= 1.0f );

	// calculate maximum velocity
	float v0 = 2.0f/(k2 - k1 + 1.0f);

	float result = 0.0f;
	if ( t < k1 )
	{ 
		result = 0.5f*v0*t*t/k1;
	}
	else if (t > k2 )
	{
		result = 0.5f*v0*k1 + v0*(k2-k1) + (v0 - 0.5f*(v0*(t-k2)/(1.0f-k2)))*(t - k2);
	}
	else
	{
		result = 0.5f*v0*k1 + v0*(t-k1);
	}

	return result;

}   // End of Player::DistanceParabolic()



DEFINE_STATE(STATE_CURVE_EDIT, CurveEditor, TickEdit, "CurveEdit");
DEFINE_STATE(STATE_CURVE_MOVE, CurveEditor, TickMove,	"CurveMove");
DEFINE_STATE(STATE_IDLE, CurveEditor, TickIdle, "CurveIdle");
CurveEditor::CurveEditor()
{
	PointCnt = 0;
	LastBezier = NULL;
	StrEditMode = "Curve Idle";
}

CurveEditor::~CurveEditor() {

	SAFE_DELETE(Draw_);	
}


void CurveEditor::ProcessInput( float fDeltaTime ) {
	float speed = 1.0f;

	float x = GetCamera()->GetDir().x * speed * (fDeltaTime / 0.17f);
	float z = GetCamera()->GetDir().z * speed * (fDeltaTime / 0.17f);
	float y = speed * (fDeltaTime / 0.17f);

	if (GetKeyDown(VK_UP)) 
		this->MoveCurrPoint(noVec3(-x, 0.0f, -z));

	if (GetKeyDown(VK_DOWN)) 
		this->MoveCurrPoint(noVec3(x, 0.0f, z));

	if (GetKeyDown(VK_LEFT)) 
		this->MoveCurrPoint(noVec3(z, 0.0f, -x));

	if (GetKeyDown(VK_RIGHT)) 
		this->MoveCurrPoint(noVec3(-z, 0.0f,x));

	if (GetKeyDown(VK_PRIOR)) 
		this->MoveCurrPoint(noVec3(0.0f,y,0.0f));

	if (GetKeyDown(VK_NEXT)) 
		this->MoveCurrPoint(noVec3(0.0f, -y, 0.0f));
}

void CurveEditor::Draw( float fDeltaTime ) {
	if (!LineList.empty()) {
		std::list<CurveLines>::iterator it = LineList.begin();
		//Draw_->m_CurrVertex = 0;
		//for( ; it != LineList.end(); it++) 			
		//	Draw_->m_CurrVertex += it->m_NumVerts; 
		
		it = LineList.begin();
		Draw_->m_BaseVertex = 0;
		int baseIndex = 0; 
		for( ; it != LineList.end(); it++) {						
			baseIndex += it->m_NumVerts;
			Draw_->m_BaseVertex = baseIndex - it->m_NumVerts;
			Draw_->m_CurrVertex = it->m_NumVerts;
			it->Draw(fDeltaTime);
		}
		
	}	

	GetApp()->text_->Begin();
	GetApp()->text_->SetInsertionPos(512, 10);
	GetApp()->text_->DrawFormattedTextLine("Curve Mode : %s LineCount : %d Num Point : %d", StrEditMode.c_str(), LineList.size(), CurrBezier->mPointList);
	GetApp()->text_->End();

}

void CurveEditor::AddBezierPoint() {
	BezierControlPoint* pCp = CurrBezier->GetCurrentControlPoint();
	BezierControlPoint cp;
	if (!pCp) 
		cp.Point = noVec3(0.0f, 2.0f, 0.0f);
	else 
		cp = *pCp;

	
	//if the last point was the final point on the curve
	if ((PointCnt-1)%3 == 0 && PointCnt != 1) {
		LastBezier = CurrBezier;
		LastLines = CurrLines;

		CreateNewBezier();

		//add two new points, one for the start of the line, and one that can be moved
		CurrBezier->AddControlPoint(cp.Point,1);
		CurrBezier->AddControlPoint(cp.Point,1);
	} else {
		//if not, then just add a point to the current bezier
		CurrBezier->AddControlPoint(cp.Point,1);			
	}
	PointCnt++;
}

void CurveEditor::MoveCurrPoint( noVec3 trans ) {
	BezierControlPoint* cp = CurrBezier->GetCurrentControlPoint();
	if(!cp) return;

	// move the point
	cp->Point += trans;

	//Draw_->m_CurrVertex -= CurrLines->m_NumVerts;
	CurrBezier->CalcDeCasteljau(CurrLines,100);

	if ((PointCnt-1)%3 == 1 && LastBezier != NULL) {


		//if (&(*LineList.begin()) != LastLines)  {		

			//assume the two beziers are called a and b.
			//get a(n-1) and a(n)
			BezierControlPoint *a2 = LastBezier->GetControlPoint(2);
			BezierControlPoint *a3 = LastBezier->GetControlPoint(3);

			//take the distance from a(n-1) to a(n).		
			float distSqr = (a3->Point - a2->Point).LengthSqr();
			float dist = noMath::Sqrt(noMath::Fabs(distSqr));
		
			//get the direction from b(0) to a(n)
			noVec3 dir = a3->Point - cp->Point;
			dir.Normalize();
			//if the direction is zero it will cause problems, so return and dont update this til the next frame
			if (dir == vec3_zero) return;

			//find the point that is in this direction, Dist away from a(n)
			noVec3 newPoint = a3->Point + dist * dir;

			// set a2 to this point
			a2->Point = newPoint;
						
			LastBezier->CalcDeCasteljau(LastLines, 100);		
		//}
	}

	int baseIndex = 0; 
	std::list<CurveLines>::iterator it = LineList.begin();

	for( ; it != LineList.end(); it++) {
		baseIndex += it->m_NumVerts;
		it->Index_ = baseIndex - it->m_NumVerts;
	}
}

void CurveEditor::CalcBezierLengths() {
	std::list<IvBezier>::iterator it = BezierList.begin();
	for( ; it != BezierList.end(); it++) {
		it->CalculateLength();
	}
}

float CurveEditor::GetNextBezierPosition( float d, float currU, noVec3* pos, noVec3* dir ) {
	int bezier = floor(currU);

	noVec3 origPos = *pos;
	int i=0;
	std::list<IvBezier>::iterator it;
	for(it = BezierList.begin(); it != BezierList.end(); it++) {
		if (i == bezier) {
			float u = it->GetPointByDist(d, currU - float(i), pos, dir);

			currU = float(i) + u;
			bezier = floor(currU);
			//if u is 1.0f or more then the object has moved onto the next bezier so dont
			//return and allow it to iterate onto the next bezier
			if (u < 1.0f) return currU;
			//if it has not returned then update _D so as to not move too far allong the
			//next bezier
			noVec3 diff = *pos - origPos;
			float len = diff.Length();
			d -= len;
		}
		i++;
	}
	return 0.0f;
}

void CurveEditor::Clear() {
	BezierList.clear();

	std::list<CurveLines>::iterator it = LineList.begin();
	for( ; it != LineList.end(); it++) {
		it->CleanUp();
	}

	LineList.clear();

	PointCnt = 0;
	LastBezier = NULL;
	CreateNewBezier();

}

void CurveEditor::CreateNewBezier() {
	IvBezier newBezier;
	BezierList.push_back(newBezier);

	CurrBezier = &BezierList.back();

	CurveLines newLine;	
	newLine.Init(Draw_);
	LineList.push_back(newLine);
	
	std::list<CurveLines>::iterator it = LineList.begin();
	Draw_->m_CurrVertex = 0;
	int baseIndex = 0; 
	for( ; it != LineList.end(); it++) { 
		baseIndex += it->m_NumVerts;
		it->Index_ = baseIndex - it->m_NumVerts;
	}
	CurrLines = &LineList.back();
}

void CurveEditor::Init() {

	Draw_ = new NavDebugDraw();
	//ddMain->setOffset(0.15f);		
	Draw_->CreateVertexBuffer(NULL, 13684, sizeof(DebugVertex));	
	//Draw_->CreateIndexBuffer(NULL, MAX_DRAWLINE);
	Draw_->SetupShader();
	Draw_->m_Prim = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
	
	CreateNewBezier();

	TransitionState(&STATE_IDLE);
}

void CurveEditor::Update( float fDeltaTime ) {

	TickState();

	if (!LineList.empty()) {
		std::list<CurveLines>::iterator it = LineList.begin();
		for( ; it != LineList.end(); it++) {
			it->Update(fDeltaTime);
		}
	}	
}

bool CurveEditor::TickIdle() {
	if (GetKeyDown('1')) {
		TransitionState(&STATE_CURVE_EDIT);
		StrEditMode = "Curve Edit";
	}
	if (GetKeyDown('2')) {
		TransitionState(&STATE_CURVE_MOVE);
		StrEditMode = "Curve Move";
	}

	return true;
}

bool CurveEditor::TickEdit() {
	static bool bDown = false;	
	if (GetKeyDown('A') && !bDown) 
	{
		AddBezierPoint();
		bDown = true;		
	}
	if (GetKeyDown('1')) 
	{
		TransitionState(&STATE_IDLE);
		StrEditMode = "Curve Idle";
	}
	if (GetKeyDown('2')) 
	{
		TransitionState(&STATE_CURVE_MOVE);
		StrEditMode = "Curve Move";
	}
	if (GetKeyDown('3')) 
	{
		bDown = false;
	}

	return true;
}

bool CurveEditor::TickMove() {
	ProcessInput(GetApp()->ddt / 1000.f);

	if (GetKeyDown('1')) 
	{
		TransitionState(&STATE_IDLE);
		StrEditMode = "Curve Idle";

	}
	if (GetKeyDown('2')) 
	{
		TransitionState(&STATE_CURVE_EDIT);
		StrEditMode = "Curve Edit";
	}

	return true;
}

