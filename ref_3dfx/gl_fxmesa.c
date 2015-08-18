/*
** GLW_IMP.C
**
** This file contains ALL Linux specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>

#include "gl_local.h"
#include "../client/keys.h"

#include <GL/fxmesa.h>

/*****************************************************************************/

qboolean GLimp_InitGL (void);

extern cvar_t *vid_fullscreen;
extern cvar_t *vid_ref;

static fxMesaContext fc = NULL;

#define NUM_RESOLUTIONS 16

static int resolutions[NUM_RESOLUTIONS][3]={ 
	{ 320,200,  GR_RESOLUTION_320x200 },
	{ 320,240,  GR_RESOLUTION_320x240 },
	{ 400,256,  GR_RESOLUTION_400x256 },
	{ 400,300,  GR_RESOLUTION_400x300 },
	{ 512,384,  GR_RESOLUTION_512x384 },
	{ 640,200,  GR_RESOLUTION_640x200 },
	{ 640,350,  GR_RESOLUTION_640x350 },
	{ 640,400,  GR_RESOLUTION_640x400 },
	{ 640,480,  GR_RESOLUTION_640x480 },
	{ 800,600,  GR_RESOLUTION_800x600 },
	{ 960,720,  GR_RESOLUTION_960x720 },
	{ 856,480,  GR_RESOLUTION_856x480 },
	{ 512,256,  GR_RESOLUTION_512x256 },
	{ 1024,768, GR_RESOLUTION_1024x768 },
	{ 1280,1024,GR_RESOLUTION_1280x1024 },
	{ 1600,1200,GR_RESOLUTION_1600x1200 }
};

static int findres(int *width, int *height)
{
	int i;

	for(i=0;i<NUM_RESOLUTIONS;i++)
		if((*width<=resolutions[i][0]) && (*height<=resolutions[i][1])) {
			*width = resolutions[i][0];
			*height = resolutions[i][1];
			return resolutions[i][2];
		}
        
	*width = 640;
	*height = 480;
	return GR_RESOLUTION_640x480;
}

/*
** GLimp_SetMode
*/
rserr_t GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen )
{
	int width, height;
	GLint attribs[32];

	if(gl_mode->value < 0.0f)
	{
		width = r_customwidth->intValue;
		height = r_customheight->intValue;
	}
	else
	{
		width=640;
		height=480;
	}
	mode=false;

	ri.Con_Printf( PRINT_ALL, "Initializing OpenGL display\n");

	ri.Con_Printf (PRINT_ALL, "...setting mode %d:", mode );

	/* FS: FIXME */
	/*if ( !ri.Vid_GetModeInfo( &width, &height, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}*/

	ri.Vid_GetModeInfo( &width, &height, mode );

	ri.Con_Printf( PRINT_ALL, " %d %d\n", width, height );

	// destroy the existing window
	GLimp_Shutdown ();

	// set fx attribs
	attribs[0] = FXMESA_DOUBLEBUFFER;
	attribs[1] = FXMESA_ALPHA_SIZE;
	attribs[2] = 1;
	attribs[3] = FXMESA_DEPTH_SIZE;
	attribs[4] = 1;
	attribs[5] = FXMESA_NONE;

//	fc = fxMesaCreateContext(0, findres(&width, &height), GR_REFRESH_60Hz, attribs);
	fc = fxMesaCreateBestContext(0, (GLint)width, (GLint)height, attribs); /* FS: This will allow us to use SST_SCREENREFRESH to set the refresh rate */
	if (!fc)
		return rserr_invalid_mode;

	*pwidth = width;
	*pheight = height;

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (width, height);

	fxMesaMakeCurrent(fc);

	return rserr_ok;
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GLimp_Shutdown( void )
{
	if (fc) {
		fxMesaDestroyContext(fc);
		fc = NULL;
	}
}

/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  
*/

int GLimp_Init( void *hinstance, void *wndproc )
{
	return true;
}

/*
** GLimp_BeginFrame
*/
void GLimp_BeginFrame( float camera_seperation )
{
}

/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
	glFlush();
	fxMesaSwapBuffers();
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate( qboolean active )
{
}

extern void gl3DfxSetPaletteEXT(GLuint *pal);

void Fake_glColorTableEXT( GLenum target, GLenum internalformat,
                             GLsizei width, GLenum format, GLenum type,
                             const GLvoid *table )
{
	byte temptable[256][4];
	byte *intbl;
	int i;

	for (intbl = (byte *)table, i = 0; i < 256; i++) {
		temptable[i][2] = *intbl++;
		temptable[i][1] = *intbl++;
		temptable[i][0] = *intbl++;
		temptable[i][3] = 255;
	}
	gl3DfxSetPaletteEXT((GLuint *)temptable);
}


