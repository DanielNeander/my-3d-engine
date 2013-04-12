// Copyright ?2008-2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

// NOTE: The Input system depends on DirectInput in order to function properly.

//
// This is the core file for the input system.  It uses DirectInput ActionMapping to manage input.  In DirectInput any controller
// attached to the PC is a "device" and any item on a device is an "object."  We use buffered input which means that when we retrieve
// data from a device it returns all of the changes made since the last time data was fetched up to the size of the receiving buffer.
//

//
// core includes
//
#include "stdafx.h"
#include "..\BaseTypes\BaseTypes.h"
#include "..\Interfaces\Interface.h"

//
// Input system includes
//
#include "InputSystem.h"
#include "InputScene.h"
#include "InputObject.h"
#include "InputTask.h"

//#include <tchar.h>

//#include "D3DX9Math.h"

//#define SAFE_RELEASE(x)		if (x) { (x)->Release();		(x) = NULL; }	//!< Safe D3D-style release

//
// How many input commands do we want to buffer between updates
//
#define INPUT_BUFFER_SIZE 25

#define MAX_KEY_NAME_LENGTH 50

extern ManagerInterfaces   g_InputManagers;


//
// These are the generic names for all of the possible actions in the demo.
//
enum DEMO_ACTIONS 
{
    //
    // Discrete input. This is for state events such as button or key presses.
    // Used by the keyboard, mouse buttons, and digital buttons on a gamepad.
    //
    INPUT_ACTION_1,
    INPUT_ACTION_2,
    //
    // Absolute axis input. This is for the joystick-like devices which report absolute measurments.
    //
    INPUT_ABS_MOVE_FORWARDBACK_AXIS,    // Move along Z axis
    INPUT_ABS_MOVE_LEFTRIGHT_AXIS,      // Move along X axis
    INPUT_ABS_ROTATE_UPDOWN_AXIS,       // Rotate around X axis
    INPUT_ABS_ROTATE_LEFTRIGHT_AXIS,    // Rotate around Y axis
    INPUT_ABS_MOVE_UPDOWN,              // Move along Y axis
    
    //
    // Turn on/off the water hose.
    //
    INPUT_USE_WATERHOSE,

    INPUT_LEFT_CTRL,                    // Left Ctrl
    INPUT_RIGHT_CTRL,                   // Right Ctrl
    INPUT_MOVE_FORWARD,
    INPUT_MOVE_BACK,
    INPUT_MOVE_LEFT,
    INPUT_MOVE_RIGHT,
    INPUT_MOVE_UP,
    INPUT_MOVE_DOWN,
    INPUT_ACTION_TIMESTOP,
    INPUT_ACTION_TOGGLE_OVERLAY_1,
    INPUT_ACTION_TOGGLE_OVERLAY_2,
    INPUT_ACTION_TOGGLE_OVERLAY_3,
    INPUT_ACTION_TOGGLE_OVERLAY_4,
    INPUT_ACTION_TOGGLE_OVERLAY_5,
    INPUT_QUIT,
    INPUT_SHOW_POLYGONS,                // Ctrl + P
    INPUT_TOGGLE_NORMALS,               // Ctrl + N
    INPUT_TOGGLE_TANGENTS,              // Ctrl + T
    INPUT_TOGGLE_BOUNDING_BOX,          // Ctrl + B
    INPUT_TOGGLE_CAPTIONS,              // Ctrl + O
    INPUT_SET_THREAD_COUNT_1,           // Ctrl + 1
    INPUT_SET_THREAD_COUNT_2,           // Ctrl + 2
    INPUT_SET_THREAD_COUNT_4,           // Ctrl + 4
    INPUT_SET_THREAD_COUNT_MAX,         // Ctrl + X

    //
    // Relative axis input. This is for the mouse-like devices which report changes since the last measurement
    //
    INPUT_REL_ROTATE_LEFTRIGHT_AXIS,  // Rotate around Y axis
    INPUT_REL_ROTATE_UPDOWN_AXIS,     // Rotate around X axis

    INPUT_NUM_ACTIONS,
    INPUT_INVALID_ACTION
};


