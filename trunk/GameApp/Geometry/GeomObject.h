// Copyright ?2008-2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#pragma once


class GeometrySystem;
class GeometryScene;


///////////////////////////////////////////////////////////////////////////////
/// <summary>
///   Implementation of the IGeometryObject interface.
///   See Interfaces\Geometry.h and Interfaces\System.h for a definition of the
///    class and its functions.
/// </summary>
///////////////////////////////////////////////////////////////////////////////

class GeometryObject : public ISystemObject, public IGeometryObject
{
    friend GeometrySystem;
    friend GeometryScene;

protected:

    GeometryObject( ISystemScene* pSystemScene );
    virtual ~GeometryObject( void );

    /// <summary cref="GeometryObject::GetSystemType">
    ///   Implementation of the ISystemObject GetSystemType function.
    /// </summary>
    /// <seealso cref="ISystemObject::GetSystemType"/>
    virtual System::Type GetSystemType( void );

    virtual Error Initialize( std::vector<Properties::Property> Properties );

    void GetProperties( Properties::Array& Properties );
    void SetProperties( Properties::Array Properties );

    virtual System::Types::BitMask GetDesiredSystemChanges( void );

    /// <summary cref="GeometryObject::ChangeOccurred">
    ///   Implementation of the IObserver ChangeOccurred function.
    /// </summary>
    /// <seealso cref="IObserver::ChangeOccurred"/>
    virtual Error ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType );

    /// <summary cref="GeometryObject::GetPotentialSystemChanges">
    ///   Implementation of the ISubject GetPotentialSystemChanges function.
    /// </summary>
    /// <seealso cref="ISubject::GetPotentialSystemChanges"/>
    virtual System::Changes::BitMask GetPotentialSystemChanges( void );

    /// <summary cref="GeometryObject::GetPosition">
    ///   Implementation of the IGeometryObject GetPosition function.
    /// </summary>
    /// <seealso cref="IGeometryObject::GetPosition"/>
    virtual const Math::Vector3* GetPosition( void );

    /// <summary cref="GeometryObject::GetOrientation">
    ///   Implementation of the IGeometryObject GetOrientation function.
    /// </summary>
    /// <seealso cref="IGeometryObject::GetOrientation"/>
    virtual const Math::Quaternion* GetOrientation( void );

    /// <summary cref="GeometryObject::GetScale">
    ///   Implementation of the IGeometryObject GetScale function.
    /// </summary>
    /// <seealso cref="IGeometryObject::GetScale"/>
    virtual const Math::Vector3* GetScale( void );


protected:

    enum PropertyTypes
    {
        Property_Position, Property_Orientation, Property_Scale,
        Property_Count
    };

    static pcstr                        sm_kapszPropertyNames[];
    static const Properties::Property   sm_kaDefaultProperties[];


private:

    Math::Vector3                   m_Position;
    Math::Quaternion                m_Orientation;
    Math::Vector3                   m_Scale;
};
