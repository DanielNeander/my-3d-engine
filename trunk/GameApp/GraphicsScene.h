#pragma once 

#include "GameApp/Light/ray.h"

#include ".\BaseTypes\TbbSpinMutex.h"


class SceneNode;
class SHLight;
class BSP;

class GraphicsSystem;
class GraphicsTask;
class GraphicsObject;

// Macro to convert from Math::Vector3 to Ogre::Vector3
#define TONOVEC(Vec)  (noVec3((Vec).x, (Vec).y, (Vec).z) )
// Macro to convert from Math::Quaternion to Ogre::Quaternion
#define TONOQUAT(Vec)	(noQuat((Vec).w, (Vec).x, (Vec).y, (Vec).z) )

#define MAX_NUM_JOBS 32

struct ProcessData
{
	std::list<GraphicsObject*> m_Objects;    // Objects to process
	f32                            m_DeltaTime;  // Delta time (needed for Update calls)
};

class GraphicsScene : public ISystemScene 
{
	friend GraphicsSystem;
	friend GraphicsTask;

public:
	DECLARE_STATIC_SPIN_MUTEX(m_mutex);

public:
	/// <summary cref="OGREGraphicsScene::Update">
	///   This function must be called every frame.  It updates the graphics scene.
	/// </summary>
	/// <param name="DeltaTime">Elapsed time since the last frame.</param>
	/// <seealso cref="ISystemTask::Update"/>
	virtual void Update( f32 DeltaTime );


	SceneNode*	GetRootSceneNode( void ) {
		return m_pSceneRoot;
	}
		
protected:
	GraphicsScene( ISystem* pSystem );
	~GraphicsScene( void );

	/// <summary cref="OGREGraphicsScene::GetSystemType">
	///   Implementation of the <c>ISystemScene::GetSystemType</c> function.
	/// </summary>
	/// <returns>System::Type - Type of this system.</returns>
	/// <seealso cref="ISystemScene::GetSystemType"/>
	virtual System::Type GetSystemType( void );

	/// <summary cref="OGREGraphicsScene::GlobalSceneStatusChanged">
	///   Called from the framework to inform the scene extension of the overall scene status.
	/// </summary>
	/// <param name="Status">GlobalSceneStatus - The overall scene status.</param>
	virtual void GlobalSceneStatusChanged( GlobalSceneStatus Status );

	/// <summary cref="OGREGraphicsScene::Initialize">
	///   Implementation of the <c>ISystemScene::Initialize</c> function.
	///   One time initialization function for the scene.
	/// </summary>
	/// <param name="Properties">Initializes the scene with the properties specified by <paramref name="Properties"/>.</param>
	/// <returns>Error.</returns>
	/// <seealso cref="ISystemScene::Initialize"/>
	virtual Error Initialize( Properties::Array Properties );
	

	/// <summary cref="OGREGraphicsScene::GetProperties">
	///   Implementation of the <c>ISystemScene::GetProperties</c> function.
	///   Gets the properties of this scene.
	/// </summary>
	/// <param name="Properties">Gets the properties of the scene</param>
	/// <seealso cref="ISystemScene::GetProperties"/>
	virtual void GetProperties( Properties::Array& Properties );

	/// <summary cref="OGREGraphicsScene::SetProperties">
	///   Implementation of the <c>ISystemScene::SetProperties</c> function.
	///   Sets the properties for this scene.
	/// </summary>
	/// <param name="Properties">Sets the properties of the scene</param>
	/// <seealso cref="ISystem::SetProperties"/>
	virtual void SetProperties( Properties::Array Properties );

	/// <summary cref="OGREGraphicsScene::GetObjectTypes">
	///   Implementation of the <c>ISystemScene::GetObjectTypes</c> function.
	///   Get all the available object types as names.
	/// </summary>
	/// <returns>pcstr* - A NULL terminated array of object type names.</returns>
	/// <seealso cref="ISystemScene::GetObjectTypes"/>
	virtual pcstr* GetObjectTypes( void );

	/// <summary cref="OGREGraphicsScene::CreateObject">
	///   Implementation of the <c>ISystemScene::CreateObject</c> function.
	///   Creates a system object used to extend a UObject.
	/// </summary>
	/// <param name="pszName">The unique name for this object.</param>
	/// <param name="pszType">The object type to create.</param>
	/// <returns>ISystemObject* - The newly created system object.</returns>
	/// <seealso cref="ISystemScene::CreateObject"/>
	virtual ISystemObject* CreateObject( pcstr pszName, pcstr pszType );


