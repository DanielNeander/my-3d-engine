#ifndef __CURVE_EDITOR_H__
#define __CURVE_EDITOR_H__

#include "FSM.h"
#include "GameApp/Animation/IvCurve.h"
#include "GameApp/Animation/CurveLines.h"


class InterpControl
{
public:
	void SetupLinear();
	void SetupBezier();
	void SetupLagrange();
	void SetupHermite();
	void SetupUniformBSpline();
	void SetupCatmullRom();

	float DistanceSinusoid( float t, float k1, float k2 );
	float DistanceParabolic( float t, float k1, float k2 );


	IvLinear    mLinearInterp;
	IvLagrange	mLagrangeCurve;
	IvHermite	mHermiteCurve;
	IvCatmullRom mCatmullRomCurve;
	IvBezier	mBezierCurve;
	IvUniformBSpline	mUBSpline;

	noVec3*  mSamplePositions;   // sample positions
	float*      mSampleTimes;       // arrival times
	noVec3   mIn, mOut;          // in, out tangents
};

enum CurveType {
	CURVE_LINEAR,
	CURVE_BEZIER,
	CURVE_LAGRANGE,
	CURVE_HERMITE,
	CURVE_BSPLINE,
	CURVE_CATMULROM,
	CURVE_MAX,
};



class CurveEditor : public FSMObject {
public:	
	CurveEditor();
	~CurveEditor();

	void Init();
	void	Update(float fDeltaTime);
	void	ProcessInput(float fDeltaTime);		
	void	Draw(float fDeltaTime);
	void AddBezierPoint();	
	void MoveCurrPoint(noVec3 trans);
	void CalcBezierLengths();
	float GetNextBezierPosition(float d, float currU, noVec3* pos, noVec3* dir);
	void Clear();

	bool TickIdle();
	bool TickEdit();
	bool TickMove();
private:
	

protected:
	void CreateNewBezier();


private:
		
	int	PointCnt;
	std::list<CurveLines>	LineList;
	std::list<IvBezier>		BezierList;
	IvBezier*	CurrBezier;
	IvBezier*	LastBezier;
	CurveLines* CurrLines;
	CurveLines* LastLines;
	idStr		StrEditMode;

	class NavDebugDraw* Draw_;
};



#endif