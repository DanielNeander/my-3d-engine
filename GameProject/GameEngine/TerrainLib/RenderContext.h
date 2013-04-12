#pragma once 


class RenderContext : public RefCounter
{
public:
	int m_iSize[2];
	float m_fFov[2];
	float m_fClipPlanes[2];
	int m_iViewportPos[2];
	int m_iViewportSize[2];
	float m_fPerspScale[2];
	float m_fOrthographicSize[2];
};