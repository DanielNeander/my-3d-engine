/**************************************************************************************************
module	:	U2Rect
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#ifndef U2_RECT_H
#define U2_RECT_H



template <class T> class U2Rect  : public U2MemObj
{
public:
	U2Rect (T left = T(0), T right = T(0), T top = T(0), T bottom = T(0))
	{
		m_left = left;
		m_right = right;
		m_top = top;
		m_bottom = bottom;
	}

	
	T m_left, m_right, m_top, m_bottom;

	T GetWidth() const;
	T GetHeight() const;

	
};


#endif 
