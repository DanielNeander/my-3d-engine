inline U2LineComp::~U2LineComp()
{
	m_eType = CT_EMPTY;
	m_fMin = FLT_MAX;
	m_fMax = -FLT_MAX;
}

inline void U2LineComp::SetInterval (float fMin, float fMax)
{
	m_eType = GetTypeFromInterval(fMin, fMax);
	m_fMin = fMin;
	m_fMax = fMax;
}


inline U2LineComp& U2LineComp::operator= (const U2LineComp& rkComponent)
{
	m_eType = rkComponent.m_eType;
	m_fMin = rkComponent.m_fMin;
	m_fMax = rkComponent.m_fMax;
	return *this;
}



// Determine the type of an interval without having to create an instance
// of a LinComp object.
inline int U2LineComp::GetTypeFromInterval (float fMin, float fMax)
{
	if (fMin < fMax)
	{
		if (fMax == FLT_MAX)
		{
			if (fMin == -FLT_MAX)
			{
				return CT_LINE;
			}
			else
			{
				return CT_RAY;
			}
		}
		else
		{
			if (fMin == -FLT_MAX)
			{
				return CT_RAY;
			}
			else
			{
				return CT_SEGMENT;
			}
		}
	}
	else if (fMin == fMax)
	{
		if (fMin != -FLT_MAX && fMax != FLT_MAX)
		{
			return CT_POINT;
		}
	}

	return CT_EMPTY;
}


inline bool U2LineComp::IsCanonical () const
{
	if (m_eType == CT_RAY)
	{
		return m_fMin == 0.f && m_fMax == FLT_MAX;
	}

	if (m_eType == CT_SEGMENT)
	{
		return m_fMin == -m_fMax;
	}

	if (m_eType == CT_POINT)
	{
		return m_fMin == 0.f; 
	}

	if (m_eType == CT_EMPTY)
	{
		return m_fMin == FLT_MAX
			&& m_fMax == -FLT_MAX;
	}

	// m_eType == CT_LINE
	return true;

}

// access the interval [min,max]
inline float U2LineComp::GetMin () const
{
	return m_fMin;
}

inline float U2LineComp::GetMax () const
{
	return m_fMax;
}

// Determine if the specified parameter is in the interval.
inline bool U2LineComp::Contains (float fParam) const
{
	return m_fMin <= fParam && fParam <= m_fMax;
}
