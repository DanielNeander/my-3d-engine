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

class uObject;

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Implements a universal scene for holding all the scenes of the different systems and acts as an
//   interface into the CMM.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class UScene : public IObserver
{
public:

    typedef std::map<System::Type, ISystemScene*>                       SystemScenes;
    typedef SystemScenes::iterator                                      SystemScenesIt;
    typedef SystemScenes::const_iterator                                SystemScenesConstIt;

    typedef std::list<uObject*>                                         Objects;
    typedef Objects::iterator                                           ObjectsIt;
    typedef Objects::const_iterator                                     ObjectsConstIt;


public:

    /// <summary>
    ///   Constructor.
    /// </summary>
    UScene( IChangeManager* pUSceneCCM, IChangeManager* pUObjectCCM );

    /// <summary>
    ///   Destructor.
    /// </summary>
    ~UScene( void );

    /// <summary>
    ///   Creates a new ISystemScene for the passed in ISystem.
    /// </summary>
    /// <param name="pSystem">The system that the scene will be created from.</param>
    /// <returns>The newly ISystemScene.</returns>
    ISystemScene* Extend( ISystem* pSystem );

    /// <summary>
    ///   Destroys a ISystemScene removing it from the UScene.
    /// </summary>
    /// <param name="pScene">The system scene to destroy.</param>
    /// <returns>An error code.</returns>
    Error Unextend( ISystemScene* pScene );

    /// <summary>
    ///   Returns a collection of system scenes.
    /// </summary>
    /// <returns>The system scene collection.</returns>
    const SystemScenes& GetSystemScenes( void ) const
    {
        return m_SystemScenes;
    }

    /// <summary>
    ///   Creates a new UObject that gets attached to this scene.
    /// </summary>
	/// <param name="pszName">The name for the object to use. (default = "")</param>
    /// <returns>The newly created object that has consequently been added to the scene.</returns>
    uObject* CreateObject( pcstr pszName = "" );

    /// <summary>
    ///   Destroys a UObject removing it from the scene.  It also deletes it's CCM.
    /// </summary>
    /// <param name="pObject">The object to remove from the scene and destroy.</param>
    /// <returns>An error code.</returns>
    Error DestroyObject( uObject* pObject );

    uObject* FindObject( pcstr pszName );

    void CreateObjectLink( ISystemObject* pSubject,
                           ISystemObject* pObserver );

    void CreateObjectLink( uObject* pSubject, ISystemObject* pObserver );

    /// <summary>
    ///   Returns a collection of system scenes.
    /// </summary>
    /// <returns>The system scene collection.</returns>
    const Objects& GetObjects( void ) const
    {
        return m_Objects;
    }

    /// <summary cref="IObserver::ChangeOccurred">
    ///   Implementation of the IObserver ChangeOccurred function.
    /// </summary>
    virtual Error ChangeOccurred( ISubject* pSubject,
                                  System::Changes::BitMask SystemChanges );


protected:

    IChangeManager*                         m_pSceneCCM;
    IChangeManager*                         m_pObjectCCM;

    SystemScenes                            m_SystemScenes;
    Objects                                 m_Objects;

	struct ObjectLinkData
	{
		ISubject*			    pSubject;
		IObserver*			    pObserver;
	};
	typedef std::list<ObjectLinkData>	    ObjectLinks;
	typedef ObjectLinks::iterator			ObjectLinksIt;

	ObjectLinks								m_ObjectLinks;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// Implements a universal object that can be extended to add functionality.  By
/// default this universal object does not have any system functionality.
/// </summary>
////////////////////////////////////////////////////////////////////////////////////////////////////

class uObject : public IObserver, public CSubject, public IGeometryObject, public IGraphicsObject
{
    friend class UScene;

    
protected:

    //
    // Constructor.
    //  pszName - the name of this object.
    //
    uObject( UScene* pScene, pcstr pszName = "" );

    //
    // Destructor.
    //
    ~uObject( void );


public:

    typedef std::map<System::Type, ISystemObject*>      SystemObjects;
    typedef SystemObjects::iterator                     SystemObjectsIt;
    typedef SystemObjects::const_iterator               SystemObjectsConstIt;


    //
    // Gets the name of the object.
    //   return - the name of the object.
    //
    pcstr GetName( void )
    {
        return m_sName.c_str();
    }

    //
    // Sets the name of the object.
    //  pszName - the new name of the object.
    //
    void SetName( pcstr pszName )
    {
        m_sName = pszName;
    }

    //
    // Used to extend the objects functionality for a given system.
    //   return - the newly created system object.
    //
    ISystemObject* Extend( ISystemScene* pSystemScene, pcstr pszSystemObjectType );

    /// <summary>
    ///   Used to extend the objects functionality for a given system.
    /// </summary>
    /// <param name="pSystemObject">A pointer to an already created object.</param>
    /// <returns>True is successful, false otherwise.</returns>
    Bool Extend( ISystemObject* pSystemObject );

    //
    // Used to unextend the objects functionality for a given system.
    //
    void Unextend( ISystemScene* pSystemScene );

    /// <summary>
    ///   Returns the system objects map for this UObject.
    /// </summary>
    /// <returns>A constant reference to the system object mapping.</returns>
    const SystemObjects& GetExtensions( void );

    //
    // Gets the specified system object.
    //  System - the system type of the object.
    //   return - this UOBject's system object for the specified type.
    //
    ISystemObject* GetExtension( System::Type System );

    /// <summary cref="IObserver::ChangeOccurred">
    ///   Implementation of the IObserver ChangeOccurred function.
    /// </summary>
    virtual Error ChangeOccurred( ISubject* pSubject,
                                  System::Changes::BitMask SystemChanges );

    /// <summary cref="ISubject::GetPotentialSystemChanges">
    ///   Implementation of the ISubject GetPotentialSystemChanges function.
    /// </summary>
    virtual System::Changes::BitMask GetPotentialSystemChanges( void );

    /// <summary cref="IGeometryObject::GetPosition">
    ///   Implementation of the IGeometryObject GetPosition function.
    /// </summary>
    virtual const Math::Vector3* GetPosition( void );

    /// <summary cref="IGeometryObject::GetOrientation">
    ///   Implementation of the IGeometryObject GetOrientation function.
    /// </summary>
	virtual const Math::Quaternion* GetOrientation( void );

    /// <summary cref="IGeometryObject::GetScale">
    ///   Implementation of the IGeometryObject GetScale function.
    /// </summary>
	virtual const Math::Vector3* GetScale( void );

    /// <summary cref="IGraphicsObject::GetSubMeshCount">
    ///   Implementation of the IGraphicsObject GetSubMeshCount function.
    /// </summary>
    virtual u32 GetSubMeshCount( void );

    /// <summary cref="IGraphicsObject::GetIndexDeclaration">
    ///   Implementation of the IGraphicsObject GetIndexDeclaration function.
    /// </summary>
    virtual u32 GetIndexDeclaration( In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetVertexDeclarationCount">
    ///   Implementation of the IGraphicsObject GetVertexDeclarationCount function.
    /// </summary>
    virtual u32 GetVertexDeclarationCount( In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetVertexDeclaration">
    ///   Implementation of the IGraphicsObject GetVertexDeclaration function.
    /// </summary>
    virtual void GetVertexDeclaration( Out VertexDecl::Element* pVertexDecl,
									   In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetIndexCount">
    ///   Implementation of the IGraphicsObject GetIndexCount function.
    /// </summary>
    virtual u32 GetIndexCount( In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetVertexCount">
    ///   Implementation of the IGraphicsObject GetVertexCount function.
    /// </summary>
    virtual u32 GetVertexCount( In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetIndices">
    ///   Implementation of the IGraphicsObject GetIndices function.
    /// </summary>
    virtual void GetIndices( Out void* pIndices,
                             In  u16 nSubMeshIndex );

    /// <summary cref="IGraphicsObject::GetVertices">
    ///   Implementation of the IGraphicsObject GetVertices function.
    /// </summary>
    virtual void GetVertices( Out void* pVertices,
                              In  u16 nSubMeshIndex,
                              In  u16 nStreamIndex,
                              In  u32 nVertexDeclCount,
                              In  VertexDecl::Element* pVertexDecl );

    /// <summary>
    ///   Implementation of the IGraphicsObject GetStreamsChanged function.
    /// </summary>
    virtual u32 GetStreamsChanged( void );

    /// <summary cref="IGraphicsObject::GetAABB">
    ///   Implementation of the IGraphicsObject GetAABB function.
    /// </summary>
    virtual void GetAABB( Out Math::Vector3& Min, Out Math::Vector3& Max );


protected:

    UScene*                                             m_pScene;
    IChangeManager*                                     m_pObjectCCM;
    std::string                                         m_sName;

    SystemObjects                                       m_ObjectExtensions;
    IGeometryObject*                                    m_pGeometryObject;
    IGraphicsObject*                                    m_pGraphicsObject;
};
