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


#if defined( _DEBUG ) || defined( PROFILE )
#define __ALLOW_DEBUG_WINDOW__

extern Handle CreateDebugWindow( void );
extern void DestroyDebugWindow( Handle hDebugWindow );
extern void SetDebugWindowCCM( Handle hDebugWindow, IChangeManager* pSceneCCM, IChangeManager* pObjectCCM );
extern void SetDebugWindowUScene( Handle hDebugWindow, UScene* pUScene );


#ifdef __DEBUG_WINDOW__
class DebugWindow : public IObserver
{
public:

    DebugWindow( void );
    ~DebugWindow( void );

    void SetUScene( UScene* pUScene );

    void SetCCM( IChangeManager* pSceneCCM, IChangeManager* pObjectCCM );


protected:

    void Unregister( void );

    virtual Error ChangeOccurred( ISubject* pSubject, System::Changes::BitMask ChangeType );

    void OnCreate( void );
    void OnDestroy( void );
    void OnLButtonDown( int x, int y );
    void OnLButtonUp( int x, int y );
    void OnMouseMove( int x, int y );
    void OnSize( int Width, int Height );
    void OnPaint( void );

    static LRESULT CALLBACK WindowProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam );


protected:

    IChangeManager*         m_pSceneCCM;
    IChangeManager*         m_pObjectCCM;

    UScene*                 m_pUScene;

    HINSTANCE               m_hInstance;
    HWND                    m_hWnd;

    RECT                    m_rcVerSplit;
    RECT                    m_rcSceneTree;
    RECT                    m_rcPropertyList;

    HWND                    m_hSceneTreeCtrl;
    HWND                    m_hPropertyListCtrl;

    BOOL                    m_bCaptured;

    HTREEITEM               m_hSceneRoot;
    HTREEITEM               m_hObjectsRoot;
};
#endif
#endif
