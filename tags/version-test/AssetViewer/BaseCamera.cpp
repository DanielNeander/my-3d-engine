#include "AppMisc.h"
#include <U2Application/U2WinApp.h>
#include "BaseCamera.h"


BaseCamera::BaseCamera()
{
	m_cKeysDown = 0;
	ZeroMemory( m_aKeys, sizeof( BYTE ) * CAM_MAX_KEYS );
    //ZeroMemory( m_GamePad, sizeof( DXUT_GAMEPAD ) * DXUT_MAX_CONTROLLERS );


      // Set attributes for the view matrix
    D3DXVECTOR3 vEyePt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );

	SetViewParams( &vEyePt, &vLookatPt );
	
	SetProjParams( D3DX_PI / 4, 1.333333f, 1.0f, 1000.0f );

	GetCursorPos( &m_ptLastMousePosition );

	m_bMouseLButtonDown = false;
    m_bMouseMButtonDown = false;
    m_bMouseRButtonDown = false;
    m_nCurrentButtonMask = 0;
    m_nMouseWheelDelta = 0;

    m_fCameraYawAngle = 0.0f;
    m_fCameraPitchAngle = 0.0f;

    SetRect( &m_rcDrag, LONG_MIN, LONG_MIN, LONG_MAX, LONG_MAX );
    m_vVelocity = D3DXVECTOR3( 0, 0, 0 );
    m_bMovementDrag = false;
    m_vVelocityDrag = D3DXVECTOR3( 0, 0, 0 );
    m_fDragTimer = 0.0f;
    m_fTotalDragTimeToZero = 0.25;
    m_vRotVelocity = D3DXVECTOR2( 0, 0 );

    m_fRotationScaler = 0.01f;
    m_fMoveScaler = 5.0f;

    m_bInvertPitch = false;
    m_bEnableYAxisMovement = true;
    m_bEnablePositionMovement = true;

    m_vMouseDelta = D3DXVECTOR2( 0, 0 );
    m_fFramesToSmoothMouseData = 2.0f;

    m_bClipToBoundary = false;
    m_vMinBoundary = D3DXVECTOR3( -1, -1, -1 );
    m_vMaxBoundary = D3DXVECTOR3( 1, 1, 1 );

    m_bResetCursorAfterMove = false;

	m_spCam = U2_NEW U2Camera();

}

void BaseCamera::Reset()
{
	 SetViewParams( &m_vDefaultEye, &m_vDefaultLookAt );
}

void BaseCamera::SetViewParams( D3DXVECTOR3* pvEyePt, D3DXVECTOR3* pvLookatPt )
{
	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();

	if ( NULL == pRenderer )
		return;
	
	if( NULL == pvEyePt || NULL == pvLookatPt )
        return;

    m_vDefaultEye = m_vEye = *pvEyePt;
    m_vDefaultLookAt = m_vLookAt = *pvLookatPt;

    // Calc the view matrix
    D3DXVECTOR3 vUp( 0,1,0 );

	D3DXVECTOR3 vRight;
	::D3DXVec3Cross(&vRight, &vUp, &m_vLookAt);	
	
	m_spCam->SetFrame(m_vEye, m_vLookAt, vUp, vRight);

	pRenderer->SetCamera(m_spCam);

	   // The axis basis vectors and camera position are stored inside the 
    // position matrix in the 4 rows of the camera's world matrix.
    // To figure out the yaw/pitch of the camera, we just need the Z basis vector
	D3DXMATRIX invView = pRenderer->GetInvViewMat();
	D3DXVECTOR3* pZBasis = ( D3DXVECTOR3* )&invView._31;

	m_fCameraYawAngle = atan2f( pZBasis->x, pZBasis->z );
	float fLen = sqrtf( pZBasis->z + pZBasis->z + pZBasis->x * pZBasis->x );
	m_fCameraPitchAngle = -atan2f( pZBasis->y, fLen);
}

