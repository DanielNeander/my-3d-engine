#ifndef DEBUG_CONSOLE_H
#define DEBUG_CONSOLE_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
// Name:   DebugConsole.h
//
// Desc:   Creates a resizable console window for recording and displaying
//         debug info.
//
//         use the debug_con macro to send text and types to the console
//         window via the << operator (just like std::cout). Flush the
//         buffer using "" or the flush macro.  eg. 
//
//        debug_con << "Hello World!" << "";
//
// Author: Mat Buckland 2001 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <vector>
#include <windows.h>
#include <iosfwd>
#include <fstream>

//#include "utils.h"
#include "WindowUtils.h"


//need to define a custom message
const int UM_SETSCROLL = WM_USER + 32;

//maximum number of lines shown in console before the buffer is flushed to 
//a file
const int MaxBufferSize = 500;

//initial dimensions of the console window
const int DEBUG_WINDOW_WIDTH  = 400;
const int DEBUG_WINDOW_HEIGHT = 400;

//undefine DEBUG to send all debug messages to hyperspace (a sink - see below)
//#define DEBUG
#if defined (DEBUG) || defined(_DEBUG) || defined(U2_PROFILE)
#define debug_con *(DebugConsole::Instance())
#else
#define debug_con *(CSink::Instance())
#endif

//use these in your code to toggle output to the console on/off
#define debug_on  DebugConsole::On();
#define debug_off DebugConsole::Off();


//this little class just acts as a sink for any input. Used in place
//of the DebugConsole class when the console is not required
class CSink
{
private:

  CSink(){};

  //copy ctor and assignment should be private
  CSink(const CSink&);
  CSink& operator=(const CSink&);
  
public:

  static CSink* Instance(){static CSink instance; return &instance;}
  
  template<class T>
  CSink& operator<<(const T&)
  {
	  return *this;
  }
};



#ifdef U2MISC_IMPORTS
#define EXPIMP_TEMPLATE_MISC extern
#else 
#define EXPIMP_TEMPLATE_MISC 
#endif

EXPIMP_TEMPLATE_MISC template class U2MISC std::vector<std::string>;
//EXPIMP_TEMPLATE_MISC template class U2MISC std::ofstream;

#define dbgout(val)  	debug_con << (val);	\
								DebugConsole::flush()



class  U2MISC DebugConsole
{
private:

  static HWND	         m_hwnd;
  
  //the string buffer. All input to debug stream is stored here
  static std::vector<std::string> m_Buffer;
  
  //if true the next input will be pushed into the buffer. If false,
  //it will be appended.
  static bool          m_bFlushed;  
  
  //position of debug window
  static int           m_iPosTop;
  static int           m_iPosLeft;

  //set to true if the window is destroyed
  static bool          m_bDestroyed;

  //if false the console will just disregard any input
  static bool          m_bActive;

  //default logging file
  static std::ofstream m_LogOut;



  //the debug window message handler
  static LRESULT CALLBACK DebugWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

  //this registers the window class and creates the window(called by the ctor)
  inline static bool             Create()

  //----------------------------- Create -----------------------------------
  //
  //------------------------------------------------------------------------
  
  {
	  m_hwnd       = NULL;
	  m_iPosLeft   = 0;
	  m_iPosTop    = 0;
	  m_bFlushed   = true;

	  //open log file
	  m_LogOut.open("DebugLog.txt");


	  WNDCLASSEX wDebugConsole = {sizeof(WNDCLASSEX), 
		  CS_HREDRAW | CS_VREDRAW,
		  DebugWindowProc,
		  0,
		  0, 
		  GetModuleHandle(NULL),
		  NULL,
		  NULL,
		  (HBRUSH)(GetStockObject(GRAY_BRUSH)),
		  NULL,
		  "Debug",
		  NULL }; 


	  //register the window class
	  if (!RegisterClassEx(&wDebugConsole))
	  {
		  MessageBox(NULL, "Registration of Debug Console Failed!", "Error", 0);

		  //exit the application
		  return false;
	  }


	  //get the size of the client window
	  // RECT rectActive;
	  // GetClientRect(GetActiveWindow(), &rectActive);

	  // Create the info window
	  m_hwnd = CreateWindow("Debug",
		  "Debug Console", 
		  WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU| WS_VSCROLL | WS_THICKFRAME,
		  0,
		  0,
		  DEBUG_WINDOW_WIDTH,
		  DEBUG_WINDOW_HEIGHT,
		  NULL,
		  NULL,
		  wDebugConsole.hInstance,
		  NULL );

	  //make sure the window creation has gone OK
	  if(!m_hwnd)
	  {
		  MessageBox(m_hwnd, "CreateWindowEx Failed!", "Error!", 0);

		  return false;
	  }

	  // Show the window
	  UpdateWindow(m_hwnd);

	  return true;

  }


  static void             DrawWindow(){InvalidateRect(m_hwnd, NULL, TRUE); UpdateWindow(m_hwnd);}

private:

  DebugConsole(){}
 
  //copy ctor and assignment should be private
  DebugConsole(const DebugConsole&);
  DebugConsole& operator=(const DebugConsole&);

public:

  ~DebugConsole(){WriteAndResetBuffer(); }

  inline static DebugConsole* Instance()
  {
	  static DebugConsole instance;     
	  static bool created = false;
	  if ( !created)
	  {Create();created = true;}

	  return &instance;
  }

             
  void ClearBuffer(){m_Buffer.clear(); flush();}


  static void flush()
  {
    if (!m_bDestroyed)
    {
      m_bFlushed = true; SendMessage(m_hwnd, UM_SETSCROLL, NULL, NULL);
    }
  }

  //writes the contents of the buffer to the file "debug_log.txt", clears
  //the buffer and resets the appropriate scroll info
  inline void WriteAndResetBuffer()
  {

	  m_iPosLeft   = 0;
	  m_iPosTop    = 0;
	  m_bFlushed   = true;

	  //write out the contents of the buffer to a file
	  std::vector<std::string>::iterator it = m_Buffer.begin();

	  for (it; it != m_Buffer.end(); ++it)
	  {
		  m_LogOut << *it << std::endl;
	  }

	  m_Buffer.clear();

	  SendMessage(m_hwnd, UM_SETSCROLL, NULL, NULL);
  }

  //use to activate deactivate
  static void  Off(){m_bActive = false;}
  static void  On()  {m_bActive = true;}

  bool Destroyed()const{return m_bDestroyed;}
 

  //overload the << to accept any type
  template <class T>
  inline DebugConsole& operator<<(const T& t)
  {
    if (!m_bActive || m_bDestroyed) return *this;
   
    //reset buffer and scroll info if it overflows. Write the excess
    //to file
    if (m_Buffer.size() > MaxBufferSize)
    {
       WriteAndResetBuffer();
    }
    
    std::ostringstream ss; ss << t;

    if (ss.str() == ""){flush(); return *this;}
    
    if (!m_bFlushed)
      {m_Buffer.back() += ss.str();}
    else
      {m_Buffer.push_back(ss.str());m_bFlushed = false;}

    return *this;
  }
};

 

#endif