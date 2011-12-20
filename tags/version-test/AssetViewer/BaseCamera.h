#pragma once 

//#include <windows.h>
#include <U2_3D/SRC/main/u2cAMERA.H>

//--------------------------------------------------------------------------------------
// used by CCamera to map WM_KEYDOWN keys
//--------------------------------------------------------------------------------------
enum D3DUtil_CameraKeys
{
    CAM_STRAFE_LEFT = 0,
    CAM_STRAFE_RIGHT,
    CAM_MOVE_FORWARD,
    CAM_MOVE_BACKWARD,
    CAM_MOVE_UP,
    CAM_MOVE_DOWN,
    CAM_RESET,
    CAM_CONTROLDOWN,
    CAM_MAX_KEYS,
    CAM_UNKNOWN     = 0xFF
};

#define KEY_WAS_DOWN_MASK 0x80
#define KEY_IS_DOWN_MASK  0x01

#define MOUSE_LEFT_BUTTON   0x01
#define MOUSE_MIDDLE_BUTTON 0x02
#define MOUSE_RIGHT_BUTTON  0x04
#define MOUSE_WHEEL         0x08


class BaseCamera
{

public:
		BaseCamera();

	virtual void		Update( FLOAT fElapsedTime ) = 0;

	virtual LRESULT HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

		// Functions to change camera matrices
    virtual void                Reset();
    virtual void                SetViewParams( D3DXVECTOR3* pvEyePt, D3DXVECTOR3* pvLookatPt );
    virtual void                SetProjParams( FLOAT fFOV, FLOAT fAspect, FLOAT fNearPlane, FLOAT fFarPlane );


	   // Functions to change behavior
    virtual void SetDragRect( RECT &rc ) { m_rcDrag = rc; }
    void SetInvertPitch( bool bInvertPitch ) { m_bInvertPitch = bInvertPitch; }
    void SetDrag( bool bMovementDrag, FLOAT fTotalDragTimeToZero = 0.25f ) { m_bMovementDrag = bMovementDrag; m_fTotalDragTimeToZero = fTotalDragTimeToZero; }
    void SetEnableYAxisMovement( bool bEnableYAxisMovement ) { m_bEnableYAxisMovement = bEnableYAxisMovement; }
    void SetEnablePositionMovement( bool bEnablePositionMovement ) { m_bEnablePositionMovement = bEnablePositionMovement; }
    void SetClipToBoundary( bool bClipToBoundary, D3DXVECTOR3* pvMinBoundary, D3DXVECTOR3* pvMaxBoundary ) { m_bClipToBoundary = bClipToBoundary; if( pvMinBoundary ) m_vMinBoundary = *pvMinBoundary; if( pvMaxBoundary ) m_vMaxBoundary = *pvMaxBoundary; }
    void SetScalers( FLOAT fRotationScaler = 0.01f, FLOAT fMoveScaler = 5.0f )  { m_fRotationScaler = fRotationScaler; m_fMoveScaler = fMoveScaler; }
    void SetNumberOfFramesToSmoothMouseData( int nFrames ) { if( nFrames > 0 ) m_fFramesToSmoothMouseData = (float)nFrames; }
    void SetResetCursorAfterMove( bool bResetCursorAfterMove ) { m_bResetCursorAfterMove = bResetCursorAfterMove; }


	bool IsBeingDragged() const         { return (m_bMouseLButtonDown || m_bMouseMButtonDown || m_bMouseRButtonDown); }
    bool IsMouseLButtonDown() const     { return m_bMouseLButtonDown; } 
    bool IsMouseMButtonDown() const     { return m_bMouseMButtonDown; } 
    bool IsMouseRButtonDown() const     { return m_bMouseRButtonDown; } 

protected:

	   // Functions to map a WM_KEYDOWN key to a D3DUtil_CameraKeys enum
    virtual D3DUtil_CameraKeys  MapKey( UINT nKey );
    bool IsKeyDown( BYTE key ) const { return( (key & KEY_IS_DOWN_MASK) == KEY_IS_DOWN_MASK ); }
    bool WasKeyDown( BYTE key ) const { return( (key & KEY_WAS_DOWN_MASK) == KEY_WAS_DOWN_MASK ); }

    void                        ConstrainToBoundary( D3DXVECTOR3* pV );
    void                        UpdateMouseDelta();
    void                        UpdateVelocity( float fElapsedTime );
    void GetInput( bool bGetKeyboardInput, bool bGetMouseInput, bool bGetGamepadInput, bool bResetCursorAfterMove );
	
	//DXUT_GAMEPAD                m_GamePad[DXUT_MAX_CONTROLLERS];  // XInput controller state
    D3DXVECTOR3 m_vGamePadLeftThumb;
    D3DXVECTOR3 m_vGamePadRightThumb;
//  double                      m_GamePadLastActive[DXUT_MAX_CONTROLLERS];

	int m_cKeysDown;            // Number of camera keys that are down.
    BYTE                        m_aKeys[CAM_MAX_KEYS];  // State of input - KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK
    D3DXVECTOR3 m_vKeyboardDirection;   // Direction vector of keyboard input
    POINT m_ptLastMousePosition;  // Last absolute position of mouse cursor
    bool m_bMouseLButtonDown;    // True if left button is down 
    bool m_bMouseMButtonDown;    // True if middle button is down 
    bool m_bMouseRButtonDown;    // True if right button is down 
    int m_nCurrentButtonMask;   // mask of which buttons are down
    int m_nMouseWheelDelta;     // Amount of middle wheel scroll (+/-) 
    D3DXVECTOR2 m_vMouseDelta;          // Mouse relative delta smoothed over a few frames
    float m_fFramesToSmoothMouseData; // Number of frames to smooth mouse data over

    D3DXVECTOR3 m_vDefaultEye;          // Default camera eye position
    D3DXVECTOR3 m_vDefaultLookAt;       // Default LookAt position
    D3DXVECTOR3 m_vEye;                 // Camera eye position
    D3DXVECTOR3 m_vLookAt;              // LookAt position

	U2CameraPtr m_spCam;

    float m_fCameraYawAngle;      // Yaw angle of camera
    float m_fCameraPitchAngle;    // Pitch angle of camera

    RECT m_rcDrag;               // Rectangle within which a drag can be initiated.
    D3DXVECTOR3 m_vVelocity;            // Velocity of camera
    bool m_bMovementDrag;        // If true, then camera movement will slow to a stop otherwise movement is instant
    D3DXVECTOR3 m_vVelocityDrag;        // Velocity drag force
    FLOAT m_fDragTimer;           // Countdown timer to apply drag
    FLOAT m_fTotalDragTimeToZero; // Time it takes for velocity to go from full to 0
    D3DXVECTOR2 m_vRotVelocity;         // Velocity of camera

    float m_fFOV;                 // Field of view
    float m_fAspect;              // Aspect ratio
    float m_fNearPlane;           // Near plane
    float m_fFarPlane;            // Far plane

    float m_fRotationScaler;      // Scaler for rotation
    float m_fMoveScaler;          // Scaler for movement

    bool m_bInvertPitch;         // Invert the pitch axis
    bool m_bEnablePositionMovement; // If true, then the user can translate the camera/model 
    bool m_bEnableYAxisMovement; // If true, then camera can move in the y-axis

    bool m_bClipToBoundary;      // If true, then the camera will be clipped to the boundary
    D3DXVECTOR3 m_vMinBoundary;         // Min point in clip boundary
    D3DXVECTOR3 m_vMaxBoundary;         // Max point in clip boundary

    bool m_bResetCursorAfterMove;// If true, the class will reset the cursor position so that the cursor always has space to move 
};
