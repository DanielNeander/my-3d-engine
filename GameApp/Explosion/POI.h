// Copyright © 2008-2009 Intel Corporation
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

/// <summary>
/// POIType enum: denotes the type of point of interest
/// </summary>
namespace POIType {
    enum POIType
    {
        e_POI_Contact,

        e_POI_None
    };
};

/*******************************************************************************
* CLASS: POI (Point of Interest)
*
* DESCRIPTION:
* POI is used to track points of interest that might affect Explosions.
*******************************************************************************/

/// <summary>
/// POI class: POI is used to track points of interest that might affect Explosions.
/// </summary>
class POI
{
public:
	POI( void )
    {
        m_Position = Math::Vector3::Zero;
        m_Type = POIType::e_POI_None;
    }

    ~POI( void ) {}

	inline Math::Vector3 GetPosition( void ) { return m_Position; }
    inline void          SetPosition( Math::Vector3& Position ) { m_Position = Position; }
    inline void          SetPosition( const Math::Vector3& Position ) { m_Position = Position; }

    inline POIType::POIType       GetType( void ) { return m_Type; }
    
    virtual inline bool  Expired( void ) { return True; }

protected:
    Math::Vector3 m_Position;    // Position of POI
    POIType::POIType       m_Type;        // Type of POI
};


/*******************************************************************************
* CLASS: POIContact (Point of Interest: Contact)
*
* DESCRIPTION:
* POIContact is used to track contact points (i.e. collisions) that might 
* affect Explosionss.
*******************************************************************************/

/// <summary>
/// POIContact class: Used to track contact points (i.e. collisions) that might 
/// </summary>
class POIContact : public POI
{
public:
	POIContact( void )
    {
        m_Position = Math::Vector3::Zero;
		m_Normal = Math::Vector3::Zero;
        m_Type = POIType::e_POI_Contact;
        m_Impact = 0.0f;
        m_bStatic = false;
    }

    ~POIContact( void ) {}

    inline f32  GetImpact( void ) { return m_Impact; }
    inline void SetImpact( f32 Impact ) { m_Impact = Impact; }
    inline void SetStatic( Bool bStatic ) { m_bStatic = bStatic; }
    inline Bool IsStatic( void ) { return m_bStatic; }
	inline Math::Vector3 GetVelocityObjectA( void ) { return m_VelocityObjectA; }
	inline void SetVelocityObjectA( Math::Vector3 velocityA ) { m_VelocityObjectA = velocityA; }
	inline Math::Vector3 GetVelocityObjectB( void ) { return m_VelocityObjectB; }
	inline void SetVelocityObjectB( Math::Vector3 velocityB ) { m_VelocityObjectB = velocityB; }
	inline Math::Vector3 GetNormal( void ) { return m_Normal; }
	inline void SetNormal( Math::Vector3 normal ) { m_Normal = normal; }

protected:
    f32 m_Impact;  // Impulse of collision
    Bool m_bStatic;
	Math::Vector3 m_VelocityObjectA;
	Math::Vector3 m_VelocityObjectB;
	Math::Vector3 m_Normal;
};