void BaseCamera::SetProjParams( FLOAT fFOV, FLOAT fAspect, FLOAT fNearPlane, FLOAT fFarPlane )
{
	IsRendererOK;	

	m_spCam->SetFrustum( fFOV, fAspect, fNearPlane, fFarPlane );
	pRenderer->SetProjectionTM( m_spCam );
}

//--------------------------------------------------------------------------------------
// Figure out the mouse delta based on mouse movement
//--------------------------------------------------------------------------------------
void BaseCamera::UpdateMouseDelta()
{
    POINT ptCurMouseDelta;
    POINT ptCurMousePos;

    // Get current position of mouse
    GetCursorPos( &ptCurMousePos );

    // Calc how far it's moved since last frame
    ptCurMouseDelta.x = ptCurMousePos.x - m_ptLastMousePosition.x;
    ptCurMouseDelta.y = ptCurMousePos.y - m_ptLastMousePosition.y;

    // Record current position for next time
    m_ptLastMousePosition = ptCurMousePos;

    if( m_bResetCursorAfterMove )
    {
		
        // Set position of camera to center of desktop, 
        // so it always has room to move.  This is very useful
        // if the cursor is hidden.  If this isn't done and cursor is hidden, 
        // then invisible cursor will hit the edge of the screen 
        // and the user can't tell what happened
        POINT ptCenter;

        // Get the center of the current monitor
        MONITORINFO mi;
        mi.cbSize = sizeof( MONITORINFO );
		AppMisc::GetMonitorInfo( AppMisc::MonitorFromWindow( ((WinApp*)WinApp::TheApplication)->GetHwnd(), MONITOR_DEFAULTTONEAREST ), &mi );
        ptCenter.x = ( mi.rcMonitor.left + mi.rcMonitor.right ) / 2;
        ptCenter.y = ( mi.rcMonitor.top + mi.rcMonitor.bottom ) / 2;
        SetCursorPos( ptCenter.x, ptCenter.y );
        m_ptLastMousePosition = ptCenter;
    }

    // Smooth the relative mouse data over a few frames so it isn't 
    // jerky when moving slowly at low frame rates.
    float fPercentOfNew = 1.0f / m_fFramesToSmoothMouseData;
    float fPercentOfOld = 1.0f - fPercentOfNew;
    m_vMouseDelta.x = m_vMouseDelta.x * fPercentOfOld + ptCurMouseDelta.x * fPercentOfNew;
    m_vMouseDelta.y = m_vMouseDelta.y * fPercentOfOld + ptCurMouseDelta.y * fPercentOfNew;

    m_vRotVelocity = m_vMouseDelta * m_fRotationScaler;
}


//--------------------------------------------------------------------------------------
// Figure out the velocity based on keyboard input & drag if any
//--------------------------------------------------------------------------------------
void BaseCamera::UpdateVelocity( float fElapsedTime )
{
    D3DXMATRIX mRotDelta;
    D3DXVECTOR2 vGamePadRightThumb = D3DXVECTOR2( m_vGamePadRightThumb.x, -m_vGamePadRightThumb.z );
    m_vRotVelocity = m_vMouseDelta * m_fRotationScaler + vGamePadRightThumb * 0.02f;

    D3DXVECTOR3 vAccel = m_vKeyboardDirection + m_vGamePadLeftThumb;

    // Normalize vector so if moving 2 dirs (left & forward), 
    // the camera doesn't move faster than if moving in 1 dir
    D3DXVec3Normalize( &vAccel, &vAccel );

    // Scale the acceleration vector
    vAccel *= m_fMoveScaler;

    if( m_bMovementDrag )
    {
        // Is there any acceleration this frame?
        if( D3DXVec3LengthSq( &vAccel ) > 0 )
        {
            // If so, then this means the user has pressed a movement key\
            // so change the velocity immediately to acceleration 
            // upon keyboard input.  This isn't normal physics
            // but it will give a quick response to keyboard input
            m_vVelocity = vAccel;
            m_fDragTimer = m_fTotalDragTimeToZero;
            m_vVelocityDrag = vAccel / m_fDragTimer;
        }
        else
        {
            // If no key being pressed, then slowly decrease velocity to 0
            if( m_fDragTimer > 0 )
            {
                // Drag until timer is <= 0
                m_vVelocity -= m_vVelocityDrag * fElapsedTime;
                m_fDragTimer -= fElapsedTime;
            }
            else
            {
                // Zero velocity
                m_vVelocity = D3DXVECTOR3( 0, 0, 0 );
            }
        }
    }
    else
    {
        // No drag, so immediately change the velocity
        m_vVelocity = vAccel;
    }
}

