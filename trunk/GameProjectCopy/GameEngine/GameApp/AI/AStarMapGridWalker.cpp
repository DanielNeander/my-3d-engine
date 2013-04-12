// AStarMapGridWalker.cpp: implementation of the AStarMapGridWalker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AStarMapGridWalker.h"
#include <math.h>

#pragma warning( disable : 4786 ) // long name truncated to 255 chars in debug info

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
const std::string AStarMapGridWalker::m_className("A*");

AStarMapGridWalker::AStarMapGridWalker()
{
	m_n = NULL;
	m_nodegrid = NULL;
	m_heuristicWeight = 1.0f;
	m_heuristicMethod = EUCLIDEANDIST;

	m_heuristics[MANHATTEN].type = MANHATTEN;
	m_heuristics[MANHATTEN].name= "Manhatten (dx + dy)";
	m_heuristics[EUCLIDEANDIST].type = EUCLIDEANDIST;
	m_heuristics[EUCLIDEANDIST].name= "Euclidean Distance";
	m_heuristics[MAX_DX_DY].type = MAX_DX_DY;
	m_heuristics[MAX_DX_DY].name= "Max (dx, dy)";
}

AStarMapGridWalker::AStarMapGridWalker(MapGrid *grid) : MapGridWalker(grid) 
{ 
	m_n = NULL;
	m_nodegrid = NULL;
	m_heuristicWeight = 1.0f;
	m_heuristicMethod = EUCLIDEANDIST;

	m_heuristics[MANHATTEN].type = MANHATTEN;
	m_heuristics[MANHATTEN].name= "Manhatten (dx + dy)";
	m_heuristics[EUCLIDEANDIST].type = EUCLIDEANDIST;
	m_heuristics[EUCLIDEANDIST].name= "Euclidean Distance";
	m_heuristics[MAX_DX_DY].type = MAX_DX_DY;
	m_heuristics[MAX_DX_DY].name= "Max (dx, dy)";
}

AStarMapGridWalker::~AStarMapGridWalker()
{
	// reset the node grid
	if(m_nodegrid != NULL)
	{
		for(int i = 0; i < m_grid->getGridSize(); i++)
		{
			delete [] m_nodegrid[i];
		}
		delete [] m_nodegrid;
	}
}

//void AStarMapGridWalker::drawState(CDC* dc, CRect gridBounds)
//{
//	int yspacing = gridBounds.Height() / m_grid->getGridSize();
//	int xspacing = gridBounds.Width() / m_grid->getGridSize();
//
//	AStarMapGridNode* currentnode = m_n;
//
//	// draw line from current node to parent...
//	CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
//
//	CPen *oldobj = (CPen*)dc->SelectObject(pen);
//
//	if(currentnode != NULL)
//		while(currentnode->m_parent != NULL)
//		{
//			dc->MoveTo(currentnode->m_x*xspacing + xspacing/2, currentnode->m_y*yspacing + yspacing/2);
//			dc->LineTo(currentnode->m_parent->m_x*xspacing + xspacing/2, currentnode->m_parent->m_y*yspacing + yspacing/2);
//
//			currentnode = (AStarMapGridNode*)currentnode->m_parent;
//		}
//}

MapGridWalker::WALKSTATETYPE AStarMapGridWalker::iterate()
{
	if(!m_open.isEmpty())
	{	
		m_n = (AStarMapGridNode*)m_open.dequeue();
		if(m_n->equals(*m_end))
		{
			return REACHEDGOAL;
		}

		int x, y;

		// add all adjacent nodes to this node
		// add the east node...
		x = m_n->m_x+1;
		y = m_n->m_y;
		if(m_n->m_x < (m_grid->getGridSize() - 1))
		{
			visitGridNode(x, y);
		}

		// add the south-east node...
		x = m_n->m_x+1;
		y = m_n->m_y+1;
		if(m_n->m_x < (m_grid->getGridSize() - 1) && m_n->m_y < (m_grid->getGridSize() - 1))
		{
			visitGridNode(x, y);
		}

		// add the south node...
		x = m_n->m_x;
		y = m_n->m_y+1;
		if(m_n->m_y < (m_grid->getGridSize() - 1))
		{
			visitGridNode(x, y);
		}

		// add the south-west node...
		x = m_n->m_x-1;
		y = m_n->m_y+1;
		if(m_n->m_y < (m_grid->getGridSize() - 1) && m_n->m_x > 0)
		{
			visitGridNode(x, y);
		}

		// add the west node...
		x = m_n->m_x-1;
		y = m_n->m_y;
		if(m_n->m_x > 0)
		{
			visitGridNode(x, y);
		}

		// add the north-west node...
		x = m_n->m_x-1;
		y = m_n->m_y-1;
		if(m_n->m_x > 0 && m_n->m_y > 0)
		{
			visitGridNode(x, y);
		}

		// add the north node...
		x = m_n->m_x;
		y = m_n->m_y-1;
		if(m_n->m_y > 0)
		{
			visitGridNode(x, y);
		}

		// add the north-east node...
		x = m_n->m_x+1;
		y = m_n->m_y-1;
		if(m_n->m_y > 0 && m_n->m_x < (m_grid->getGridSize() - 1))
		{
			visitGridNode(x, y);
		}
		
		m_closed.enqueue(m_n);

		return STILLLOOKING;
	}
	
	return UNABLETOREACHGOAL;
}

