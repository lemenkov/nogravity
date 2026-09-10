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

#ifndef __RLX32_H
#define __RLX32_H

#include <assert.h>
#include <SDL3/SDL.h>
/* Compiler setup ***********************************************************/

#include "config.h"
#include <sys/types.h>

/* C/C++ Interface **********************************************************/

#ifndef __extern_c

  #ifdef __cplusplus
     #define __extern_c            extern "C" {
     #define __end_extern_c        }
  #else
     #define __extern_c
     #define __end_extern_c
  #endif
#endif

  #define UNUSED(var)  (void)var

/* Dll export  **************************************************************/

  #define _RLXEXPORTDATA
  #define _RLXEXPORTFUNC

/* Functions register conventions *******************************************/

#if defined _MSC_VER
    // MS Visual C
    #define CALLING_C
	#define CALLING_STD __stdcall
    #define RLXAPI

#else
    // Others compiler
    #define CALLING_C
	#define CALLING_STD
    #define RLXAPI
#endif

/* Filename and stream conventions ******************************************/

#ifndef DUMMYUNIONNAMEN
  #if defined(__cplusplus) || !defined(NONAMELESSUNION)
	#define DUMMYUNIONNAMEN(n)
  #else
	#define DUMMYUNIONNAMEN(n)      u##n
  #endif
#endif

/* Low level structures *****************************************************/

#ifndef _WIN32
   typedef unsigned BOOLEAN;
#endif

#ifndef FALSE
  enum {
         FALSE,
         TRUE
   };
#endif


/* Data Types */
#include <stdint.h>
#include <stddef.h>
#if defined _WIN32 && !defined __CYGWIN__
   /* BSD style unsigned names used throughout the engine. */
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


/* Thread, and I/O handle *******************************************************/

#include <stdio.h>

typedef FILE * SYS_FILEHANDLE ; /* I/O file handle. */

/* Error/Debug/Trace functions *****************************************************/

#if defined _DEBUG || defined DEBUG
#define SYS_ASSERT(_condition) assert(_condition)
#else
#define SYS_ASSERT(_condition)
#endif


__extern_c
_RLXEXPORTFUNC int      RLXAPI   RLX_ErrorGetCodeString(int error_code);
__end_extern_c

#endif
