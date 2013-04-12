#ifndef __MACRO_H__
#define __MACRO_H__

#include <assert.h>

////////////////////////////////////////////////////////////////////////
//Macro Trick #1: Turning Enums into Strings

/*
#define CaseEnum(a)   case(a): LogMsgToFile(#a, id, time)

switch( msg_passed_in )
{
  CaseEnum( MSG_YouWereHit );
    ReactToHit();
    break;

  CaseEnum( MSG_GameReset );
    ResetLogic();
    break;
}
*/

/*
// data.h

DATA(MSG_YouWereHit)
DATA(MSG_GameReset)
DATA(MSG_HealthRestored)

// data.cpp

#define DATA(x) x,

enum GameMessages
{
  #include "data.h"
};

#undef DATA
#define DATA(x) #x, // make enums into strings

static const char* GameMessageNames[] =
{
  #include "data.h"
};

#undef DATA
*/


#endif