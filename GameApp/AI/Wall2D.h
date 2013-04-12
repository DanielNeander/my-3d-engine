#ifndef WALL_H
#define WALL_H
//------------------------------------------------------------------------
//
//  Name:   Wall2D.h
//
//  Desc:   class to create and render 2D walls. Defined as the two 
//          vectors A - B with a perpendicular normal. 
//          
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
//#include "2d/noVec3.h"
//#include <fstream>
#include <fstream>
#include "DebugDraw.h"
#include "NavDebugDraw.h"


class Wall2D 
{
protected:

	NavDebugDraw* dd;

	noVec3    m_vA,	m_vB, m_vN;

	void CalculateNormal()
	{
		noVec3 temp = (m_vB - m_vA);
		temp.Normalize();

		m_vN.x = -temp.y;
		m_vN.y = temp.x;
		//m_vN.yUP = 1.0;
	}

public:

	Wall2D()
	{
		dd = new NavDebugDraw();		
		dd->setOffset(0.35f);
	}

	Wall2D(noVec3 A, noVec3 B):m_vA(A), m_vB(B)
	{
		CalculateNormal();
		dd = new NavDebugDraw();
		//dd->setMaterialScript(std::string("NavMeshTestLines"));
		dd->setOffset(0.35f);
	}

	Wall2D(noVec3 A, noVec3 B, noVec3 N):m_vA(A), m_vB(B), m_vN(N)
	{
		dd = new NavDebugDraw();
		//dd->setMaterialScript(std::string("NavMeshTestLines"));
		dd->setOffset(0.35f);
	}

	Wall2D(std::ifstream& in)
	{
		Read(in);
		dd = new NavDebugDraw();
		//dd->setMaterialScript(std::string("NavMeshTestLines"));
		dd->setOffset(0.35f);
	}

	~Wall2D()
	{
		if(dd)
		{
			delete dd;
			dd = NULL;
		}
	}

	virtual void Render(bool RenderNormals = false)const
	{
		//dd->clear();

		//dd->begin(DU_DRAW_LINES_STRIP, 10.0f);

		//duAppendBoxWire(dd, m_vA.x, m_vA.yUP, m_vA.y, m_vB.x, m_vB.yUP, m_vB.y, (unsigned int)0);
		////render the normals if needed
		//if (RenderNormals)
		//{
		//	int MidX = (int)((m_vA.x+m_vB.x)/2);
		//	int MidY = (int)((m_vA.y+m_vB.y)/2);
		//	int MidYUP = (int)((m_vA.yUP+m_vB.yUP)/2);
		//	duAppendBoxWire(dd, MidX, MidYUP, MidY, (int)(MidX+(m_vN.x * 5)), (int)(MidYUP+(m_vN.yUP * 5)), (int)(MidY+(m_vN.y * 5)), (unsigned int)0);
		//}
		//dd->end();
	}

	noVec3 From()const  {return m_vA;}
	void     SetFrom(noVec3 v){m_vA = v; CalculateNormal();}

	noVec3 To()const    {return m_vB;}
	void     SetTo(noVec3 v){m_vB = v; CalculateNormal();}

	noVec3 Normal()const{return m_vN;}
	void     SetNormal(noVec3 n){m_vN = n;}

	noVec3 Center()const{return (m_vA+m_vB)/2.0;}

	std::ostream& Wall2D::Write(std::ostream& os)const
	{
		/*os << std::endl;
		os << From() << ",";
		os << To() << ",";
		os << Normal();*/
		return os;
	}

	void Read(std::ifstream& in)
	{
		/*double x,y;

		in >> x >> y;
		SetFrom(noVec3(x,y));

		in >> x >> y;
		SetTo(noVec3(x,y));

		in >> x >> y;
		SetNormal(noVec3(x,y));*/
	}

};

#endif