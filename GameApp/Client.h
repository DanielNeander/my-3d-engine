#pragma once 


enum EMouseCursor
{
	MC_None,
	MC_NoChange,		// Keeps the platform client from calling setcursor so a cursor can be set elsewhere (ie using wxSetCursor).
	MC_Arrow,
	MC_Cross,
	MC_SizeAll,
	MC_SizeUpRightDownLeft,
	MC_SizeUpLeftDownRight,
	MC_SizeLeftRight,
	MC_SizeUpDown,
	MC_Hand
};
//
//	EInputEvent
//
enum EInputEvent
{
	IE_Pressed              =0,
	IE_Released             =1,
	IE_Repeat               =2,
	IE_DoubleClick          =3,
	IE_Axis                 =4,
	IE_MAX                  =5,
};

/**
 * A render target.
 */
class FRenderTarget
{
public:

	// Destructor
	virtual ~FRenderTarget(){}

	/**
	* Accessor for the surface RHI when setting this render target
	* @return render target surface RHI resource
	*/
	//virtual const FSurfaceRHIRef& GetRenderTargetSurface() const;

	// Properties.
	virtual UINT GetSizeX() const = 0;
	virtual UINT GetSizeY() const = 0;

	/** 
	* @return display gamma expected for rendering to this render target 
	*/
	virtual FLOAT GetDisplayGamma() const;

	/**
	* Handles freezing/unfreezing of rendering
	*/
	virtual void ProcessToggleFreezeCommand() {};
	
	/**
	 * Returns if there is a command to toggle freezerendering
	 */
	virtual UBOOL HasToggleFreezeCommand() { return FALSE; };

	/**
	* Reads the viewport's displayed pixels into the given color buffer.
	* @param OutputBuffer - RGBA8 values will be stored in this buffer
	* @param CubeFace - optional cube face for when reading from a cube render target
	* @return True if the read succeeded.
	*/
	//UBOOL ReadPixels(TArray<FColor>& OutputBuffer,ECubeFace CubeFace=CubeFace_PosX);

protected:
	//FSurfaceRHIRef RenderTargetSurfaceRHI;
};


/**
 * An interface to the platform-specific implementation of a UI frame for a viewport.
 */
class FViewportFrame
{
public:

	virtual class FViewport* GetViewport() = 0;
	virtual void Resize(UINT NewSizeX,UINT NewSizeY,UBOOL NewFullscreen,INT InPosX = -1, INT InPosY = -1) = 0;
};


class FViewport : public FRenderTarget
{
public:
	// Constructor.
	FViewport(class FViewportClient* InViewportClient);
	// Destructor
	virtual ~FViewport(){}

	// FViewport interface.
	virtual void*	GetWindow() = 0;

	// New MouseCapture/MouseLock API
	virtual UBOOL	HasMouseCapture() const				{ return TRUE; }
	virtual UBOOL	HasFocus() const					{ return TRUE; }
	virtual void	CaptureMouse( UBOOL bCapture )		{ }
	virtual void	LockMouseToWindow( UBOOL bLock )	{ }
	virtual void	ShowCursor( UBOOL bVisible )		{ }

	virtual UBOOL	CaptureJoystickInput(UBOOL Capture) = 0;
	virtual UBOOL	KeyState(FName Key) const = 0;
	virtual INT		GetMouseX() = 0;
	virtual INT		GetMouseY() = 0;
	virtual void	GetMousePos( FIntPoint& MousePosition ) = 0;
	virtual void	SetMouse(INT x, INT y) = 0;
	virtual UBOOL	IsFullscreen()						{ return bIsFullscreen; }
	virtual void	ProcessInput( FLOAT DeltaTime ) = 0;

	/**
	 * @return whether or not this Controller has Tilt Turned on
	 **/
	virtual UBOOL IsControllerTiltActive( INT ControllerID ) const { return FALSE; }

	/**
	 * sets whether or not the the player wants to utilize the Tilt functionality
	 **/
	virtual void SetControllerTiltDesiredIfAvailable( INT ControllerID, UBOOL bActive ) { }

	/**
	 * sets whether or not the Tilt functionality is turned on
	 **/
	virtual void SetControllerTiltActive( INT ControllerID, UBOOL bActive ) { }

	/**
	 * sets whether or not to ONLY use the tilt input controls
	 **/
	virtual void SetOnlyUseControllerTiltInput( INT ControllerID, UBOOL bActive ) { }

