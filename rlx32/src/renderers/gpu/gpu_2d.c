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
// 2D primitives and sprites on top of the batching in gpu_display.c.
//
//-------------------------------------------------------------------------

#include <string.h>
#include "gpu_render.h"
#include "v3xdefs.h"
#include "v3xrend.h"

static void SetVertex(GPU_VERTEX *v, float x, float y, float u, float vv, const rgb32_t *cl)
{
	v->x = x; v->y = y; v->z = 0.f;
	v->u = u; v->v = vv; v->q = 1.f;
	v->color[0] = cl->r; v->color[1] = cl->g; v->color[2] = cl->b; v->color[3] = cl->a;
}

static void ColorOf(rgb32_t *cl, u_int32_t colour, u_int8_t alpha)
{
	g_pRLX->pfGetPixelFormat((rgb24_t *)cl, colour);
	cl->a = alpha;
}

// ---- lines, points, rectangles ------------------------------------------

static void Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const rgb32_t *cl)
{
	GPU_STATE st = { GPU_PRIM_LINES, GPU_BLEND_NONE, GPU_DEPTH_OFF, GPU_SAMPLER_CLAMP, NULL };
	GPU_VERTEX *v = GPU_AddVertices(&st, 2);
	SetVertex(v + 0, (float)x1 + 0.5f, (float)y1 + 0.5f, 0.f, 0.f, cl);
	SetVertex(v + 1, (float)x2 + 0.5f, (float)y2 + 0.5f, 0.f, 0.f, cl);
}

static void Quad(float x1, float y1, float x2, float y2, const rgb32_t *cl, int blend)
{
	GPU_STATE st = { GPU_PRIM_TRIANGLES, (u_int8_t)blend, GPU_DEPTH_OFF, GPU_SAMPLER_CLAMP, NULL };
	GPU_VERTEX *v = GPU_AddVertices(&st, 6);
	SetVertex(v + 0, x1, y1, 0.f, 0.f, cl);
	SetVertex(v + 1, x1, y2, 0.f, 0.f, cl);
	SetVertex(v + 2, x2, y2, 0.f, 0.f, cl);
	SetVertex(v + 3, x1, y1, 0.f, 0.f, cl);
	SetVertex(v + 4, x2, y2, 0.f, 0.f, cl);
	SetVertex(v + 5, x2, y1, 0.f, 0.f, cl);
}

static void CALLING_C drawAnyLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, u_int32_t colour)
{
	rgb32_t cl;
	ColorOf(&cl, colour, 255);
	Line(x1, y1, x2, y2, &cl);
}

static void CALLING_C drawHorizontalLine(int32_t x1, int32_t y1, int32_t lx, u_int32_t colour)
{
	rgb32_t cl;
	ColorOf(&cl, colour, 255);
	Line(x1, y1, x1 + lx, y1, &cl);
}

static void CALLING_C drawVerticalLine(int32_t x1, int32_t y1, int32_t ly, u_int32_t colour)
{
	rgb32_t cl;
	ColorOf(&cl, colour, 255);
	Line(x1, y1, x1, y1 + ly, &cl);
}

static void CALLING_C drawWiredRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, u_int32_t colour)
{
	rgb32_t cl;
	ColorOf(&cl, colour, 255);
	Line(x1, y2, x2, y2, &cl);
	Line(x2, y2, x2, y1, &cl);
	Line(x2, y1, x1, y1, &cl);
	Line(x1, y1, x1, y2, &cl);
}

static void CALLING_C drawShadedRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, void *palette)
{
	rgb32_t cl;
	UNUSED(palette);
	ColorOf(&cl, g_pRLX->pGX->csp_cfg.color, (u_int8_t)g_pRLX->pGX->csp_cfg.alpha);
	Quad((float)x1, (float)y1, (float)(x2 + 1), (float)(y2 + 1), &cl, GPU_BLEND_ALPHA);
}

static void CALLING_C drawMeshedRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, u_int32_t colour)
{
	rgb32_t cl;
	ColorOf(&cl, colour, (u_int8_t)g_pRLX->pGX->csp_cfg.alpha);
	Quad((float)x1, (float)y1, (float)(x2 + 1), (float)(y2 + 1), &cl, GPU_BLEND_ALPHA);
}

static void CALLING_C drawFilledRect(int32_t x1, int32_t y1, int32_t x2, int32_t y2, u_int32_t colour)
{
	rgb32_t cl;
	ColorOf(&cl, colour, 255);
	Quad((float)x1, (float)y1, (float)(x2 + 1), (float)(y2 + 1), &cl, GPU_BLEND_NONE);
}

