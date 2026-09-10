// SPDX-FileCopyrightText: 1996-2004 realtech VR
// SPDX-FileCopyrightText: 2005 Matt Williams
// SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later
//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2004 - realtech VR

This file is part of No Gravity 1.9

No Gravity is free software; you can redistribute it and/or
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
#include <SDL3/SDL.h>
#include "_rlx32.h"
#include "_rlx.h"
#include "sysctrl.h"

extern SDL_Window *g_pSDLWindow; // Owned by the display driver.
extern int g_SDLWheelDelta;      // Accumulated by the event pump in syskeyb.c.
extern int g_SDLCursorRefresh;   // Bumped by the event pump on enter/focus.

// While the cursor is hidden and the window is fullscreen and focused,
// the mouse runs in relative mode so aiming never stops at a screen edge;
// the absolute position is then tracked here as a virtual cursor.  In a
// window the pointer stays free, so it can reach the window controls.
static int g_bHidden = 0;
static int g_bRelative = 0;
static SDL_Window *g_pCursorWindow = NULL; // window the cursor state was applied to
static int g_nCursorRefresh = 0;

// SDL_HideCursor() only affects windows that exist when it is called, and
// the game hides the cursor before creating its window; re-apply the state
// on a new window and whenever the pointer or focus comes back.
static void ApplyCursorVisibility(void)
{
  if ((g_pSDLWindow != g_pCursorWindow) || (g_nCursorRefresh != g_SDLCursorRefresh))
  {
    g_pCursorWindow = g_pSDLWindow;
    g_nCursorRefresh = g_SDLCursorRefresh;
    if (g_bHidden)
      SDL_HideCursor();
    else
      SDL_ShowCursor();
  }
}
static float g_fVirtualX = 0.f, g_fVirtualY = 0.f;

static void ApplyRelativeMode(void)
{
  int want = 0;
  if (g_pSDLWindow && g_bHidden)
  {
    SDL_WindowFlags flags = SDL_GetWindowFlags(g_pSDLWindow);
    want = (flags & SDL_WINDOW_FULLSCREEN) && (flags & SDL_WINDOW_INPUT_FOCUS);
  }
  if (want && !g_bRelative)
  {
    SDL_GetMouseState(&g_fVirtualX, &g_fVirtualY);
    g_bRelative = SDL_SetWindowRelativeMouseMode(g_pSDLWindow, true) ? 1 : 0;
  }
  else if (!want && g_bRelative)
  {
    SDL_SetWindowRelativeMouseMode(g_pSDLWindow, false);
    if (g_pSDLWindow)
      SDL_WarpMouseInWindow(g_pSDLWindow, g_fVirtualX, g_fVirtualY);
    g_bRelative = 0;
  }
}

static int MouseOpen(void *hnd)
{
  UNUSED(hnd);
  sMOU->device = NULL;
  sMOU->numControllers = 1;
  sMOU->numButtons = 3;
  sMOU->numAxes = 3;
  return TRUE;
}

static void MouseRelease(void)
{
  g_bHidden = 0;
  ApplyRelativeMode();
}

static void MouseShow(void)
{
  g_bHidden = 0;
  ApplyRelativeMode();
  SDL_ShowCursor();
}

static void MouseHide(void)
{
  SDL_HideCursor();
  g_bHidden = 1;
  ApplyRelativeMode();
}

static void MouseSetPosition(u_int32_t x, u_int32_t y)
{
  g_fVirtualX = (float)x;
  g_fVirtualY = (float)y;
  if (g_pSDLWindow && !g_bRelative)
    SDL_WarpMouseInWindow(g_pSDLWindow, (float)x, (float)y);
}

static unsigned long MouseUpdate(void *dev)
{
  SDL_MouseButtonFlags buttons;
  float rx = 0.f, ry = 0.f, ax = 0.f, ay = 0.f;
  UNUSED(dev);

  // Copy the current button state to be the old button state.
  memcpy(sMOU->steButtons, sMOU->rgbButtons, sMOU->numButtons);

  // Follow window, fullscreen and focus changes.
  ApplyCursorVisibility();
  ApplyRelativeMode();

  buttons = SDL_GetRelativeMouseState(&rx, &ry);
  sMOU->lX = (int)rx;
  sMOU->lY = (int)ry;
  sMOU->rgbButtons[0] = (u_int8_t)((buttons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) ? 1 : 0);
  sMOU->rgbButtons[1] = (u_int8_t)((buttons & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) ? 1 : 0);
  sMOU->rgbButtons[2] = (u_int8_t)((buttons & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) ? 1 : 0);

  // The wheel arrives as events; report the movement since the last update.
  sMOU->lZ = g_SDLWheelDelta;
  g_SDLWheelDelta = 0;

  if (g_bRelative)
  {
    int w = 0, h = 0;
    if (g_pSDLWindow)
      SDL_GetWindowSize(g_pSDLWindow, &w, &h);
    g_fVirtualX += rx;
    g_fVirtualY += ry;
    if (g_fVirtualX < 0.f) g_fVirtualX = 0.f;
    if (g_fVirtualY < 0.f) g_fVirtualY = 0.f;
    if ((w > 0) && (g_fVirtualX > (float)(w - 1))) g_fVirtualX = (float)(w - 1);
    if ((h > 0) && (g_fVirtualY > (float)(h - 1))) g_fVirtualY = (float)(h - 1);
    ax = g_fVirtualX;
    ay = g_fVirtualY;
  }
  else
  {
    (void)SDL_GetMouseState(&ax, &ay);
    g_fVirtualX = ax;
    g_fVirtualY = ay;
  }
  sMOU->x = (int)ax;
  sMOU->y = (int)ay;
  return TRUE;
}

_RLXEXPORTFUNC MSE_ClientDriver *MSE_SystemGetInterface_STD(void)
{
  static MSE_ClientDriver driver =
  {
    MouseOpen,
    MouseRelease,
    MouseShow,
    MouseHide,
    MouseSetPosition,
    MouseUpdate
  };
  // Set the driver.
  sMOU = &driver;
  return sMOU;
}
