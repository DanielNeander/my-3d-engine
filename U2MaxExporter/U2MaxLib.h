#pragma once 


//SIMPLE TYPE

#include <windows.h>
#include <assert.h>
#include <tchar.h>
#include <commctrl.h> // for InitCommonControls() 

#include <U2Lib/Src/U2Lib.h>
#include <U2Misc/Src/U2Misc.h>
#include <U2_3D/src/U2_3DLib.h>

#ifdef DegToRad 
#undef DegToRad
#endif 

#ifdef RadToDeg
#undef RadToDeg
#endif 

#include "max.h"
#include "shaders.h"
#include "macrorec.h"
#include "gport.h"
#include "utilapi.h"
#include "simpobj.h"
#include "modstack.h"
#include "stdmat.h"
#include "spline3d.h"
#include "splshape.h"
#include "decomp.h"
#include "bmmlib.h"
#include "ikctrl.h"
#include "strclass.h"
#include "interpik.h"
#include "notetrck.h"
#include "lslights.h"

#include "iparamb2.h"
#include "iparamm2.h"
#include "simpspl.h"
#include "ISkin.h"

#include "iiksys.h"	// IK sub system

#include "cs/phyexp.h"
#include "cs/bipexp.h"


#define SECONDSPERTICK (1.0f / 4800.0f)






