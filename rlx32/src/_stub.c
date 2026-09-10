// SPDX-FileCopyrightText: 1996-2005 realtech VR
// SPDX-License-Identifier: GPL-2.0-or-later
//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2005 - realtech VR

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
*/
//-------------------------------------------------------------------------
#define STUB_VERSION "1.98"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "_rlx32.h"
#include "_rlx.h"

#include "_stub.h"

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

void STUB_Down(void)
{
 #ifdef _DEBUG
     SYS_Debug("Release input devices..\n");
 #endif
  	if (sKEY)
		sKEY->Release();

	if (sJOY)
		sJOY->Release();

	if (sMOU)
	    sMOU->Release();

 #ifdef _DEBUG
     SYS_Debug("Release audio device..\n");
 #endif
 	V3XA.Client->Release();

 #ifdef _DEBUG
     SYS_Debug("Release 3d engine..\n");
 #endif
	V3XKernel_Release();

 #ifdef _DEBUG
     SYS_Debug("Release 2d engine..\n");
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
	sysInitFS();

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
