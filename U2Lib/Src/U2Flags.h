/************************************************************************
module	:	U2Flags
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once 
#ifndef U2_FLAGS_H
#define U2_FLAGS_H

#define U2DeclareFlags(type) \
	private: \
	type m_uFlags; \
		void SetField(type uVal, type uMask, type uPos) \
		{ \
		m_uFlags = (m_uFlags & ~uMask) | (uVal << uPos); \
		} \
		type GetField(type uMask, type uPos) const \
		{ \
		return (m_uFlags & uMask) >> uPos; \
		} \
		void SetBit(bool bVal, type uMask) \
		{ \
		if (bVal) \
			{ \
			m_uFlags |= uMask; \
			} \
			else \
			{ \
			m_uFlags &= ~uMask; \
			} \
		}\
		bool GetBit(type uMask) const \
		{ \
		return (m_uFlags & uMask) != 0; \
		}

#endif