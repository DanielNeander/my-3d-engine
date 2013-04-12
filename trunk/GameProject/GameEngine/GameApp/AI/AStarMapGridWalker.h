// AStarMapGridWalker.h: interface for the AStarMapGridWalker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ASTARMAPGRIDWALKER_H__402877CF_C6F9_4BA3_B0B5_F607EBE15907__INCLUDED_)
#define AFX_ASTARMAPGRIDWALKER_H__402877CF_C6F9_4BA3_B0B5_F607EBE15907__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MapGridWalker.h"

#pragma warning( disable : 4786 ) // long name truncated to 255 chars in debug info

class AStarMapGridWalker : public MapGridWalker  
{
public:
	typedef enum HEURISTICMETHOD { MANHATTEN, EUCLIDEANDIST, MAX_DX_DY, NUM_HEURISTICS } HEURISTICMETHODTYPE;

	struct HEURISTICMETHODSTRUCT
	{
		 HEURISTICMETHODTYPE type;
		 std::string name;
	};

	AStarMapGridWalker();
	AStarMapGridWalker(MapGrid* grid);
	virtual ~AStarMapGridWalker();

	//virtual void drawState(CDC* dc, CRect gridBounds);
	virtual WALKSTATETYPE iterate();
	virtual void reset();
	virtual bool weightedGraphSupported() { return true; }

	int goalEstimate(AStarMapGridNode *from);

	void setHeuristic(const HEURISTICMETHODTYPE method) { m_heuristicMethod = method; }
	HEURISTICMETHODTYPE getHeuristic() const { return m_heuristicMethod; }
	void setHeuristicWeight(const float weight) { m_heuristicWeight = weight; }
	float getHeuristicWeight() const { return m_heuristicWeight; }

	virtual bool heuristicsSupported() { return true; }
	virtual stringvec heuristicTypesSupported();

	virtual std::string getClassDescription();

	AStarMapGridNode *PathEnd() { return m_n; }

	const static std::string m_className;

	HEURISTICMETHODSTRUCT m_heuristics[NUM_HEURISTICS];

protected:
	virtual void visitGridNode(int x, int y);

private:
	MapGridPriorityQueue m_open;
	MapGridPriorityQueue m_closed;
	AStarMapGridNode *m_start, *m_n, *m_end;
	AStarMapGridNode **m_nodegrid;

	HEURISTICMETHODTYPE m_heuristicMethod;
	float m_heuristicWeight;

};

#endif // !defined(AFX_ASTARMAPGRIDWALKER_H__402877CF_C6F9_4BA3_B0B5_F607EBE15907__INCLUDED_)
