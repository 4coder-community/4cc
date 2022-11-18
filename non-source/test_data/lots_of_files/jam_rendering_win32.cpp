/*
 *  Overreact - Mr. 4th Dimention
 *      Allen Webster
 *  03.21.2015 (mm.dd.yyyy)
 *
 * Win32 Rendering System Layer.
 */

#define SIG_STARTUP(name) void(name)(Win32RenderingVars *render_vars)
#define SIG_SHUTDOWN(name) void(name)(Win32RenderingVars *render_vars)
#define SIG_REDRAW(name) void(name)(Win32RenderingVars *render_vars)
#define SIG_SET_SIZE(name) void(name)(Win32RenderingVars *render_vars, i32 port_width, i32 port_height, i32 view_width, i32 view_height)
#define SIG_GET_RENDER_TARGET(name) Game_Render_Target(name)(Win32RenderingVars *render_vars)

#if RENDER_MODE == SOFTWARE

struct Win32RenderingVars{
	HDC hdc;
	BITMAPINFO bmp_info;
	i32 width, height, pitch;
	i32 pixel_data_size;
	void *pixel_data;
};

internal
SIG_STARTUP(render_startup){}

internal
SIG_SHUTDOWN(render_shutdown){}

internal
SIG_REDRAW(render_redraw_screen){
	render_vars->bmp_info.bmiHeader.biHeight =
		-render_vars->bmp_info.bmiHeader.biHeight;
	SetDIBitsToDevice(render_vars->hdc, 0, 0,
					  render_vars->width, render_vars->height,
					  0, 0,
					  0, render_vars->height,
					  render_vars->pixel_data,
					  &render_vars->bmp_info,
					  DIB_RGB_COLORS);
	render_vars->bmp_info.bmiHeader.biHeight =
		-render_vars->bmp_info.bmiHeader.biHeight;
}

// TODO(allen): This version ignores view / port ratio.  It simply resize
// to the dimensions of the port and the game will render in the top
// left leaving the rest of the window ignored.  Either implement
// software stretching, or at least center the view like the original Push.
internal
SIG_SET_SIZE(render_set_screen_size){
	render_vars->width = port_width;
	render_vars->height = port_height;
	render_vars->pitch = port_width*4;
	
	render_vars->bmp_info = {};
	BITMAPINFOHEADER bmi_header = {};
	bmi_header.biSize = sizeof(BITMAPINFOHEADER);
	bmi_header.biWidth = render_vars->width;
	bmi_header.biHeight = render_vars->height;
	bmi_header.biPlanes = 1;
	bmi_header.biBitCount = 32;
	bmi_header.biCompression = BI_RGB;
	render_vars->bmp_info.bmiHeader = bmi_header;
	
	// TODO(allen): Bulletproof this.
	
	i32 new_size = port_height*port_width*4;
	if (new_size > render_vars->pixel_data_size){
		if (render_vars->pixel_data){
			VirtualFree(render_vars->pixel_data,
						0, MEM_RELEASE);
		}
		
		render_vars->pixel_data_size = new_size;
		render_vars->pixel_data =
			VirtualAlloc(0, height*width*4,
						 MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		
		if (!render_vars->pixel_data){
			WINERROR("Failed to setup window pixel memory");
		}
	}
}

SIG_GET_RENDER_TARGET(render_get_render_target){
	Game_Render_Target render;
	
	render.pixels = (u8*)render_vars->pixel_data;
	render.width = render_vars->width;
	render.height = render_vars->height;
	render.pitch = render_vars->pitch;

	return render;
}

#elif RENDER_MODE == OPENGL

struct Win32RenderingVars{
	HDC hdc;
	HGLRC context;
	i32 width, height;
};

internal
SIG_STARTUP(render_startup){
	int nPixelFormat;
	
	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),          //size of structure
		1,                                      //default version
		PFD_DRAW_TO_WINDOW |                    //window drawing support
		PFD_SUPPORT_OPENGL |                    //opengl support
		PFD_DOUBLEBUFFER,                       //double buffering support
		PFD_TYPE_RGBA,                          //RGBA color mode
		32,                                     //32 bit color mode
		0, 0, 0, 0, 0, 0,                       //ignore color bits
		0,                                      //no alpha buffer
		0,                                      //ignore shift bit
		0,                                      //no accumulation buffer
		0, 0, 0, 0,                             //ignore accumulation bits
		16,                                     //16 bit z-buffer size
		0,                                      //no stencil buffer
		0,                                      //no aux buffer
		PFD_MAIN_PLANE,                         //main drawing plane
		0,                                      //reserved
		0, 0, 0 };                              //layer masks ignored
	
	nPixelFormat = ChoosePixelFormat(render_vars->hdc, &pfd);
	SetPixelFormat(render_vars->hdc, nPixelFormat, &pfd);
	
	render_vars->context = wglCreateContext(render_vars->hdc);
	wglMakeCurrent(render_vars->hdc, render_vars->context);
    
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    //glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    GLenum error;
    error = glGetError();
    error = error;
}

internal
SIG_SHUTDOWN(render_shutdown){
	wglMakeCurrent(render_vars->hdc, 0);
	wglDeleteContext(render_vars->context);
}

internal
SIG_REDRAW(render_redraw_screen){
	glFlush();
	SwapBuffers(render_vars->hdc);
}

// NOTE(allen): This changes the aspect ratio of the view to perfectly
// fill the window.  The height is untouched, the width is the only part
// of the view that is effected.
internal
SIG_SET_SIZE(render_set_screen_size){
	render_vars->height = view_height;
	render_vars->width = (i32)((port_width / ((real32)port_height)) * view_height);
	
	glViewport(0, 0, port_width, port_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.f, render_vars->width, view_height, 0.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// NOTE(allen): This maintains a fixed aspect ratio and
// puts the game port in the center, expecting the game to
// add bars
#if 0
internal
SIG_SET_SIZE(render_set_screen_size){
	render_vars->width = view_width;
	render_vars->height = view_height;
	
	real32 ratio_width, ratio_height;
	ratio_width = (real32)(port_width) / view_width;
	ratio_height = (real32)(port_height) / view_height;
	
	i32 start_x, start_y;
	if (ratio_width > ratio_height){
		ratio_width = ratio_height;
		start_y = 0;
		start_x = (port_width - (ratio_width*view_width))/2;
	}
	else{
		ratio_height = ratio_width;
		start_x = 0;
		start_y = (port_height - (ratio_height*view_height))/2;
	}
	
	glViewport(start_x, start_y, ratio_width*view_width, ratio_height*view_height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.f, view_width, view_height, 0.f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
#endif

SIG_GET_RENDER_TARGET(render_get_render_target){
	Game_Render_Target render = {};
	render.width = render_vars->width;
	render.height = render_vars->height;
	return render;
}

#endif
