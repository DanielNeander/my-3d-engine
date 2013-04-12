/*=============================================================================
	AnimTreeEditor.h: AnimTree editing
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __ANIMTREEEDITOR_H__
#define __ANIMTREEEDITOR_H__

#include "TrackableWindow.h"


static INT	DuplicateOffset = 30;


class FAnimTreeEdPreviewVC : public FEditorLevelViewportClient
{
public:
	class WxAnimTreeEditor*		AnimTreeEd;

	//FPreviewScene				PreviewScene;

	/** Helper class that draws common scene elements. */
	//FEditorCommonDrawHelper		DrawHelper;

	UBOOL						bDrawingInfoWidget;
	UBOOL						bManipulating;
	EAxis						ManipulateAxis;
	INT							ManipulateWidgetIndex;

	FLOAT						DragDirX;
	FLOAT						DragDirY;
	FVector						WorldManDir;
	FVector						LocalManDir;

	FAnimTreeEdPreviewVC(class WxAnimTreeEditor* InAnimTreeEd);

	// FEditorLevelViewportClient interface

	//virtual FSceneInterface* GetScene() { return PreviewScene.GetScene(); }
	virtual FLinearColor GetBackgroundColor();
	virtual void Draw(FViewport* Viewport, FCanvas* Canvas);
	//virtual void Draw(const FSceneView* View,FPrimitiveDrawInterface* PDI);
	virtual void Tick(FLOAT DeltaSeconds);

	virtual UBOOL InputKey(FViewport* Viewport, INT ControllerId, FName Key, EInputEvent Event,FLOAT AmountDepressed = 1.f,UBOOL bGamepad=FALSE);
	virtual UBOOL InputAxis(FViewport* Viewport, INT ControllerId, FName Key, FLOAT Delta, FLOAT DeltaTime, UBOOL bGamepad=FALSE);
	virtual void MouseMove(FViewport* Viewport,INT x, INT y);

	//virtual void Serialize(FArchive& Ar);
};
/*-----------------------------------------------------------------------------
	WxAnimTreePreview
-----------------------------------------------------------------------------*/

/**
 * wxWindows Holder for FAnimTreeEdPreviewVC
 */
class WxAnimTreePreview : public wxWindow
{
public:
	FAnimTreeEdPreviewVC* AnimTreePreviewVC;

	WxAnimTreePreview(wxWindow* InParent, wxWindowID InID, class WxAnimTreeEditor* InAnimTreeEd, FVector& InViewLocation, FRotator& InViewRotation);
	~WxAnimTreePreview();

private:
	void OnSize( wxSizeEvent& In );

	DECLARE_EVENT_TABLE()
};



class WxAnimTreeEditor : public WxTrackableFrame, public FNotifyHook, public FDockingParent
{
public:
	/** AnimTree currently being edited. */
	class UAnimTree* AnimTree;

	/** All nodes in current tree (including the AnimTree itself). */
	TArray<class noAnimNode*> TreeNodes;

	/** Currently selected UAnimNodes. */
	TArray<class noAnimNode*> SelectedNodes;

	/** All USkelControls within tree. */
	TArray<class USkelControlBase*> TreeControls;

	/** Currently selected USkelControls. */
	TArray<class USkelControlBase*> SelectedControls;

	/** All UMorphNodes within tree. */
	TArray<class UMorphNodeBase*> TreeMorphNodes;

	/** Currently selected UMorphNodes. */
	TArray<class UMorphNodeBase*> SelectedMorphNodes;

	/** Array of nodes containing extra information/callbacks for editing custom node types. */
	TArray<class UAnimNodeEditInfo*> AnimNodeEditInfos; 

	/** Object containing selected connector. */
	//UObject* ConnObj;

	/** Type of selected connector. */
	INT ConnType;

	/** Index of selected connector. */
	INT ConnIndex;

	/** Whether we should be advancing the animations in the preview window. */
	UBOOL	bTickAnimTree;

	/** If true, draw bones on preview skeleton. */
	UBOOL	bShowSkeleton;

	/** If true, draw bone names on preview skeleton. */
	UBOOL	bShowBoneNames;

	/** If true, draw skeletal mesh in wireframe. */
	UBOOL	bShowWireframe;

	/** If true, draw floor mesh and allow feet to collide with it. */
	UBOOL	bShowFloor;

	/** If TRUE, render spline curves; if FALSE, render straight-line curves. */
	UBOOL bDrawCurves;

