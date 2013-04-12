/*=============================================================================
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#ifndef __PROPERTIES_H__
#define __PROPERTIES_H__

#include "Bitmaps.h"

#define PROP_CategoryHeight		24
#define PROP_CategorySpacer		8
#define PROP_GenerateHeight		-1
#define	PROP_DefaultItemHeight	20
#define PROP_Indent				16

class WxPropertyWindow;
class WxPropertyWindowFrame;
class UPropertyDrawProxy;
class UPropertyInputProxy;
class WxPropertyWindow_Objects;


///////////////////////////////////////////////////////////////////////////////
//
// Property windows.
//
///////////////////////////////////////////////////////////////////////////////

class FDeferredInitializationWindow
{
	/** whether Create has been called on this window yet */
	UBOOL	bCreated;

public:
	FDeferredInitializationWindow()
		:	bCreated( FALSE )
	{}

	virtual ~FDeferredInitializationWindow() {}

	/** returns whether Create has been called on this window yet */
	UBOOL IsCreated() const
	{
		return bCreated;
	}

	/** Changes the value of bCreated to true...called when this window is created */
	void RegisterCreation()
	{
		bCreated = TRUE;
	}
};

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Base
-----------------------------------------------------------------------------*/

/**
 * Base class for all window types that appear in the property window.
 */
class WxPropertyWindow_Base : public wxWindow, public FDeferredInitializationWindow
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow_Base);

	/** Destructor */
	virtual ~WxPropertyWindow_Base();

	/**
	 * Initialize this property window.  Must be the first function called after creating the window.
	 */
	virtual void Create(	wxWindow* InParent,
							WxPropertyWindow_Base* InParentItem,
							WxPropertyWindow* InTopPropertyWindow,
							UProperty* InProperty,
							INT InPropertyOffset,
							INT	 InArrayIdx,
							UBOOL bInSupportsCustomControls=FALSE );

	/** How far the text label for this item is indented when drawn. */
	INT IndentX;

	/** The items parented to this one. */
	TArray<WxPropertyWindow_Base*> ChildItems;

	/** Is this item expanded? */
	UBOOL					bExpanded;
	/** Does this item allow expansion? */
	UBOOL					bCanBeExpanded;
	/** Parent item/window. */
	WxPropertyWindow_Base*	ParentItem;

protected:
	/** The top level property window.  Always valid. */
	WxPropertyWindow*		TopPropertyWindow;

public:
	/** LL next. */
	WxPropertyWindow_Base*	Next;

	/** LL prev. */
	WxPropertyWindow_Base*	Prev;

	/** The property being displayed/edited. */
	UProperty*				Property;

	/** The proxy used to draw this windows property. */
	UPropertyDrawProxy*		DrawProxy;

	/** The proxy used to handle user input for this property. */
	UPropertyInputProxy*	InputProxy;

	/** Offset to the properties data. */
	INT						PropertyOffset;
	INT						ArrayIndex;

	/** Used when the property window is positioning/sizing child items. */
	INT						ChildHeight;
	/** Used when the property window is positioning/sizing child items. */
	INT						ChildSpacer;

	/** The last x position the splitter was at during a drag operation. */
	INT						LastX;

protected:
	/** Should the child items of this window be sorted? */
	UBOOL					bSorted;

public:

	/** TRUE if the property can be expanded into the property window. */
	UBOOL bEditInline;

	/** TRUE if the property is EditInline with a use button. */
	UBOOL bEditInlineUse;

	/** Flag indicating whether or not only a single object is selected. */
	UBOOL bSingleSelectOnly;

	/** Flag indicating whether this property window supports custom controls */
	UBOOL bSupportsCustomControls;

	/**
	 * Returns the property window this node belongs to.  The returned pointer is always valid.
	 */
	WxPropertyWindow* GetPropertyWindow();

	/**
	 *  @return	The height of the property item, as determined by the input and draw proxies.
	 */
	INT GetPropertyHeight();

	/**
	 * Returns a string representation of the contents of the property.
	 */
	FString GetPropertyText();

};

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Objects
-----------------------------------------------------------------------------*/

/**
 * This holds all the child controls and anything related to
 * editing the properties of a collection of UObjects.
 */
class WxPropertyWindow_Objects : public WxPropertyWindow_Base
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow_Objects);

	//////////////////////////////////////////////////////////////////////////
	// Constructors

	/**
	 * Initialize this property window.  Must be the first function called after creating the window.
	 */
	virtual void Create(	wxWindow* InParent,
							WxPropertyWindow_Base* InParentItem,
							WxPropertyWindow* InTopPropertyWindow,
							UProperty* InProperty,
							INT InPropertyOffset,
							INT InArrayIdx,
							UBOOL bInSupportsCustomControls=FALSE);

};

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Category
-----------------------------------------------------------------------------*/

/**
 * The header item for a category of items.
 */
