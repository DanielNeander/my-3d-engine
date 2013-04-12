#pragma once 

/**
* Utility class for creating popup menus.
*/
class FTrackPopupMenu
{
public:
	/** Inputs must be valid pointers.  FTrackPopupMenu does not assume ownership of InWindow or InMenu. */
	FTrackPopupMenu( wxWindow* InWindow, wxMenu* InMenu );

	/**
	* Display the popup menu at the specified position.  If either of InX or InY is less than zero,
	* the menu appears at the current mouse position.
	*/
	void Show( INT InX = -1, INT InY = -1 );

private:
	/** The parent window. */
	wxWindow*	Window;
	/** The popup menu. */
	wxMenu*		Menu;
};