///////////////////////////////////////////////////////////////////////  
//  CoordSys.h
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com
//
//  *** Release version 5.0 ***


///////////////////////////////////////////////////////////////////////  
//  Preprocessor

#pragma once
#include "ExportBegin.h"
#include "EngineCore/Math/Matrix.h"
#include <cassert>


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  Class CCoordSysBase

    class ST_STORAGE_CLASS CCoordSysBase
    {
    public:
    virtual                         ~CCoordSysBase( )                                   { }

    virtual bool                    IsLeftHanded(void) const = 0;
    virtual bool                    IsYAxisUp(void) const = 0;

    virtual noVec3                    ConvertToStd(float x, float y, float z) const = 0;
            noVec3                    ConvertToStd(const float afCoord[3]) const     { return ConvertToStd(afCoord[0], afCoord[1], afCoord[2]); }
    virtual noVec3                    ConvertFromStd(float x, float y, float z) const = 0;
            noVec3                    ConvertFromStd(const float afCoord[3]) const   { return ConvertFromStd(afCoord[0], afCoord[1], afCoord[2]); }

    virtual const noVec3&             OutAxis(void) const = 0;
    virtual const noVec3&             RightAxis(void) const = 0;
    virtual const noVec3&             UpAxis(void) const = 0;

    virtual float              OutComponent(float x, float y, float z) const = 0;
    virtual float              RightComponent(float x, float y, float z) const = 0;
    virtual float              UpComponent(float x, float y, float z) const = 0;

    virtual void                    RotateUpAxis(noMat3& mMatrix, float fRadians) const = 0;
    virtual void                    RotateUpAxis(noMat4& mMatrix, float fRadians) const = 0;
    virtual void                    RotateOutAxis(noMat3& mMatrix, float fRadians) const = 0;
    virtual void                    RotateOutAxis(noMat4& mMatrix, float fRadians) const = 0;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CCoordSys

    class ST_STORAGE_CLASS CCoordSys
    {
    public:
            enum ECoordSysType
            {
                COORD_SYS_RIGHT_HANDED_Z_UP,
                COORD_SYS_RIGHT_HANDED_Y_UP,
                COORD_SYS_LEFT_HANDED_Z_UP,
                COORD_SYS_LEFT_HANDED_Y_UP,
                COORD_SYS_CUSTOM
            };

    static  void                    SetCoordSys(ECoordSysType eType, const CCoordSysBase* pCustomConverter = NULL);
    static  ECoordSysType           GetCoordSysType(void);
    static  bool                    IsDefaultCoordSys(void) { return true; }
    static  const CCoordSysBase*    GetBuiltInConverter(ECoordSysType eType);

    static  bool                    IsLeftHanded(void);
    static  bool                    IsYAxisUp(void);

    static  noVec3                    ConvertToStd(const float afCoord[3]) { { return noVec3(afCoord[0], afCoord[1], afCoord[2]); }}
    static  noVec3                    ConvertToStd(float x, float y, float z) { return noVec3(x, y, z); }
    static  noVec3                    ConvertFromStd(const float afCoord[3]) { { return noVec3(afCoord[0], afCoord[1], afCoord[2]); }}
    static  noVec3                    ConvertFromStd(float x, float y, float z) { return noVec3(x, y, z); }

    static  const noVec3&             OutAxis(void);
    static  const noVec3&             RightAxis(void);
    static  const noVec3&             UpAxis(void);

    static  float              OutComponent(const float afCoord[3]);
    static  float              OutComponent(float x, float y, float z);
    static  float              RightComponent(const float afCoord[3]);
    static  float              RightComponent(float x, float y, float z);
    static  float              UpComponent(const float afCoord[3]);
    static  float              UpComponent(float x, float y, float z);

    static  void                    RotateUpAxis(noMat3& mMatrix, float fRadians);
    static  void                    RotateUpAxis(noMat4& mMatrix, float fRadians);
    static  void                    RotateOutAxis(noMat3& mMatrix, float fRadians);
    static  void                    RotateOutAxis(noMat4& mMatrix, float fRadians);

    static  const CCoordSysBase*    m_pCoordSys;
    static  ECoordSysType           m_eCoordSysType;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CDefaultCoordinateSystem
    //
    //  The default coordinate system is right-handed with the +Z pointing "up."
    //  The conversion functions in this class will essentially be no-ops.

    // CRHCS_Zup
    class CDefaultCoordinateSystem : public CCoordSysBase
    {
    public:
    virtual                         ~CDefaultCoordinateSystem( )                                    { }

            bool                    IsLeftHanded(void) const                                        { return false; }
            bool                    IsYAxisUp(void) const                                           { return false; }

            // conversions for this coordinate system are symmetric (it's the default)
            noVec3                    ConvertToStd(float x, float y, float z) const    { return noVec3(x, y, z); }
            noVec3                    ConvertFromStd(float x, float y, float z) const  { return ConvertToStd(x, y, z); }

            const noVec3&             OutAxis(void) const                                             { return m_vOut; }
            const noVec3&             RightAxis(void) const                                           { return m_vRight; }
            const noVec3&             UpAxis(void) const                                              { return m_vUp; }

            // first assignment in these functions is to quiet unreferenced parameter warnings
            float              OutComponent(float /*x*/, float y, float /*z*/) const    { return y; }
            float              RightComponent(float x, float /*y*/, float /*z*/) const  { return x; }
            float              UpComponent(float /*x*/, float /*y*/, float z) const     { return z; }

            /*void                    RotateUpAxis(noMat3& mMatrix, float fRadians) const        { mMatrix.RotateZ(fRadians); }
            void                    RotateUpAxis(noMat4& mMatrix, float fRadians) const        { mMatrix.RotateZ(fRadians); }
            void                    RotateOutAxis(noMat3& mMatrix, float fRadians) const       { mMatrix.RotateY(fRadians); }
            void                    RotateOutAxis(noMat4& mMatrix, float fRadians) const       { mMatrix.RotateY(fRadians); }*/

    private:
            static  const noVec3      m_vOut;
            static  const noVec3      m_vRight;
            static  const noVec3      m_vUp;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CRHCS_Yup
    //
    //  Right-handed coordinate system, with +Y axis pointing up

    static const noVec3 c_vRHCS_Yup_Out = noVec3(0.0f, 0.0f, -1.0f);
    static const noVec3 c_vRHCS_Yup_Right = noVec3(1.0f, 0.0f, 0.0f);
    static const noVec3 c_vRHCS_Yup_Up = noVec3(0.0f, 1.0f, 0.0f);

    class CRHCS_Yup : public CCoordSysBase
    {
    public:
    virtual                 ~CRHCS_Yup( )                                           { }

            bool            IsLeftHanded(void) const                                { return false; }
            bool            IsYAxisUp(void) const                                   { return true; }

            // conversions for this coordinate system are NOT symmetric
            noVec3            ConvertToStd(float x, float y, float z) const           { return noVec3(x, -z, y); }
            noVec3            ConvertFromStd(float x, float y, float z) const         { return noVec3(x, z, -y); }

            const noVec3&     OutAxis(void) const                                     { return c_vRHCS_Yup_Out; }
            const noVec3&     RightAxis(void) const                                   { return c_vRHCS_Yup_Right; }
            const noVec3&     UpAxis(void) const                                      { return c_vRHCS_Yup_Up; }

            float           OutComponent(float /*x*/, float /*y*/, float z) const   { return -z; }
            float           RightComponent(float x, float /*y*/, float /*z*/) const { return x; }
            float           UpComponent(float /*x*/, float y, float /*z*/) const    { return y; }

           /* void            RotateUpAxis(noMat3& mMatrix, float fRadians) const     { mMatrix.RotateY(fRadians); }
            void            RotateUpAxis(noMat4& mMatrix, float fRadians) const     { mMatrix.RotateY(fRadians); }
            void            RotateOutAxis(noMat3& mMatrix, float fRadians) const    { mMatrix.RotateZ(fRadians); }
            void            RotateOutAxis(noMat4& mMatrix, float fRadians) const    { mMatrix.RotateZ(fRadians); }*/
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CLHCS_Yup
    //
    //  Left-handed coordinate system, with +Y axis pointing up

    static const noVec3 c_vLHCS_Yup_Out = noVec3(0.0f, 0.0f, 1.0f);
    static const noVec3 c_vLHCS_Yup_Right = noVec3(1.0f, 0.0f, 0.0f);
    static const noVec3 c_vLHCS_Yup_Up = noVec3(0.0f, 1.0f, 0.0f);

    class CLHCS_Yup : public CCoordSysBase
    {
    public:
    virtual                 ~CLHCS_Yup( )                                           { }

            bool            IsLeftHanded(void) const                                { return true; }
            bool            IsYAxisUp(void) const                                   { return true; }

            // conversions for this coordinate system are symmetric
            noVec3            ConvertToStd(float x, float y, float z) const           { return noVec3(x, z, y); }
            noVec3            ConvertFromStd(float x, float y, float z) const         { return ConvertToStd(x, y, z); }

            const noVec3&     OutAxis(void) const                                     { return c_vLHCS_Yup_Out; }
            const noVec3&     RightAxis(void) const                                   { return c_vLHCS_Yup_Right; }
            const noVec3&     UpAxis(void) const                                      { return c_vLHCS_Yup_Up; }

            float           OutComponent(float /*x*/, float /*y*/, float z) const   { return z; }
            float           RightComponent(float x, float /*y*/, float /*z*/) const { return x; }
            float           UpComponent(float /*x*/, float y, float /*z*/) const    { return y; }

            //void            RotateUpAxis(noMat3& mMatrix, float fRadians) const     { mMatrix.RotateY(fRadians); }
            //void            RotateUpAxis(noMat4& mMatrix, float fRadians) const     { mMatrix.RotateY(fRadians); }
            //void            RotateOutAxis(noMat3& mMatrix, float fRadians) const    { mMatrix.RotateZ(fRadians); }
            //void            RotateOutAxis(noMat4& mMatrix, float fRadians) const    { mMatrix.RotateZ(fRadians); }
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CLHCS_Zup
    //
    //  Left-handed coordinate system, with +Z axis pointing up

    static const noVec3 c_vLHCS_Zup_Out = noVec3(0.0f, -1.0f, 0.0f);
    static const noVec3 c_vLHCS_Zup_Right = noVec3(1.0f, 0.0f, 0.0f);
    static const noVec3 c_vLHCS_Zup_Up = noVec3(0.0f, 0.0f, 1.0f);

    class CLHCS_Zup : public CCoordSysBase
    {
    public:
    virtual                 ~CLHCS_Zup( )                                           { }

            bool            IsLeftHanded(void) const                                { return true; }
            bool            IsYAxisUp(void) const                                   { return false; }

            // conversions for this coordinate system are symmetric
            noVec3            ConvertToStd(float x, float y, float z) const           { return noVec3(x, -y, z); }
            noVec3            ConvertFromStd(float x, float y, float z) const         { return ConvertToStd(x, y, z); }

            const noVec3&     OutAxis(void) const                                     { return c_vLHCS_Zup_Out; }
            const noVec3&     RightAxis(void) const                                   { return c_vLHCS_Zup_Right; }
            const noVec3&     UpAxis(void) const                                      { return c_vLHCS_Zup_Up; }

            float           OutComponent(float /*x*/, float y, float /*z*/) const   { return -y; }
            float           RightComponent(float x, float /*y*/, float /*z*/) const { return x; }
            float           UpComponent(float /*x*/, float /*y*/, float z) const    { return z; }

       /*     void            RotateUpAxis(noMat3& mMatrix, float fRadians) const     { mMatrix.RotateZ(fRadians); }
            void            RotateUpAxis(noMat4& mMatrix, float fRadians) const     { mMatrix.RotateZ(fRadians); }
            void            RotateOutAxis(noMat3& mMatrix, float fRadians) const    { mMatrix.RotateY(fRadians); }
            void            RotateOutAxis(noMat4& mMatrix, float fRadians) const    { mMatrix.RotateY(fRadians); }*/
    };

} // end namespace SpeedTree


#include "ExportEnd.h"