//
// This is the action map that directinput will attempt to map to each device that is connected.  We define our generic name
// for the action along with a predefined value that DirectInput will use in an attempt to provide a proper mapping. This doesn't
// work so well so generic values are used for the most part (e.g. DIAXIS_ANY_1)
//
DIACTION g_rgGameAction[] =
{
    // Device input pre-defined by DirectInput, according to genre
    // joystick
    {INPUT_ACTION_1,                  DIBUTTON_MECHA_FIRE,    0,            _T("Reset view"),           },
    {INPUT_ACTION_2,                  DIBUTTON_MECHA_WEAPONS, 0,            _T("Enable movement"),      },
    {INPUT_ABS_MOVE_FORWARDBACK_AXIS, DIAXIS_ANY_1,           0,            _T("Move forward/back"),    },
    {INPUT_ABS_MOVE_LEFTRIGHT_AXIS,   DIAXIS_ANY_2,           0,            _T("Move left/right"),      },
    {INPUT_ABS_ROTATE_UPDOWN_AXIS,    DIAXIS_ANY_3,           0,            _T("Rotate up/down"),       },
    {INPUT_ABS_ROTATE_LEFTRIGHT_AXIS, DIAXIS_ANY_4,           0,            _T("Rotate left/right"),    },
    {INPUT_ABS_MOVE_UPDOWN,           DIAXIS_ANY_Z_1,         0,            _T("Move up/down"),         },

    // Keyboard input mappings
    {INPUT_LEFT_CTRL,                 DIKEYBOARD_LCONTROL,    0,            _T("Left control key"),     },
    {INPUT_RIGHT_CTRL,                DIKEYBOARD_RCONTROL,    0,            _T("Right control key"),    },
    {INPUT_MOVE_FORWARD,              DIKEYBOARD_W,           0,            _T("MoveForward"),          },
    {INPUT_MOVE_BACK,                 DIKEYBOARD_S,           0,            _T("MoveBack"),             },
    {INPUT_MOVE_LEFT,                 DIKEYBOARD_A,           0,            _T("MoveLeft"),             },
    {INPUT_MOVE_RIGHT,                DIKEYBOARD_D,           0,            _T("MoveRight"),            },
    {INPUT_MOVE_UP,                   DIKEYBOARD_R,           0,            _T("MoveUp"),               },
    {INPUT_MOVE_DOWN,                 DIKEYBOARD_F,           0,            _T("MoveDown"),             },
    {INPUT_ACTION_1,                  DIKEYBOARD_C,           0,            _T("ResetView"),            },
    {INPUT_ACTION_TIMESTOP,           DIKEYBOARD_PAUSE,       0,            _T("Pause"),                },
    {INPUT_ACTION_TOGGLE_OVERLAY_1,   DIKEYBOARD_F1,          0,            _T("ToggleOverlay1"),       },
    {INPUT_ACTION_TOGGLE_OVERLAY_2,   DIKEYBOARD_F2,          0,            _T("ToggleOverlay2"),       },
    {INPUT_ACTION_TOGGLE_OVERLAY_3,   DIKEYBOARD_F3,          0,            _T("ToggleOverlay3"),       },
    {INPUT_ACTION_TOGGLE_OVERLAY_4,   DIKEYBOARD_F4,          0,            _T("ToggleOverlay4"),       },
    {INPUT_ACTION_TOGGLE_OVERLAY_5,   DIKEYBOARD_F5,          0,            _T("ToggleOverlay5"),       },
    {INPUT_QUIT,                      DIKEYBOARD_ESCAPE,      DIA_APPFIXED, _T("Quit"),                 },
    {INPUT_SHOW_POLYGONS,             DIKEYBOARD_P,           0,            _T("PolygonMode"),          },

    // Water System input mapping
    {INPUT_USE_WATERHOSE,             DIKEYBOARD_SPACE,       0,            _T("UseFireHose"),        },

    {INPUT_TOGGLE_NORMALS,            DIKEYBOARD_N,           0,            _T("ShowNormals"),          },
    {INPUT_TOGGLE_TANGENTS,           DIKEYBOARD_T,           0,            _T("ShowTangents"),         },
    {INPUT_TOGGLE_BOUNDING_BOX,       DIKEYBOARD_B,           0,            _T("ShowBoundingBox"),      },
    {INPUT_TOGGLE_CAPTIONS,           DIKEYBOARD_O,           0,            _T("ShowCaptions"),         },
    {INPUT_SET_THREAD_COUNT_1,        DIKEYBOARD_1,           0,            _T("SetThreadCountTo1"),    },
    {INPUT_SET_THREAD_COUNT_2,        DIKEYBOARD_2,           0,            _T("SetThreadCountTo2"),    },
    {INPUT_SET_THREAD_COUNT_4,        DIKEYBOARD_4,           0,            _T("SetThreadCountTo4"),    },
    {INPUT_SET_THREAD_COUNT_MAX,      DIKEYBOARD_X,           0,            _T("SetThreadCountToMax"),  },

    // Mouse input mappings
    {INPUT_REL_ROTATE_LEFTRIGHT_AXIS, DIMOUSE_XAXIS,          0,            _T("Look left/right"),      },
    {INPUT_REL_ROTATE_UPDOWN_AXIS,    DIMOUSE_YAXIS,          0,            _T("Look up/down"),         },
    {INPUT_ACTION_2,                  DIMOUSE_BUTTON1,        0,            _T("Enable movement"),      },
};

#define NUM_ACTIONMAPS (sizeof(g_rgGameAction)/sizeof(g_rgGameAction[0]))