void AStarMapGridWalker::visitGridNode(int x, int y)
{
	int newg;

	// if the node is blocked or has been visited, early out
	if(m_grid->getCost(x, y) == MapGridNode::BLOCKED)
		return;

	// we are visitable
	newg = m_n->m_g + m_grid->getCost(x, y);
	
	if( (m_open.contains(&m_nodegrid[x][y]) || m_closed.contains(&m_nodegrid[x][y])) && m_nodegrid[x][y].m_g <= newg)
	{
		// do nothing... we are already in the queue and we have a cheaper way to get there...
	}
	else
	{
		m_nodegrid[x][y].m_parent = m_n;
		m_nodegrid[x][y].m_g = newg;
		m_nodegrid[x][y].m_h = goalEstimate( &m_nodegrid[x][y] );
		m_nodegrid[x][y].m_f = m_nodegrid[x][y].m_g + m_nodegrid[x][y].m_h;
		
		if(m_closed.contains(&m_nodegrid[x][y]))
		{
			// remove it
			m_closed.remove(&m_nodegrid[x][y]);
		}

		if(!m_open.contains(&m_nodegrid[x][y]))
		{
			m_open.enqueue(&m_nodegrid[x][y]);
		}
		else
		{
			// update this item's position in the queue as its cost has changed
			// and the queue needs to know about it
			m_open.remove(&m_nodegrid[x][y]);
			m_open.enqueue(&m_nodegrid[x][y]);
		}
	}
}

void AStarMapGridWalker::reset()
{
	// empty the Queue
	m_open.makeEmpty();
	m_closed.makeEmpty();

	// reset the node grid
	if(m_nodegrid != NULL)
	{
		for(int i = 0; i < m_grid->getGridSize(); i++)
		{
			delete [] m_nodegrid[i];
		}
		delete [] m_nodegrid;
	}

	m_n = NULL;
	m_nodegrid = NULL;

	m_nodegrid = new AStarMapGridNode*[m_grid->getGridSize()];
	for(int i = 0; i < m_grid->getGridSize(); i++)
	{
		m_nodegrid[i] = new AStarMapGridNode[m_grid->getGridSize()];
		for(int j = 0; j < m_grid->getGridSize(); j++)
		{
			m_nodegrid[i][j].m_x = i;
			m_nodegrid[i][j].m_y = j;
			m_nodegrid[i][j].m_cost = INT_MAX;
		}
	}

	int x, y;
	m_grid->getStart(x, y);
	m_start = &m_nodegrid[x][y];

	m_grid->getEnd(x, y);
	m_end   = &m_nodegrid[x][y];

	m_start->m_g = 0;		
	m_start->m_h = goalEstimate( m_start );
	m_start->m_f = m_start->m_g + m_start->m_h;

	m_open.enqueue(m_start);
}

int AStarMapGridWalker::goalEstimate(AStarMapGridNode *from)
{
	int est = 1000;

	switch(m_heuristicMethod)
	{
	case MAX_DX_DY:
		est = abs(from->m_x - m_end->m_x) > abs(from->m_y - m_end->m_y) ? abs(from->m_x - m_end->m_x) : abs(from->m_y - m_end->m_y);
		break;
	case EUCLIDEANDIST:
		est = (int)floorf(sqrtf((float)(from->m_x - m_end->m_x)*(from->m_x - m_end->m_x) + (float)(from->m_y - m_end->m_y)*(from->m_y - m_end->m_y)));
		break;
	case MANHATTEN:
		est = abs(from->m_x - m_end->m_x) + abs(from->m_y - m_end->m_y);
		break;
	default:
		break;
	}

	return (int)(est*m_heuristicWeight);
}

stringvec AStarMapGridWalker::heuristicTypesSupported() 
{ 
	std::vector<std::string> types;

	for(int i = 0; i < NUM_HEURISTICS; i++)
	{
		types.push_back(m_heuristics[i].name);
	}

	return types;
}

std::string AStarMapGridWalker::getClassDescription()
{
	return m_className;
}
