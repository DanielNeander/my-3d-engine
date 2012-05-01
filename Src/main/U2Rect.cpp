#include <U2_3D/src/U23DLibPCH.h>
#include "U2Rect.h"



template <class T>
inline T U2Rect<T>::GetWidth() const
{
	return m_right > m_left ? (m_right - m_left) : (m_left - m_right);
}
//---------------------------------------------------------------------------
template <class T>
inline T U2Rect<T>::GetHeight() const
{
	return m_top > m_bottom ? (m_top - m_bottom) : (m_bottom - m_top);
}