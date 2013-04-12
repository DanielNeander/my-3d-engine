#pragma once 

#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  undef NOMINMAX
#  define NOMINMAX // required to stop windows.h screwing up std::min definition

class _OgreExport WindowEventListener
	{
	public:
		virtual ~WindowEventListener() {};

		/**
		@Remarks
			Window has moved position
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowMoved(RenderWindow* rw)   {}

		/**
		@Remarks
			Window has resized
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowResized(RenderWindow* rw) {}

		/**
		@Remarks
			Window has been closed
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowClosed(RenderWindow* rw)  {}

		/**
		@Remarks
			Window has lost/gained focuw
		@param rw
			The RenderWindow which created this events
		*/
		virtual void windowFocusChange(RenderWindow* rw) {}
	};

	/**
	@Remarks
		Utility class to handle Window Events/Pumping/Messages
	*/
	class _OgreExport WindowEventUtilities
	{
	public:
		/**
		@Remarks
			Call this once per frame if not using Root:startRendering(). This will update all registered
			RenderWindows (If using external Windows, you can optionally register those yourself)
		*/
		static void messagePump();

		/**
		@Remarks
			Add a listener to listen to renderwindow events (multiple listener's per renderwindow is fine)
			The same listener can listen to multiple windows, as the Window Pointer is sent along with
			any messages.
		@param window
			The RenderWindow you are interested in monitoring
		@param listner
			Your callback listener
		*/
		static void addWindowEventListener( RenderWindow* window, WindowEventListener* listener );

		/**
		@Remarks
			Remove previously added listener
		@param window
			The RenderWindow you registered with
		@param listner
			The listener registered
		*/
		static void removeWindowEventListener( RenderWindow* window, WindowEventListener* listener );

		/**
		@Remarks
			Called by RenderWindows upon creation for Ogre generated windows. You are free to add your
			external windows here too if needed.
		@param window
			The RenderWindow to monitor
		*/
		static void _addRenderWindow(RenderWindow* window);

		/**
		@Remarks
			Called by RenderWindows upon creation for Ogre generated windows. You are free to add your
			external windows here too if needed.
		@param window
			The RenderWindow to remove from list
		*/
		static void _removeRenderWindow(RenderWindow* window);

//#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		//! Internal winProc (RenderWindow's use this when creating the Win32 Window)
		static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE && !defined __OBJC__
//        //! Internal UPP Window Handler (RenderWindow's use this when creating the OS X Carbon Window
//        static OSStatus _CarbonWindowHandler(EventHandlerCallRef nextHandler, EventRef event, void* wnd);
//#endif

		//These are public only so GLXProc can access them without adding Xlib headers header
		typedef std::multimap<RenderWindow*, WindowEventListener*> WindowEventListeners;
		static WindowEventListeners _msListeners;

		typedef std::vector<RenderWindow*> Windows;
		static Windows _msWindows;
	};