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
#ifndef _RLXREGISTRY
#define _RLXREGISTRY

// Run time configuration shared between the game and the rlx32 drivers.

enum {
    RLXVIDEO_Windowed = 0x10
};

enum {
    RLXCTRL_Uncalibrated = 0x4
};

typedef struct {
    u_int8_t  ChannelToMix;
}RLX_RegisterAudio;

typedef struct {
    u_int32_t  Config;
    u_int8_t  Gamma;
}RLX_RegisterVideo;

struct _RClientDriver_Mouse;
struct _RClientDriver_Joystick;
struct _RClientDriver_Keybrd;

typedef struct {
	struct _RClientDriver_Mouse *mouse;
	struct _RClientDriver_Joystick *joystick;
	struct _RClientDriver_Keybrd *keyboard;
}RLX_RegisterController;

typedef struct {
    int32_t MinX, MinY;
    int32_t MaxX, MaxY;
    int32_t MinZ, MaxZ;
    int32_t MinR, MaxR;
}RLX_RegisterJoystick;

typedef struct {
    RLX_RegisterJoystick J[2];
	u_int32_t Config;
}RLX_RegisterJoystickCal;

typedef struct {
    char   *Developper;
}RLX_RegisterDevelopper;

typedef struct {
    char    UserName[16];
}RLX_RegisterApplication;


struct _gx_rgb24;
struct _gx_viewport;

typedef void			(*RLXAPI PFGXSETUPVIEWPORT)(struct _gx_viewport *pView, int width, int height, int bitsPerPixel); // setup viewport size
typedef u_int8_t		*	(*RLXAPI PFRGB_SMARTCONVERTER)(void *dst, struct _gx_rgb24 *dst_pal, int dst_bpp, void *src, const struct _gx_rgb24 *src_pal, int src_bpp, u_int32_t size);
typedef void			(*RLXAPI PFRGB_GETPIXELFORMAT)(struct _gx_rgb24 *rgb, u_int32_t c);
typedef unsigned		(*RLXAPI PFRGB_SETPIXELFORMAT)(int r, int g, int b);

struct _sys_memory;

typedef struct RLXSYSTEM{
    RLX_RegisterAudio       Audio;
    RLX_RegisterVideo       Video;
    RLX_RegisterController  Control;
    RLX_RegisterJoystickCal Joy;
    RLX_RegisterDevelopper  Dev;
    RLX_RegisterApplication App;
    char 					IniPath[256];
	PFGXSETUPVIEWPORT		pfSetViewPort;			// Set Viewport callback
	PFRGB_SMARTCONVERTER	pfSmartConverter;		// Color conversion callback
	PFRGB_GETPIXELFORMAT	pfGetPixelFormat;		// Get Pixel Format callback
	PFRGB_SETPIXELFORMAT	pfSetPixelFormat;		// Set Pixel Format calbback
	struct _sys_memory	*	mm_heap;
	struct GXSYSTEM		*	pGX;
	struct V3XSYSTEM	*	pV3X;
}STUB_Registry;

__extern_c
    _RLXEXPORTDATA extern STUB_Registry RLX;
__end_extern_c

#endif
