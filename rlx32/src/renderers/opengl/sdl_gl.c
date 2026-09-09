//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2004 - realtech VR

This file is part of Space Girl 1.9

Space Girl is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

Original Source: 1996 - Stephane Denis
Prepared for public release: 02/24/2004 - Stephane Denis, realtech VR
Linux/SDL Port: 2005 - Matt Williams
*/
//-------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <SDL3/SDL.h>
#include "_rlx32.h"
#include "_rlx.h"
#include "systools.h"
#include "gx_struc.h"
#include "gx_init.h"
#include "gx_csp.h"
#include "gx_rgb.h"
#include "v3xdefs.h"
#include "v3x_2.h"
#include "v3xrend.h"
#include "gl_v3x.h"

struct RLXSYSTEM *g_pRLX;

#define ISFULLSCREEN() (!(g_pRLX->Video.Config&RLXVIDEO_Windowed))

int GL_IsSupported(const char *extension);

// The window is shared with the mouse driver.
SDL_Window *g_pSDLWindow = NULL;

static SDL_GLContext g_GLContext = NULL;
static GXDISPLAYMODEINFO *g_pDisplays = NULL;
static int g_Mode = -1;

static int g_VSync = -1;

static void RLXAPI Flip(void)
{
    int vsync = (g_pRLX->pGX->View.Flags & GX_CAPS_VSYNC) ? 1 : 0;
    if (vsync != g_VSync)
    {
      SDL_GL_SetSwapInterval(vsync);
      g_VSync = vsync;
    }
    glFinish();
    SDL_GL_SwapWindow(g_pSDLWindow);
    return;
}

static u_int8_t RLXAPI *Lock(void)
{
    return NULL;
}

static void RLXAPI Unlock(void)
{
    return;
}

static int HasMode(const GXDISPLAYMODEINFO *list, int n, int w, int h)
{
  for (int i = 0; i < n; i++)
    if ((list[i].lWidth == w) && (list[i].lHeight == h))
      return 1;
  return 0;
}

// Build the list of usable resolutions once: every fullscreen mode of the
// primary display, largest first, reported at the requested colour depth.
static GXDISPLAYMODEINFO RLXAPI *EnumDisplayList(int bpp)
{
  GXDISPLAYMODEINFO *displays;
  int n;

  if ((bpp != 16) && (bpp != 24) && (bpp != 32))
    bpp = 32;

  if (g_pDisplays == NULL)
  {
    int count = 0;
    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    SDL_DisplayMode **modes = display ? SDL_GetFullscreenDisplayModes(display, &count) : NULL;

    g_pDisplays = (GXDISPLAYMODEINFO *)g_pRLX->mm_heap->malloc((count + 3) * sizeof(GXDISPLAYMODEINFO));
    n = 0;
    for (int i = 0; modes && (i < count); i++)
    {
      if (HasMode(g_pDisplays, n, modes[i]->w, modes[i]->h))
        continue;
      g_pDisplays[n].lWidth = modes[i]->w;
      g_pDisplays[n].lHeight = modes[i]->h;
      g_pDisplays[n].BitsPerPixel = bpp;
      g_pDisplays[n].mode = n;
      n++;
    }
    SDL_free(modes);

    if (n == 0)
    {
      static const int fallback[2][2] = {{800, 600}, {640, 480}};
      for (int i = 0; i < 2; i++, n++)
      {
        g_pDisplays[n].lWidth = fallback[i][0];
        g_pDisplays[n].lHeight = fallback[i][1];
        g_pDisplays[n].BitsPerPixel = bpp;
        g_pDisplays[n].mode = n;
      }
    }
    g_pDisplays[n].lWidth = 0;
    g_pDisplays[n].lHeight = 0;
    g_pDisplays[n].BitsPerPixel = 0;
    g_pDisplays[n].mode = 0;
  }

  for (n = 0; g_pDisplays[n].BitsPerPixel != 0; n++)
  {
    // Count entries.
  }
  displays = (GXDISPLAYMODEINFO *)g_pRLX->mm_heap->malloc((n + 1) * sizeof(GXDISPLAYMODEINFO));
  memcpy(displays, g_pDisplays, (n + 1) * sizeof(GXDISPLAYMODEINFO));
  return displays;
}