static void CALLING_C drawPixel(int32_t x, int32_t y, u_int32_t colour)
{
	GPU_STATE st = { GPU_PRIM_POINTS, GPU_BLEND_NONE, GPU_DEPTH_OFF, GPU_SAMPLER_CLAMP, NULL };
	rgb32_t cl;
	GPU_VERTEX *v;
	ColorOf(&cl, colour, 255);
	v = GPU_AddVertices(&st, 1);
	SetVertex(v, (float)x + 0.5f, (float)y + 0.5f, 0.f, 0.f, &cl);
}

static void CALLING_C clearBackBuffer(void)
{
	GPU_ClearColor();
}

static void CALLING_C clearVideo(void)
{
	GPU_ClearColor();
}

static void CALLING_C waitDrawing(void)
{
}

static void CALLING_C setPalette(u_int32_t a, u_int32_t b, void *pal)
{
	UNUSED(a); UNUSED(b); UNUSED(pal);
}

static u_int32_t CALLING_C getPixel(int32_t x, int32_t y)
{
	// Reading the frame back is not supported (never used by the game).
	UNUSED(x); UNUSED(y);
	return 0;
}

static void CALLING_C blit(u_int32_t dest, u_int32_t src)
{
	UNUSED(dest); UNUSED(src);
}

static void CALLING_C setCursor(int32_t x, int32_t y) { UNUSED(x); UNUSED(y); }
static void CALLING_C copyCursor(u_int8_t *map) { UNUSED(map); }
static void CALLING_C setGammaRamp(const rgb24_t *ramp) { UNUSED(ramp); }

// ---- sprites --------------------------------------------------------------

// Sprite drawing modes, as the old renderer numbered them.
enum { SPR_OPACITY = 1, SPR_OPAQUE, SPR_ADD, SPR_ALPHA, SPR_SUB };

static void DrawSprite(GXSPRITE *sp, float x, float y, float lx, float ly, int mode)
{
	GPU_TEXTURE *tex = (GPU_TEXTURE *)sp->handle;
	GPU_STATE st = { GPU_PRIM_TRIANGLES, GPU_BLEND_NONE, GPU_DEPTH_OFF, GPU_SAMPLER_CLAMP, NULL };
	rgb32_t cl;
	GPU_VERTEX *v;
	if (!tex)
		return;
	st.texture = tex;
	g_pRLX->pfGetPixelFormat((rgb24_t *)&cl, g_pRLX->pGX->csp_cfg.color);
	cl.a = 255;
	switch (mode)
	{
		case SPR_OPACITY: st.blend = GPU_BLEND_ALPHA; break;
		case SPR_OPAQUE:  st.blend = GPU_BLEND_NONE; break;
		case SPR_ADD:     st.blend = GPU_BLEND_ADD; break;
		case SPR_ALPHA:   st.blend = GPU_BLEND_ALPHA; cl.a = (u_int8_t)g_pRLX->pGX->csp_cfg.alpha; break;
		case SPR_SUB:     st.blend = GPU_BLEND_ADD; break;	// the GL renderer drew "sub" additively too
	}
	v = GPU_AddVertices(&st, 6);
	SetVertex(v + 0, x, y, 0.f, 0.f, &cl);
	SetVertex(v + 1, x, y + ly, 0.f, 1.f, &cl);
	SetVertex(v + 2, x + lx, y + ly, 1.f, 1.f, &cl);
	SetVertex(v + 3, x, y, 0.f, 0.f, &cl);
	SetVertex(v + 4, x + lx, y + ly, 1.f, 1.f, &cl);
	SetVertex(v + 5, x + lx, y, 1.f, 0.f, &cl);
}

static void CALLING_C csp_put(int32_t x, int32_t y, GXSPRITE *sp)   { DrawSprite(sp, (float)x, (float)y, (float)sp->LX, (float)sp->LY, SPR_OPACITY); }
static void CALLING_C csp_pset(int32_t x, int32_t y, GXSPRITE *sp)  { DrawSprite(sp, (float)x, (float)y, (float)sp->LX, (float)sp->LY, SPR_OPAQUE); }
static void CALLING_C csp_add(int32_t x, int32_t y, GXSPRITE *sp)   { DrawSprite(sp, (float)x, (float)y, (float)sp->LX, (float)sp->LY, SPR_ADD); }
static void CALLING_C csp_50(int32_t x, int32_t y, GXSPRITE *sp)    { DrawSprite(sp, (float)x, (float)y, (float)sp->LX, (float)sp->LY, SPR_ALPHA); }
static void CALLING_C csp_sub(int32_t x, int32_t y, GXSPRITE *sp)   { DrawSprite(sp, (float)x, (float)y, (float)sp->LX, (float)sp->LY, SPR_SUB); }
static void CALLING_C csp_alpha(int32_t x, int32_t y, GXSPRITE *sp) { DrawSprite(sp, (float)x, (float)y, (float)sp->LX, (float)sp->LY, SPR_ALPHA); }

