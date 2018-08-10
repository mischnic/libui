// 8 september 2015
#include "uipriv_windows.hpp"
#include "area.hpp"
#include "GL/gl.h"

static BOOL WGLExtensionSupported(const char *extension_name)
{
    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    // determine pointer to wglGetExtensionsStringEXT function
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");

    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
    {
        // string was not found
        return false;
    }

    // extension is supported
    return true;
}

// TODO handle WM_DESTROY/WM_NCDESTROY
// TODO same for other Direct2D stuff
static LRESULT CALLBACK areaWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uiOpenGLArea *a;
	CREATESTRUCTW *cs = (CREATESTRUCTW *) lParam;
	RECT client;
	WINDOWPOS *wp = (WINDOWPOS *) lParam;
	LRESULT lResult;

	a = (uiOpenGLArea *) GetWindowLongPtrW(hwnd, GWLP_USERDATA);
	if (a == NULL) {
		if (uMsg == WM_CREATE) {
			a = (uiOpenGLArea *) (cs->lpCreateParams);
			// assign a->hwnd here so we can use it immediately
			a->hwnd = hwnd;
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR) a);

			PIXELFORMATDESCRIPTOR pfd = {
				sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
				1,		     // version number
				PFD_DRAW_TO_WINDOW |   // support window
					PFD_SUPPORT_OPENGL |   // support OpenGL
					PFD_DOUBLEBUFFER,      // double buffered 	uiOpenGLAttributeDoubleBuffer uiOpenGLAttributeStereo
				PFD_TYPE_RGBA,	 // RGBA type
				24,		    // 24-bit color depth   			uiOpenGLAttributeRedBits uiOpenGLAttributeGreenBits uiOpenGLAttributeBlueBits uiOpenGLAttributeAlphaBits,
				0, 0, 0, 0, 0, 0,      // color bits ignored
				0,		     // no alpha buffer
				0,		     // shift bit ignored
				0,		     // no accumulation buffer
				0, 0, 0, 0,	    // accum bits ignored
				32,		    // 32-bit z-buffer					uiOpenGLAttributeDepthBits
				0,		     // no stencil buffer 				uiOpenGLAttributeStencilBits
				0,		     // no auxiliary buffer
				PFD_MAIN_PLANE,	// main layer
				0,		     // reserved
				0, 0, 0		// layer masks ignored
			};

			int iPixelFormat;
			a->hDC = GetDC(a->hwnd);

			// get the best available match of pixel format for the device context
			iPixelFormat = ChoosePixelFormat(a->hDC, &pfd);

			// make that the pixel format of the device context
			SetPixelFormat(a->hDC, iPixelFormat, &pfd);

			//TODO
			// WGL_CONTEXT_MAJOR_VERSION_ARB
			// WGL_CONTEXT_MINOR_VERSION_ARB
			// WGL_CONTEXT_CORE_PROFILE_BIT_ARB
			// WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB
			// WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
			// WGL_CONTEXT_DEBUG_BIT_ARB
			a->hglrc = wglCreateContext(a->hDC);
			uiOpenGLAreaMakeCurrent(a);
			(*(a->ah->InitGL))(a->ah, a);
		}

		// fall through to DefWindowProcW() anyway
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	 if (uMsg == WM_DESTROY || uMsg == WM_NCDESTROY) {
		if (a->hglrc) {
			wglMakeCurrent(NULL, NULL);
			ReleaseDC(a->hwnd, a->hDC);
			wglDeleteContext(a->hglrc);
			a->hDC = NULL;
			a->hglrc = NULL;
		}
	}

	// always recreate the render target if necessary
	if (a->rt == NULL)
		a->rt = makeHWNDRenderTarget(a->hwnd);

	if (areaDoDraw((uiArea *) a, uMsg, wParam, lParam, &lResult, TRUE) != FALSE)
		return lResult;

	if (uMsg == WM_WINDOWPOSCHANGED) {
		if ((wp->flags & SWP_NOSIZE) != 0)
			return DefWindowProcW(hwnd, uMsg, wParam, lParam);
		uiWindowsEnsureGetClientRect(a->hwnd, &client);
		areaDrawOnResize((uiArea *) a, &client);
		areaScrollOnResize((uiArea *) a, &client);
		return 0;
	}

	if (areaDoScroll((uiArea *) a, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;
	if (areaDoEvents((uiArea *) a, uMsg, wParam, lParam, &lResult) != FALSE)
		return lResult;

	// nothing done
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// control implementation

uiWindowsControlAllDefaults(uiOpenGLArea)

static void uiOpenGLAreaMinimumSize(uiWindowsControl *c, int *width, int *height)
{
	// TODO
	*width = 0;
	*height = 0;
}

ATOM registerOpenGLAreaClass(HICON hDefaultIcon, HCURSOR hDefaultCursor)
{
	WNDCLASSW wc;

	ZeroMemory(&wc, sizeof (WNDCLASSW));
	wc.lpszClassName = areaClass;
	wc.lpfnWndProc = areaWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = hDefaultIcon;
	wc.hCursor = hDefaultCursor;
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	// this is just to be safe; see the InvalidateRect() call in the WM_WINDOWPOSCHANGED handler for more details
	wc.style = CS_HREDRAW | CS_VREDRAW;
	return RegisterClassW(&wc);
}

void unregisterOpenGLArea(void)
{
	if (UnregisterClassW(areaClass, hInstance) == 0)
		logLastError(L"error unregistering uiOpenGLArea window class");
}

void uiOpenGLAreaSetSize(uiOpenGLArea *a, int width, int height)
{
	a->scrollWidth = width;
	a->scrollHeight = height;
	areaUpdateScroll((uiArea *) a);
}

void uiOpenGLAreaGetSize(uiOpenGLArea *a, double *width, double *height)
{

}

void uiOpenGLAreaSetVSync(uiOpenGLArea *a, int si)
{
	if (WGLExtensionSupported("WGL_EXT_swap_control")) {
	    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");

		uiOpenGLAreaMakeCurrent(a);
	    wglSwapIntervalEXT(si);
	}
	// TODO
	// Use the WGL_EXT_swap_control extension to control swap interval.
	// Check both the standard extensions string via glGetString(GL_EXTENSIONS)
	// and the WGL-specific extensions string via wglGetExtensionsStringARB() to
	// verify that WGL_EXT_swap_control is actually present.

	// The extension provides the wglSwapIntervalEXT() function, which directly
	// specifies the swap interval. wglSwapIntervalEXT(1) is used to enable vsync;
	// wglSwapIntervalEXT(0) to disable vsync.
}

void uiOpenGLAreaQueueRedrawAll(uiOpenGLArea *a)
{
	// don't erase the background; we do that ourselves in doPaint()
	invalidateRect(a->hwnd, NULL, FALSE);
}

void uiOpenGLAreaMakeCurrent(uiOpenGLArea *a)
{
	wglMakeCurrent(a->hDC, a->hglrc);
}

void uiOpenGLAreaSwapBuffers(uiOpenGLArea *a)
{
	SwapBuffers(a->hDC);
}

// void uiOpenGLAreaBeginUserWindowMove(uiOpenGLArea *a)
// {
// 	HWND toplevel;

// 	// TODO restrict execution
// 	ReleaseCapture();		// TODO use properly and reset internal data structures
// 	toplevel = parentToplevel(a->hwnd);
// 	if (toplevel == NULL) {
// 		// TODO
// 		return;
// 	}
// 	// see http://stackoverflow.com/questions/40249940/how-do-i-initiate-a-user-mouse-driven-move-or-resize-for-custom-window-borders-o#40250654
// 	SendMessageW(toplevel, WM_SYSCOMMAND,
// 		SC_MOVE | 2, 0);
// }

// void uiOpenGLAreaBeginUserWindowResize(uiOpenGLArea *a, uiWindowResizeEdge edge)
// {
// 	HWND toplevel;
// 	WPARAM wParam;

// 	// TODO restrict execution
// 	ReleaseCapture();		// TODO use properly and reset internal data structures
// 	toplevel = parentToplevel(a->hwnd);
// 	if (toplevel == NULL) {
// 		// TODO
// 		return;
// 	}
// 	// see http://stackoverflow.com/questions/40249940/how-do-i-initiate-a-user-mouse-driven-move-or-resize-for-custom-window-borders-o#40250654
// 	wParam = SC_SIZE;
// 	switch (edge) {
// 	case uiWindowResizeEdgeLeft:
// 		wParam |= 1;
// 		break;
// 	case uiWindowResizeEdgeTop:
// 		wParam |= 3;
// 		break;
// 	case uiWindowResizeEdgeRight:
// 		wParam |= 2;
// 		break;
// 	case uiWindowResizeEdgeBottom:
// 		wParam |= 6;
// 		break;
// 	case uiWindowResizeEdgeTopLeft:
// 		wParam |= 4;
// 		break;
// 	case uiWindowResizeEdgeTopRight:
// 		wParam |= 5;
// 		break;
// 	case uiWindowResizeEdgeBottomLeft:
// 		wParam |= 7;
// 		break;
// 	case uiWindowResizeEdgeBottomRight:
// 		wParam |= 8;
// 		break;
// 	}
// 	SendMessageW(toplevel, WM_SYSCOMMAND,
// 		wParam, 0);
// }

uiOpenGLArea *uiNewOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs)
{
	uiOpenGLArea *a;

	uiWindowsNewControl(uiOpenGLArea, a);

	a->ah = ah;
	a->scrolling = FALSE;
	a->attribs = attribs;
	uiprivClickCounterReset(&(a->cc));

	// a->hwnd is assigned in areaWndProc()
	uiWindowsEnsureCreateControlHWND(0,
		areaClass, L"",
		0,
		hInstance, a,
		FALSE);

	return a;
}

// uiOpenGLArea *uiNewScrollingOpenGLArea(uiOpenGLAreaHandler *ah, uiOpenGLAttributes *attribs, int width, int height)
// {
// 	uiOpenGLArea *a;

// 	uiWindowsNewControl(uiOpenGLArea, a);

// 	a->ah = ah;
// 	a->scrolling = TRUE;
// 	a->scrollWidth = width;
// 	a->scrollHeight = height;
// 	uiprivClickCounterReset(&(a->cc));

// 	// a->hwnd is assigned in areaWndProc()
// 	uiWindowsEnsureCreateControlHWND(0,
// 		areaClass, L"",
// 		WS_HSCROLL | WS_VSCROLL,
// 		hInstance, a,
// 		FALSE);

// 	// set initial scrolling parameters
// 	areaUpdateScroll((uiArea *) a);

// 	return a;
// }