extern GXGRAPHICINTERFACE GI_OpenGL;
extern GXSPRITEINTERFACE CSP_OpenGL;

static void RLXAPI SetPrimitive()
{
  g_pRLX->pGX->View.Flip = Flip;
  g_pRLX->pGX->gi = GI_OpenGL;
  g_pRLX->pGX->csp = CSP_OpenGL;
  g_pRLX->pGX->csp_cfg.put.fonct = g_pRLX->pGX->csp.put;
  g_pRLX->pGX->csp_cfg.pset.fonct = g_pRLX->pGX->csp.pset;
  g_pRLX->pGX->csp_cfg.transp.fonct = g_pRLX->pGX->csp.Trsp50;
  g_pRLX->pGX->csp_cfg.op = g_pRLX->pGX->csp.put;
}

static void RLXAPI GetDisplayInfo(GXDISPLAYMODEHANDLE mode)
{
  SYS_ASSERT(g_pDisplays != NULL);
  if ((mode < 0) && (g_pDisplays[0].BitsPerPixel != 0))
  {
    mode = 0;
  }
  g_pRLX->pfSetViewPort(&g_pRLX->pGX->View, g_pDisplays[mode].lWidth, g_pDisplays[mode].lHeight, g_pDisplays[mode].BitsPerPixel);
  if (g_pDisplays[mode].BitsPerPixel == 16)
  {
    g_pRLX->pGX->View.ColorMask.RedMaskSize = 5;
    g_pRLX->pGX->View.ColorMask.GreenMaskSize = 6;
    g_pRLX->pGX->View.ColorMask.BlueMaskSize = 5;
    g_pRLX->pGX->View.ColorMask.RsvdMaskSize = 0;
    g_pRLX->pGX->View.ColorMask.RedFieldPosition = 0;
    g_pRLX->pGX->View.ColorMask.GreenFieldPosition = 5;
    g_pRLX->pGX->View.ColorMask.BlueFieldPosition = 11;
    g_pRLX->pGX->View.ColorMask.RsvdFieldPosition = 16;
  }
  else if (g_pDisplays[mode].BitsPerPixel == 24)
  {
    g_pRLX->pGX->View.ColorMask.RedMaskSize = 8;
    g_pRLX->pGX->View.ColorMask.GreenMaskSize = 8;
    g_pRLX->pGX->View.ColorMask.BlueMaskSize = 8;
    g_pRLX->pGX->View.ColorMask.RsvdMaskSize = 0;
    g_pRLX->pGX->View.ColorMask.RedFieldPosition = 0;
    g_pRLX->pGX->View.ColorMask.GreenFieldPosition = 8;
    g_pRLX->pGX->View.ColorMask.BlueFieldPosition = 16;
    g_pRLX->pGX->View.ColorMask.RsvdFieldPosition = 24;
  }
  else
  {
    g_pRLX->pGX->View.ColorMask.RedMaskSize = 8;
    g_pRLX->pGX->View.ColorMask.GreenMaskSize = 8;
    g_pRLX->pGX->View.ColorMask.BlueMaskSize = 8;
    g_pRLX->pGX->View.ColorMask.RsvdMaskSize = 8;
    g_pRLX->pGX->View.ColorMask.RedFieldPosition = 0;
    g_pRLX->pGX->View.ColorMask.GreenFieldPosition = 8;
    g_pRLX->pGX->View.ColorMask.BlueFieldPosition = 16;
    g_pRLX->pGX->View.ColorMask.RsvdFieldPosition = 24;
  }
  SetPrimitive();
  GL_FakeViewPort();
  return;
}

