
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "sciter-x-graphics.hpp"
#include <time.h>   


//#include <gl\gl.h>			// Header File For The OpenGL32 Library
//#include <gl\glu.h>			// Header File For The GLu32 Library

#include <stdio.h>

#include <windows.h>													// Header File For The Windows Library

#include "NeHeGL.h"														// Header File For The NeHeGL Basecode

//ROACH
#include "ARB_MULTISAMPLE.h"



HWND hwndMain=NULL;



BOOL DestroyWindowGL (GL_Window* window);
BOOL CreateWindowGL (GL_Window* window);
//ENDROACH

#define WM_TOGGLEFULLSCREEN (WM_USER+1)									// Application Define Message For Toggling

static BOOL g_isProgramLooping;											// Window Creation Loop, For FullScreen/Windowed Toggle																		// Between Fullscreen / Windowed Mode
static BOOL g_createFullScreen;											// If TRUE, Then Create Fullscreen



void printGLInfo()
{
	const GLubyte* name = glGetString(GL_VENDOR); //返回负责当前OpenGL实现厂商的名字  
    const GLubyte* biaoshifu = glGetString(GL_RENDERER); //返回一个渲染器标识符，通常是个硬件平台  
    const GLubyte* OpenGLVersion =glGetString(GL_VERSION); //返回当前OpenGL实现的版本号  
    const GLubyte* Extensions  =glGetString(GL_EXTENSIONS);  
    const GLubyte* gluVersion= gluGetString(GLU_VERSION); //返回当前GLU工具库版本  
    printf("OpenGL实现厂商的名字：%s\n", name);  
    printf("渲染器标识符：%s\n", biaoshifu);  
    printf("OpenGL实现的版本号：%s\n",OpenGLVersion );  
    printf("OpenGL支持的扩展：%s\n",Extensions );  
    printf("OGLU工具库版本：%s\n", gluVersion);  
}


void TerminateApplication (GL_Window* window)							// Terminate The Application
{
	PostMessage (window->hWnd, WM_QUIT, 0, 0);							// Send A WM_QUIT Message
	g_isProgramLooping = FALSE;											// Stop Looping Of The Program
}