InputScene::InputScene(
    ISystem* pSystem
    )
    : ISystemScene( pSystem )
    , m_pInputTask( NULL )
{
    m_pInputSystem = (InputSystem*)pSystem;
    // numbers
    m_aKeyboard["1"] = DIKEYBOARD_1;  m_aKeyboard["2"] = DIKEYBOARD_2;  m_aKeyboard["3"] = DIKEYBOARD_3;  m_aKeyboard["4"] = DIKEYBOARD_4;
    m_aKeyboard["5"] = DIKEYBOARD_5;  m_aKeyboard["6"] = DIKEYBOARD_6;  m_aKeyboard["7"] = DIKEYBOARD_7;  m_aKeyboard["8"] = DIKEYBOARD_8;
    m_aKeyboard["9"] = DIKEYBOARD_9;  m_aKeyboard["0"] = DIKEYBOARD_0;
    // letters
    m_aKeyboard["A"] = DIKEYBOARD_A;  m_aKeyboard["B"] = DIKEYBOARD_B;  m_aKeyboard["C"] = DIKEYBOARD_C;  m_aKeyboard["D"] = DIKEYBOARD_D;
    m_aKeyboard["E"] = DIKEYBOARD_E;  m_aKeyboard["F"] = DIKEYBOARD_F;  m_aKeyboard["G"] = DIKEYBOARD_G;  m_aKeyboard["H"] = DIKEYBOARD_G;
    m_aKeyboard["I"] = DIKEYBOARD_I;  m_aKeyboard["J"] = DIKEYBOARD_J;  m_aKeyboard["K"] = DIKEYBOARD_K;  m_aKeyboard["L"] = DIKEYBOARD_L;
    m_aKeyboard["M"] = DIKEYBOARD_M;  m_aKeyboard["N"] = DIKEYBOARD_N;  m_aKeyboard["O"] = DIKEYBOARD_O;  m_aKeyboard["P"] = DIKEYBOARD_P;
    m_aKeyboard["Q"] = DIKEYBOARD_Q;  m_aKeyboard["R"] = DIKEYBOARD_R;  m_aKeyboard["S"] = DIKEYBOARD_S;  m_aKeyboard["T"] = DIKEYBOARD_T;
    m_aKeyboard["U"] = DIKEYBOARD_U;  m_aKeyboard["V"] = DIKEYBOARD_V;  m_aKeyboard["W"] = DIKEYBOARD_W;  m_aKeyboard["X"] = DIKEYBOARD_X;
    m_aKeyboard["Y"] = DIKEYBOARD_Y;  m_aKeyboard["Z"] = DIKEYBOARD_Z;  
    // function keys
    m_aKeyboard["F1"] = DIKEYBOARD_F1;  m_aKeyboard["F2"] = DIKEYBOARD_F2;  m_aKeyboard["F3"] = DIKEYBOARD_F3;  m_aKeyboard["F4"] = DIKEYBOARD_F4;
    m_aKeyboard["F5"] = DIKEYBOARD_F5;  m_aKeyboard["F6"] = DIKEYBOARD_F6;  m_aKeyboard["F7"] = DIKEYBOARD_F7;  m_aKeyboard["F8"] = DIKEYBOARD_F8;
    m_aKeyboard["F9"] = DIKEYBOARD_F9;  m_aKeyboard["F10"] = DIKEYBOARD_F10;  m_aKeyboard["F11"] = DIKEYBOARD_F11;  m_aKeyboard["F12"] = DIKEYBOARD_F12;
    // other keys
    m_aKeyboard["PAUSE"] = DIKEYBOARD_PAUSE;  m_aKeyboard["ESCAPE"] = DIKEYBOARD_ESCAPE; m_aKeyboard["SPACE"] = DIKEYBOARD_SPACE;

	m_bControlKeyPressed = False;

    // apply hot keys defined in Input.sdf
    char currKeyName[MAX_KEY_NAME_LENGTH];
    std::string currentKey;
    std::string newKey;
    std::map<std::string, unsigned long>::iterator keysIt;
    // loop through the default input actions and update anything according to the contents of Input.sdf
    for( u32 h = 0; h < NUM_ACTIONMAPS; h++ )
    {
        // convert the lptszActionName from Unicode LPWSTR to ASCII pcstr
        //WideCharToMultiByte(CP_ACP, 0, g_rgGameAction[h].lptszActionName, -1, currKeyName, MAX_KEY_NAME_LENGTH , NULL, NULL);
        currentKey = g_rgGameAction[h].lptszActionName; //currKeyName;
        // set new hot key from Input.sdf into g_rgGameAction
        for( m_pHotKeyIter = m_pInputSystem->m_aHotKeys.begin(); m_pHotKeyIter != m_pInputSystem->m_aHotKeys.end(); m_pHotKeyIter++ )
        {
            // find the hotkey that matches the current input action 
            if( _stricmp(m_pHotKeyIter->first, currKeyName) == 0 )
            {
                // get the value of the key from Input.sdf
                newKey = m_pHotKeyIter->second->psKey;
                // get the appropriate DIKEYBOARD corresponding to the key
                keysIt = m_aKeyboard.find( newKey );
                // assign the DIKEYBOARD to the input actions
                g_rgGameAction[h].dwSemantic = keysIt->second;
                break;
            }
        }
    }
}

InputScene::~InputScene(
    void
    )
{
    //
    // Release all of the enumerated input devices and the direct input system itself.
    //
    for( u32 i = 0; i < m_nDevices; i++ )
    {
        SAFE_RELEASE( m_pInputDevices[i] );
    }
    SAFE_RELEASE( m_pDI );
    //
    // Free all the remaining objects.
    //
    for( std::list<InputObject*>::iterator it=m_Objects.begin(); it != m_Objects.end(); it++ )
    {
        delete *it;
    }

    m_Objects.clear();

    //
    // Delete the task object
    //
    SAFE_DELETE( m_pInputTask );
}


