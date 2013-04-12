///////////////////////////////////////////////////////////////////////  
//  Extents.h
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
//
//  *** Release version 5.0 ***


/////////////////////////////////////////////////////////////////////
// Preprocessor

#pragma once
#include "ExportBegin.h"
#include "CoordSys.h"
#include "EngineCore/Math/Vector.h"
#include <cfloat>
#include <cassert>


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    /////////////////////////////////////////////////////////////////////
    // class CExtents
    //
    // Represents an axis-aligned bounding box

    class ST_STORAGE_CLASS CExtents
    {
    public:
                            CExtents( );
                            CExtents(const float afExtents[6]); // [0-2] = min(x,y,z), [3-5] = max(x,y,z)
                            CExtents(const noVec3& cMin, const noVec3& cMax);
                            ~CExtents( );                                           

            void            Reset(void);
            void            SetToZeros(void);
            void            Order(void);
            bool            Valid(void) const;

            void            ExpandAround(const float afPoint[3]);
            void            ExpandAround(const noVec3& vPoint);
            void            ExpandAround(const noVec3& vPoint, float fRadius);
            void            ExpandAround(const CExtents& cOther);
            void            Scale(float fScale);
            void            Translate(const noVec3& vTranslation);
            void            Rotate(float fRadians);    // around 'up' axis

            float      ComputeRadiusFromCenter3D(void);
            float      ComputeRadiusFromCenter2D(void);

            const noVec3&     Min(void) const;
            const noVec3&     Max(void) const;
            float      Midpoint(unsigned int uiAxis) const;
            noVec3            GetCenter(void) const;
            noVec3            GetDiagonal(void) const;

                            operator float*(void);
                            operator const float*(void) const;

    private:
            noVec3            m_cMin;
            noVec3            m_cMax;
    };

    // include inline functions
    #include "EngineCore/SpeedTree/Extents.inl"

} // end namespace SpeedTree


#include "EngineCore/SpeedTree/ExportEnd.h"



