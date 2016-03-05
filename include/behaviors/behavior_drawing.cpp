
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "sciter-x-graphics.hpp"
#include <time.h>   

#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <time.h>

// opengl func
int InitGL(GLvoid);
int DrawGLScene(GLvoid);
GLvoid ReSizeGLScene(GLsizei width, GLsizei height)	;
int fps;

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(1.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	
	return true;										// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
	glLoadIdentity();									// Reset The Current Modelview Matrix

	glTranslatef(0.0f, 0.0f, -6.0f);      // 移入屏幕 6.0
	glBegin(GL_TRIANGLES);       // 绘制三角形
	glVertex3f( 0.0f, 1.0f, 0.0f);     // 上顶点
	glVertex3f(-1.0f,-1.0f, 0.0f);     // 左下
	glVertex3f( 1.0f,-1.0f, 0.0f);     // 右下
	glEnd();        // 三角形绘制结束

	return true;										// Everything Went OK
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

namespace sciter
{
	/*
	BEHAVIOR: native-clock
	- draws content layer using sciter-x-graphics.hpp primitives.
	o
	SAMPLE:
	See: samples/behaviors/native-drawing.htm
	*/

	HDC			hDC=NULL;		// Private GDI Device Context
	HGLRC		hRC=NULL;		// Permanent Rendering Context
	HINSTANCE hinst; 
	HWND hwndMain=NULL;
	HWND hwndChild=NULL;
	BOOL	done=false;	

	BOOL CreateGLWindow(HELEMENT he);
	GLvoid KillGLWindow(GLvoid);
	LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	DWORD SetWindows(LPRECT lpRect);



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

			CreateGLWindow(he);
			dom::element(he).start_timer(1);
		}

		virtual void detached  (HELEMENT he ) {

			KillGLWindow();		
			delete this; 
		}


		virtual bool handle_timer  (HELEMENT he,TIMER_PARAMS& params )
		{			
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
			RECT rect;
			// 获取本窗口客户区的RECT（矩形方框的四个边界点）
			// 1064 585
			GetClientRect(hwndChild,&rect);
			ReSizeGLScene(rect.right - rect.left, rect.bottom - rect.top);	
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
		/*
		if (fullscreen)										// Are We In Fullscreen Mode?
		{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
		}
		*/
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

	DWORD SetWindows(LPRECT lpRect)
	{
		// Child View
		SetWindowPos(hwndChild, HWND_TOP,
			lpRect->left, lpRect->top,
			lpRect->right ,lpRect->bottom,
			SWP_SHOWWINDOW);
		return 0;
	}

}