//--------------------------------------------------------------------------------------
// Clamps pV to lie inside m_vMinBoundary & m_vMaxBoundary
//--------------------------------------------------------------------------------------
void BaseCamera::ConstrainToBoundary( D3DXVECTOR3* pV )
{
    // Constrain vector to a bounding box 
    pV->x = __max( pV->x, m_vMinBoundary.x );
    pV->y = __max( pV->y, m_vMinBoundary.y );
    pV->z = __max( pV->z, m_vMinBoundary.z );

    pV->x = __min( pV->x, m_vMaxBoundary.x );
    pV->y = __min( pV->y, m_vMaxBoundary.y );
    pV->z = __min( pV->z, m_vMaxBoundary.z );
}


void BaseCamera::GetInput( bool bGetKeyboardInput, bool bGetMouseInput, bool bGetGamepadInput, bool bResetCursorAfterMove )
{
	    m_vKeyboardDirection = D3DXVECTOR3( 0, 0, 0 );
    if( bGetKeyboardInput )
    {
        // Update acceleration vector based on keyboard state
        if( IsKeyDown( m_aKeys[CAM_MOVE_FORWARD] ) )
            m_vKeyboardDirection.z += 1.0f;
        if( IsKeyDown( m_aKeys[CAM_MOVE_BACKWARD] ) )
            m_vKeyboardDirection.z -= 1.0f;
        if( m_bEnableYAxisMovement )
        {
            if( IsKeyDown( m_aKeys[CAM_MOVE_UP] ) )
                m_vKeyboardDirection.y += 1.0f;
            if( IsKeyDown( m_aKeys[CAM_MOVE_DOWN] ) )
                m_vKeyboardDirection.y -= 1.0f;
        }
        if( IsKeyDown( m_aKeys[CAM_STRAFE_RIGHT] ) )
            m_vKeyboardDirection.x += 1.0f;
        if( IsKeyDown( m_aKeys[CAM_STRAFE_LEFT] ) )
            m_vKeyboardDirection.x -= 1.0f;
    }

    if( bGetMouseInput )
    {
        UpdateMouseDelta();
    }

    if( bGetGamepadInput )
    {
        m_vGamePadLeftThumb = D3DXVECTOR3( 0, 0, 0 );
        m_vGamePadRightThumb = D3DXVECTOR3( 0, 0, 0 );

		// 현재는 패드를 지원하지 않으므로 패드 관련 함수 처리는 생략.

        // Get controller state
        //for( DWORD iUserIndex = 0; iUserIndex < DXUT_MAX_CONTROLLERS; iUserIndex++ )
        //{
        //    DXUTGetGamepadState( iUserIndex, &m_GamePad[iUserIndex], true, true );

        //    // Mark time if the controller is in a non-zero state
        //    if( m_GamePad[iUserIndex].wButtons ||
        //        m_GamePad[iUserIndex].sThumbLX || m_GamePad[iUserIndex].sThumbLX ||
        //        m_GamePad[iUserIndex].sThumbRX || m_GamePad[iUserIndex].sThumbRY ||
        //        m_GamePad[iUserIndex].bLeftTrigger || m_GamePad[iUserIndex].bRightTrigger )
        //    {
        //        m_GamePadLastActive[iUserIndex] = DXUTGetTime();
        //    }
        //}

        //// Find out which controller was non-zero last
        //int iMostRecentlyActive = -1;
        //double fMostRecentlyActiveTime = 0.0f;
        //for( DWORD iUserIndex = 0; iUserIndex < DXUT_MAX_CONTROLLERS; iUserIndex++ )
        //{
        //    if( m_GamePadLastActive[iUserIndex] > fMostRecentlyActiveTime )
        //    {
        //        fMostRecentlyActiveTime = m_GamePadLastActive[iUserIndex];
        //        iMostRecentlyActive = iUserIndex;
        //    }
        //}

        //// Use the most recent non-zero controller if its connected
        //if( iMostRecentlyActive >= 0 && m_GamePad[iMostRecentlyActive].bConnected )
        //{
        //    m_vGamePadLeftThumb.x = m_GamePad[iMostRecentlyActive].fThumbLX;
        //    m_vGamePadLeftThumb.y = 0.0f;
        //    m_vGamePadLeftThumb.z = m_GamePad[iMostRecentlyActive].fThumbLY;

        //    m_vGamePadRightThumb.x = m_GamePad[iMostRecentlyActive].fThumbRX;
        //    m_vGamePadRightThumb.y = 0.0f;
        //    m_vGamePadRightThumb.z = m_GamePad[iMostRecentlyActive].fThumbRY;
        //}
    }
}