static int RLXAPI SetDisplayMode(GXDISPLAYMODEHANDLE mode)
{
  if ((mode < 0) && (g_pDisplays[0].BitsPerPixel != 0))
  {
    mode = 0;
  }
  g_Mode = mode;
  return 0;
}

static GXDISPLAYMODEHANDLE RLXAPI SearchDisplayMode(int lx, int ly, int bpp)
{
  GXDISPLAYMODEHANDLE mode;
  SYS_ASSERT(g_pDisplays != NULL);
  for (mode = 0; g_pDisplays[mode].BitsPerPixel != 0; mode ++)
  {
    if ((g_pDisplays[mode].lWidth == lx) &&
        (g_pDisplays[mode].lHeight == ly) &&
        (g_pDisplays[mode].BitsPerPixel == bpp))
    {
      break;
    }
  }
  if (g_pDisplays[mode].BitsPerPixel == 0)
  {
    mode = (g_pDisplays[0].BitsPerPixel != 0) ? 0 : -1;
  }
  return mode;
}

// Apply size and fullscreen state to an existing window.
static void ConfigureWindow(SDL_Window *window, int w, int h)
{
  if (ISFULLSCREEN())
  {
    SDL_DisplayMode closest;
    SDL_DisplayID display = SDL_GetDisplayForWindow(window);
    if (SDL_GetClosestFullscreenDisplayMode(display, w, h, 0.f, false, &closest))
      SDL_SetWindowFullscreenMode(window, &closest);
    SDL_SetWindowFullscreen(window, true);
  }
  else
  {
    SDL_SetWindowFullscreen(window, false);
    SDL_SetWindowSize(window, w, h);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  }
  SDL_SyncWindow(window);
}

static SDL_Window *CreateGLWindow(int w, int h, int bpp, int multisampling)
{
  SDL_WindowFlags flags = SDL_WINDOW_OPENGL;
  SDL_Window *window;

  if (ISFULLSCREEN())
    flags |= SDL_WINDOW_FULLSCREEN;

  SDL_GL_ResetAttributes();
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  if (bpp == 16)
  {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  }
  if (multisampling)
  {
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, multisampling);
  }

  window = SDL_CreateWindow("No Gravity", w, h, flags);
  if (window == NULL)
    return NULL;

  ConfigureWindow(window, w, h);

  g_GLContext = SDL_GL_CreateContext(window);
  if (g_GLContext == NULL)
  {
    SDL_DestroyWindow(window);
    return NULL;
  }
  SDL_GL_MakeCurrent(window, g_GLContext);
  return window;
}

static int RLXAPI CreateSurface(int BackBufferCount)
{
	int multisampling = 0;
  	SYS_ASSERT(g_pDisplays != NULL);
	SYS_ASSERT(g_Mode != -1);

	// A mode change keeps the window and the GL context (and with it the
	// uploaded textures); only the size and fullscreen state change.
	if (g_pSDLWindow != NULL)
	{
		ConfigureWindow(g_pSDLWindow, g_pDisplays[g_Mode].lWidth, g_pDisplays[g_Mode].lHeight);
		GL_ResetViewport();
		g_pRLX->pGX->Surfaces.maxSurface = BackBufferCount;
		SYS_Msg("Video: %dx%d %s", g_pDisplays[g_Mode].lWidth, g_pDisplays[g_Mode].lHeight, ISFULLSCREEN() ? "fullscreen" : "windowed");
		return 0;
	}

	// Try multisampling first if it was asked for, then fall back.
	if ((g_pRLX->pGX->View.Flags & GX_CAPS_MULTISAMPLING) &&
	    (g_pRLX->pGX->View.Multisampling != 0))
	{
	  multisampling = g_pRLX->pGX->View.Multisampling;
	  g_pSDLWindow = CreateGLWindow(g_pDisplays[g_Mode].lWidth, g_pDisplays[g_Mode].lHeight, g_pDisplays[g_Mode].BitsPerPixel, multisampling);
	}
	if (g_pSDLWindow == NULL)
	{
	  multisampling = 0;
	  g_pSDLWindow = CreateGLWindow(g_pDisplays[g_Mode].lWidth, g_pDisplays[g_Mode].lHeight, g_pDisplays[g_Mode].BitsPerPixel, 0);
	}
	if (g_pSDLWindow == NULL)
	{
		SYS_Msg("Unable to create an OpenGL window: %s", SDL_GetError());
		return -1;
	}

	// Reset engine
	GL_InstallExtensions();
	GL_ResetViewport();
#ifdef _DEBUG
	SYS_Debug("...%s\n", glGetString(GL_VENDOR));
	SYS_Debug("...%s\n", glGetString(GL_VERSION));
	SYS_Debug("...%s\n", glGetString(GL_RENDERER));
#endif
	if (multisampling)
	{
	  glEnable(GL_MULTISAMPLE_ARB);
	}
	SYS_Msg("Video: %dx%d %s, %s", g_pDisplays[g_Mode].lWidth, g_pDisplays[g_Mode].lHeight, ISFULLSCREEN() ? "fullscreen" : "windowed", (const char *)glGetString(GL_RENDERER));
	g_pRLX->pGX->Surfaces.maxSurface = BackBufferCount;
	return 0;
}

