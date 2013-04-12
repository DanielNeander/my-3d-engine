#include "EditorPCH.h"
#include "EditorUtil.h"
#include "Client.h"
#include "EditorViewport.h"
#include "UnLinkedObjDrawUtils.h"
#include "UnLinkedObjEditor.h"
#include "AnimTreeEditor.h"
#include "Properties.h"
#include "DlgGenericComboEntry.h"


/*-----------------------------------------------------------------------------
	WxMBAnimTreeEdNewNode
-----------------------------------------------------------------------------*/

class WxMBAnimTreeEdNewNode : public wxMenu
{
public:
	WxMBAnimTreeEdNewNode(WxAnimTreeEditor* AnimTreeEd)
	{

	}
};

/*-----------------------------------------------------------------------------
	WxMBAnimTreeEdNodeOptions
-----------------------------------------------------------------------------*/

class WxMBAnimTreeEdNodeOptions : public wxMenu
{
public:
	WxMBAnimTreeEdNodeOptions(WxAnimTreeEditor* AnimTreeEd)
	{
		INT NumSelected = AnimTreeEd->SelectedNodes.Num();
		if(NumSelected == 1)
		{

		}
	}
};

class WxMBAnimTreeEdConnectorOptions : public wxMenu
{
public:
	WxMBAnimTreeEdConnectorOptions(WxAnimTreeEditor* AnimTreeEd)
	{

	}
};

void WxAnimTreeEditor::OpenNewObjectMenu()
{
	WxMBAnimTreeEdNewNode menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxAnimTreeEditor::OpenObjectOptionsMenu()
{
	WxMBAnimTreeEdNodeOptions menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxAnimTreeEditor::OpenConnectorOptionsMenu()
{
	WxMBAnimTreeEdConnectorOptions menu( this );
	FTrackPopupMenu tpm( this, &menu );
	tpm.Show();
}

void WxAnimTreeEditor::DrawObjects(FViewport* Viewport, FCanvas* Canvas)
{

}

/*-----------------------------------------------------------------------------
	WxAnimTreeEditorToolBar.
-----------------------------------------------------------------------------*/

class WxAnimTreeEditorToolBar : public wxToolBar
{
public:
	WxAnimTreeEditorToolBar( wxWindow* InParent, wxWindowID InID );

private:
	WxMaskedBitmap TickTreeB;
	WxMaskedBitmap PreviewNodeB;
	WxMaskedBitmap ShowNodeWeightB;
	WxMaskedBitmap ShowBonesB;
	WxMaskedBitmap ShowBoneNamesB;
	WxMaskedBitmap ShowWireframeB;
	WxMaskedBitmap ShowFloorB;
	WxMaskedBitmap CurvesB;
};

WxAnimTreeEditorToolBar::WxAnimTreeEditorToolBar( wxWindow* InParent, wxWindowID InID )
	:	wxToolBar( InParent, InID, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL | wxTB_3DBUTTONS )
{
	// create the return to parent sequence button
	TickTreeB.Load(TEXT("AnimTree_TickTree"));
	PreviewNodeB.Load(TEXT("AnimTree_PrevNode"));
	ShowNodeWeightB.Load(TEXT("AnimTree_ShowNodeWeight"));
	ShowBonesB.Load(TEXT("AnimTree_ShowBones"));
	ShowBoneNamesB.Load(TEXT("AnimTree_ShowBoneNames"));
	ShowWireframeB.Load(TEXT("AnimTree_ShowWireframe"));
	ShowFloorB.Load(TEXT("AnimTree_ShowFloor"));
	CurvesB.Load(TEXT("KIS_DrawCurves"));

	SetToolBitmapSize( wxSize( 16, 16 ) );

	AddSeparator();
	AddCheckTool(IDM_ANIMTREE_TOGGLETICKTREE, TEXT("PauseAnimTree"), TickTreeB, wxNullBitmap, TEXT("PauseAnimTree"));
	AddSeparator();
	AddTool(IDM_ANIMTREE_PREVIEWSELECTEDNODE, PreviewNodeB, TEXT("PreviewSelectedNode"));
	AddCheckTool(IDM_ANIMTREE_SHOWNODEWEIGHT, TEXT("ShowNodeWeight"), ShowNodeWeightB, wxNullBitmap, TEXT("ShowNodeWeight"));
	AddSeparator();
	AddCheckTool(IDM_ANIMTREE_SHOWHIERARCHY, TEXT("ShowSkeleton"), ShowBonesB, wxNullBitmap, TEXT("ShowSkeleton"));
	AddCheckTool(IDM_ANIMTREE_SHOWBONENAMES, TEXT("ShowBoneNames"), ShowBoneNamesB, wxNullBitmap, TEXT("ShowBoneNames"));
	AddCheckTool(IDM_ANIMTREE_SHOWWIREFRAME, TEXT("ShowWireframe"), ShowWireframeB, wxNullBitmap, TEXT("ShowWireframe"));
	AddCheckTool(IDM_ANIMTREE_SHOWFLOOR, TEXT("ShowFloor"), ShowFloorB, wxNullBitmap, TEXT("ShowFloor"));
	AddCheckTool(IDM_ANIMTREE_SHOWCURVES, TEXT("ToggleCurvedConnections"), CurvesB, wxNullBitmap, TEXT("ToggleCurvedConnections"));

	Realize();
}