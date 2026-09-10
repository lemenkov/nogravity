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

// Set when the window manager asks us to close; polled by STUB_TaskControl().
int g_bSDLQuitRequested = 0;

// Mouse wheel movement accumulated since the last mouse update.
int g_SDLWheelDelta = 0;

// Bumped when the window (re)gains the pointer or focus, so the mouse
// driver re-applies cursor visibility.
int g_SDLCursorRefresh = 0;

static int KeyboardOpen(void *hnd)
{
  UNUSED(hnd);
  return TRUE;
}

static void KeyboardRelease(void)
{
  // Nothing to do.
}

static char *KeyboardNameScanCode(int scn)
{
  static const char *name[] =
  {
    "", "ESCAPE", "1", "2", "3", "4", "5", "6",
    "7", "8", "9", "0", "-", "=", "BACKSPACE", "TAB",
    "Q", "W", "E", "R", "T", "Y", "U", "I",
    "O", "P", "[", "]", "RETURN", "CTRL", "A", "S",
    "D", "F", "G", "H", "J", "K", "L", ";",
    "'", "~", "LEFT SHIFT", "\\", "Z", "X", "C", "V",
    "B", "N", "M", ",", ".", "/", "RIGHT SHIFT", "PRINT SCREEN",
    "ALT", "SPACE", "CAPS LOCK", "F1", "F2", "F3", "F4", "F5",
    "F6", "F7", "F8", "F9", "F10", "NUM LOCK", "SCROLL LOCK", "KEYPAD 7",
    "KEYPAD 8", "KEYPAD 9", "KEYPAD -", "KEYPAD 4", "KEYPAD 5", "KEYPAD 6", "KEYPAD +", "KEYPAD 1",
    "KEYPAD 2", "KEYPAD 3", "KEYPAD 0", "KEYPAD .", "", "", "", "F11",
    "F12", "", "", "LEFT WINDOWS", "RIGHT WINDOWS", "MENU", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "JOY LEFT", "JOY RIGHT",
    "JOY UP", "JOY DOWN", "", "", "", "", "", "",
    "JOY BUTTON 1", "JOY BUTTON 2", "JOY BUTTON 3", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "HOME",
    "UP", "PAGE UP", "", "LEFT", "", "RIGHT", "", "END",
    "DOWN", "PAGE DOWN", "INSERT", "DELETE", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
    "", "", "", "", "", "", "", "",
  };
  if ((scn < 0) || (scn >= (int)(sizeof(name) / sizeof(name[0]))))
    return (char *)"";
  return (char *)name[scn];
}

