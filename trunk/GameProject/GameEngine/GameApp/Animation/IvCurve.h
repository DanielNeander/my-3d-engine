#pragma once 


class IvLinear
{
public:
	// constructor/destructor
	IvLinear();
	~IvLinear();

	// text output (for debugging)
	friend std::ostream& operator<<( std::ostream& out, const IvLinear& source );

	// set up
	bool Initialize( const noVec3* samples, const float* times,
		unsigned int count );

	// clean up
	void Clean();

	// evaluate position
	noVec3 Evaluate( float t );

	// render curve
	void Render();

protected:
	noVec3*      mPositions; // sample points
	float*          mTimes;     // time to arrive at each point
	unsigned int    mCount;     // number of points and times

private:
	// copy operations
	// made private so they can't be used
	IvLinear( const IvLinear& other );
	IvLinear& operator=( const IvLinear& other );
};

class IvCatmullRom
{
public:
	// constructor/destructor
	IvCatmullRom();
	~IvCatmullRom();

	// text output (for debugging)
	friend std::ostream& operator<<( std::ostream& out, const IvCatmullRom& source );

	// set up
	bool Initialize( const noVec3* positions, 
		const float* times,
		unsigned int count );

	// break down
	void Clean();

	// evaluate position
	noVec3 Evaluate( float t );

	// evaluate derivative
	noVec3 Velocity( float t );

	// evaluate second derivative
	noVec3 Acceleration( float t );

	// find parameter that moves s distance from Q(t1)
	float FindParameterByDistance( float t1, float s );

	// return length of curve between t1 and t2
	float ArcLength( float t1, float t2 );

	// get total length of curve
	inline float GetLength() { return mTotalLength; }

	// render curve
	void Render();

protected:
	// return length of curve between u1 and u2
	float SegmentArcLength( uint32 i, float u1, float u2 );

	noVec3*      mPositions;     // sample positions
	float*          mTimes;         // time to arrive at each point
	float*          mLengths;       // length of each curve segment
	float           mTotalLength;   // total length of curve
	unsigned int    mCount;         // number of points and times

private:
	// copy operations
	// made private so they can't be used
	IvCatmullRom( const IvCatmullRom& other );
	IvCatmullRom& operator=( const IvCatmullRom& other );
};

class IvLagrange
{
public:
	// constructor/destructor
	IvLagrange();
	~IvLagrange();

	// text output (for debugging)
	friend std::ostream& operator<<( std::ostream& out, const IvLagrange& source );

	// set up
	bool Initialize( const noVec3* samples, const float* times,
		unsigned int count );

	// destroy
	void Clean();

	// evaluate position
	noVec3 Evaluate( float t );

	// render curve
	void Render();

protected:
	noVec3*      mPositions; // sample points
	float*          mTimes;     // time to arrive at each point
	float*          mDenomRecip;// 1/lagrange denomenator for each point
	unsigned int    mCount;     // number of points and times

private:
	// copy operations
	// made private so they can't be used
	IvLagrange( const IvLagrange& other );
	IvLagrange& operator=( const IvLagrange& other );
};


class IvHermite
{
public:
	// constructor/destructor
	IvHermite();
	~IvHermite();

	// text output (for debugging)
	friend std::ostream& operator<<( std::ostream& out, const IvHermite& source );

	// set up
	// default
	bool Initialize( const noVec3* positions, 
		const noVec3* inTangents,
		const noVec3* outTangents,
		const float* times,
		unsigned int count );
	// clamped spline
	bool InitializeClamped( const noVec3* positions, 
		const float* times,
		unsigned int count,
		const noVec3& inTangent,
		const noVec3& outTangent );
	// natural spline
	bool InitializeNatural( const noVec3* positions, 
		const float* times,
		unsigned int count );
	// cyclic spline
	bool InitializeCyclic( const noVec3* positions, 
		const float* times,
		unsigned int count );
	// acyclic spline
	bool InitializeAcyclic( const noVec3* positions, 
		const float* times,
		unsigned int count );

	// destroy
	void Clean();

	// evaluate position
	noVec3 Evaluate( float t );

	// evaluate derivative
	noVec3 Velocity( float t );

	// evaluate second derivative
	noVec3 Acceleration( float t );

	// find parameter that moves s distance from Q(t1)
	float FindParameterByDistance( float t1, float s );

	// return length of curve between t1 and t2
	float ArcLength( float t1, float t2 );

	// get total length of curve
	inline float GetLength() { return mTotalLength; }

