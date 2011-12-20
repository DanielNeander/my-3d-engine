/**********************************************************************
 *<
	FILE: U2MaxExporter.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#include "U2MaxExporter.h"
#include "resource.h"

#include "U2MaxSceneExport.h"
#include "U2MaxAnimExport.h"
#include "U2MaxMeshExport.h"
#include "U2MaxSkinPartition.h"
#include "U2MaxUtil.h"

#define U2Export_CLASS_ID	Class_ID(0xc60421b9, 0xc6635e1a)

static DWORD WINAPI fn(LPVOID arg) { return(0); }


class U2Export : public SceneExport {
	public:
		
		static HWND hParams;
		
		int				ExtCount();					// Number of extensions supported
		const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
		const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
		const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
		const TCHAR *	AuthorName();				// ASCII Author name
		const TCHAR *	CopyrightMessage();			// ASCII Copyright message
		const TCHAR *	OtherMessage1();			// Other message #1
		const TCHAR *	OtherMessage2();			// Other message #2
		unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
		void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

		BOOL SupportsOptions(int ext, DWORD options);
		int				DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);

		//Constructor/Destructor
		U2Export();
		~U2Export();		

};



class U2ExportClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new U2Export(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return U2Export_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return TSTR("U2Export"); }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }					// returns owning module handle
	

};

static U2ExportClassDesc U2ExportDesc;
ClassDesc2* GetU2ExportDesc() { return &U2ExportDesc; }





INT_PTR CALLBACK U2ExportOptionsDlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam) {
	static U2Export *imp = NULL;

	switch(message) {
		case WM_INITDIALOG:
			imp = (U2Export *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return 1;

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_OK:
				{

					//BOOL ok;
					//UINT val = GetDlgItemInt(hWnd, IDC_EDIT, &ok, false);
					//if (ok)
					//{
					//	imp->setSampleDelta(val);
					//}				
				}
				EndDialog(hWnd, 1);
				break;
			case IDC_BUTTON_CANCEL:
				EndDialog(hWnd, 0);
				break;
			}
		default:
			return FALSE;
	}
	return TRUE;
}


//--- U2Export -------------------------------------------------------
U2Export::U2Export()
{

}

U2Export::~U2Export() 
{

}

int U2Export::ExtCount()
{
	//TODO: Returns the number of file name extensions supported by the plug-in.
	return 1;
}

const TCHAR *U2Export::Ext(int n)
{		
	//TODO: Return the 'i-th' file name extension (i.e. "3DS").
	return TSTR("");
}

const TCHAR *U2Export::LongDesc()
{
	//TODO: Return long ASCII description (i.e. "Targa 2.0 Image File")
	return TSTR("");
}
	
const TCHAR *U2Export::ShortDesc() 
{			
	//TODO: Return short ASCII description (i.e. "Targa")
	return TSTR("");
}

const TCHAR *U2Export::AuthorName()
{			
	//TODO: Return ASCII Author name
	return TSTR("");
}

const TCHAR *U2Export::CopyrightMessage() 
{	
	// Return ASCII Copyright message
	return TSTR("");
}

const TCHAR *U2Export::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return TSTR("");
}

const TCHAR *U2Export::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return TSTR("");
}

unsigned int U2Export::Version()
{				
	//TODO: Return Version number * 100 (i.e. v3.01 = 301)
	return 100;
}

void U2Export::ShowAbout(HWND hWnd)
{			
	// Optional
}

BOOL U2Export::SupportsOptions(int ext, DWORD options)
{
	// TODO Decide which options to support.  Simply return
	// true for each option supported by each Extension 
	// the exporter supports.

	return TRUE;
}


int	U2Export::DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts, DWORD options)
{
	//TODO: Implement the actual file Export here and 
	//		return TRUE If the file is exported properly

	if(!suppressPrompts)
		DialogBoxParam(hInstance, 
				MAKEINTRESOURCE(IDD_PANEL), 
				GetActiveWindow(), 
				U2ExportOptionsDlgProc, (LPARAM)this);

	// Check Selected Scene  
	bool bExportSelected = false;
	bool bParam = SCENE_EXPORT_SELECTED;
	if(options && bParam)
	{
		bExportSelected = true;
	}

	i->ProgressStart(_T("Exporting to MRC File: ..."), TRUE, fn, NULL);
	
	U2MaxSceneExport sceneExport;

	sceneExport.ExportScene(i, name);
	
	i->ProgressEnd();


	return FALSE;
}


