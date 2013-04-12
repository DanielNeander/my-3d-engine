#ifndef __DRAWUTIL_H__
#define __DRAWUTIL_H__

#define MAX_DEBUG_LINES			16384

typedef struct debugLine_s {
	noVec4		rgb;
	noVec3		start;
	noVec3		end;
	bool		depthTest;
	int			lifeTime;
} debugLine_t;


#define MAX_DEBUG_POLYGONS		8192

typedef struct debugPolygon_s {
	noVec4		rgb;
	idWinding	winding;
	bool		depthTest;
	int			lifeTime;
} debugPolygon_t;


// basic colors
extern	noVec4 colorBlack;
extern	noVec4 colorWhite;
extern	noVec4 colorRed;
extern	noVec4 colorGreen;
extern	noVec4 colorBlue;
extern	noVec4 colorYellow;
extern	noVec4 colorMagenta;
extern	noVec4 colorCyan;
extern	noVec4 colorOrange;
extern	noVec4 colorPurple;
extern	noVec4 colorPink;
extern	noVec4 colorBrown;
extern	noVec4 colorLtGrey;
extern	noVec4 colorMdGrey;
extern	noVec4 colorDkGrey;

class DrawUtil {
public:
	static void RB_ClearDebugLines( int time );
	static void RB_AddDebugLine( const noVec4 &color, const noVec3 &start, const noVec3 &end, const int lifeTime, const bool depthTest );
	static void			DebugLine( const noVec4 &color, const noVec3 &start, const noVec3 &end, const int lifetime = 0, const bool depthTest = false );
	static void			DebugArrow( const noVec4 &color, const noVec3 &start, const noVec3 &end, int size, const int lifetime = 0 );
	static void			DebugCone( const noVec4 &color, const noVec3 &apex, const noVec3 &dir, float radius1, float radius2, const int lifetime = 0 );
	static void			DebugAxis( const noVec3 &origin, const noMat3 &axis );

	static debugLine_t		rb_debugLines[ MAX_DEBUG_LINES ];
	static int				rb_numDebugLines;
	static int				rb_debugLineTime;
	static debugPolygon_t	rb_debugPolygons[ MAX_DEBUG_POLYGONS ];
};



#endif