	/**
 	 * sets whether or not to use the tilt forward and back input controls
	 **/
	virtual void SetUseTiltForwardAndBack( INT ControllerID, UBOOL bActive ) { }

	/**
	 * @return whether or not this Controller has a keyboard available to be used
	 **/
	virtual UBOOL IsKeyboardAvailable( INT ControllerID ) const { return TRUE; }

	/**
	 * @return whether or not this Controller has a mouse available to be used
	 **/
	virtual UBOOL IsMouseAvailable( INT ControllerID ) const { return TRUE; }


	/** 
	* @return aspect ratio that this viewport should be rendered at
	*/
	virtual FLOAT GetDesiredAspectRatio() const
	{
        return (FLOAT)GetSizeX()/(FLOAT)GetSizeY();        
	}

	/**
	 * Invalidates the viewport's displayed pixels.
	 */
	virtual void InvalidateDisplay() = 0;

	/**
	 * Updates the viewport's displayed pixels with the results of calling ViewportClient->Draw.
	 */
	void Draw();

	/**
	 * Invalidates cached hit proxies
	 */
	void InvalidateHitProxy();	

	/**
	 * Invalidates cached hit proxies and the display.
	 */
	void Invalidate();	


	/**
	 * Retrieves the interface to the viewport's frame, if it has one.
	 * @return The viewport's frame interface.
	 */
	virtual FViewportFrame* GetViewportFrame() = 0;
	
	/**
	 * Calculates the view inside the viewport when the aspect ratio is locked.
	 * Used for creating cinematic bars.
	 * @param Aspect [in] ratio to lock to
	 * @param CurrentX [in][out] coordinates of aspect locked view
	 * @param CurrentY [in][out]
	 * @param CurrentSizeX [in][out] size of aspect locked view
	 * @param CurrentSizeY [in][out]
	 */
	void CalculateViewExtents( FLOAT AspectRatio, INT& CurrentX, INT& CurrentY, UINT& CurrentSizeX, UINT& CurrentSizeY );

	// FRenderTarget interface.
	virtual UINT GetSizeX() const { return SizeX; }
	virtual UINT GetSizeY() const { return SizeY; }	

	// Accessors.
	FViewportClient* GetClient() const { return ViewportClient; }

	/**
	 * Globally enables/disables rendering
	 *
	 * @param bIsEnabled TRUE if drawing should occur
	 * @param PresentAndStopMovieDelay Number of frames to delay before enabling bPresent in RHIEndDrawingViewport, and before stopping the movie
	 */
	static void SetGameRenderingEnabled(UBOOL bIsEnabled, INT PresentAndStopMovieDelay=0);

	/**
	 * Handles freezing/unfreezing of rendering
	 */
	virtual void ProcessToggleFreezeCommand();

	/**
	 * Returns if there is a command to freeze
	 */
	virtual UBOOL HasToggleFreezeCommand();

protected:

		/** The viewport's client. */
	FViewportClient* ViewportClient;


	/** The width of the viewport. */
	UINT SizeX;

	/** The height of the viewport. */
	UINT SizeY;

	/** The size of the region to check hit proxies */
	UINT HitProxySize;

	/** True if the viewport is fullscreen. */
	BITFIELD bIsFullscreen : 1;

	/** True if the viewport client requires hit proxy storage. */
	BITFIELD bRequiresHitProxyStorage : 1;

	/** True if the hit proxy buffer buffer has up to date hit proxies for this viewport. */
	BITFIELD bHitProxiesCached : 1;

	/** If a toggle freeze request has been made */
	BITFIELD bHasRequestedToggleFreeze : 1;

	/** TRUE if we should draw game viewports (has no effect on Editor viewports) */
	static UBOOL bIsGameRenderingEnabled;

	/** Delay in frames to disable present (but still render scene) and stopping of a movie. This is useful to keep playing a movie while driver caches things on the first frame, which can be slow. */
	static INT PresentAndStopMovieDelay;
		

};


/**
 * An abstract interface to a viewport's client.
 * The viewport's client processes input received by the viewport, and draws the viewport.
 */
class FViewportClient
{
public:

	virtual void Precache() {}
	virtual void RedrawRequested(FViewport* Viewport) { Viewport->Draw(); }
	virtual void Draw(FViewport* Viewport,class FCanvas* Canvas) {}