void ToggleFullscreen (GL_Window* window)								// Toggle Fullscreen/Windowed
{
	PostMessage (window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}

BOOL ChangeScreenResolution (int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory (&dmScreenSettings, sizeof (DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize				= sizeof (DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth		= width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight		= height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel		= bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings (&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;													// Display Change Failed, Return False
	}
	return TRUE;														// Display Change Was Successful, Return True
}


BOOL CreateWindowGL (GL_Window* window)									// This Code Creates Our OpenGL Window
{

		// 窗口样式 ：可见，子窗口，可改变大小，具体窗口标题
		// WS_TILED 
	DWORD windowStyle =  WS_VISIBLE | WS_CHILD;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style

	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		window->init.bitsPerPixel,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		1,																// Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)  
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	RECT windowRect = {0, 0, window->init.width, window->init.height};	// Define Our Window Coordinates

	GLuint PixelFormat;													// Will Hold The Selected Pixel Format

	if (window->init.isFullScreen == TRUE)								// Fullscreen Requested, Try Changing Video Modes
	{
		if (ChangeScreenResolution (window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			// Fullscreen Mode Failed.  Run In Windowed Mode Instead
			//MessageBox (HWND_DESKTOP,  "Mode Switch Failed.\nRunning In Windowed Mode.",  "Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;							// Set isFullscreen To False (Windowed Mode)
		}
		else															// Otherwise (If Fullscreen Mode Was Successful)
		{
			ShowCursor (FALSE);											// Turn Off The Cursor
			windowStyle = WS_POPUP;										// Set The WindowStyle To WS_POPUP (Popup Window)
			windowExtendedStyle |= WS_EX_TOPMOST;						// Set The Extended Window Style To WS_EX_TOPMOST
		}																// (Top Window Covering Everything Else)
	}
	else																// If Fullscreen Was Not Selected
	{
		// Adjust Window, Account For Window Borders
		AdjustWindowRectEx (&windowRect, windowStyle, 0, windowExtendedStyle);
	}

	/*
	window->hWnd = CreateWindow(
		   (LPCWSTR) window->init.application->className,	// Class Name
			(LPCWSTR) window->init.title,					// Window Title
			// 窗口样式 ：可见，子窗口，可改变大小，具体窗口标题
			WS_VISIBLE
			| WS_CHILD 
			// | WS_SIZEBOX 
			// | WS_TILED 
			,
			// 默认大小和位置，后面使用SetWindows函数设置
			CW_USEDEFAULT,	
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hwndMain,		// 父窗口句柄
			(HMENU)NULL,	// 没有菜单
			window->init.application->hInstance, // Pass The Window Instance
			window);		// 没有图标
 */

	// Create The OpenGL Window
	window->hWnd = CreateWindowEx (0,//windowExtendedStyle,					// Extended Style
								   (LPCWSTR) window->init.application->className,	// Class Name
								    (LPCWSTR) window->init.title,					// Window Title
								   windowStyle,							// Window Style
								   0, 0,								// Window X,Y Position
								   windowRect.right - windowRect.left,	// Window Width
								   windowRect.bottom - windowRect.top,	// Window Height
								   hwndMain,						// Desktop Is Window's Parent
								   0,									// No Menu
								   window->init.application->hInstance, // Pass The Window Instance
								   window);
								  
		
	/*RECT rect;
		// 获取本窗口客户区的RECT（矩形方框的四个边界点）
		GetClientRect(hwndMain,&rect);
		// 设置子窗口的大小的位置
		SetWindowPos(window->hWnd, HWND_TOP,
			rect.left, rect.top,
			rect.right ,rect.bottom,
			SWP_SHOWWINDOW);
			*/

	if (window->hWnd == 0)												// Was Window Creation A Success?
	{
		return FALSE;													// If Not Return False
	}

	window->hDC = GetDC (window->hWnd);									// Grab A Device Context For This Window
	if (window->hDC == 0)												// Did We Get A Device Context?
	{
			// Failed
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

//ROACH
	/*
	Our first pass, Multisampling hasn't been created yet, so we create a window normally
	If it is supported, then we're on our second pass
	that means we want to use our pixel format for sampling
	so set PixelFormat to arbMultiSampleformat instead
  */
	if(!arbMultisampleSupported)
	{
		PixelFormat = ChoosePixelFormat (window->hDC, &pfd);				// Find A Compatible Pixel Format
		if (PixelFormat == 0)												// Did We Find A Compatible Format?
		{
			// Failed
			ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
			window->hDC = 0;												// Zero The Device Context
			DestroyWindow (window->hWnd);									// Destroy The Window
			window->hWnd = 0;												// Zero The Window Handle
			return FALSE;													// Return False
		}

	}
	else
	{
		PixelFormat = arbMultisampleFormat;
	}
//ENDROACH

	if (SetPixelFormat (window->hDC, PixelFormat, &pfd) == FALSE)		// Try To Set The Pixel Format
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}


	window->hRC = wglCreateContext (window->hDC);						// Try To Get A Rendering Context
	if (window->hRC == 0)												// Did We Get A Rendering Context?
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	// Make The Rendering Context Our Current Rendering Context
	if (wglMakeCurrent (window->hDC, window->hRC) == FALSE)
	{
		// Failed
		wglDeleteContext (window->hRC);									// Delete The Rendering Context
		window->hRC = 0;												// Zero The Rendering Context
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	
//ROACH
	/*
	Now that our window is created, we want to queary what samples are available
	we call our InitMultiSample window
	if we return a valid context, we want to destroy our current window
	and create a new one using the multisample interface.
	*/
	if(!arbMultisampleSupported && CHECK_FOR_MULTISAMPLE)
	{
	
		if(InitMultisample(window->init.application->hInstance,window->hWnd,pfd))
		{
			DestroyWindowGL (window);
			return CreateWindowGL(window);
		}
	}

//ENDROACH

	ShowWindow (window->hWnd, SW_NORMAL);								// Make The Window Visible
	window->isVisible = TRUE;											// Set isVisible To True

	ReshapeGL (window->init.width, window->init.height);				// Reshape Our GL Window

	ZeroMemory (window->keys, sizeof (Keys));							// Clear All Keys

	window->lastTickCount = GetTickCount ();							// Get Tick Count

	printGLInfo();

	return TRUE;														// Window Creating Was A Success
																		// Initialization Will Be Done In WM_CREATE
}

BOOL DestroyWindowGL (GL_Window* window)								// Destroy The OpenGL Window & Release Resources
{
	if (window->hWnd != 0)												// Does The Window Have A Handle?
	{	
		if (window->hDC != 0)											// Does The Window Have A Device Context?
		{
			wglMakeCurrent (window->hDC, 0);							// Set The Current Active Rendering Context To Zero
			if (window->hRC != 0)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext (window->hRC);							// Release The Rendering Context
				window->hRC = 0;										// Zero The Rendering Context

			}
			ReleaseDC (window->hWnd, window->hDC);						// Release The Device Context
			window->hDC = 0;											// Zero The Device Context
		}
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
	}

	if (window->init.isFullScreen)										// Is Window In Fullscreen Mode
	{
		ChangeDisplaySettings (NULL,0);									// Switch Back To Desktop Resolution
		ShowCursor (TRUE);												// Show The Cursor
	}	
	return TRUE;														// Return True
}

// Process Window Message Callbacks
LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Get The Window Context
	GL_Window* window = (GL_Window*)(GetWindowLong (hWnd, GWL_USERDATA));

	switch (uMsg)														// Evaluate Window Message
	{
		case WM_SYSCOMMAND:												// Intercept System Commands
		{
			switch (wParam)												// Check System Calls
			{
				case SC_SCREENSAVE:										// Screensaver Trying To Start?
				case SC_MONITORPOWER:									// Monitor Trying To Enter Powersave?
				return 0;												// Prevent From Happening
			}
			break;														// Exit
		}
		return 0;														// Return

		case WM_CREATE:													// Window Creation
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			// Store Window Structure Pointer
			window = (GL_Window*)(creation->lpCreateParams);
			SetWindowLong (hWnd, GWL_USERDATA, (LONG)(window));
		}
		return 0;														// Return

		case WM_CLOSE:													// Closing The Window
			//TerminateApplication(window);								// Terminate The Application
		return 0;														// Return

		case WM_SIZE:													// Size Action Has Taken Place
			switch (wParam)												// Evaluate Size Action
			{
				case SIZE_MINIMIZED:									// Was Window Minimized?
					window->isVisible = FALSE;							// Set isVisible To False
				return 0;												// Return

				case SIZE_MAXIMIZED:									// Was Window Maximized?
					window->isVisible = TRUE;							// Set isVisible To True
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
				return 0;												// Return

				case SIZE_RESTORED:										// Was Window Restored?
					window->isVisible = TRUE;							// Set isVisible To True
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
				return 0;												// Return
			}
		break;															// Break

		case WM_KEYDOWN:												// Update Keyboard Buffers For Keys Pressed
			if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
			{
				window->keys->keyDown [wParam] = TRUE;					// Set The Selected Key (wParam) To True
				return 0;												// Return
			}
		break;															// Break

		case WM_KEYUP:													// Update Keyboard Buffers For Keys Released
			if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
			{
				window->keys->keyDown [wParam] = FALSE;					// Set The Selected Key (wParam) To False
				return 0;												// Return
			}
		break;															// Break

		case WM_TOGGLEFULLSCREEN:										// Toggle FullScreen Mode On/Off
			g_createFullScreen = (g_createFullScreen == TRUE) ? FALSE : TRUE;
			PostMessage (hWnd, WM_QUIT, 0, 0);
		break;															// Break
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);					// Pass Unhandled Messages To DefWindowProc
}

BOOL RegisterWindowClass (Application* application)						// Register A Window Class For This Application.
{																		// TRUE If Successful
	// Register A Window Class
	WNDCLASSEX windowClass;												// Window Class
	ZeroMemory (&windowClass, sizeof (WNDCLASSEX));						// Make Sure Memory Is Cleared
	windowClass.cbSize			= sizeof (WNDCLASSEX);					// Size Of The windowClass Structure
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	windowClass.lpfnWndProc		= (WNDPROC)(WindowProc);				// WindowProc Handles Messages
	windowClass.hInstance		= application->hInstance;				// Set The Instance
	windowClass.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE);			// Class Background Brush Color
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName	= (LPCWSTR) application->className;				// Sets The Applications Classname
	if (RegisterClassEx (&windowClass) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		//MessageBox (HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;														// Return True (Success)
}



	GL_Window			window;											// Window Structure
	Application			application;									// Application Structure
	Keys				keys;											// Key Structure
	BOOL				isMessagePumpActive;							// Message Pump Active?
	MSG					msg;											// Window Message Structure
	DWORD				tickCount;										// Used For The Tick Counter

	HINSTANCE	hInstance	 = GetModuleHandle(NULL);				// Grab An Instance For Our Window

	
namespace sciter
{
	/*
	BEHAVIOR: native-clock
	- draws content layer using sciter-x-graphics.hpp primitives.
	o
	SAMPLE:
	See: samples/behaviors/native-drawing.htm
	*/

	struct native_clock: public event_handler
	{
		// ctor
		native_clock() {}
		virtual ~native_clock() {}

		virtual bool subscription( HELEMENT he, UINT& event_groups )
		{
			event_groups = HANDLE_DRAW   // it does drawing
				| HANDLE_TIMER | HANDLE_KEY | HANDLE_SIZE; // and handles timer
			return true;
		}

		virtual void attached  (HELEMENT he ) 
		{
					hwndMain = dom::element(he).get_element_hwnd(false);



	// Fill Out Application Data
	application.className = "OpenGL";									// Application Class Name
	application.hInstance = hInstance;									// Application Instance

	// Fill Out Window
	ZeroMemory (&window, sizeof (GL_Window));							// Make Sure Memory Is Zeroed
	window.keys					= &keys;								// Window Key Structure
	window.init.application		= &application;							// Window Application
	window.init.title			= "TestWin32APIOpenGL";	// Window Title
	window.init.width			= 640;									// Window Width
	window.init.height			= 480;									// Window Height
	window.init.bitsPerPixel	= 32;									// Bits Per Pixel
	window.init.isFullScreen	= TRUE;									// Fullscreen? (Set To TRUE)

	ZeroMemory (&keys, sizeof (Keys));									// Zero keys Structure

	// Ask The User If They Want To Start In FullScreen Mode?
	//if (MessageBox (HWND_DESKTOP,  "Would You Like To Run In Fullscreen Mode?",  "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		window.init.isFullScreen = FALSE;								// If Not, Run In Windowed Mode
	}

	// Register A Class For Our Window To Use
	if (RegisterWindowClass (&application) == FALSE)					// Did Registering A Class Fail?
	{
		// Failure
		//MessageBox (HWND_DESKTOP,  "Error Registering Window Class!",  "Error", MB_OK | MB_ICONEXCLAMATION);
		return;														// Terminate Application
	}

	g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE
	g_createFullScreen = window.init.isFullScreen;						// g_createFullScreen Is Set To User Default



			//CreateGLWindow(he);
			dom::element(he).start_timer(1);
		}


		virtual void detached  (HELEMENT he ) {

			//KillGLWindow();		
			delete this; 
		}


		virtual bool handle_timer  (HELEMENT he,TIMER_PARAMS& params )
		{	


			while (g_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		// Create A Window
		window.init.isFullScreen = g_createFullScreen;					// Set Init Param Of Window Creation To Fullscreen?
		if (CreateWindowGL (&window) == TRUE)							// Was Window Creation Successful?
		{
			// At This Point We Should Have A Window That Is Setup To Render OpenGL
			if (Initialize (&window, &keys) == FALSE)					// Call User Intialization
			{
				// Failure
				//TerminateApplication (&window);							// Close Window, This Will Handle The Shutdown
			}
			else														// Otherwise (Start The Message Pump)
			{	// Initialize was a success
				isMessagePumpActive = TRUE;								// Set isMessagePumpActive To TRUE
				while (isMessagePumpActive == TRUE)						// While The Message Pump Is Active
				{
					// Success Creating Window.  Check For Window Messages
					if (PeekMessage (&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						// Check For WM_QUIT Message
						if (msg.message != WM_QUIT)						// Is The Message A WM_QUIT Message?
						{
							DispatchMessage (&msg);						// If Not, Dispatch The Message
						}
						else											// Otherwise (If Message Is WM_QUIT)
						{
							isMessagePumpActive = FALSE;				// Terminate The Message Pump
						}
					}
					else												// If There Are No Messages
					{
						if (window.isVisible == FALSE)					// If Window Is Not Visible
						{
							TranslateMessage(&msg);				// Translate The Message
							DispatchMessage(&msg);				// Dispatch The Message
							//WaitMessage ();								// Application Is Minimized Wait For A Message
						}
						else											// If Window Is Visible
						{
							// Process Application Loop
							tickCount = GetTickCount ();				// Get The Tick Count
							Update (tickCount - window.lastTickCount);	// Update The Counter
							window.lastTickCount = tickCount;			// Set Last Count To Current Count
							Draw ();									// Draw Our Scene

							  // Swap the screen buffers
							SwapBuffers (window.hDC);					// Swap Buffers (Double Buffering)
						}
					}
				}														// Loop While isMessagePumpActive == TRUE
			}															// If (Initialize (...

			// Application Is Finished
			Deinitialize ();											// User Defined DeInitialization

			DestroyWindowGL (&window);									// Destroy The Active Window
		}
		else															// If Window Creation Failed
		{
			// Error Creating Window
			//MessageBox (HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									// Terminate The Loop
		}
	}																	// While (isProgramLooping)

			/*
			MSG		msg;	

			// fps count
			fps = 0;
			int nbFrames = 0;
			clock_t lastTime = clock();

			while(!done)									// Loop That Runs While done=FALSE
			{
				if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))	// Is There A Message Waiting?
				{
					if (msg.message==WM_QUIT)				// Have We Received A Quit Message?
					{
						done=TRUE;							// If So done=TRUE
					}
					else									// If Not, Deal With Window Messages
					{
						TranslateMessage(&msg);				// Translate The Message
						DispatchMessage(&msg);				// Dispatch The Message
					}
				}
				else										// If There Are No Messages
				{

					clock_t currentTime  = clock();
					nbFrames++;
					//fps = float(n_frame)/(finish-start)*CLOCKS_PER_SEC;
					long time = (currentTime - lastTime) / CLOCKS_PER_SEC;
					if (  (currentTime - lastTime) / CLOCKS_PER_SEC >= 1.0) 
					{
						fps = nbFrames;
						nbFrames = 0;
						lastTime += CLOCKS_PER_SEC;
					}

					DrawGLScene();					// Draw The Scene
					SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
				}
			}
			*/


			dom::element(he).refresh(); // refresh element's area
			return true; // keep ticking
		}

		virtual bool handle_draw   (HELEMENT he, DRAW_PARAMS& params ) 
		{
			//if( params.cmd != DRAW_CONTENT ) return false; // drawing only content layer
			return true;
		}

		virtual bool handle_key    (HELEMENT he, KEY_PARAMS& params )
		{
			if ( VK_F1 == params.key_code )
			{
				printf("");
			}
			return true;
		}

		virtual void handle_size  (HELEMENT he ) 
		{
			//RECT rect;
			// 获取本窗口客户区的RECT（矩形方框的四个边界点）
			// 1064 585
			//GetClientRect(hwndChild,&rect);
			//ReSizeGLScene(rect.right - rect.left, rect.bottom - rect.top);	
		}

		//
	};

	struct native_clock_factory: public behavior_factory {

		native_clock_factory(): behavior_factory("native-clock") { }

		// the only behavior_factory method:
		virtual event_handler* create(HELEMENT he) {
			return new native_clock();
		}

	};

	// instantiating and attaching it to the global list
	native_clock_factory native_clock_factory_instance;


	/*
	BOOL CreateGLWindow(HELEMENT he) 
	{
		hwndMain = dom::element(he).get_element_hwnd(false);

		hinst	 = GetModuleHandle(NULL);				// Grab An Instance For Our Window

		WNDCLASS   chd;//子窗口类   
		chd.style         = CS_HREDRAW | CS_VREDRAW ; //创建窗体是加子(WS_CHILDWINDOW)窗体风格   
		chd.lpfnWndProc   = ChildWndProc ; //修改项：换成指定的窗口过程   
		chd.cbClsExtra    = 0 ;  
		chd.cbWndExtra    = sizeof(long) ;  
		chd.hInstance     = hinst ;  
		chd.hIcon         = NULL;//修改项：不需要图标，设为NULL   
		chd.hCursor       = LoadCursor (NULL, IDC_ARROW) ;  
		chd.hbrBackground = (HBRUSH) GetStockObject (GRAY_BRUSH) ;//修改项：改为黑色   
		chd.lpszMenuName  = NULL ;  
		chd.lpszClassName = L"ChildClass"; // 窗口类名

		// 注册窗口类
		if(!RegisterClass(&chd))
		{
			printf("");
		}

		hwndChild = CreateWindow(
			L"ChildClass",		//	窗口类
			L"子窗口",	// 窗口的标题
			// 窗口样式 ：可见，子窗口，可改变大小，具体窗口标题
			WS_VISIBLE
			| WS_CHILD 
			// | WS_SIZEBOX 
			// | WS_TILED 
			,
			// 默认大小和位置，后面使用SetWindows函数设置
			CW_USEDEFAULT,	
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			hwndMain,		// 父窗口句柄
			(HMENU)NULL,	// 没有菜单
			hinst,		// 应用程序实例
			NULL);		// 没有图标

		RECT rect;
		// 获取本窗口客户区的RECT（矩形方框的四个边界点）
		GetClientRect(hwndMain,&rect);
		// 设置子窗口的大小的位置
		SetWindows(&rect);


		// opengl 
		GLuint		PixelFormat;			// Holds The Results After Searching For A Match
		static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
		{
			sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
			1,											// Version Number
			PFD_DRAW_TO_WINDOW |						// Format Must Support Window
			PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
			PFD_DOUBLEBUFFER,							// Must Support Double Buffering
			PFD_TYPE_RGBA,								// Request An RGBA Format
			24,										// Select Our Color Depth
			0, 0, 0, 0, 0, 0,							// Color Bits Ignored
			0,											// No Alpha Buffer
			0,											// Shift Bit Ignored
			0,											// No Accumulation Buffer
			0, 0, 0, 0,									// Accumulation Bits Ignored
			16,											// 16Bit Z-Buffer (Depth Buffer)  
			0,											// No Stencil Buffer
			0,											// No Auxiliary Buffer
			PFD_MAIN_PLANE,								// Main Drawing Layer
			0,											// Reserved
			0, 0, 0										// Layer Masks Ignored
		};

		if (!(hDC=GetDC(hwndChild)))							// Did We Get A Device Context?
		{
			KillGLWindow();								// Reset The Display
			MessageBox(NULL, L"Can't Create A GL Device Context.", L"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return false;								// Return FALSE
		}

		if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
		{
			KillGLWindow();								// Reset The Display
			MessageBox(NULL, L"Can't Find A Suitable PixelFormat.", L"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return false;								// Return FALSE
		}

		if(!SetPixelFormat(hDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
		{
			KillGLWindow();								// Reset The Display
			MessageBox(NULL, L"Can't Set The PixelFormat.", L"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return false;								// Return FALSE
		}

		if (!(hRC=wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
		{
			KillGLWindow();								// Reset The Display
			MessageBox(NULL, L"Can't Create A GL Rendering Context.", L"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return false;								// Return FALSE
		}

		if(!wglMakeCurrent(hDC,hRC))					// Try To Activate The Rendering Context
		{
			KillGLWindow();								// Reset The Display
			MessageBox(NULL, L"Can't Activate The GL Rendering Context.", L"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return false;								// Return FALSE
		}


		// show window
		// 显示窗口 
		ShowWindow(hwndMain, SW_SHOWNORMAL); 
		// ShowWindow(hWnd,SW_SHOW);						// Show The Window
		UpdateWindow(hwndMain); 

		SetForegroundWindow(hwndMain);						// Slightly Higher Priority
		SetFocus(hwndMain);									// Sets Keyboard Focus To The Window

		ReSizeGLScene(rect.right - rect.left, rect.bottom - rect.top);					// Set Up Our Perspective GL Screen

		if (!InitGL())									// Initialize Our Newly Created GL Window
		{
			KillGLWindow();								// Reset The Display
			MessageBox(NULL, L"Initialization Failed.", L"ERROR",MB_OK|MB_ICONEXCLAMATION);
			return FALSE;								// Return FALSE
		}

		// attach
		dom::element(he).attach_hwnd(hwndChild);
		printf("");

		return true;
	}

	GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
	{
		done = true;
		if (hRC)											// Do We Have A Rendering Context?
		{
			if (!wglMakeCurrent(NULL,NULL))					// Are We Able To Release The DC And RC Contexts?
			{
				MessageBox(NULL, L"Release Of DC And RC Failed.", L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			}

			if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
			{
				MessageBox(NULL, L"Release Rendering Context Failed.", L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			}
			hRC=NULL;										// Set RC To NULL
		}

		if (hDC && !ReleaseDC(hwndChild,hDC))					// Are We Able To Release The DC
		{
			MessageBox(NULL, L"Release Device Context Failed.", L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			hDC=NULL;										// Set DC To NULL
		}

		if (hwndChild && !DestroyWindow(hwndChild))					// Are We Able To Destroy The Window?
		{
			MessageBox(NULL, L"Could Not Release hWnd.", L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			hwndChild=NULL;										// Set hWnd To NULL
		}

		if (!UnregisterClass( L"ChildClass",hinst))			// Are We Able To Unregister Class
		{
			MessageBox(NULL, L"Could Not Unregister Class.", L"SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
			hinst=NULL;									// Set hInstance To NULL
		}
	}

	LRESULT CALLBACK ChildWndProc(HWND hwnd,
		UINT uMsg,
		WPARAM wParam,
		LPARAM lParam
		)
	{
		switch (uMsg) 
		{ 
		case WM_DESTROY: 
			//KillGLWindow();		
			//PostQuitMessage(0);
			//    ExitThread(0);
			return 0;
		default: 
			return DefWindowProc(hwnd, uMsg, wParam, lParam); 
		} 
	}



	*/



}