static void CALLING_C csp_put_zoom(GXSPRITE *sp, int32_t x, int32_t y, int32_t lx, int32_t ly)   { DrawSprite(sp, (float)x, (float)y, (float)lx, (float)ly, SPR_OPACITY); }
static void CALLING_C csp_pset_zoom(GXSPRITE *sp, int32_t x, int32_t y, int32_t lx, int32_t ly)  { DrawSprite(sp, (float)x, (float)y, (float)lx, (float)ly, SPR_OPAQUE); }
static void CALLING_C csp_add_zoom(GXSPRITE *sp, int32_t x, int32_t y, int32_t lx, int32_t ly)   { DrawSprite(sp, (float)x, (float)y, (float)lx, (float)ly, SPR_ADD); }
static void CALLING_C csp_50_zoom(GXSPRITE *sp, int32_t x, int32_t y, int32_t lx, int32_t ly)    { DrawSprite(sp, (float)x, (float)y, (float)lx, (float)ly, SPR_ALPHA); }
static void CALLING_C csp_sub_zoom(GXSPRITE *sp, int32_t x, int32_t y, int32_t lx, int32_t ly)   { DrawSprite(sp, (float)x, (float)y, (float)lx, (float)ly, SPR_SUB); }
static void CALLING_C csp_alpha_zoom(GXSPRITE *sp, int32_t x, int32_t y, int32_t lx, int32_t ly) { DrawSprite(sp, (float)x, (float)y, (float)lx, (float)ly, SPR_ALPHA); }

// Upload a sprite: 8 bit indexed with palette (index 0 transparent), or
// 24 bit RGB.
static void RLXAPI GPU_UploadSprite(GXSPRITE *sp, rgb24_t *colorTable, int bpp)
{
	u_int8_t *pixels = NULL;
	SDL_GPUTextureFormat fmt = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	int n = (int)(sp->LX * sp->LY);

	sp->handle = NULL;
	if (!sp->data || !GPU_IsReady())
		return;
	if (bpp == 1)
		pixels = GPU_Expand8(sp->data, colorTable, n, 1, &fmt);
	else if (bpp == 3)
		pixels = GPU_Expand24(sp->data, n, fmt);
	else if (bpp == 4)
		fmt = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
	sp->handle = GPU_CreateTexture((int)sp->LX, (int)sp->LY, fmt, 0, pixels ? pixels : sp->data);
	SDL_free(pixels);
}

static void RLXAPI GPU_ReleaseSprite(GXSPRITE *sp)
{
	if (sp->data)
	{
		g_pRLX->mm_heap->free(sp->data);
		sp->data = NULL;
	}
	if (sp->handle)
	{
		GPU_DestroyTexture((GPU_TEXTURE *)sp->handle);
		sp->handle = NULL;
	}
}

static unsigned RLXAPI GPU_UpdateSprite(GXSPRITE *sp, const u_int8_t *bitmap, const rgb24_t *colorTable)
{
	GPU_TEXTURE *tex = (GPU_TEXTURE *)sp->handle;
	if (tex)
	{
		SDL_GPUTextureFormat fmt;
		u_int8_t *pixels = GPU_Expand8(bitmap, colorTable, (int)(sp->LX * sp->LY), 1, &fmt);
		GPU_UpdateTexture(tex, pixels);
		SDL_free(pixels);
	}
	return 0;
}

GXGRAPHICINTERFACE GI_GPU =
{
	drawAnyLine,
	drawAnyLine,
	drawHorizontalLine,
	drawVerticalLine,
	drawWiredRect,
	drawShadedRect,
	drawMeshedRect,
	drawFilledRect,
	drawPixel,
	drawPixel,
	getPixel,
	clearBackBuffer,
	clearVideo,
	blit,
	waitDrawing,
	setPalette,
	setCursor,
	copyCursor,
	setGammaRamp
};

GXSPRITEINTERFACE CSP_GPU =
{
	0,
	csp_put,
	csp_pset,
	csp_pset,
	csp_50,
	csp_add,
	csp_sub,
	csp_alpha,
	csp_pset_zoom,
	csp_put_zoom,
	csp_50_zoom,
	csp_add_zoom,
	csp_sub_zoom,
	csp_alpha_zoom
};

extern GXCLIENTDRIVER GX_GPU;

void GPU_SetPrimitiveSprites(void)
{
	GX_GPU.UploadSprite = GPU_UploadSprite;
	GX_GPU.UpdateSprite = GPU_UpdateSprite;
	GX_GPU.ReleaseSprite = GPU_ReleaseSprite;
}
