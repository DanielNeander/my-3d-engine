///////////////////////////////////////////////////////////////////////  
//  CRandom.inl
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement
//
//      Copyright (c) 2003-2006 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      http://www.idvinc.com


/////////////////////////////////////////////////////////////////////
// Preprocessor/Includes

#ifdef _WIN32 // Windows or Xbox 360
    #pragma warning(disable : 4146)
#endif


/////////////////////////////////////////////////////////////////////
// CRandom::CRandom

inline CRandom::CRandom( )
{
    Seed(0);
}


/////////////////////////////////////////////////////////////////////
// CRandom::CRandom

inline CRandom::CRandom(unsigned int uiSeed)
{
    Seed(uiSeed);
}


/////////////////////////////////////////////////////////////////////
// CRandom::CRandom

inline CRandom::CRandom(const CRandom& cCopy)
{
    Seed(cCopy.m_uiSeed);
}


/////////////////////////////////////////////////////////////////////
// CRandom::~CRandom

inline CRandom::~CRandom( )
{

}


/////////////////////////////////////////////////////////////////////
// CRandom::Seed

inline void CRandom::Seed(unsigned int uiSeed)
{
    m_uiSeed = uiSeed;
    memset(m_auiTable, 0, (SIZE + 1) * sizeof(unsigned int));
    unsigned int* pS = m_auiTable;
    unsigned int* pR = m_auiTable;

    *pS = m_uiSeed & 0xffffffff;
    ++pS;
    for (unsigned int i = 1; i < SIZE; ++i)
    {
        *pS = (1812433253U * (*pR ^ (*pR >> 30)) + i) & 0xffffffff;
        ++pS;
        ++pR;
    }
    Reload( );
}


/////////////////////////////////////////////////////////////////////
// CRandom::GetInteger

inline int CRandom::GetInteger(int nLow, int nHigh)
{
     return (GetRawInteger( ) % (nHigh - nLow) + nLow);
}


/////////////////////////////////////////////////////////////////////
// CRandom::GetFloat

inline float CRandom::GetFloat(float fLow, float fHigh)
{
    return ((float(GetRawInteger( )) + 0.5f) * (1.0f / 4294967296.0f)) * (fHigh - fLow) + fLow;
}


/////////////////////////////////////////////////////////////////////
// CRandom::GetDouble

inline double CRandom::GetDouble(double fLow, double fHigh)
{
    return ((double(GetRawInteger( )) + 0.5) * (1.0 / 4294967296.0)) * (fHigh - fLow) + fLow;
}


/////////////////////////////////////////////////////////////////////
// CRandom::GetGaussianFloat

inline float CRandom::GetGaussianFloat(void)
{
    float fX1 = 0.0f;
    float fX2 = 0.0f;
    float fW = 1.0f;
 
    while (fW >= 1.0f)
    {
        fX1 = 2.0f * ((float(GetRawInteger( )) + 0.5f) * (1.0f / 4294967296.0f)) - 1.0f;
        fX2 = 2.0f * ((float(GetRawInteger( )) + 0.5f) * (1.0f / 4294967296.0f)) - 1.0f;
        fW = fX1 * fX1 + fX2 * fX2;
    }

    return (fX1 * sqrt((-2.0f * log(fW)) / fW));
}


/////////////////////////////////////////////////////////////////////
// CRandom::GetGaussianDouble

inline double CRandom::GetGaussianDouble(void)
{
    double fX1 = 0.0;
    double fX2 = 0.0;
    double fW = 1.0;
 
    while (fW >= 1.0)
    {
        fX1 = 2.0 * ((double(GetRawInteger( )) + 0.5) * (1.0 / 4294967296.0)) - 1.0;
        fX2 = 2.0 * ((double(GetRawInteger( )) + 0.5) * (1.0 / 4294967296.0)) - 1.0;
        fW = fX1 * fX1 + fX2 * fX2;
    }

    return (fX1 * sqrt((-2.0 * log(fW)) / fW));
}


/////////////////////////////////////////////////////////////////////
// CRandom::Reload

inline void CRandom::Reload(void)
{
    unsigned int* pTemp = m_auiTable;

    for (unsigned int i = SIZE - PERIOD; i > 0; --i)
    {
        *pTemp = Twist(pTemp[PERIOD], pTemp[0], pTemp[1]);
        ++pTemp;
    }

    for (unsigned int i = PERIOD; i > 0; --i)
    {
        *pTemp = Twist(pTemp[PERIOD - SIZE], pTemp[0], pTemp[1]);
        ++pTemp;
    }

    *pTemp = Twist(pTemp[PERIOD - SIZE], pTemp[0], m_auiTable[0]);

    m_nCount = SIZE;
    m_pNext = m_auiTable;
}


/////////////////////////////////////////////////////////////////////
// CRandom::GetRawInteger

inline unsigned int CRandom::GetRawInteger(void)
{
    if (!m_nCount) 
        Reload( );

    --m_nCount;
        
    unsigned int uiTemp;
    uiTemp = *m_pNext;
    ++m_pNext;
    uiTemp ^= (uiTemp >> 11);
    uiTemp ^= (uiTemp << 7) & 0x9d2c5680;
    uiTemp ^= (uiTemp << 15) & 0xefc60000;
    uiTemp ^= (uiTemp >> 18);

    return uiTemp;
}


/////////////////////////////////////////////////////////////////////
// CRandom::Twist

inline unsigned int CRandom::Twist(unsigned int uiPrime, unsigned int uiInput0, unsigned int uiInput1) const
{
    return (uiPrime ^ (((uiInput0 & 0x80000000) | (uiInput1 & 0x7fffffff)) >> 1) ^ (-(uiInput1 & 0x00000001) & 0x9908b0df));
}