System::Type
InputScene::GetSystemType(
    void
    )
{
    return System::Types::Input;
}


Error
InputScene::Initialize(
    std::vector<Properties::Property> Properties
    )
{
    ASSERT( !m_bInitialized );

    m_pInputTask = new InputTask( this );
    ASSERT( m_pInputTask != NULL );
    if( m_pInputTask == NULL )
    {
        return Errors::Failure;
    }

    m_nDevices         = 0;
    m_nRotateLeftRight = 0;
    m_nRotateUpDown    = 0;
    m_nMoveLeftRight   = 0;
    m_nMoveForwardBack = 0;
    m_nMoveUpDown      = 0;
    m_nOldForward      = 0;
    m_nOldBack         = 0;
    m_nOldLeft         = 0;
    m_nOldRight        = 0;
    m_nOldUp           = 0;
    m_nOldDown         = 0;
    m_pDI              = NULL;
    m_MouseMoveEnabled = false;

    //
    // Here, we obtain the interface to the directinput system.
    //
    HRESULT hr = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pDI, NULL );
    if( FAILED(hr) )
    {
        ASSERT( false );
        return Errors::Failure;
    }

    // Setup action map
    // {31ACCC99-5AA8-4946-9F54-0E698F914FD4}
    static const GUID guidActionMap = { 0x31accc99, 0x5aa8, 0x4946, { 0x9f, 0x54, 0xe, 0x69, 0x8f, 0x91, 0x4f, 0xd4 } };


    //
    // Here is where we setup the action map structure. We then enumerate the devices using this structure. DirectInput will
    // attempt to map the controls to the attached device. The callback is a static function so we need to pass the "this"
    // pointer to it so it can access member variables of the class.
    //
    ZeroMemory(&m_diaf, sizeof(m_diaf));
    m_diaf.dwSize        = sizeof(DIACTIONFORMAT);
    m_diaf.dwActionSize  = sizeof(DIACTION);
    m_diaf.dwDataSize    = NUM_ACTIONMAPS * sizeof(DWORD);
    m_diaf.dwNumActions  = NUM_ACTIONMAPS;
    m_diaf.rgoAction     = g_rgGameAction;
    m_diaf.guidActionMap = guidActionMap;
    m_diaf.dwGenre       = DIVIRTUAL_DRIVING_MECHA;
    m_diaf.dwBufferSize  = INPUT_BUFFER_SIZE;
    m_diaf.lAxisMin      = -10;
    m_diaf.lAxisMax      = 10;
    //wcscpy_s(m_diaf.tszActionMap, _T("Smoke Demo Controls"));
	strcpy(m_diaf.tszActionMap, _T("Smoke Demo Controls"));

    hr = m_pDI->EnumDevicesBySemantics( NULL, &m_diaf, (LPDIENUMDEVICESBYSEMANTICSCB) &InputScene::EnumSuitableDevicesCB, this, 0L );
    if( FAILED(hr) )
    {
        ASSERT( false );
        return Errors::Failure;
    }

    m_bInitialized = True;

    return Errors::Success;
}


void
InputScene::GetProperties(
    Properties::Array& Properties
    )
{
    UNREFERENCED_PARAM( Properties );
}


void
InputScene::SetProperties(
    Properties::Array Properties
    )
{
    ASSERT( m_bInitialized );
}

    
pcstr*
InputScene::GetObjectTypes(
    void
    )
{
    return NULL;
}


ISystemObject*
InputScene::CreateObject(
    pcstr pszName,
    pcstr pszType
    )
{
    ASSERT( m_bInitialized );

    //
    // Create the object and add it to the object list.
    //
    InputObject* pObject = NULL;
    if( pszType && pszName ) 
    {
        pObject = new InputObject( this, pszType, pszName);
    } 
    else 
    {
        pObject = new InputObject ( this );
    }

    ASSERT( pObject != NULL );
    if( pObject != NULL )
    {
        m_Objects.push_back( pObject );
    }

    return pObject;
}


Error
InputScene::DestroyObject(
    ISystemObject* pSystemObject
    )
{
    ASSERT( m_bInitialized );
    ASSERT( pSystemObject != NULL );

    //
    // Cast to a InputObject so that the correct destructor will be called.
    //
    InputObject* pObject = reinterpret_cast<InputObject*>(pSystemObject);

    //
    // Remove the object from the list and delete it.
    //
    m_Objects.remove( pObject );

    SAFE_DELETE( pSystemObject );

    return Errors::Success;
}


ISystemTask*
InputScene::GetSystemTask(
    void
    )
{
    return m_pInputTask;
}


System::Changes::BitMask
InputScene::GetPotentialSystemChanges(
    void
    )
{
    return System::Changes::Graphics::GUI;
}


const void*
InputScene::GetSystemChangeData(
    System::Change SystemChange
    )
{
    UNREFERENCED_PARAM( SystemChange );

    return NULL;
}