	/**
	 * Check a key event received by the viewport.
	 * If the viewport client uses the event, it should return true to consume it.
	 * @param	Viewport - The viewport which the key event is from.
	 * @param	ControllerId - The controller which the key event is from.
	 * @param	Key - The name of the key which an event occured for.
	 * @param	Event - The type of event which occured.
	 * @param	AmountDepressed - For analog keys, the depression percent.
	 * @param	bGamepad - input came from gamepad (ie xbox controller)
	 * @return	True to consume the key event, false to pass it on.
	 */
	virtual UBOOL InputKey(FViewport* Viewport,INT ControllerId,FName Key,EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE) { InputKey(Viewport,Key,Event,AmountDepressed,bGamepad); return FALSE; }

	/**
	 * Check an axis movement received by the viewport.
	 * If the viewport client uses the movement, it should return true to consume it.
	 * @param	Viewport - The viewport which the axis movement is from.
	 * @param	ControllerId - The controller which the axis movement is from.
	 * @param	Key - The name of the axis which moved.
	 * @param	Delta - The axis movement delta.
	 * @param	DeltaTime - The time since the last axis update.
	 * @return	True to consume the axis movement, false to pass it on.
	 */
	virtual UBOOL InputAxis(FViewport* Viewport,INT ControllerId,FName Key,FLOAT Delta,FLOAT DeltaTime,UBOOL bGamepad=FALSE) { InputAxis(Viewport,Key,Delta,DeltaTime,bGamepad); return FALSE; }

	/**
	 * Check a character input received by the viewport.
	 * If the viewport client uses the character, it should return true to consume it.
	 * @param	Viewport - The viewport which the axis movement is from.
	 * @param	ControllerId - The controller which the axis movement is from.
	 * @param	Character - The character.
	 * @return	True to consume the character, false to pass it on.
	 */
	virtual UBOOL InputChar(FViewport* Viewport,INT ControllerId,TCHAR Character) { InputChar(Viewport,Character); return FALSE; }

	/** @name Obsolete input interface.  Called by the new interface to ensure implementors of the old interface aren't broken. */
	//@{
	virtual void InputKey(FViewport* Viewport,FName Key, EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE) {}
	virtual void InputAxis(FViewport* Viewport,FName Key,FLOAT Delta,FLOAT DeltaTime,UBOOL bGamepad=FALSE) {}
	virtual void InputChar(FViewport* Viewport,TCHAR Character) {}
	//@}

	
	/**
	 * @return whether or not this Controller has Tilt Turned on
	 **/
	virtual UBOOL IsControllerTiltActive() const { return FALSE; }

	/**
	 * sets whether or not the Tilt functionality is turned on
	 **/
	virtual void SetControllerTiltDesiredIfAvailable( UBOOL bActive ) { }

	/**
	 * sets whether or not the Tilt functionality is turned on
	 **/
	virtual void SetControllerTiltActive( UBOOL bActive ) { }

	/**
	 * sets whether or not to ONLY use the tilt input controls
	 **/
	virtual void SetOnlyUseControllerTiltInput( UBOOL bActive ) { }


	virtual void MouseMove(FViewport* Viewport,INT X,INT Y) {}

	/**
	 * Retrieves the cursor that should be displayed by the OS
	 *
	 * @param	Viewport	the viewport that contains the cursor
	 * @param	X			the x position of the cursor
	 * @param	Y			the Y position of the cursor
	 * 
	 * @return	the cursor that the OS should display
	 */
	virtual EMouseCursor GetCursor(FViewport* Viewport,INT X,INT Y) { return MC_Arrow; }

	virtual void LostFocus(FViewport* Viewport) {}
	virtual void ReceivedFocus(FViewport* Viewport) {}
	virtual UBOOL IsFocused(FViewport* Viewport) { return TRUE; }

	virtual void CloseRequested(FViewport* Viewport) {}

	virtual UBOOL RequiresHitProxyStorage() { return TRUE; }

	/**
	 * Determines whether this viewport client should receive calls to InputAxis() if the game's window is not currently capturing the mouse.
	 * Used by the UI system to easily receive calls to InputAxis while the viewport's mouse capture is disabled.
	 */
	virtual UBOOL RequiresUncapturedAxisInput() const { return FALSE; }

	/**
	* Determine if the viewport client is going to need any keyboard input
	* @return TRUE if keyboard input is needed
	*/
	virtual UBOOL RequiresKeyboardInput() const { return TRUE; }
};

