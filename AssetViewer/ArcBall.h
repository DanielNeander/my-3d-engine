#pragma once

#include <d3dx9math.h>

// Graphics Gems IV --ArcBall 

class CD3DArcBall
{
public:
                                    CD3DArcBall();

    // Functions to change behavior
    void                            Reset();
    void SetTranslationRadius( FLOAT fRadiusTranslation ) { m_fRadiusTranslation = fRadiusTranslation; }
    void SetWindow( INT nWidth, INT nHeight, FLOAT fRadius = 0.9f ) { m_nWidth = nWidth; m_nHeight = nHeight; m_fRadius = fRadius; m_vCenter = D3DXVECTOR2(m_nWidth/2.0f,m_nHeight/2.0f); }
    void SetOffset( INT nX, INT nY ) { m_Offset.x = nX; m_Offset.y = nY; }

    // Call these from client and use GetRotationMatrix() to read new rotation matrix
    void                            OnBegin( int nX, int nY );  // start the rotation (pass current mouse position)
    void                            OnMove( int nX, int nY );   // continue the rotation (pass current mouse position)
    void                            OnEnd();                    // end the rotation 

    // Or call this to automatically handle left, middle, right buttons
    LRESULT                         HandleMessages( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

    // Functions to get/set state
    const D3DXMATRIX* GetRotationMatrix()                   { return D3DXMatrixRotationQuaternion(&m_mRotation, &m_qNow); };
    const D3DXMATRIX* GetTranslationMatrix() const          { return &m_mTranslation; }
    const D3DXMATRIX* GetTranslationDeltaMatrix() const     { return &m_mTranslationDelta; }
    bool        IsBeingDragged() const                      { return m_bDrag; }
    D3DXQUATERNION GetQuatNow() const                       { return m_qNow; }
    void        SetQuatNow( D3DXQUATERNION q ) { m_qNow = q; }

    static D3DXQUATERNION WINAPI    QuatFromBallPoints( const D3DXVECTOR3& vFrom, const D3DXVECTOR3& vTo );


protected:
    D3DXMATRIXA16 m_mRotation;         // Matrix for arc ball's orientation
    D3DXMATRIXA16 m_mTranslation;      // Matrix for arc ball's position
    D3DXMATRIXA16 m_mTranslationDelta; // Matrix for arc ball's position

    POINT m_Offset;   // window offset, or upper-left corner of window
    INT m_nWidth;   // arc ball's window width
    INT m_nHeight;  // arc ball's window height
    D3DXVECTOR2 m_vCenter;  // center of arc ball 
    FLOAT m_fRadius;  // arc ball's radius in screen coords
    FLOAT m_fRadiusTranslation; // arc ball's radius for translating the target

    D3DXQUATERNION m_qDown;             // Quaternion before button down
    D3DXQUATERNION m_qNow;              // Composite quaternion for current drag
    bool m_bDrag;             // Whether user is dragging arc ball

    POINT m_ptLastMouse;      // position of last mouse point
    D3DXVECTOR3 m_vDownPt;           // starting point of rotation arc
    D3DXVECTOR3 m_vCurrentPt;        // current point of rotation arc

    D3DXVECTOR3                     ScreenToVector( float fScreenPtX, float fScreenPtY );
};