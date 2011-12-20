#ifndef U2_MAXNULLVIEW_H
#define U2_MAXNULLVIEW_H
#pragma once 
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
class U2MaxNullView : public View
{
public:
	U2MaxNullView();
	virtual ~U2MaxNullView();

	Point2 ViewToScreen(Point3 p);

};
//-----------------------------------------------------------------------------
/**
*/
U2MaxNullView::U2MaxNullView()
{
	worldToView.IdentityMatrix();
	screenW = 640.0f;
	screenH = 480.0f;
}
//-----------------------------------------------------------------------------
/**
*/
U2MaxNullView::~U2MaxNullView()
{
}
//-----------------------------------------------------------------------------
/**
Project a point in 3D to 2D.
*/
inline
Point2 
U2MaxNullView::ViewToScreen(Point3 p)
{
	return Point2(p.x, p.y);
}
//-----------------------------------------------------------------------------
#endif