//--------------------------------------------------------------------------------------
// Call this from your message proc so this class can handle window messages
//--------------------------------------------------------------------------------------
LRESULT BaseCamera::HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER( hWnd );
    UNREFERENCED_PARAMETER( lParam );

    switch( uMsg )
    {
        case WM_KEYDOWN:
        {
            // Map this key to a D3DUtil_CameraKeys enum and update the
            // state of m_aKeys[] by adding the KEY_WAS_DOWN_MASK|KEY_IS_DOWN_MASK mask
            // only if the key is not down
            D3DUtil_CameraKeys mappedKey = MapKey( ( UINT )wParam );
            if( mappedKey != CAM_UNKNOWN )
            {
                if( FALSE == IsKeyDown( m_aKeys[mappedKey] ) )
                {
                    m_aKeys[ mappedKey ] = KEY_WAS_DOWN_MASK | KEY_IS_DOWN_MASK;
                    ++m_cKeysDown;
                }
            }
            break;
        }

        case WM_KEYUP:
        {
            // Map this key to a D3DUtil_CameraKeys enum and update the
            // state of m_aKeys[] by removing the KEY_IS_DOWN_MASK mask.
            D3DUtil_CameraKeys mappedKey = MapKey( ( UINT )wParam );
            if( mappedKey != CAM_UNKNOWN && ( DWORD )mappedKey < 8 )
            {
                m_aKeys[ mappedKey ] &= ~KEY_IS_DOWN_MASK;
                --m_cKeysDown;
            }
            break;
        }

        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_LBUTTONDBLCLK:
            {
                // Compute the drag rectangle in screen coord.
                POINT ptCursor = { ( short )LOWORD( lParam ), ( short )HIWORD( lParam ) };

                // Update member var state
                if( ( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK ) && PtInRect( &m_rcDrag, ptCursor ) )
                {
                    m_bMouseLButtonDown = true; m_nCurrentButtonMask |= MOUSE_LEFT_BUTTON;
                }
                if( ( uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK ) && PtInRect( &m_rcDrag, ptCursor ) )
                {
                    m_bMouseMButtonDown = true; m_nCurrentButtonMask |= MOUSE_MIDDLE_BUTTON;
                }
                if( ( uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK ) && PtInRect( &m_rcDrag, ptCursor ) )
                {
                    m_bMouseRButtonDown = true; m_nCurrentButtonMask |= MOUSE_RIGHT_BUTTON;
                }

                // Capture the mouse, so if the mouse button is 
                // released outside the window, we'll get the WM_LBUTTONUP message
                SetCapture( hWnd );
                GetCursorPos( &m_ptLastMousePosition );
                return TRUE;
            }

        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_LBUTTONUP:
            {
                // Update member var state
                if( uMsg == WM_LBUTTONUP )
                {
                    m_bMouseLButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_LEFT_BUTTON;
                }
                if( uMsg == WM_MBUTTONUP )
                {
                    m_bMouseMButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_MIDDLE_BUTTON;
                }
                if( uMsg == WM_RBUTTONUP )
                {
                    m_bMouseRButtonDown = false; m_nCurrentButtonMask &= ~MOUSE_RIGHT_BUTTON;
                }

                // Release the capture if no mouse buttons down
                if( !m_bMouseLButtonDown &&
                    !m_bMouseRButtonDown &&
                    !m_bMouseMButtonDown )
                {
                    ReleaseCapture();
                }
                break;
            }

        case WM_CAPTURECHANGED:
        {
            if( ( HWND )lParam != hWnd )
            {
                if( ( m_nCurrentButtonMask & MOUSE_LEFT_BUTTON ) ||
                    ( m_nCurrentButtonMask & MOUSE_MIDDLE_BUTTON ) ||
                    ( m_nCurrentButtonMask & MOUSE_RIGHT_BUTTON ) )
                {
                    m_bMouseLButtonDown = false;
                    m_bMouseMButtonDown = false;
                    m_bMouseRButtonDown = false;
                    m_nCurrentButtonMask &= ~MOUSE_LEFT_BUTTON;
                    m_nCurrentButtonMask &= ~MOUSE_MIDDLE_BUTTON;
                    m_nCurrentButtonMask &= ~MOUSE_RIGHT_BUTTON;
                    ReleaseCapture();
                }
            }
            break;
        }

        case WM_MOUSEWHEEL:
            // Update member var state
            m_nMouseWheelDelta += ( short )HIWORD( wParam );
            break;
    }

    return FALSE;
}