void InputScene::InputUpdate(f32 DeltaTime)
{
    DIDEVICEOBJECTDATA didod[INPUT_BUFFER_SIZE];
    bool bPositionModified = false;
    bool bOrientationModified = false;
    bool bOrientationReset = false;
    bool bPositionReset = false;
    bool bUseFireHose = false;
    Math::Vector3 Direction( 0.0f, 0.0f, 0.0f );
    const float        fMovementSpeed = 800.0f;
    static float       xSensitivity = 1.25f;
    static float       ySensitivity = 2.5f;
    u32                mModified = 0;
    f32                total_yaw = 0.0f;
    f32                total_pitch = 0.0f;
    int                nToggleOverlay = 0;

    static bool bScenePaused = false;

    //
    // Cap DeltaTime
    //
    DeltaTime = Math::Min( 1.0, DeltaTime );
    //
    // Cycle through each device and process any updates from them.  We are using the buffered method
    // of input so the data goes into an array which we need to cycle through.
    //
    for( u32 uDevice = 0; uDevice < m_nDevices; uDevice++ )
    {

        DWORD dwElements = INPUT_BUFFER_SIZE;
        HRESULT hr = m_pInputDevices[uDevice]->Poll();
        if( FAILED(hr) )
        {
            ASSERT(false);
            return;
        }
        hr = m_pInputDevices[uDevice]->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
        if( FAILED(hr) )
        {
            ASSERT(false);
            return;
        }

        for( u32 i = 0; i < dwElements; i++)
        {
            char currKeyName[MAX_KEY_NAME_LENGTH];
            for( u32 d = 0; d < NUM_ACTIONMAPS; d++ )
            {
                if( g_rgGameAction[d].uAppData == didod[i].uAppData )
                {
                    //WideCharToMultiByte(CP_ACP, 0, g_rgGameAction[d].lptszActionName, -1, currKeyName, MAX_KEY_NAME_LENGTH, NULL, NULL);					
					strcpy(currKeyName, g_rgGameAction[d].lptszActionName);
                    break;
                }
            }
            for( m_pHotKeyIter = m_pInputSystem->m_aHotKeys.begin(); m_pHotKeyIter != m_pInputSystem->m_aHotKeys.end(); m_pHotKeyIter++ )
            {
                if( strcmp(m_pHotKeyIter->first, currKeyName) == 0 )
                {
                    break;
                }
            }

			// we'll use this later to determine if the control key is in the proper state
			// no key pressed OR (ctrl key required AND pressed) OR ctrl key not required
            BOOL bControlKeyValid = (
				( m_pHotKeyIter == m_pInputSystem->m_aHotKeys.end() ) ||
				( m_pHotKeyIter->second->bCtrl && m_bControlKeyPressed ) ||
				!m_pHotKeyIter->second->bCtrl
			);

            switch( didod[i].uAppData )
            {
            //
            // In the case of button (keyboard) input we apply the change when the key is pressed (0x80) and
            //  reverse it when the key is released.
            //
            case INPUT_LEFT_CTRL:
            case INPUT_RIGHT_CTRL:
                if( didod[i].dwData & 0x80 )
                {
                    m_bControlKeyPressed = True;
                } 
                else 
                {
                    m_bControlKeyPressed = False;
                }
                break;
            case INPUT_SHOW_POLYGONS:
                if( didod[i].dwData & 0x80 )
                {
                    if( bControlKeyValid )
                    {
                        i32 polygonMode = g_InputManagers.pEnvironment->Variables().GetAsInt( "Graphics::PolygonMode" ) + 1;
                        char t[5];
                        sprintf_s( t, 5, "%d", polygonMode );
                        g_InputManagers.pEnvironment->Variables().SetValue( "Graphics::PolygonMode", t );
                    }
                }
                break;
            case INPUT_USE_WATERHOSE:
                if (didod[i].dwData & 0x80)
                {
                    bUseFireHose = true;
                }
                else
                {
                    bUseFireHose = false;
                }
                break;
            case INPUT_TOGGLE_NORMALS:  // HOT KEY: Ctrl + N
                if( didod[i].dwData & 0x80 )
                {
                    if( bControlKeyValid )
                    {
                        pcstr pszValue = "False";
                        Bool bShowNormals = g_InputManagers.pEnvironment->Variables().GetAsBool( "Graphics::ShowNormals" );
                        bShowNormals = !bShowNormals;
                        if( bShowNormals ) 
                        {
                            pszValue = "True";
                        }
                        g_InputManagers.pEnvironment->Variables().SetValue( "Graphics::ShowNormals", pszValue );
                    }
                }
                break;
            case INPUT_TOGGLE_TANGENTS: // HOT KEY: Ctrl + T
                if( didod[i].dwData & 0x80 )
                {
                    if( bControlKeyValid )
                    {
                        pcstr pszValue = "False";
                        Bool bShowTangents = g_InputManagers.pEnvironment->Variables().GetAsBool( "Graphics::ShowTangents" );
                        bShowTangents = !bShowTangents;
                        if( bShowTangents ) 
                        {
                            pszValue = "True";
                        }
                        g_InputManagers.pEnvironment->Variables().SetValue( "Graphics::ShowTangents", pszValue );
                    }
                }
                break;
            case INPUT_TOGGLE_BOUNDING_BOX: // HOT KEY: Ctrl + B
                if( didod[i].dwData & 0x80 )
                {
                    if( bControlKeyValid )
                    {
                        pcstr pszValue = "False";
                        Bool bShowBoundingBox = g_InputManagers.pEnvironment->Variables().GetAsBool( "Graphics::ShowBoundingBox" );
                        bShowBoundingBox = !bShowBoundingBox;
                        if( bShowBoundingBox ) 
                        {
                            pszValue = "True";
                        }
                        g_InputManagers.pEnvironment->Variables().SetValue( "Graphics::ShowBoundingBox", pszValue );
                    }
                }
                break;
            case INPUT_TOGGLE_CAPTIONS:     // HOT KEY: Ctrl + O
                if( didod[i].dwData & 0x80 )
                {
                    if( bControlKeyValid )
                    {
                        pcstr pszValue = "False";
                        Bool bShowCaptions = g_InputManagers.pEnvironment->Variables().GetAsBool( "Graphics::ShowCaptions" );
                        bShowCaptions = !bShowCaptions;
                        if( bShowCaptions ) 
                        {
                            pszValue = "True";
                        }
                        g_InputManagers.pEnvironment->Variables().SetValue( "Graphics::ShowCaptions", pszValue );
                    }
                }
                break;
            case INPUT_SET_THREAD_COUNT_4:  // HOT KEY: Ctrl + 4
                if( didod[i].dwData & 0x80 )
                {
                    if(
						( bControlKeyValid ) &&
						( g_InputManagers.pTask != NULL )
					)
					{
                        g_InputManagers.pTask->SetNumberOfThreads( 4 );
                    }
                }
				break;
            case INPUT_SET_THREAD_COUNT_2:  // HOT KEY: Ctrl + 2
                if( didod[i].dwData & 0x80 )
                {
                    if(
						( bControlKeyValid ) &&
						( g_InputManagers.pTask != NULL )
					)
					{
                        g_InputManagers.pTask->SetNumberOfThreads( 2 );
                    }
                }
				break;
            case INPUT_SET_THREAD_COUNT_1:  // HOT KEY: Ctrl + 1
                if( didod[i].dwData & 0x80 )
                {
                    if(
						( bControlKeyValid ) &&
						( g_InputManagers.pTask != NULL )
					)
					{
                        g_InputManagers.pTask->SetNumberOfThreads( 1 );
                    }
                }
                break;
            case INPUT_SET_THREAD_COUNT_MAX: // HOT KEY: Ctrl + X
                if( didod[i].dwData & 0x80 )
                {
                    if(
						( bControlKeyValid ) &&
						( g_InputManagers.pTask != NULL )
					)
					{
                        g_InputManagers.pTask->SetNumberOfThreads( 0 );
                    }
                }
                break;
            case INPUT_MOVE_FORWARD:
                if( bControlKeyValid )
                {
                    if( didod[i].dwData & 0x80 )
                    {
                        m_nOldForward       = -fMovementSpeed;
                        m_nMoveForwardBack += m_nOldForward;
                    } 
                    else 
                    {
                        m_nMoveForwardBack -= m_nOldForward;
                    }
                }
                break;
            case INPUT_MOVE_BACK:
                if( bControlKeyValid )
                {
                    if( didod[i].dwData & 0x80 )
                    {
                        m_nOldBack          = fMovementSpeed;
                        m_nMoveForwardBack += m_nOldBack;
                    } 
                    else 
                    {
                        m_nMoveForwardBack -= m_nOldBack;
                    }
                }
                break;
            case INPUT_MOVE_LEFT:
                if( bControlKeyValid )
                {
                    if( didod[i].dwData & 0x80 ) 
                    {
                        m_nOldLeft        = -fMovementSpeed;
                        m_nMoveLeftRight += m_nOldLeft;
                    } 
                    else 
                    {
                        m_nMoveLeftRight -= m_nOldLeft;
                    }
                }
                break;
            case INPUT_MOVE_RIGHT:
                if( bControlKeyValid )
                {
                    if( didod[i].dwData & 0x80 )
                    {
                        m_nOldRight       = fMovementSpeed;
                        m_nMoveLeftRight += m_nOldRight;
                    } 
                    else 
                    {
                        m_nMoveLeftRight -= m_nOldRight;
                    }
                }
                break;
            case INPUT_MOVE_UP:
                if( bControlKeyValid )
                {
                    if( didod[i].dwData & 0x80 ) 
                    {
                        m_nOldUp       = fMovementSpeed;
                        m_nMoveUpDown += m_nOldUp;
                    } 
                    else 
                    {
                        m_nMoveUpDown -= m_nOldUp;
                    }
                }
                break;
            case INPUT_MOVE_DOWN:
                if( bControlKeyValid )
                {
                    if( didod[i].dwData & 0x80 )
                    {
                        m_nOldDown     = -fMovementSpeed;
                        m_nMoveUpDown += m_nOldDown;
                    } 
                    else 
                    {
                        m_nMoveUpDown -= m_nOldDown;
                    }
                }
                break;
            case INPUT_ACTION_TIMESTOP:
                if( bControlKeyValid )
                {
                    if( didod[i].dwData & 0x80 )
                    {
                        // keydown
                    } 
                    else 
                    {
                        switch( g_InputManagers.pEnvironment->Runtime().GetStatus() )
                        {
                            case IEnvironment::IRuntime::Status::Running:
                                g_InputManagers.pEnvironment->Runtime().SetStatus( IEnvironment::IRuntime::Status::Paused );
                                bScenePaused = true;
                                break;
                            case IEnvironment::IRuntime::Status::Paused:
                                g_InputManagers.pEnvironment->Runtime().SetStatus( IEnvironment::IRuntime::Status::Running );
                                bScenePaused = false;
                                break;
                        }

                    }
                }
                break;
            case INPUT_ACTION_TOGGLE_OVERLAY_5:
                nToggleOverlay++;
            case INPUT_ACTION_TOGGLE_OVERLAY_4:
                nToggleOverlay++;
            case INPUT_ACTION_TOGGLE_OVERLAY_3:
                nToggleOverlay++;
            case INPUT_ACTION_TOGGLE_OVERLAY_2:
                nToggleOverlay++;
            case INPUT_ACTION_TOGGLE_OVERLAY_1:
                nToggleOverlay++;
                if( bControlKeyValid )
                {
                    if( didod[i].dwData & 0x80 )
                    {
                        // keydown
                    } 
                    else 
                    {
                        std::list<InputObject*>& Objects = m_Objects;
                        for( std::list<InputObject*>::iterator it=Objects.begin(); it != Objects.end(); it++ )
                        {
                            InputObject* pObject = *it;

                            if( pObject->m_Type == pObject->Type_GUI )
                            {
                                if( pObject->m_nFunctionKey == nToggleOverlay )
                                {
                                    pObject->PostChanges(System::Changes::Graphics::GUI);
                                }
                            }
                        }
                    }
                }
                return;
                break;
            case INPUT_QUIT:
                if( bControlKeyValid )
                {
                    g_InputManagers.pEnvironment->Runtime().SetStatus( IEnvironment::IRuntime::Status::Quit );
                }
                break;
            //
            // In the case of relative input we can get more than one update in the buffer so we need to
            // concatanate them all together.
            //
            case INPUT_REL_ROTATE_LEFTRIGHT_AXIS:
                if( m_MouseMoveEnabled )
                {
                    // NOTE: there's nothing special about the number 0.05f.  It was determined by trial
                    // and error in order to smooth out the movement
                    total_yaw -= Math::Angle::Deg2Rad( (long)didod[i].dwData * ySensitivity * 0.05f );
                    bOrientationModified = true;
                }
                break;
            case INPUT_REL_ROTATE_UPDOWN_AXIS:
                if( m_MouseMoveEnabled )
                {
                    // NOTE: there's nothing special about the number 0.05f.  It was determined by trial
                    // and error in order to smooth out the movement
                    total_pitch -= Math::Angle::Deg2Rad( (long)didod[i].dwData * xSensitivity * 0.05f );
                    bOrientationModified = true;
                }
                break;
            //
            // In the case of absolute input we can get more than one update in the buffer but we only care
            // about the last one i.e. what position the control stick is currently in.
            //
            case INPUT_ABS_ROTATE_LEFTRIGHT_AXIS:
                m_nRotateLeftRight = (f32)(long)didod[i].dwData;
                break;
            case INPUT_ABS_ROTATE_UPDOWN_AXIS:
                m_nRotateUpDown = (f32)(long)didod[i].dwData;
                break;
            case INPUT_ABS_MOVE_LEFTRIGHT_AXIS:
                // NOTE: there's nothing special about the number 10.  It was determined by trial
                // and error in order to smooth out the movement
                m_nMoveLeftRight = (fMovementSpeed / 10) * (long)didod[i].dwData;
                break;
            case INPUT_ABS_MOVE_FORWARDBACK_AXIS:
                // NOTE: there's nothing special about the number 10.  It was determined by trial
                // and error in order to smooth out the movement
                m_nMoveForwardBack = (fMovementSpeed / 10) * (long)didod[i].dwData;
                break;
            case INPUT_ABS_MOVE_UPDOWN:
                // NOTE: there's nothing special about the number 10.  It was determined by trial
                // and error in order to smooth out the movement
                m_nMoveUpDown = (fMovementSpeed / 10) * (long)didod[i].dwData;
                break;
            // reset view
            case INPUT_ACTION_1:
                bOrientationModified = true;
                bPositionModified = true;
                bOrientationReset = true;
                bPositionReset = true;
                break;
            case INPUT_ACTION_2:
                if( didod[i].dwData & 0x80 )
                {
                    m_MouseMoveEnabled = true;
                } 
                else 
                {
                    m_MouseMoveEnabled = false;
                }
                break;
            default:
                bPositionModified = false;
                bOrientationModified = false;
                break;
            }
        }

        if( m_nRotateLeftRight || m_nRotateUpDown )
        {
            total_yaw   -= Math::Angle::Deg2Rad( m_nRotateLeftRight * DeltaTime * ySensitivity );
            total_pitch -= Math::Angle::Deg2Rad( m_nRotateUpDown * DeltaTime * xSensitivity );
            bOrientationModified = true;
        }

        if (m_nMoveLeftRight || m_nMoveForwardBack || m_nMoveUpDown) {
            // When paused DeltaTime is 0 which prevents camera movement on paused scene.  
            // If paused, set DeltaTime to some constant to allow camera movement around the scene.
            if( bScenePaused )
            {
                DeltaTime = 0.02f;
            }
            Direction.x += m_nMoveLeftRight * DeltaTime;
            Direction.z += m_nMoveForwardBack * DeltaTime;
            Direction.y += m_nMoveUpDown * DeltaTime;
            bPositionModified = true;
        }
    }

    std::list<InputObject*>& Objects = m_Objects;

    //
    // Cycle through all of our objects and apply the changes. Also post our change notifications to the CCM.
    //
    if( bPositionModified || bOrientationModified || bUseFireHose )
    {
        for( std::list<InputObject*>::iterator it=Objects.begin(); it != Objects.end(); it++ )
        {
            InputObject* pObject = *it;

            if( bUseFireHose )
            {
                mModified |= System::Changes::Input::Firehose;
            }

            //--------------------------------------------------------------------------
            // GUI Objects do not need to be modified.
            //
            if( pObject->m_Type == pObject->Type_GUI ) 
            {
                continue;
            }
            if( bPositionModified ) 
            {
                pObject->m_Position = Direction;

                mModified |= System::Changes::Geometry::Position;
            }

            if( bOrientationModified ) 
            {            
                // NOTE: there's nothing special about the number 0.2f.  It was determined by trial
                // and error in order to smooth out the movement
                total_pitch = Math::Min( total_pitch, 0.2f );
                total_pitch = Math::Max( total_pitch, -0.2f );
                total_yaw = Math::Min( total_yaw, 0.2f );
                total_yaw = Math::Max( total_yaw, -0.2f );

                if( bOrientationReset ) 
                {
                    pObject->m_Pitch = pObject->m_Yaw = 0.0f;
                }

                pObject->m_Orientation.x = total_yaw;
                pObject->m_Orientation.y = total_pitch;
                pObject->m_Orientation.z = 0.0f;
                pObject->m_Orientation.w = 0.0f;

                mModified |= System::Changes::Geometry::Orientation;
            }
            pObject->PostChanges( mModified );
        }
    }
    return;
}

