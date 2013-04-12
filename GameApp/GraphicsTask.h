#pragma once 

class GraphicsScene;

class GraphicsTask : public ISystemTask {
	friend GraphicsScene;

protected:
	GraphicsTask( GraphicsScene* pScene );
	~GraphicsTask( void );

	/// <summary cref="OGREGraphicsTask::GetSystemType">
	///   Implementation of the <c>ISystemTask::GetSystemType</c> function.
	///   Gets the system type for this system task.
	/// </summary>
	/// <returns>System::Type - The type of the system.</returns>
	/// <seealso cref="ISystemTask::GetSystemType"/>
	virtual System::Type GetSystemType( void );

	/// <summary cref="OGREGraphicsTask::Update">
	///   Implementation of the <c>ISystemTask::Update</c> function.
	///   Function informing the task to perform its updates.  This does
	///   all the work needed to update graphics for this frame.
	/// </summary>
	/// <param name="DeltaTime">The time delta from the last call.</param>
	/// <seealso cref="ISystemTask::Update"/>
	virtual void Update( f32 DeltaTime );

	/// <summary cref="OGREGraphicsTask::IsPrimaryThreadOnly">
	///   Implementation of the <c>ISystemTask::IsPrimaryThreadOnly</c> function.
	/// </summary>
	/// <returns>Bool - Returns if this system should only run on the primary thread.</returns>
	/// <seealso cref="ISystemTask::IsPrimaryThreadOnly"/>
	virtual Bool IsPrimaryThreadOnly( void );

private:
	GraphicsScene*		m_pScene;
};