#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-behavior.h"
#include "sciter-x-graphics.hpp"

#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library

namespace sciter
{
	/*
	BEHAVIOR: gdi-drawing
	- draws content layer using GDI primitives.

	SAMPLE:
	See: samples/behaviors/gdi-drawing.htm
	*/

	// dib (bitmap) combined with DC

	class dib32
	{
		const unsigned _width;
		const unsigned _height;
		void*      _bits;
		mutable HBITMAP    _old_bitmap;
		mutable HDC        _dc;
		HBITMAP    _bitmap;

		BITMAPINFO _bitmap_info;

	public:

		dib32(unsigned w, unsigned h) :
			_width(w),
			_height(h),
			_bits(0),
			_old_bitmap(0),
			_dc(0) {

				memset(&_bitmap_info,0,sizeof(_bitmap_info));
				_bitmap_info.bmiHeader.biSize = sizeof(_bitmap_info.bmiHeader);
				_bitmap_info.bmiHeader.biWidth = _width;
				_bitmap_info.bmiHeader.biHeight = 0 - int(_height);
				_bitmap_info.bmiHeader.biPlanes = 1;
				_bitmap_info.bmiHeader.biBitCount = 32;
				_bitmap_info.bmiHeader.biCompression = BI_RGB;

				_bitmap = ::CreateDIBSection(
					NULL, // device context
					&_bitmap_info,
					DIB_RGB_COLORS,
					&_bits,
					0, // file mapping object
					0); // file offset

				if (0 == _bits) {
					return;
					//throw std::bad_alloc();
				}

				memset(_bits,0, _width * _height * 4);

		}

		~dib32() {
			if( _dc )
			{
				::SelectObject(_dc,_old_bitmap);
				::DeleteDC(_dc);
			}
			if( _bitmap )
				::DeleteObject(_bitmap);
		}

		void set_white()
		{
			memset(_bits,0xff, _width * _height * 4);
		}

		unsigned  width() const { return _width; }
		unsigned  height() const { return _height; }
		void* bits() const { return _bits; }
		BYTE* bytes() const { return (BYTE*)_bits; }
		HDC   DC() const 
		{ 
			if(!_dc) {
				_dc = ::CreateCompatibleDC(NULL);
				if (_dc)
					_old_bitmap = (HBITMAP)::SelectObject(_dc,_bitmap);
			}
			return _dc; 
		}

		HBITMAP   detach() { auto r = _bitmap; _bitmap = 0; return r; }

		const BITMAPINFO& get_info() const { return _bitmap_info; }
	};


	struct gdi_drawing: public event_handler
	{
		// ctor
		gdi_drawing() {}
		virtual ~gdi_drawing() {}

		virtual bool subscription( HELEMENT he, UINT& event_groups )
		{
			event_groups = HANDLE_DRAW;  // it does drawing
			return true;
		}

		virtual void attached  (HELEMENT he ) { 
			printf("hello");
		}
		virtual void detached  (HELEMENT he ) { delete this; }

		virtual bool handle_draw   (HELEMENT he, DRAW_PARAMS& params ) 
		{
			if( params.cmd != DRAW_CONTENT ) return false; // drawing only content layer

			unsigned w = unsigned(params.area.right - params.area.left);
			unsigned h = unsigned(params.area.bottom - params.area.top);

			dib32 buffer(w,h);

			do_draw(buffer.DC(), w,h);

			sciter::image img = sciter::image::create(w,h,false,buffer.bytes());

			sciter::graphics gfx(params.gfx);

			gfx.draw_image(&img, POS(params.area.left), POS(params.area.top));

			return true;

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

		int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
		{
			glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
			glClearColor(1.0f, 0.0f, 0.0f, 0.5f);				// Black Background
			glClearDepth(1.0f);									// Depth Buffer Setup
			glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
			glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
			glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
			return TRUE;										// Initialization Went OK
		}

		int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
		{
			glClearColor(0.5,0.7,0.9,1.0); //指定背景颜色（依次为RGBA）
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
			glLoadIdentity();									// Reset The Current Modelview Matrix
			return TRUE;										// Everything Went OK
		}

		void do_draw( HDC hDC, unsigned w, unsigned h ) {

			/*
			RECT rc = { 0,0,w,h };
			HBRUSH hbr1 = ::CreateSolidBrush(0x000000);
			HBRUSH hbr2 = ::CreateSolidBrush(0x0000FF);
			// fill it by black color:
			::FillRect(hDC,&rc,hbr1);
			::InflateRect(&rc,-40,-40);
			::FillRect(hDC,&rc,hbr2);
			::DeleteObject(hbr1);
			::DeleteObject(hbr2);
			*/

			//HDC hDC=buffer.DC(); //获取一个DC，TForm1.Handle中保存有Form的窗口句柄
			//调整该DC的象素格式

			//本函数用于调整DC的象素格式，如缓冲区、颜色数等
			//先不深究，只要知道它的作用就行
			int nPixelFormat;
			static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
			{
				sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
				1,											// Version Number
				PFD_DRAW_TO_WINDOW |						// Format Must Support Window
				PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
				PFD_DOUBLEBUFFER,							// Must Support Double Buffering
				PFD_TYPE_RGBA,								// Request An RGBA Format
				16,										// Select Our Color Depth
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
			// Choose a pixel format that best matches that described in pfd
			nPixelFormat = ChoosePixelFormat(hDC, &pfd);
			// Set the pixel format for the device context
			SetPixelFormat(hDC, nPixelFormat, &pfd);

			HGLRC hRC=wglCreateContext(hDC); //用这种DC去创建一个RC
			wglMakeCurrent(hDC,hRC); //指定当前DC、当前RC为hDC、hRC

			ReSizeGLScene(w, h);

			if (!InitGL())									// Initialize Our Newly Created GL Window
			{
				MessageBox(NULL, L"Initialization Failed.", L"ERROR",MB_OK|MB_ICONEXCLAMATION);
			}

			//DrawGLScene();					// Draw The Scene
			//SwapBuffers(hDC);				// Swap Buffers (Double Buffering)
		}

	};

	struct gdi_drawing_factory: public behavior_factory {

		gdi_drawing_factory(): behavior_factory("gdi-drawing") { }

		// the only behavior_factory method:
		virtual event_handler* create(HELEMENT he) {
			return new gdi_drawing();
		}

	};

	// instantiating and attaching it to the global list
	gdi_drawing_factory gdi_drawing_factory_instance;


}
