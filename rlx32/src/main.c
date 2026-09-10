// SPDX-FileCopyrightText: 1996-2005 realtech VR
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
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "rlx32.h"
#include "stub.h"
#include "systools.h"
#include "sysctrl.h"
#include "sysresmx.h"

#include "gx_struc.h"
#include "gx_init.h"
#include "gx_rgb.h"
#include "iss_defs.h"
#include "v3xdefs.h"
#include "v3xtrig.h"
#include "v3x_2.h"

STUB_Registry RLX = {
	{ 8 },			// Audio: channels to mix
	{ 0, 0 },		// Video: config, gamma
	{ NULL, NULL, NULL },	// Control: drivers, filled in by the SDL layer
	{ { { 0 }, { 0 } }, 0 },	// Joy: calibration
	{ "Realtech" },		// Dev: developer logo name
	{ "" },			// App: pilot name
	"",			// IniPath
};

void STUB_Down(void)
{
 #ifdef _DEBUG
     SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Release input devices..");
 #endif
  	if (sKEY)
		sKEY->Release();

	if (sJOY)
		sJOY->Release();

	if (sMOU)
	    sMOU->Release();

 #ifdef _DEBUG
     SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Release audio device..");
 #endif
 	V3XA.Client->Release();

 #ifdef _DEBUG
     SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Release 3d engine..");
 #endif
	V3XKernel_Release();

 #ifdef _DEBUG
     SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Release 2d engine..");
 #endif

    if (GX.Client)
		GX.Client->Shutdown();

    return;
}

// Bring everything up: memory and file system, input, audio, the 3D
// engine and the display.
void STUB_CheckUp(void)
{
	RLX.mm_heap = &MM_heap;
	RLX.pGX = &GX;
	RLX.pV3X = &V3X;

	sKEY = KEY_SystemGetInterface_STD();
	sKEY->Open(NULL);
	sMOU = MSE_SystemGetInterface_STD();
	sMOU->Open(NULL);
	sJOY = JOY_SystemGetInterface_STD();
	sJOY->Open(NULL, 0);
	RLX.Control.mouse = sMOU;
	RLX.Control.joystick = sJOY;
	RLX.Control.keyboard = sKEY;

	V3XA_EntryPoint(&RLX);
	if (!V3XA.Client->Initialize(NULL))
		V3XA.State |= 1;
	else
		V3XA.State &= ~1;

	GX_KernelAlloc();
	TRG_Generate();
	V3X_EntryPoint(&RLX);
	SYS_ASSERT(V3X.Client);
	V3XKernel_Alloc();

	SYS_ASSERT(GX.Client);
	GX.Client->Open(NULL);
}

extern int g_bSDLQuitRequested;

void STUB_OsStartup(void)
{
	// Per-user settings directory, created by SDL if needed
	// (e.g. ~/.local/share/realtech/nogravity on Linux).
	char *pref = SDL_GetPrefPath("realtech", "nogravity");
	if (pref)
	{
		size_t n = strlen(pref);
		if (n && ((pref[n - 1] == '/') || (pref[n - 1] == '\\')))
			pref[n - 1] = 0;
		snprintf(RLX.IniPath, sizeof(RLX.IniPath), "%s", pref);
		SDL_free(pref);
	}
	else
	{
		snprintf(RLX.IniPath, sizeof(RLX.IniPath), ".");
	}
}

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