	/** Show the global percentage weight of each node in the top-right of the node. */
	UBOOL	bShowNodeWeights;

	/** 
	 *	If we are shutting down (will be true after OnClose is called). 
	 *	Needed because wxWindows does not call destructor right away, so things can get ticked after OnClose.
	 */
	UBOOL	bEditorClosing;

	/** 
	 *	Indicates if AnimNodeClasses and SkelControlClasses arrays have already been initialised. 
	 *	This is only done once, as new classes cannot be created at runtime. 
	 */
	static UBOOL			bAnimClassesInitialized;

	class WxPropertyWindow*				PropertyWindow;
	class WxAnimTreeEditorToolBar*	ToolBar;

	WxAnimTreeEditor( wxWindow* InParent, wxWindowID InID, class UAnimTree* InAnimTree );
	~WxAnimTreeEditor();

	void OnClose( wxCloseEvent& In );
	void RefreshViewport();

	/**
	 * This function is called when the window has been selected from within the ctrl + tab dialog.
	 */
	virtual void OnSelected();
	
	// FLinkedObjViewportClient interface

	virtual void DrawObjects(FViewport* Viewport, FCanvas* Canvas);
	virtual void OpenNewObjectMenu();
	virtual void OpenObjectOptionsMenu();
	virtual void OpenConnectorOptionsMenu();
	//virtual void DoubleClickedObject(UObject* Obj);
	virtual void UpdatePropertyWindow();
	virtual UBOOL DrawCurves() { return true; }

	virtual void EmptySelection();
	virtual INT GetNumSelected() const;

	virtual void MoveSelectedObjects( INT DeltaX, INT DeltaY );
	virtual void EdHandleKeyInput(FViewport* Viewport, FName Key, EInputEvent Event);
	virtual void SpecialDrag( INT DeltaX, INT DeltaY, INT NewX, INT NewY, INT SpecialIndex );
	virtual UBOOL SpecialClick( INT NewX, INT NewY, INT SpecialIndex, FViewport* Viewport );

	virtual void NotifyDestroy( void* Src );
	virtual void NotifyExec( void* Src, const TCHAR* Cmd );

	void SetPreviewNode(class noAnimNode* NewPreviewNode);
	void TickPreview(FLOAT DeltaSeconds);

	// Menu handlers
	void OnNewAnimNode( wxCommandEvent& In );
	void OnNewSkelControl( wxCommandEvent& In );
	void OnNewMorphNode( wxCommandEvent& In );
	void OnBreakLink( wxCommandEvent& In );
	void OnBreakAllLinks( wxCommandEvent& In );
	void OnAddInput( wxCommandEvent& In );
	void OnRemoveInput( wxCommandEvent& In );
	void OnNameInput( wxCommandEvent& In );
	void OnAddControlHead( wxCommandEvent& In );
	void OnRemoveControlHead( wxCommandEvent& In );
	void OnChangeBoneControlHead(wxCommandEvent& In);
	void OnDeleteObjects( wxCommandEvent& In );

	void OnToggleTickAnimTree( wxCommandEvent& In );
	void OnPreviewSelectedNode( wxCommandEvent& In );
	void OnShowNodeWeights( wxCommandEvent& In );
	void OnShowSkeleton( wxCommandEvent& In );
	void OnShowBoneNames( wxCommandEvent& In );
	void OnShowWireframe( wxCommandEvent& In );
	void OnShowFloor( wxCommandEvent& In );

	/** Toggle drawing logic connections as straight lines or curves. */
	void OnButtonShowCurves( wxCommandEvent &In );

	// Utils
	void DeleteSelectedObjects();
	void DuplicateSelectedObjects();
	void ReInitAnimTree();
	void BreakLinksToNode(noAnimNode* InNode);
	void BreakLinksToControl(USkelControlBase* InControl);
	void BreakLinksToMorphNode(UMorphNodeBase* InNode);

	// Static, initialisation stuff
	static void InitAnimClasses();
	
	protected:
	/**
	 *	This function returns the name of the docking parent.  This name is used for saving and loading the layout files.
	 *  @return A string representing a name to use for this docking parent.
	 */
	virtual const TCHAR* GetDockingParentName() const;

	/**
	 * @return The current version of the docking parent, this value needs to be increased every time new docking windows are added or removed.
	 */
	virtual const INT GetDockingParentVersion() const;

	DECLARE_EVENT_TABLE()
};


#endif