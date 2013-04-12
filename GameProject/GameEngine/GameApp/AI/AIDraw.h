#pragma once 


#include "OpenSteer/AbstractVehicle.h"

class NavDebugDraw;

namespace OpenSteer {


	class AIDraw {

	public:
		static bool Init();

		enum DrawType {
			DRAW_AXIS, 
			DRAW_CIRCLE,			
			DRAW_LINE,
			//DRAW_XZARC,
			MAX_DRAW_TYPES
		};

		static void drawAxes  (const AbstractLocalSpace& localSpace,
			const Vec3& size,
			const Vec3& color);

		static void drawCircleOrDisk (const float radius,
			const Vec3& axis,
			const Vec3& center,
			const Vec3& color,
			const int segments,
			const bool filled,
			const bool in3d);

		static void drawXZCircleOrDisk (const float radius,
			const Vec3& center,
			const Vec3& color,
			const int segments,
			const bool filled);

		static void draw3dCircleOrDisk (const float radius,
			const Vec3& center,
			const Vec3& axis,
			const Vec3& color,
			const int segments,
			const bool filled);

		inline void drawXZCircle (const float radius,
			const Vec3& center,
			const Vec3& color,
			const int segments);


		inline void drawXZDisk (const float radius,
			const Vec3& center,
			const Vec3& color,
			const int segments);

		inline void draw3dCircle (const float radius,
			const Vec3& center,
			const Vec3& axis,
			const Vec3& color,
			const int segments);

		void drawXZArc (const Vec3& start,
			const Vec3& center,
			const float arcLength,
			const int segments,
			const Vec3& color);

		void drawLine (const Vec3& startPoint,
			const Vec3& endPoint,
			const Vec3& color);


		// ----------------------------------------------------------------------------
		// draw 2d lines in screen space: x and y are the relevant coordinates


		void draw2dLine (const Vec3& startPoint,
			const Vec3& endPoint,
			const Vec3& color);
		

		static NavDebugDraw* drawPen_;
	};

}