//
// This callback is called once for each device attached to the system. Some devices will trigger two calls such as a
// combination keyboard/mouse.  After directinput applies the actionmap we can make changes to it if we wish.
//
BOOL InputScene::EnumSuitableDevicesCB(
  LPCDIDEVICEINSTANCE lpddi,
  LPDIRECTINPUTDEVICE8 lpdid,
  DWORD dwFlags,
  DWORD dwRemaining,
  LPVOID pvRef
)
{
    UNREFERENCED_PARAM( dwFlags );
    UNREFERENCED_PARAM( dwRemaining );

    DIPROPDWORD dipdw;
    HRESULT hr;

    //
    // Directinput will attempt to apply the action map to the currently enumerated device. We can examine
    // and make changes to the DIACTIONFORMAT structure if we wish to change anything.
    //
    hr = lpdid->BuildActionMap( &(((InputScene*)pvRef)->m_diaf), NULL, 0L );
    if( FAILED(hr) )
    {
        ASSERT( false );
        return false;
    }

    //
    // If the device is not a keyboard or mouse then it is most likely a joystick and most likely has some analog
    // controls.  We set a dead zone so that no movement is registered while the sticks are at rest. This sets it for
    // the entire device.
    //
    if (!((lpddi->dwDevType & 0xFF) == DI8DEVTYPE_KEYBOARD || (lpddi->dwDevType & 0xFF) == DI8DEVTYPE_MOUSE)) 
    {

        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = 2500;  // 0 - 10000 range
        hr = lpdid->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
        if( FAILED(hr) )
        {
            ASSERT( false );
            return Errors::Failure;
        }
    }

    //
    // Finalizes the action map and any other settings to the device using the data in the
    // DIACTIONFORMAT structure.
    //
    hr = lpdid->SetActionMap( &(((InputScene*)pvRef)->m_diaf), NULL, 0L );
    if( FAILED(hr) )
    {
        ASSERT( false );
        return false;
    }

    //
    // This simply allows us to receive data from the device.
    //
    hr = lpdid->Acquire();
    if( FAILED(hr) )
    {
        ASSERT( false );
        return false;
    }

    //
    // Since we copy the device to local storage we need to increase its reference count so it is not destroyed
    // by the garbage collector.
    //
    ((InputScene*)pvRef)->m_pInputDevices[((InputScene*)pvRef)->m_nDevices] = lpdid;
    ((InputScene*)pvRef)->m_pInputDevices[((InputScene*)pvRef)->m_nDevices]->AddRef();
    ((InputScene*)pvRef)->m_nDevices++;

    return true;
}
