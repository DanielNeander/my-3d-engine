#pragma once 





#include "GameApp/stdafx.h"

#ifdef _DEBUG
#pragma comment( lib, "wxmsw28d_core.lib" )	// wxCore Debug Lib
#pragma comment( lib, "wxmsw28d_adv.lib" )
#pragma comment( lib, "wxmsw28d_qa.lib" )
#pragma comment( lib, "wxmsw28d_aui.lib" )
#pragma comment(lib, "cximagecrtd.lib")
#pragma comment(lib, "libmpqd.lib")
// Winsock 2
#pragma comment( lib, "ws2_32.lib ") // This lib is required by wxbase28_net lib	

#pragma comment( lib, "wxzlibd.lib" )
#pragma comment( lib, "wxregexd.lib" )
#pragma comment( lib, "wxbase28d.lib" )
#pragma comment( lib, "wxbase28d_net.lib" )
#pragma comment( lib, "wxexpatd.lib" )
#pragma comment( lib, "wxbase28d_xml.lib" )

#pragma comment( lib, "Effects11d.lib")
//#pragma comment( lib, "Dxutd.lib")
//#pragma comment( lib, "DxutOptd.lib")
#pragma comment( lib, "openSteerLibd.lib" )
#pragma comment( lib, "d3dx9d.lib" )

#pragma comment( lib, "DXTCompressorDLL_2010D.lib")

// cxImage
//#ifdef _WINDOWS
//#if _MSC_VER==1600		// If VC100 (VS2010)
//#ifdef _WIN64
//#pragma message("     Adding library: cximagecrt64d_VC100.lib" ) 
//#pragma comment( lib, "cximagecrt64d_VC100.lib" )
//#else
//#pragma message("     Adding library: cximagecrt32d_VC100.lib" ) 
//#pragma comment( lib, "cximagecrt32d_VC100.lib" )
//#endif
//#elif _MSC_VER==1500	// If VC90 (VS2008)
//#ifdef _WIN64
//#pragma message("     Adding library: cximagecrt64d_VC90.lib" ) 
//#pragma comment( lib, "cximagecrt64d_VC90.lib" )
//#else
//#pragma message("     Adding library: cximagecrt32d_VC90.lib" ) 
//#pragma comment( lib, "cximagecrt32d_VC90.lib" )
//#endif
//#else					// Otherwise
//#pragma message("     Adding library: cximagecrtd.lib" ) 
//#pragma comment( lib, "cximagecrtd.lib" )
//#endif
//#else
//#pragma message("     Adding library: cximagecrtd.lib" ) 
//#pragma comment( lib, "cximagecrtd.lib" )
//#endif
#else // Release
#define NDEBUG			// Disables Asserts in release
#define VC_EXTRALEAN	// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

#ifdef _WINDOWS
#pragma comment( lib, "wxmsw28_core.lib" )
#pragma comment( lib, "wxmsw28_adv.lib" )
#pragma comment( lib, "wxmsw28_qa.lib" )
#pragma comment( lib, "wxmsw28_aui.lib" )

#pragma comment( lib, "ws2_32.lib ") // This lib is required by wxbase28_net lib
#elif _MAC
#pragma comment( lib, "wxmac28_core.lib" )
#pragma comment( lib, "wxmac28_adv.lib" )
#pragma comment( lib, "wxmac28_gl.lib" )
#pragma comment( lib, "wxmac28_qa.lib" )
#pragma comment( lib, "wxmac28_aui.lib" )
#endif

#pragma comment( lib, "wxzlib.lib" )
#pragma comment( lib, "wxregex.lib" )
#pragma comment( lib, "wxbase28.lib" )
#pragma comment( lib, "wxbase28_net.lib" )
#pragma comment( lib, "wxexpat.lib" )
#pragma comment( lib, "wxbase28_xml.lib" )

#pragma comment(lib, "cximagecrt.lib")
#pragma comment( lib, "Effects11.lib")

#pragma comment( lib, "d3dx9.lib" )
//#pragma comment( lib, "Dxut.lib")	
//#pragma comment( lib, "DxutOpt.lib")

#pragma comment( lib, "DXTCompressorDLL_2010.lib")

//#pragma comment( lib, "openSteerLib.lib" )


//// cxImage
//#if defined(_WINDOWS)
//#if _MSC_VER==1600		// If VC100 (VS2010)
//#ifdef _WIN64
//#pragma message("     Adding library: cximagecrt64_VC100.lib" ) 
//#pragma comment( lib, "cximagecrt64_VC100.lib" )
//#else
//#pragma message("     Adding library: cximagecrt32_VC100.lib" ) 
//#pragma comment( lib, "cximagecrt32_VC100.lib" )
//#endif
//#elif _MSC_VER==1500	// If VC90 (VS2008)
//#ifdef _WIN64
//#pragma message("     Adding library: cximagecrt64_VC90.lib" ) 
//#pragma comment( lib, "cximagecrt64_VC90.lib" )
//#else
//#pragma message("     Adding library: cximagecrt32_VC90.lib" ) 
//#pragma comment( lib, "cximagecrt32_VC90.lib" )
//#endif
//#else					// Otherwise
//#pragma message("     Adding library: cximagecrt.lib" ) 
//#pragma comment( lib, "cximagecrt.lib" )
//#endif
//#else
//#pragma message("     Adding library: cximagecrt.lib" ) 
//#pragma comment( lib, "cximagecrt.lib" )
//#endif
#endif // _DEBUG

#define wxUSE_GUI 1
#ifdef _DEBUG
#define __WXDEBUG__
#define WXDEBUG 1
#endif

// use wxWidgets as a DLL
#if defined(WXUSINGDLL) && !WXUSINGDLL
#undef WXUSINGDLL
#endif

#ifndef WXUSINGDLL
#define WXUSINGDLL 0
#endif

// this is needed in VC8 land to get the XP look and feel.  (c.f. http://www.wxwidgets.org/wiki/index.php/MSVC )
#if wxUSE_GUI
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='X86' publicKeyToken='6595b64144ccf1df'\"")
#endif // wxUSE_GUI

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include <wx/notebook.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/listbox.h>
#include <wx/listctrl.h>
#include <wx/xrc/xmlres.h>
#include <wx/spinbutt.h>
#include <wx/colordlg.h>
#include <wx/scrolbar.h>
#include <wx/scrolwin.h>
#include <wx/image.h>
#include <wx/gauge.h>
#include <wx/toolbar.h>
#include <wx/dialog.h>
#include <wx/statusbr.h>
#include <wx/valgen.h>
#include <wx/dnd.h>
#include <wx/wizard.h>
#include <wx/html/htmlwin.h>
#include <wx/splash.h>
#include <wx/imaglist.h>
#include <wx/tglbtn.h>
#include <wx/statline.h>
#include <wx/spinctrl.h>
#include <wx/grid.h>
#include <wx/dcbuffer.h>
#include <wx/tipdlg.h>
#include <wx/wfstream.h>
#include <wx/tooltip.h>

#include <wx/aui/framemanager.h>


#include "FCallbackDevice.h"

#include "ResourceIDs.h"
#include "Bitmaps.h"
#include "DockingParent.h"
#include "Docking.h"

#include "GameApp/Client.h"
#include "GameApp/Canvas.h"