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

#ifndef RLX32_H
#define RLX32_H

#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include "config.h"

#define UNUSED(var)  (void)var

#ifndef FALSE
  enum {
         FALSE,
         TRUE
   };
#endif

#if defined _WIN32 && !defined __CYGWIN__
   typedef uint8_t  u_int8_t;
   typedef uint16_t u_int16_t;
   typedef uint32_t u_int32_t;
   typedef uint64_t u_int64_t;
#else
   #include <sys/types.h>
#endif

typedef struct _gx_sprite
{
    u_int32_t    LX, LY;
    u_int8_t   *data;
    void    *handle;
} GXSPRITE;

typedef struct _gx_rgb24{
    u_int8_t    r, g, b;
} rgb24_t;

typedef struct _gx_rgb32{
    u_int8_t    r, g, b, a;
} rgb32_t;

typedef struct _gx_bgr32
{
    u_int8_t			a, b, g, r;
} bgr32_t;

#ifdef __BIG_ENDIAN__
#define RGBENDIAN bgr32_t
#else
#define RGBENDIAN rgb32_t
#endif

typedef FILE * SYS_FILEHANDLE ; /* I/O file handle. */

#if defined _DEBUG || defined DEBUG
#define SYS_ASSERT(_condition) assert(_condition)
#else
#define SYS_ASSERT(_condition)
#endif

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


struct _sys_memory;

typedef struct RLXSYSTEM{
    RLX_RegisterAudio       Audio;
    RLX_RegisterVideo       Video;
    RLX_RegisterController  Control;
    RLX_RegisterJoystickCal Joy;
    RLX_RegisterDevelopper  Dev;
    RLX_RegisterApplication App;
    char 					IniPath[256];
	struct _sys_memory	*	mm_heap;
	struct GXSYSTEM		*	pGX;
	struct V3XSYSTEM	*	pV3X;
}STUB_Registry;

extern STUB_Registry RLX;

#endif
