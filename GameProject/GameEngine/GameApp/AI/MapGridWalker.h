#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable : 4786 ) // long name truncated to 255 chars in debug info

#include "MapGrid.h"
#include "MapGridNode.h"

#include <string>

typedef std::vector<std::string> stringvec;

class MapGridWalker  
{
public:

	typedef enum WALKSTATE { STILLLOOKING, REACHEDGOAL, UNABLETOREACHGOAL } WALKSTATETYPE;

	MapGridWalker();
	MapGridWalker(MapGrid* grid) { m_grid = grid; }
	virtual ~MapGridWalker();

	//virtual void drawState(CDC* dc, CRect gridBounds) = 0;
	virtual WALKSTATETYPE iterate() = 0;
	virtual void reset() = 0;
	virtual bool weightedGraphSupported() { return false; };
	virtual bool heuristicsSupported() { return false; }
	virtual stringvec heuristicTypesSupported() { stringvec empty; return empty; }

	virtual std::string getClassDescription() = 0;

	void setMapGrid(MapGrid *grid) { m_grid = grid; }
	MapGrid *getMapGrid() { return m_grid; }

protected:
	virtual void visitGridNode(int x, int y) = 0;

	MapGrid *m_grid;

};
