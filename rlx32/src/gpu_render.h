// SPDX-FileCopyrightText: 1996-2005 realtech VR
// SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later
//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2005 - realtech VR

This file is part of No Gravity

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
SDL_GPU renderer: 2026 - Peter Lemenkov
*/
//-------------------------------------------------------------------------
//
// SDL_GPU renderer, internal interface.
//
// The engine draws in an immediate style: individual lines, rectangles,
// sprites and per-face triangles with state changes in between.  All of
// it is recorded into one vertex array and a small command list per
// frame, and replayed into a single SDL_GPU command buffer at Flip().
//
//-------------------------------------------------------------------------
#ifndef __GPU_RENDER_H
#define __GPU_RENDER_H

#include <SDL3/SDL.h>
#include "_rlx32.h"
#include "_rlx.h"
#include "gx_struc.h"
#include "gx_tools.h"
#include "systools.h"
#include "gx_csp.h"
#include "gx_rgb.h"
#include "sysctrl.h"
#include "v3xdefs.h"
#include "v3xrend.h"

// One vertex: projected position (logical pixels, depth 0..1),
// homogeneous texture coordinates and colour.
typedef struct
{
	float x, y, z;
	float u, v, q;
	u_int8_t color[4]; // r, g, b, a
} GPU_VERTEX;

enum { GPU_PRIM_TRIANGLES, GPU_PRIM_LINES, GPU_PRIM_POINTS, GPU_PRIM_COUNT };
enum { GPU_BLEND_NONE, GPU_BLEND_ALPHA, GPU_BLEND_ADD, GPU_BLEND_SUB, GPU_BLEND_COUNT };
enum { GPU_DEPTH_OFF, GPU_DEPTH_READWRITE, GPU_DEPTH_READONLY, GPU_DEPTH_COUNT };
enum { GPU_SAMPLER_CLAMP, GPU_SAMPLER_REPEAT, GPU_SAMPLER_REPEAT_MIPMAP, GPU_SAMPLER_COUNT };

// Everything the driver keeps about a texture.  Sprites and 3D textures
// share it; sp->handle / material texture handles point to one of these.
typedef struct
{
	SDL_GPUTexture	*texture;
	SDL_GPUTextureFormat format;
	int		width, height;
	int		levels;		// mip levels
	u_int8_t	*tmpbuf;	// staging copy for dynamic (modifiable) textures
} GPU_TEXTURE;

// Draw state for a batch.
typedef struct
{
	u_int8_t	prim;
	u_int8_t	blend;
	u_int8_t	depth;
	u_int8_t	sampler;
	GPU_TEXTURE	*texture;	// NULL: untextured (white)
} GPU_STATE;

__extern_c

extern struct RLXSYSTEM *g_pRLX;

// gpu_display.c
int		GPU_IsReady(void);
GPU_VERTEX	*GPU_AddVertices(const GPU_STATE *state, int count);
void		GPU_ClearColor(void);
void		GPU_ClearDepth(void);
GPU_TEXTURE	*GPU_CreateTexture(int w, int h, SDL_GPUTextureFormat format, int mipmaps, const void *pixels);
void		GPU_UpdateTexture(GPU_TEXTURE *tex, const void *pixels);
void		GPU_DestroyTexture(GPU_TEXTURE *tex);

// Pixel conversion helpers (gpu_display.c).  Output byte order matches
// what the old OpenGL renderer fed to GL_BGRA / GL_RGB, so the textures
// look the same; the matching SDL_GPU format is returned.
u_int8_t	*GPU_Expand8(const u_int8_t *src, const rgb24_t *pal, int n, int colorkey, SDL_GPUTextureFormat *fmt);
u_int8_t	*GPU_Expand24(const u_int8_t *src, int n, SDL_GPUTextureFormat fmt);

// gpu_2d.c
void		GPU_SetPrimitiveSprites(void);
extern GXGRAPHICINTERFACE	GI_GPU;
extern GXSPRITEINTERFACE	CSP_GPU;

// gpu_3d.c
extern V3X_GXSystem		V3X_GPU;

__end_extern_c

#endif
