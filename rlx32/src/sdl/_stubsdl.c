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
#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "_rlx32.h"
#include "_rlx.h"
#include "_stub.h"

extern int g_bSDLQuitRequested;

int STUB_TaskControl(void)
{
  // Non-zero makes the game loops bail out: the window was closed.
  return g_bSDLQuitRequested;
}

int main(int argc, char *argv[])
{
  UNUSED(argc);
  UNUSED(argv);

  SDL_SetAppMetadata("No Gravity", "3.0.0", "com.realtech-vr.nogravity");
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
  {
    fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    return 1;
  }
  atexit(SDL_Quit);

  // Standard main function.
  STUB_OsStartup();
  STUB_Default();
  STUB_CheckUp();
  STUB_ReadyToRun();
  STUB_MainCode();
  STUB_Down();
  STUB_QuitRequested();

  return 0;
}