	// render curve
	void Render();

protected:
	// return length of curve between u1 and u2
	float SegmentArcLength( uint32 i, float u1, float u2 );

	noVec3*      mPositions;     // sample positions
	noVec3*      mInTangents;    // incoming tangents on each segment
	noVec3*      mOutTangents;   // outgoing tangents on each segment
	float*          mTimes;         // time to arrive at each point
	float*          mLengths;       // length of each curve segment
	float           mTotalLength;   // total length of curve
	unsigned int    mCount;         // number of points and times

private:
	// copy operations
	// made private so they can't be used
	IvHermite( const IvHermite& other );
	IvHermite& operator=( const IvHermite& other );
};


class IvUniformBSpline
{
public:
	// constructor/destructor
	IvUniformBSpline();
	~IvUniformBSpline();

	// text output (for debugging)
	friend std::ostream& operator<<( std::ostream& out, const IvUniformBSpline& source );

	// set up
	bool Initialize( const noVec3* positions, 
		unsigned int count,
		float startTime, float endTime );

	// remove data
	void Clean();

	// evaluate position
	noVec3 Evaluate( float t );

	// evaluate derivative
	noVec3 Velocity( float t );

	// evaluate second derivative
	noVec3 Acceleration( float t );

	// find parameter that moves s distance from Q(t1)
	float FindParameterByDistance( float t1, float s );

	// return length of curve between t1 and t2
	float ArcLength( float t1, float t2 );

	// get total length of curve
	inline float GetLength() { return mTotalLength; }

	// render curve
	void Render();

protected:
	// return length of curve between u1 and u2
	float SegmentArcLength( uint32 i, float u1, float u2 );

	noVec3*      mPositions;     // positions of control points
	float*          mTimes;         // time to approach each point
	float*          mLengths;       // length of each curve segment
	float           mTotalLength;   // total length of curve
	unsigned int    mCount;         // number of points and times

private:
	// copy operations
	// made private so they can't be used
	IvUniformBSpline( const IvUniformBSpline& other );
	IvUniformBSpline& operator=( const IvUniformBSpline& other );
};

struct BezierControlPoint
{
	noVec3 Point;
	float Weight;
};

class IvBezier
{
public:


	// constructor/destructor
	IvBezier();
	~IvBezier();

	// text output (for debugging)
	friend std::ostream& operator<<( std::ostream& out, const IvBezier& source );

	// set up
	bool Initialize( const noVec3* positions, 
		const noVec3* controls,
		const float* times,
		unsigned int count );

	// automatically create control points
	bool Initialize( const noVec3* positions, 
		const float* times,
		unsigned int count );
		
	bool AddControlPoint(const noVec3& pt, float weight);
	void CalcDeCasteljau(class CurveLines *Lines, int NumSamples);
	BezierControlPoint * GetControlPoint(int _Index);
	BezierControlPoint * GetCurrentControlPoint();
	float	GetPointByDist(float d, float currU, noVec3* pos, noVec3* dir);
	noVec3 GetPoint(float _T);
	void CalculateLength();

	// clean out
	void Clean();

	// evaluate position
	noVec3 Evaluate( float t );

	// evaluate velocity
	noVec3 Velocity( float t );

	// evaluate acceleration
	noVec3 Acceleration( float t );

	// find parameter that moves s distance from Q(t1)
	float FindParameterByDistance( float t1, float s );

	// return length of curve between 1 and u2
	float ArcLength( float t1, float t2 );

	// get total length of curve
	inline float GetLength() { return mTotalLength; }

	// render curve
	void Render();


	idList<BezierControlPoint>	mPointList;	

protected:
	void SubdivideRender( const noVec3& P0, const noVec3& P1, 
		const noVec3& P2, const noVec3& P3 );
	float SubdivideLength( const noVec3& P0, const noVec3& P1, 
		const noVec3& P2, const noVec3& P3 );

	// return length of curve between u1 and u2
	float SegmentArcLength( uint32 i, float u1, float u2 );

	noVec3*      mPositions;     // interpolating positions
	noVec3*      mControls;      // approximating positions
	float*          mTimes;         // time to arrive at each point
	float*          mLengths;       // length of each curve segment
	float           mTotalLength;   // total length of curve
	unsigned int    mCount;         // number of points and times

	float	mLength;

private:
	// copy operations
	// made private so they can't be used
	//IvBezier( const IvBezier& other );
	//IvBezier& operator=( const IvBezier& other );
};