//--------------------------------------------------------------------------------------
// Maps a windows virtual key to an enum
//--------------------------------------------------------------------------------------
D3DUtil_CameraKeys BaseCamera::MapKey( UINT nKey )
{
    // This could be upgraded to a method that's user-definable but for 
    // simplicity, we'll use a hardcoded mapping.
    switch( nKey )
    {
        case VK_CONTROL:
            return CAM_CONTROLDOWN;
        case VK_LEFT:
            return CAM_STRAFE_LEFT;
        case VK_RIGHT:
            return CAM_STRAFE_RIGHT;
        case VK_UP:
            return CAM_MOVE_FORWARD;
        case VK_DOWN:
            return CAM_MOVE_BACKWARD;
        case VK_PRIOR:
            return CAM_MOVE_UP;        // pgup
        case VK_NEXT:
            return CAM_MOVE_DOWN;      // pgdn

        case 'A':
            return CAM_STRAFE_LEFT;
        case 'D':
            return CAM_STRAFE_RIGHT;
        case 'W':
            return CAM_MOVE_FORWARD;
        case 'S':
            return CAM_MOVE_BACKWARD;
        case 'Q':
            return CAM_MOVE_DOWN;
        case 'E':
            return CAM_MOVE_UP;

        case VK_NUMPAD4:
            return CAM_STRAFE_LEFT;
        case VK_NUMPAD6:
            return CAM_STRAFE_RIGHT;
        case VK_NUMPAD8:
            return CAM_MOVE_FORWARD;
        case VK_NUMPAD2:
            return CAM_MOVE_BACKWARD;
        case VK_NUMPAD9:
            return CAM_MOVE_UP;
        case VK_NUMPAD3:
            return CAM_MOVE_DOWN;

        case VK_HOME:
            return CAM_RESET;
    }

    return CAM_UNKNOWN;
}
