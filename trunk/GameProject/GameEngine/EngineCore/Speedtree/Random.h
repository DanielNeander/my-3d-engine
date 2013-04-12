///////////////////////////////////////////////////////////////////////  
//  Random.h
//
//  This class uses the Mersenne Twister algorithm, developed in 1997 
//  by Makoto Matsumoto and Takuji Nishimura. It provides for fast 
//  generation of very high quality random numbers.
//
//  reference:
//  M. Matsumoto and T. Nishimura, "Mersenne twister: A 623-dimensionally
//      equidistributed uniform pseudorandom number generator," ACM Trans. 
//      on Modeling and Computer Simulations, 1998.
//
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      http://www.idvinc.com


/////////////////////////////////////////////////////////////////////
// Preprocessor/Includes

#pragma once
#include "ExportBegin.h"
#include "EngineCore/Types.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    /////////////////////////////////////////////////////////////////////
    // class CRandom

    class ST_STORAGE_CLASS CRandom
    {
    public:
                            CRandom( );
                            CRandom(unsigned int uiSeed);
                            CRandom(const CRandom& cCopy);
                            ~CRandom( );
            
            // seeding
            void            Seed(unsigned int uiSeed);

            // uniform random numbers
            int        GetInteger(int nLow, int nHigh);      // returns [nLow, nHigh-1]
            float      GetFloat(float fLow, float fHigh);
            double      GetDouble(double fLow, double fHigh);

            // gaussian (normal) random numbers with mean = 0 and stddev = 1
            float      GetGaussianFloat(void);
            double      GetGaussianDouble(void);

    protected:
            void            Reload(void);
            unsigned int       GetRawInteger(void);
            unsigned int       Twist(unsigned int uiPrime, unsigned int uiInput0, unsigned int uiInput1) const;

    protected:
            enum { SIZE = 624, PERIOD = 397 };
            unsigned int       m_uiSeed;
            unsigned int       m_auiTable[SIZE + 1];
            unsigned int*      m_pNext; 
            unsigned int       m_nCount;
    };

    // include inlined functions
    #include "Random.inl"

} // end namespace SpeedTree


#include "ExportEnd.h"