	/// <summary cref="OGREGraphicsScene::DestroyObject">
	///   Implementation of the <c>ISystemScene::DestroyObject</c> function.
	///   Destroys a system object.
	/// </summary>
	/// <param name="pSystemObject">The system object to destroy.</param>
	/// <returns>Error - Any error codes.</returns>
	/// <seealso cref="ISystemScene::DestroyObject"/>
	virtual Error DestroyObject( ISystemObject* pSystemObject );

	/// <summary cref="OGREGraphicsScene::GetSystemTask">
	///   Implementation of the <c>ISystemScene::GetSystemTask</c> function.
	///   Returns a pointer to the task that this scene needs to perform on its objects.
	/// </summary>
	/// <returns>ISystemTask* - The task for this scene.</returns>
	/// <seealso cref="ISystemScene::GetSystemTask"/>
	virtual ISystemTask* GetSystemTask( void );

	/// <summary cref="OGREGraphicsScene::GetPotentialSystemChanges">
	///   Implementation of the <c>ISubject::GetPotentialSystemChanges</c> function.
	///   Identies the system changes that this subject could possibly make.
	/// </summary>
	/// <returns>System::Changes::BitMask - A bitmask of the possible system changes.</returns>
	/// <seealso cref="ISubject::GetPotentialSystemChanges"/>
	virtual System::Changes::BitMask GetPotentialSystemChanges( void );

	/// <summary cref="OGREGraphicsScene::GetDesiredSystemChanges">
	///   Implementation of the <c>ISystemScene::GetDesiredSystemChanges</c> function.
	///   Returns a bit mask of System Changes that this scene wants to receive changes for.  
	///   Used to inform the change control manager if this scene should be informed of the
	///   change.
	/// </summary>
	/// <returns>System::Changes::BitMask - A bitmask of the desired system changes.</returns>
	/// <seealso cref="ISystemScene::GetDesiredSystemChanges"/>
	virtual const void* GetSystemChangeData( System::Change SystemChange );

public:
	typedef std::vector<GraphicsObject*>    ObjectsList;

			
	const BoundingBox& GetExtends() { return m_Extends; }
	void SetExtends( BoundingBox a_Box ) { m_Extends = a_Box; }
	
	void LoadOBJ( char* filename, noVec3& a_Pos, float a_Scale, bool a_Static );
	
	void storeLcoeffs( SHLight* shlight );
	SHLight* GetCurrentLight() { return m_shLights[m_currentLight]; }
	const SHLight* GetCurrentLight() const { return m_shLights[m_currentLight]; }
		void	SetActiveLight( int lid );

	bool Trace(HitInfo& minHit, const Ray& ray,
			float tMin = 0.0f, float tMax = 1e-12) const;

	void GraphicsScene::BuildKDTree();

protected:
	enum PropertyTypes
	{
		Property_ResourceLocation, Property_DelResourceLocation,
		Property_AmbientLight, Property_Shadows, 
		Property_ShadowColor, Property_DrawBoundingBox, 
		Property_ShowNormals, Property_ShowTangents,
		Property_UseStaticGeom, Property_UseInstancedGeom,
		Property_FogColor, Property_Fog, Property_Font,
		Property_PagedGeometry, Property_PagedGeometryTerrain,
		Property_PagedGeometryTerrainOffset,
		Property_Count
	};

	static pcstr                        sm_kapszPropertyNames[];
	static const Properties::Property   sm_kaDefaultProperties[];

	ObjectsList                         m_Objects;

	GraphicsTask*						m_pTask;

	SceneNode*							m_pSceneRoot;

	Bool                                m_bUseStaticGeom;
	Bool                                m_bUseInstancedGeom;
	
	ProcessData                         m_ProcessData[ MAX_NUM_JOBS ];

	// Fog
	ColourValue					        m_FogColor;
	FogMode					            m_FogMode;
	float                               m_ExpDensity;
	float                               m_LinearStart;
	float                               m_LinearEnd;

	/// <summary cref="OGREGraphicsScene::UpdateCallback">
	///   Invoked by ParalellFor algorithm to update a range of objects.
	/// </summary>
	static void UpdateCallback( void *param, u32 begin, u32 end );

	/// <summary cref="OGREGraphicsScene::UpdateCallback">
	///   Updates the given range of fire objects.
	/// </summary>
	void ProcessRange( u32 begin, u32 end );


	Bool    m_bParallelize;
	Bool    m_bPause;
	f32     m_fDeltaTime;

	typedef std::vector<SHLight*>	SHLightList;
	//BSPTree* m_bsp;
	BSP bsp;	

	SHLightList	m_shLights;	//our list of SH lights
	size_t		m_currentLight;
	bool		m_drawPRT;

private:
	
	BoundingBox m_Extends;	

};