// SDL scancodes (USB HID usage ids, layout independent) to the engine's
// PC scancode set.  Anything not listed is ignored.
static const u_int8_t g_ScanMap[SDL_SCANCODE_COUNT] =
{
  [SDL_SCANCODE_A] = s_a, [SDL_SCANCODE_B] = s_b, [SDL_SCANCODE_C] = s_c,
  [SDL_SCANCODE_D] = s_d, [SDL_SCANCODE_E] = s_e, [SDL_SCANCODE_F] = s_f,
  [SDL_SCANCODE_G] = s_g, [SDL_SCANCODE_H] = s_h, [SDL_SCANCODE_I] = s_i,
  [SDL_SCANCODE_J] = s_j, [SDL_SCANCODE_K] = s_k, [SDL_SCANCODE_L] = s_l,
  [SDL_SCANCODE_M] = s_m, [SDL_SCANCODE_N] = s_n, [SDL_SCANCODE_O] = s_o,
  [SDL_SCANCODE_P] = s_p, [SDL_SCANCODE_Q] = s_q, [SDL_SCANCODE_R] = s_r,
  [SDL_SCANCODE_S] = s_s, [SDL_SCANCODE_T] = s_t, [SDL_SCANCODE_U] = s_u,
  [SDL_SCANCODE_V] = s_v, [SDL_SCANCODE_W] = s_w, [SDL_SCANCODE_X] = s_x,
  [SDL_SCANCODE_Y] = s_y, [SDL_SCANCODE_Z] = s_z,

  [SDL_SCANCODE_1] = s_1, [SDL_SCANCODE_2] = s_2, [SDL_SCANCODE_3] = s_3,
  [SDL_SCANCODE_4] = s_4, [SDL_SCANCODE_5] = s_5, [SDL_SCANCODE_6] = s_6,
  [SDL_SCANCODE_7] = s_7, [SDL_SCANCODE_8] = s_8, [SDL_SCANCODE_9] = s_9,
  [SDL_SCANCODE_0] = s_0,

  [SDL_SCANCODE_RETURN] = s_return, [SDL_SCANCODE_ESCAPE] = s_esc,
  [SDL_SCANCODE_BACKSPACE] = s_backspace, [SDL_SCANCODE_TAB] = s_tab,
  [SDL_SCANCODE_SPACE] = s_space, [SDL_SCANCODE_MINUS] = s_minus,
  [SDL_SCANCODE_EQUALS] = s_equals, [SDL_SCANCODE_LEFTBRACKET] = s_opensquare,
  [SDL_SCANCODE_RIGHTBRACKET] = s_closesquare, [SDL_SCANCODE_BACKSLASH] = s_backslash,
  [SDL_SCANCODE_NONUSHASH] = s_backslash, [SDL_SCANCODE_SEMICOLON] = s_semicolon,
  [SDL_SCANCODE_APOSTROPHE] = s_quote, [SDL_SCANCODE_GRAVE] = s_tilda,
  [SDL_SCANCODE_COMMA] = s_coma, [SDL_SCANCODE_PERIOD] = s_period,
  [SDL_SCANCODE_SLASH] = s_slash, [SDL_SCANCODE_CAPSLOCK] = s_capslock,

  [SDL_SCANCODE_F1] = s_f1, [SDL_SCANCODE_F2] = s_f2, [SDL_SCANCODE_F3] = s_f3,
  [SDL_SCANCODE_F4] = s_f4, [SDL_SCANCODE_F5] = s_f5, [SDL_SCANCODE_F6] = s_f6,
  [SDL_SCANCODE_F7] = s_f7, [SDL_SCANCODE_F8] = s_f8, [SDL_SCANCODE_F9] = s_f9,
  [SDL_SCANCODE_F10] = s_f10, [SDL_SCANCODE_F11] = s_f11, [SDL_SCANCODE_F12] = s_f12,

  [SDL_SCANCODE_PRINTSCREEN] = s_printscreen, [SDL_SCANCODE_SCROLLLOCK] = s_scrolllock,
  [SDL_SCANCODE_INSERT] = s_insert, [SDL_SCANCODE_HOME] = s_home,
  [SDL_SCANCODE_PAGEUP] = s_pageup, [SDL_SCANCODE_DELETE] = s_delete,
  [SDL_SCANCODE_END] = s_end, [SDL_SCANCODE_PAGEDOWN] = s_pagedown,
  [SDL_SCANCODE_RIGHT] = s_right, [SDL_SCANCODE_LEFT] = s_left,
  [SDL_SCANCODE_DOWN] = s_down, [SDL_SCANCODE_UP] = s_up,

  [SDL_SCANCODE_NUMLOCKCLEAR] = s_numlock, [SDL_SCANCODE_KP_MINUS] = s_numminus,
  [SDL_SCANCODE_KP_PLUS] = s_numplus, [SDL_SCANCODE_KP_ENTER] = s_return,
  [SDL_SCANCODE_KP_1] = s_numend, [SDL_SCANCODE_KP_2] = s_numdown,
  [SDL_SCANCODE_KP_3] = s_numpagedown, [SDL_SCANCODE_KP_4] = s_numleft,
  [SDL_SCANCODE_KP_5] = s_num5, [SDL_SCANCODE_KP_6] = s_numright,
  [SDL_SCANCODE_KP_7] = s_numhome, [SDL_SCANCODE_KP_8] = s_numup,
  [SDL_SCANCODE_KP_9] = s_numpageup, [SDL_SCANCODE_KP_0] = s_numinsert,
  [SDL_SCANCODE_KP_PERIOD] = s_numdelete,

  [SDL_SCANCODE_LCTRL] = s_ctrl, [SDL_SCANCODE_RCTRL] = s_ctrl,
  [SDL_SCANCODE_LSHIFT] = s_leftshift, [SDL_SCANCODE_RSHIFT] = s_rightshift,
  [SDL_SCANCODE_LALT] = s_alt, [SDL_SCANCODE_RALT] = s_alt,
  [SDL_SCANCODE_LGUI] = s_winleft, [SDL_SCANCODE_RGUI] = s_winright,
  [SDL_SCANCODE_APPLICATION] = s_winapp,
};

