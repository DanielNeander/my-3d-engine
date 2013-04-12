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

//#include <windows.h>
//#include <commctrl.h>
#include "stdafx.h"

#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

#include "Universal.h"
#define __DEBUG_WINDOW__
#include "DebugWindow.h"

#if defined( _DEBUG ) || defined( PROFILE )


LPCTSTR sm_pszClassName = TEXT( "Engine_Debug_Window" );


Handle
CreateDebugWindow(
    void
    )
{
    DebugWindow* pDebugWindow = new DebugWindow();
    return pDebugWindow;
}


void
DestroyDebugWindow(
    Handle hDebugWindow
    )
{
    delete reinterpret_cast<DebugWindow*>(hDebugWindow);
}


void
SetDebugWindowCCM(
    Handle hDebugWindow,
    IChangeManager* pSceneCCM,
    IChangeManager* pObjectCCM
    )
{
    reinterpret_cast<DebugWindow*>(hDebugWindow)->SetCCM( pSceneCCM, pObjectCCM );
}


void
SetDebugWindowUScene(
    Handle hDebugWindow,
    UScene* pUScene
    )
{
    reinterpret_cast<DebugWindow*>(hDebugWindow)->SetUScene( pUScene );
}


DebugWindow::DebugWindow(
    void
    )
    : m_hInstance( NULL )
    , m_hWnd( NULL )
    , m_hSceneTreeCtrl( NULL )
    , m_hPropertyListCtrl( NULL )
{
    //
    // Initialize the common controls.
    //
    INITCOMMONCONTROLSEX icce;
    icce.dwSize = sizeof icce;
    icce.dwICC = ICC_LISTVIEW_CLASSES | ICC_TREEVIEW_CLASSES;
    ::InitCommonControlsEx( &icce );

    //
    // Get the instance.
    //
    m_hInstance = (HINSTANCE)::GetCurrentProcess();

    //
    // Register a new class for the window.
    //
    WNDCLASSEX wcex;

    wcex.cbSize         = sizeof wcex;
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WindowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = m_hInstance;
    wcex.hIcon          = NULL;
    wcex.hCursor        = ::LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground  = (HBRUSH)(COLOR_APPWORKSPACE+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = sm_pszClassName;
    wcex.hIconSm        = NULL;

    if ( ::RegisterClassEx( &wcex ) != 0 )
    {
        //
        // Create the window.
        //
        m_hWnd = ::CreateWindow( sm_pszClassName, TEXT( "Engine Debug Window" ), WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT, 0, 800, 600, NULL, NULL, m_hInstance,
                                 this );

        if ( m_hWnd != NULL )
        {
            ::ShowWindow( m_hWnd, SW_SHOWNORMAL );
            ::UpdateWindow( m_hWnd );
        }
        else
        {
            ASSERT( False );
        }
    }
    else
    {
        ASSERT( FALSE );
    }
}


DebugWindow::~DebugWindow(
    void
    )
{
    Unregister();

    if ( m_hWnd != NULL )
    {
        ::DestroyWindow( m_hWnd );
    }

    ::UnregisterClass( sm_pszClassName, m_hInstance );
}


void
DebugWindow::SetCCM(
    IChangeManager* pSceneCCM,
    IChangeManager* pObjectCCM
    )
{
    m_pSceneCCM = pSceneCCM;
    m_pObjectCCM = pObjectCCM;
}


void
DebugWindow::SetUScene(
    UScene* pUScene
    )
{
#ifdef _UNICODE
    wchar_t   szOEM[ 256 ];
#endif

    //
    // Save the UScene pointer.
    //
    m_pUScene = pUScene;

    //
    // Clear out the tree and start over.
    //
    Unregister();
    TreeView_DeleteAllItems( m_hSceneTreeCtrl );

    //
    // Set the scene root item.
    //
    TVINSERTSTRUCT tvisRoot;
    tvisRoot.hParent = TVI_ROOT;
    tvisRoot.hInsertAfter = TVI_LAST;
    tvisRoot.item.mask = TVIF_TEXT;
    tvisRoot.item.pszText = TEXT( "Scene" );

    m_hSceneRoot = TreeView_InsertItem( m_hSceneTreeCtrl, &tvisRoot );

    //
    // Add the system scenes to the tree.
    //
    UScene::SystemScenes Scenes = m_pUScene->GetSystemScenes();

    for ( UScene::SystemScenesConstIt it=Scenes.begin(); it != Scenes.end(); it++ )
    {
        //
        // Get the scene.
        //
        ISystemScene* pScene = it->second;

        m_pSceneCCM->Register( pScene, Interface::System::Changes::All, this );

        //
        // Add the scene to the tree.
        //
        TVINSERTSTRUCT tvis;
        tvis.hParent = m_hSceneRoot;
        tvis.hInsertAfter = TVI_LAST;
        tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
#ifdef _UNICODE
        ::OemToChar( pScene->GetSystem()->GetName(), szOEM );
        tvis.item.pszText = szOEM;
#else
        tvis.item.pszText = (CHAR*)pScene->GetSystem()->GetName();
#endif
        tvis.item.lParam = (LPARAM)pScene;

        TreeView_InsertItem( m_hSceneTreeCtrl, &tvis );
    }

    //
    // Set the objects root item.
    //
    tvisRoot.hParent = TVI_ROOT;
    tvisRoot.hInsertAfter = TVI_LAST;
    tvisRoot.item.mask = TVIF_TEXT;
    tvisRoot.item.pszText = TEXT( "Objects" );

    m_hObjectsRoot = TreeView_InsertItem( m_hSceneTreeCtrl, &tvisRoot );

    //
    // Add the objects to the tree.
    //
    UScene::Objects Objects = m_pUScene->GetObjects();

    for ( UScene::ObjectsConstIt it=Objects.begin(); it != Objects.end(); it++ )
    {
        //
        // Get the UObject.
        //
        uObject* pUObject = *it;

        //
        // Add the UObject to the tree.
        //
        TVINSERTSTRUCT tvis;
        tvis.hParent = m_hObjectsRoot;
        tvis.hInsertAfter = TVI_LAST;
        tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
#ifdef _UNICODE
        ::OemToChar( pUObject->GetName(), szOEM );
        tvis.item.pszText = szOEM;
#else
        tvis.item.pszText = (CHAR*)pUObject->GetName();
#endif
        tvis.item.lParam = (LPARAM)pUObject;

        HTREEITEM hti = TreeView_InsertItem( m_hSceneTreeCtrl, &tvis );

        //
        // Add the system objects to the tree.
        //
        uObject::SystemObjects SystemObjects = pUObject->GetExtensions();

        for ( uObject::SystemObjectsConstIt it=SystemObjects.begin();
              it != SystemObjects.end(); it++ )
        {
            //
            // Get the system object.
            //
            ISystemObject* pObject = it->second;

            m_pObjectCCM->Register( pObject, Interface::System::Changes::All, this );

            //
            // Add the system object to the tree.
            //
            TVINSERTSTRUCT tvis;
            tvis.hParent = hti;
            tvis.hInsertAfter = TVI_LAST;
            tvis.item.mask = TVIF_TEXT | TVIF_PARAM;
#ifdef _UNICODE
            ::OemToChar( pObject->GetSystemScene()->GetSystem()->GetName(), szOEM );
            tvis.item.pszText = szOEM;
#else
            tvis.item.pszText = (CHAR*)pObject->GetSystemScene()->GetSystem()->GetName();
#endif
            tvis.item.lParam = (LPARAM)pObject;

            TreeView_InsertItem( m_hSceneTreeCtrl, &tvis );
        }
    }
    TreeView_SortChildren( m_hSceneTreeCtrl, m_hObjectsRoot, FALSE );
    TreeView_Expand( m_hSceneTreeCtrl, m_hObjectsRoot, TVE_EXPAND );
}


void
DebugWindow::Unregister(
    void
    )
{
    for ( HTREEITEM hTreeItem = TreeView_GetChild( m_hSceneTreeCtrl, m_hSceneRoot );
          hTreeItem != NULL; hTreeItem=TreeView_GetNextSibling( m_hSceneTreeCtrl, hTreeItem ) )
    {
        TVITEM tvItem;
        tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
        tvItem.hItem = hTreeItem;
        TreeView_GetItem( m_hSceneTreeCtrl, &tvItem );

        ISystemScene* pSystemScene = (ISystemScene*)tvItem.lParam;

        m_pSceneCCM->Unregister( pSystemScene, this );
    }

    for ( HTREEITEM htiObject = TreeView_GetChild( m_hSceneTreeCtrl, m_hObjectsRoot );
          htiObject != NULL; htiObject=TreeView_GetNextSibling( m_hSceneTreeCtrl, htiObject ) )
    {
        for ( HTREEITEM hTreeItem = TreeView_GetChild( m_hSceneTreeCtrl, htiObject );
              hTreeItem != NULL; hTreeItem=TreeView_GetNextSibling( m_hSceneTreeCtrl, hTreeItem ) )
        {
            TVITEM tvItem;
            tvItem.mask = TVIF_HANDLE | TVIF_PARAM;
            tvItem.hItem = hTreeItem;
            TreeView_GetItem( m_hSceneTreeCtrl, &tvItem );

            ISystemObject* pSystemObject = (ISystemObject*)tvItem.lParam;

            m_pObjectCCM->Unregister( pSystemObject, this );
        }
    }
}


Error
DebugWindow::ChangeOccurred(
    ISubject* pSubject,
    System::Changes::BitMask ChangeType
    )
{
    DBG_UNREFERENCED_PARAM( pSubject );
    DBG_UNREFERENCED_PARAM( ChangeType );

    return Errors::Success;
}


void
DebugWindow::OnCreate(
    void
    )
{
    //
    // Create the views for the debug window.
    //
    UINT Styles = WS_VISIBLE | WS_CHILD;

    m_hSceneTreeCtrl =
        ::CreateWindow( WC_TREEVIEW, NULL,
                        Styles | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS,
                        0, 0, 1, 1,
                        m_hWnd, NULL,
                        m_hInstance, NULL );

    m_hPropertyListCtrl =
        ::CreateWindow( WC_LISTVIEW, NULL,
                        Styles | LVS_LIST,
                        0, 0, 1, 1,
                        m_hWnd, NULL,
                        m_hInstance, NULL );

    RECT rcClient;
    ::GetClientRect( m_hWnd, &rcClient );
    m_rcVerSplit = rcClient;
    m_rcVerSplit.left = (rcClient.right / 3) - 2;
    m_rcVerSplit.right = m_rcVerSplit.left + 3;

    OnSize( rcClient.right, rcClient.bottom );

    m_bCaptured = FALSE;
}


void
DebugWindow::OnDestroy(
    void
    )
{
    Unregister();
    m_hWnd = NULL;
}



void
DebugWindow::OnLButtonDown(
    int x,
    int y
    )
{
    POINT pt = { x, y };

    if ( ::PtInRect( &m_rcVerSplit, pt ) )
    {
        ::SetCursor( ::LoadCursor( NULL, IDC_SIZEWE ) );
        ::SetCapture( m_hWnd );
        m_bCaptured = TRUE;
    }
}


void
DebugWindow::OnLButtonUp(
    int x,
    int y
    )
{
    UNREFERENCED_PARAM( x );
    UNREFERENCED_PARAM( y );

    if ( m_bCaptured )
    {
        ::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
        ::ReleaseCapture();
        m_bCaptured = FALSE;
    }
}


void
DebugWindow::OnMouseMove(
    int x,
    int y
    )
{
    if ( m_bCaptured )
    {
        RECT rcClient;
        ::GetClientRect( m_hWnd, &rcClient );

        m_rcVerSplit.left = x;
        m_rcVerSplit.right = m_rcVerSplit.left + 3;

        if ( m_rcVerSplit.left < 10 )
        {
            m_rcVerSplit.left = 10;
        }

        if ( m_rcVerSplit.right > rcClient.right - 10 )
        {
            m_rcVerSplit.left = rcClient.right - 13;
            m_rcVerSplit.right = m_rcVerSplit.left + 3;
        }

        OnSize( rcClient.right, rcClient.bottom );

        ::InvalidateRect( m_hWnd, NULL, FALSE );
        ::UpdateWindow( m_hWnd );
    }
    else
    {
        POINT pt = { x, y };

        if ( ::PtInRect( &m_rcVerSplit, pt ) )
        {
            ::SetCursor( ::LoadCursor( NULL, IDC_SIZEWE ) );
        }
        else
        {
            ::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
        }
    }
}


void
DebugWindow::OnSize(
    int Width,
    int Height
    )
{
    RECT rcClient;
    rcClient.left = 0;
    rcClient.top = 0;
    rcClient.right = Width;
    rcClient.bottom = Height;

    m_rcVerSplit.bottom = Height;

    m_rcSceneTree = rcClient;
    m_rcSceneTree.right = m_rcVerSplit.left - 1;

    m_rcPropertyList = rcClient;
    m_rcPropertyList.left = m_rcVerSplit.right + 1;

    if ( m_hSceneTreeCtrl != NULL )
    {
        ::SetWindowPos( m_hSceneTreeCtrl,
                        NULL,
                        m_rcSceneTree.left,
                        m_rcSceneTree.top,
                        m_rcSceneTree.right + m_rcSceneTree.left,
                        m_rcSceneTree.bottom + m_rcSceneTree.top,
                        SWP_SHOWWINDOW | SWP_NOOWNERZORDER );
    }

    if ( m_hPropertyListCtrl != NULL )
    {
        ::SetWindowPos( m_hPropertyListCtrl,
                        NULL,
                        m_rcPropertyList.left,
                        m_rcPropertyList.top,
                        m_rcPropertyList.right + m_rcPropertyList.left,
                        m_rcPropertyList.bottom + m_rcPropertyList.top,
                        SWP_SHOWWINDOW | SWP_NOOWNERZORDER );
    }
}


void
DebugWindow::OnPaint(
    void
    )
{
    PAINTSTRUCT Paint;
    HDC hDc = ::BeginPaint( m_hWnd, &Paint );

    ::FillRect( hDc, &m_rcVerSplit, (HBRUSH)(COLOR_HIGHLIGHT+1) );

    ::EndPaint( m_hWnd, &Paint );
}


LRESULT CALLBACK
DebugWindow::WindowProc(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
#pragma warning( push )
#pragma warning( disable : 4312 )
    DebugWindow* pDebugWindow = (DebugWindow*)::GetWindowLongPtr( hWnd, GWLP_USERDATA );
#pragma warning( pop )

    switch ( Msg )
    {
    case WM_NCCREATE:
    {
        pDebugWindow = (DebugWindow*)((LPCREATESTRUCT)lParam)->lpCreateParams;
        pDebugWindow->m_hWnd = hWnd;

#pragma warning( push )
#pragma warning( disable : 4244 )
        ::SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR)pDebugWindow );
#pragma warning( pop )
        
        return ::DefWindowProc( hWnd, Msg, wParam, lParam );
    }

    case WM_CREATE:
        pDebugWindow->OnCreate();
        break;

    case WM_DESTROY:
        pDebugWindow->OnDestroy();
        break;

    case WM_LBUTTONDOWN:
        pDebugWindow->OnLButtonDown( LOWORD( lParam ), HIWORD( lParam ) );
        break;

    case WM_LBUTTONUP:
        pDebugWindow->OnLButtonUp( LOWORD( lParam ), HIWORD( lParam ) );
        break;

    case WM_MOUSEMOVE:
        pDebugWindow->OnMouseMove( LOWORD( lParam ), HIWORD( lParam ) );
        break;

    case WM_SIZE:
        pDebugWindow->OnSize( LOWORD( lParam ), HIWORD( lParam ) );
        break;

	case WM_PAINT:
        pDebugWindow->OnPaint();
		break;

    default:
        return ::DefWindowProc( hWnd, Msg, wParam, lParam );
    }

    return 0;
}
#endif