static void DestroyGLWindow(void)
{
  if (g_GLContext)
  {
    SDL_GL_DestroyContext(g_GLContext);
    g_GLContext = NULL;
  }
  if (g_pSDLWindow)
  {
    SDL_DestroyWindow(g_pSDLWindow);
    g_pSDLWindow = NULL;
  }
  g_VSync = -1;
}

// The window itself is kept until Shutdown() so a mode change does not
// lose the GL context.
static void RLXAPI ReleaseSurfaces(void)
{
  g_pRLX->pGX->Surfaces.maxSurface = 0;
  return;
}

static int RLXAPI RegisterMode(GXDISPLAYMODEHANDLE mode)
{
  if ((mode < 0) && (g_pDisplays[0].BitsPerPixel != 0))
  {
    mode = 0;
  }
  g_pRLX->pGX->View.DisplayMode = (u_int16_t)mode;
  g_pRLX->pGX->Client->GetDisplayInfo(mode);
  return g_pRLX->pGX->Client->SetDisplayMode(mode);
}

static void RLXAPI Shutdown(void)
{
	DestroyGLWindow();
	if (g_pDisplays != NULL)
	{
		g_pRLX->mm_heap->free(g_pDisplays);
		g_pDisplays = NULL;
	}
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static int Open(void *hnd)
{
	UNUSED(hnd);
	return SDL_InitSubSystem(SDL_INIT_VIDEO) ? 0 : -1;
}

static unsigned NotifyEvent(enum GX_EVENT_MODE mode, int x, int y)
{
    UNUSED(x);
    UNUSED(y);
    return mode;
}

GXCLIENTDRIVER GX_OpenGL = {
    Lock,
    Unlock,
    EnumDisplayList,
    GetDisplayInfo,
    SetDisplayMode,
    SearchDisplayMode,
    CreateSurface,
    ReleaseSurfaces,
    NULL,
    NULL,
    NULL,
    RegisterMode,
    Shutdown,
    Open,
    NotifyEvent,
    "OpenGL"
};

extern void SetPrimitiveSprites();

_RLXEXPORTFUNC void RLXAPI GX_EntryPoint(struct RLXSYSTEM *p)
{
    g_pRLX = p;
	SetPrimitiveSprites();
	g_pRLX->pGX->Client = &GX_OpenGL;
    return;
}

extern V3X_GXSystem V3X_OpenGL;

void RLXAPI V3X_EntryPoint(struct RLXSYSTEM *p)
{
	GX_EntryPoint(p);
    g_pRLX->pV3X->Client = &V3X_OpenGL;
    return;
}