// Scripted key presses for automated runs: NOGRAVITY_TEST_KEYS is a
// comma separated list of "<milliseconds>:<key>" where key is an SDL
// key name ("Return", "Down", "A"...).  Each entry is pressed at that
// time and released on the next update.
static void InjectScriptedKeys(void)
{
  static int parsed = 0;
  static struct { Uint64 at; SDL_Scancode scancode; } script[64];
  static int count = 0, next = 0, release = -1;
  SDL_Event e;
  Uint64 now;

  if (!parsed)
  {
    const char *env = SDL_getenv("NOGRAVITY_TEST_KEYS");
    parsed = 1;
    while (env && *env && (count < 64))
    {
      char name[32];
      int n = 0;
      Uint64 at = (Uint64)SDL_strtoull(env, (char **)&env, 10);
      if (*env != ':')
        break;
      env++;
      while (*env && (*env != ',') && (n < 31))
        name[n++] = *env++;
      name[n] = 0;
      script[count].at = at;
      script[count].scancode = SDL_GetScancodeFromName(name);
      if (script[count].scancode != SDL_SCANCODE_UNKNOWN)
        count++;
      if (*env == ',')
        env++;
    }
  }
  memset(&e, 0, sizeof(e));
  if (release >= 0)
  {
    e.type = SDL_EVENT_KEY_UP;
    e.key.scancode = script[release].scancode;
    SDL_PushEvent(&e);
    release = -1;
  }
  now = SDL_GetTicks();
  if ((next < count) && (now >= script[next].at))
  {
    e.type = SDL_EVENT_KEY_DOWN;
    e.key.scancode = script[next].scancode;
    SDL_PushEvent(&e);
    release = next;
    next++;
  }
}

// This is the only place that pumps the SDL event queue, so window and
// mouse wheel events are picked up here as well.
static unsigned long KeyboardUpdate(void *dev)
{
  SDL_Event evt;
  static u_int8_t keys[SKEY_SCANTABLESIZE] = {0};
  UNUSED(dev);

  // Copy the current key state to be the old key state.
  memcpy(sKEY->steButtons, sKEY->rgbButtons, SKEY_SCANTABLESIZE);

  InjectScriptedKeys();

  while (SDL_PollEvent(&evt))
  {
    switch (evt.type)
    {
      case SDL_EVENT_QUIT:
        g_bSDLQuitRequested = 1;
        break;

      case SDL_EVENT_MOUSE_WHEEL:
        g_SDLWheelDelta += (int)evt.wheel.y;
        break;

      case SDL_EVENT_WINDOW_MOUSE_ENTER:
      case SDL_EVENT_WINDOW_FOCUS_GAINED:
        g_SDLCursorRefresh++;
        break;

      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
      {
        int scan;
        if (evt.key.repeat || (evt.key.scancode >= SDL_SCANCODE_COUNT))
          break;
        scan = g_ScanMap[evt.key.scancode];
        if (scan == 0)
          break;
        if (evt.type == SDL_EVENT_KEY_DOWN)
        {
          // Unmodified key symbol, as the engine expects plain ASCII.
          SDL_Keycode sym = SDL_GetKeyFromScancode(evt.key.scancode, SDL_KMOD_NONE, false);
          SKEY_SET_BIT(keys, scan, 1);
          sKEY->scanCode = (u_int8_t)scan;
          sKEY->charCode = ((sym >= 32) && (sym < 127)) ? (char)sym : 0;
        }
        else
        {
          SKEY_SET_BIT(keys, scan, 0);
          sKEY->scanCode = 0;
          sKEY->charCode = 0;
        }
        break;
      }

      default:
        break;
    }
  }

  // Copy our private key state into the keyboard state structure.
  memcpy(sKEY->rgbButtons, keys, SKEY_SCANTABLESIZE);
  return TRUE;
}

_RLXEXPORTFUNC KEY_ClientDriver *KEY_SystemGetInterface_STD(void)
{
  static KEY_ClientDriver driver =
  {
    KeyboardOpen,
    KeyboardRelease,
    KeyboardNameScanCode,
    KeyboardUpdate
  };
  // Set the driver.
  sKEY = &driver;
  return sKEY;
}
