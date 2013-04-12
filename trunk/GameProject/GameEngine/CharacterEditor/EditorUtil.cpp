#include "EditorPCH.h"
#include "EditorUtil.h"

/*-----------------------------------------------------------------------------
	FTrackPopupMenu.
-----------------------------------------------------------------------------*/

FTrackPopupMenu::FTrackPopupMenu( wxWindow* InWindow, wxMenu* InMenu ):
Window( InWindow ),
Menu( InMenu )
{
	assert( Window );
	assert( Menu );
}

void FTrackPopupMenu::Show( INT InX, INT InY )
{
	wxPoint pt( InX, InY );

	// Display at the current mouse position?
	if( InX < 0 || InY < 0 )
	{
		pt = Window->ScreenToClient( wxGetMousePosition() );
	}

	Window->PopupMenu( Menu, pt );
}