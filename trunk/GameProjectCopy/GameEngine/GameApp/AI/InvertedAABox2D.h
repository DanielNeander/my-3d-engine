#ifndef INVAABBOX2D_H
#define INVAABBOX2D_H
//-----------------------------------------------------------------------------
//
//  Name:   InvertedAABBox2D.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   v simple inverted (y increases down screen) axis aligned bounding
//          box class
//-----------------------------------------------------------------------------
#include "DebugDraw.h"

class InvertedAABBox2D
{
private:

	noVec3  m_vTopLeft;
	noVec3  m_vBottomRight;

	noVec3  m_vCenter;

	float	m_minHeight, m_maxHeight;

public:

	InvertedAABBox2D(noVec3 tl,
		noVec3 br):m_vTopLeft(tl),
		m_vBottomRight(br),
		m_vCenter((tl+br)/2.0)
	{
		m_minHeight = m_maxHeight = 0.0f;
	}

	//returns true if the bbox described by other intersects with this one
	bool isOverlappedWith(const InvertedAABBox2D& other)const
	{
		return !((other.Top() > this->Bottom()) ||
			(other.Bottom() < this->Top()) ||
			(other.Left() > this->Right()) ||
			(other.Right() < this->Left()));
	}


	noVec3 TopLeft()const{return m_vTopLeft;}
	noVec3 BottomRight()const{return m_vBottomRight;}

	void	SetMinHeight(float minH) { m_minHeight = minH; }
	void	SetMaxHeight(float maxH) { m_maxHeight = maxH; }
	double    Top()const{return m_vTopLeft.y;}
	double    Left()const{return m_vTopLeft.x;}
	double    Bottom()const{return m_vBottomRight.y;}
	double    Right()const{return m_vBottomRight.x;}
	noVec3 Center()const{return m_vCenter;}

	void     Render(struct duDebugDraw* dd, bool RenderCenter = false, double offSetX = 0.0, double offSetY = 0.0)const
	{
		//dd->clear();
		const noVec2 offSets(offSetX, offSetY);
		dd->begin(DU_DRAW_LINES_STRIP, 10.0f);
		duAppendBoxWire(dd, (float)Left()+offSets.x, m_minHeight, (float)Top()+offSets.y, (float)Right()+offSets.x, m_maxHeight, (float)Bottom()+offSets.y, (unsigned int)0);

		if (RenderCenter)
		{
			duAppendCircle(dd, m_vCenter.x+offSets.x, (m_minHeight + m_maxHeight) * 0.5f, m_vCenter.y+offSets.y, 5, (unsigned int)0);
		}
		dd->end();
	}

};

#endif