class WxPropertyWindow_Category : public WxPropertyWindow_Base
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow_Category);

	/**
	 * Initialize this property window.  Must be the first function called after creating the window.
	 */
	virtual void Create(	FName InCategoryName,
							wxWindow* InParent,
							WxPropertyWindow_Base* InParentItem,
							WxPropertyWindow* InTopPropertyWindow,
							UProperty* InProperty,
							INT InPropertyOffset,
							INT InArrayIdx,
							UBOOL bInSupportsCustomControls=FALSE);

};

/*-----------------------------------------------------------------------------
	WxPropertyWindow_Item
-----------------------------------------------------------------------------*/

class WxPropertyWindow_Item : public WxPropertyWindow_Base
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow_Item);

	/**
	 * Initialize this property window.  Must be the first function called after creating the window.
	 */
	virtual void Create(	wxWindow* InParent,
							WxPropertyWindow_Base* InParentItem,
							WxPropertyWindow* InTopPropertyWindow,
							UProperty* InProperty,
							INT InPropertyOffset,
							INT InArrayIdx,
							UBOOL bInSupportsCustomControls=FALSE);

	virtual BYTE* GetBase( BYTE* Base );
	virtual BYTE* GetContents( BYTE* Base );
};

///////////////////////////////////////////////////////////////////////////////

/**
 * Top level window for any property window (other than the optional WxPropertyWindowFrame).
 * WxPropertyWindow objects automatically register/unregister themselves with GPropertyWindowManager
 * on construction/destruction.
 */
class WxPropertyWindow : public wxWindow, public FDeferredInitializationWindow
{
public:
	DECLARE_DYNAMIC_CLASS(WxPropertyWindow);

	/** Destructor */
	virtual ~WxPropertyWindow();

	/**
	 * Initialize this window class.  Must be the first function called after creation.
	 *
	 * @param	parent			The parent window.
	 * @param	InNotifyHook	An optional callback that receives property PreChange and PostChange notifies.
	 */
	virtual void Create( wxWindow* InParent, FNotifyHook* InNotifyHook );


	DECLARE_EVENT_TABLE();

protected:
	/** The path names to every expanded item in the tree. */
	//TArray<FString> ExpandedItems;

	/** Flag for whether or not the ACTOR DESELECT command is executed when the Escape key is pressed. */
	UBOOL bExecDeselectOnEscape;

	/** The position of the break between the variable names and their values/editing areas. */
	INT SplitterPos;

	/** The item window that last had focus. */
	WxPropertyWindow_Base* LastFocused;

	/** If non-zero, we do not process rebuild requests, this should be set before operations that may update the property window multiple times and cleared afterwards. */
	INT RebuildLocked;

	/** If 1, the user is dragging the splitter bar. */
	UBOOL bDraggingSplitter;

	/** Should the child items of this window be sorted? */
	UBOOL bSorted;

	/** If TRUE, show property categories in this window. */
	UBOOL bShowCategories;

	/** If TRUE, show non-editable properties in this window. */
	UBOOL bShowNonEditable;

	/** The property is read-only		*/
	UBOOL bReadOnly;

	/** Flag indicating whether this property window supports custom controls */
	UBOOL bSupportsCustomControls;

	/** Flag indicating whether this property window can be hidden by the PropertyWindowManager. */
	UBOOL bCanBeHiddenByPropertyWindowManager;

	/** Flag indicating whether the property window should react to color pick events. */
	UBOOL bColorPickModeEnabled;

	/** True if we want to allow the ENTER key to be intercepted by text property fields and used to
	    apply the text input.  This is usually what you want.  Otherwise, the ENTER key will be ignored
		and the parent dialog window will have a chance to handle the event */
	UBOOL bAllowEnterKeyToApplyChanges;

	wxScrollBar* ScrollBar;
	INT ThumbPos;
	INT MaxH;

	FNotifyHook* NotifyHook;

	/** 
	* Command handler for the rebuild event.
	*
	* @param	Event	Information about the event.
	*/
	virtual void OnRebuild(wxCommandEvent &Event);

private:
	/**
	 * Performs necessary tasks (remember expanded items, etc.) performed before setting an object.
	 */
	void PreSetObject();
	
	/**
	 * Performs necessary tasks (restoring expanded items, etc.) performed after setting an object.
	 */
	void PostSetObject(UBOOL InExpandCategories, UBOOL InSorted, UBOOL InShowCategories,UBOOL InShowNonEditable);

	/**
	 * Links up the Next/Prev pointers the children.  Needed for things like keyboard navigation.
	 */
	void LinkChildren();

	/**
	 * Recursive minion of LinkChildren.
	 */
	WxPropertyWindow_Base* LinkChildrenForItem( WxPropertyWindow_Base* InItem );

	/**
	 * Recursive minion of PositionChildren.
	 *
	 * @param	InX		The horizontal position of the child item.
	 * @param	InY		The vertical position of the child item.
	 * @return			An offset to the current Y for the next position.
	 */
	INT PositionChild( WxPropertyWindow_Base* InItem, INT InX, INT InY );

	/** The first item window of this property window. */
	WxPropertyWindow_Objects* Root;

	friend class WxPropertyWindowFrame;
	friend class WxPropertyWindow_Objects;
	friend class WxPropertyWindow_Category;
};

#endif