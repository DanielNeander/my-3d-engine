#ifndef RENDERER_VIEWPORT_3D_H
#define RENDERER_VIEWPORT_3D_H

#include <Graphics/Common/input/Mouse/hkgMouseDefines.h>

typedef unsigned int HKG_CAMERA_NAVIGATION_MODE;
/// Ignore all mouse/pad/keyboard events that would normally be used for the navigation
#define	HKG_CAMERA_NAV_DISABLED 0
/// Virtual trackball around the point of interest of the camera
#define	HKG_CAMERA_NAV_TRACKBALL 1

class noViewport3D {

public:

	/// Set the extents in absolute window coordinates. It will assume that you
	/// want to then want it to ignore window resizes. It is from LowerLeft to UpperRight (OpenGL style).
	inline void SetExtentsAbsolute( int ll_x, int ll_y,
		int ur_x, int ur_y );

	/// Set the extents in relative (normalized) coordinates, from 0.0 to 1.0 of 
	/// window width and window height. It will then assume that you want it to 
	/// resize with the window so will set the absoulte coords on resize to match
	/// these fractions. It is from LowerLeft to UpperRight (OpenGL style).
	inline void SetExtentsRelative( float ll_x, float ll_y,
		float ur_x, float ur_y );

	/// Get the width, in pixels, of the viewport.
	inline int GetWidth() const;

	/// Get the height, in pixels, of the viewport.
	inline int GetHeight() const;

	/// Update the extents, and set the viewport up so that the camera is correct 
	/// for the current extents etc. Does not alter the hardware viewport until it
	/// is set as current, and so at the moment no platform specific viewport 
	/// overrides this function yet (this base implementation is enough).
	virtual void UpdateExtents();

	/// Process / handle the new mouse x and y coordinates and corresponding delta
	/// movement since the last frame. The delta difference is given so that the viewport
	/// doesn't have to remember mouse position explicitly.
	void processMouseMove(int x, int y, int dx, int dy, bool isVirtualMouse);

	/// Process / handle the mouse wheel delta movement, where the mouse is located at 'x' and 'y'.
	void processMouseWheel(int delta, int x, int y, bool isVirtualMouse);

	/// Process / handle the given mouse button changing to the given state, at location 'x' and 'y'.
	/// State is true or false representing pressed and released respectfully.
	void processMouseButton(HKG_MOUSE_BUTTON button, bool state, int x, int y, bool isVirtualMouse); 


	//bool pick( int x, int y, const hkgDisplayWorld* world, hkgViewportPickData& data);

	//inline HKG_VIEWPORT_SIDE isWindowCoordOnBorder( int wx, int wy, unsigned int width) const;

	// Internal reallt. Updates the camera if in fly mode based on current input and the delta time (in secs)
	void updateCamera(float dt);

protected:

	int m_winLL[2]; // from, absolute
	int m_winUR[2]; // to, absolute

	float m_winLLRelative[2]; // from, relative to win size
	float m_winURRelative[2]; // to, relative to win size

	bool m_absolute; //do we use the absolute as the reference or the relative

	class CameraBase*  m_camera; // camera to use when rendering

	HKG_CAMERA_NAVIGATION_MODE m_navMode; // trackball or fly

	bool	m_wantMouseEvents;
	bool	m_wantPadEvents;
	bool	m_flyInvertLook; // for the other 50% of the poplulation..
	float   m_flyModeUp[3];

	int m_speedMousePan;
	int m_speedMouseWheelZoom;
	int m_speedMouseButtonZoom;
	int m_speedPadPan;
	int m_speedPadZoom;
	int m_speedFlyStrafe;
	int m_speedFlyMove;
	bool m_mouseAbsoluteSpeed;

	HKG_MOUSE_BUTTON   m_viewMouseButton;
};


#endif