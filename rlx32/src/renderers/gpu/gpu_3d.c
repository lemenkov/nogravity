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
// 3D polygon list.  The engine transforms, lights, clips and sorts the
// polygons itself; what arrives here are screen space faces with a
// material, so the work is to turn them into batched triangles.
//
//-------------------------------------------------------------------------

#include <string.h>
#include "gpu_render.h"
#include "v3xdefs.h"
#include "v3xrend.h"
#include "sysctrl.h"

static float g_fInvZFar = 1.f;

static unsigned V3XAPI ZbufferClear(rgb24_t *color, V3XSCALAR z, void *bitmap)
{
	UNUSED(color); UNUSED(z); UNUSED(bitmap);
	return 0;
}

static unsigned V3XAPI SetState(unsigned command, u_int32_t value)
{
	switch (command)
	{
		case V3XCMD_SETBACKGROUNDCOLOR:
		{
			rgb24_t *cl = (rgb24_t *)(uintptr_t)value;
			g_pRLX->pV3X->ViewPort.backgroundColor = RGB_Make32bit(cl->r, cl->g, cl->b, 255);
		}
		return 1;
		case V3XCMD_SETZBUFFERSTATE:
			if (value) g_pRLX->pV3X->Client->Capabilities |= GXSPEC_ENABLEZBUFFER;
			else g_pRLX->pV3X->Client->Capabilities &= ~GXSPEC_ENABLEZBUFFER;
			return 1;
		case V3XCMD_SETZBUFFERCOMP:
			return 1;
	}
	return 0;
}

// ---- textures -------------------------------------------------------------

static void V3XAPI *UploadTexture(const GXSPRITE *sp, const rgb24_t *colorTable, int bpp, unsigned options)
{
	GPU_TEXTURE *tex;
	u_int8_t *pixels = NULL;
	SDL_GPUTextureFormat fmt = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
	int n = (int)(sp->LX * sp->LY);
	int dynamic = (options & V3XTEXDWNOPTION_DYNAMIC) != 0;

	SYS_ASSERT(sp->data);
	if (!sp->data || !GPU_IsReady())
		return NULL;
	if (bpp <= 8)
		pixels = GPU_Expand8(sp->data, colorTable, n, (options & V3XTEXDWNOPTION_COLORKEY) != 0, &fmt);
	else if (bpp == 24)
		pixels = GPU_Expand24(sp->data, n, fmt);
	// 32 bit: raw data, read as BGRA like the OpenGL path did.

	tex = GPU_CreateTexture((int)sp->LX, (int)sp->LY, fmt, !dynamic, pixels ? pixels : sp->data);
	if (tex && dynamic)
	{
		// Keep a staging copy so TextureModify() can rewrite it.
		tex->tmpbuf = (u_int8_t *)SDL_malloc((size_t)n * 4);
		memcpy(tex->tmpbuf, pixels ? pixels : sp->data, (size_t)n * 4);
	}
	SDL_free(pixels);
	return tex;
}

static void V3XAPI FreeTexture(void *handle)
{
	GPU_DestroyTexture((GPU_TEXTURE *)handle);
}

static int V3XAPI TextureModify(GXSPRITE *sp, u_int8_t *bitmap, const rgb24_t *colorTable)
{
	GPU_TEXTURE *tex = (GPU_TEXTURE *)sp->handle;
	SDL_GPUTextureFormat fmt;
	u_int8_t *pixels;
	if (!tex || !tex->tmpbuf)
		return -1;
	pixels = GPU_Expand8(bitmap, colorTable, (int)(sp->LX * sp->LY), 0, &fmt);
	memcpy(tex->tmpbuf, pixels, (size_t)sp->LX * sp->LY * 4);
	GPU_UpdateTexture(tex, tex->tmpbuf);
	SDL_free(pixels);
	return 1;
}

// ---- polygons -------------------------------------------------------------

#define xMUL8(a, b) (((unsigned)(a) * (unsigned)(b)) >> 8)
#define MIN255(x) ((x) > 255u ? 255u : (x))

static const V3XMATERIAL *g_pMat;
static GPU_STATE g_State;
static u_int8_t g_FlatColor[4];

// Lit vertex colour: ambient/4 + light * diffuse, like the GL renderer.
static void LitColor(const V3XPOLY *fce, int i, u_int8_t *out)
{
	out[0] = (u_int8_t)MIN255((g_pMat->ambient.r >> 2) + xMUL8(fce->rgb[i].r, g_pMat->diffuse.r));
	out[1] = (u_int8_t)MIN255((g_pMat->ambient.g >> 2) + xMUL8(fce->rgb[i].g, g_pMat->diffuse.g));
	out[2] = (u_int8_t)MIN255((g_pMat->ambient.b >> 2) + xMUL8(fce->rgb[i].b, g_pMat->diffuse.b));
	out[3] = fce->rgb[i].a;
}

static void ChangeMaterial(const V3XMATERIAL *pMat)
{
	int depth_enabled = (g_pRLX->pV3X->Client->Capabilities & (GXSPEC_ENABLEZBUFFER | GXSPEC_ENABLEWBUFFER)) != 0;
	g_pMat = pMat;

	g_State.prim = (pMat->Render == V3XRCLASS_wired) ? GPU_PRIM_LINES : GPU_PRIM_TRIANGLES;
	switch (pMat->info.Transparency)
	{
		case V3XBLENDMODE_SUB:   g_State.blend = GPU_BLEND_SUB; break;
		case V3XBLENDMODE_ADD:   g_State.blend = GPU_BLEND_ADD; break;
		case V3XBLENDMODE_ALPHA: g_State.blend = GPU_BLEND_ALPHA; break;
		default:                 g_State.blend = GPU_BLEND_NONE; break;
	}
	if (!depth_enabled)
		g_State.depth = GPU_DEPTH_OFF;
	else
		g_State.depth = pMat->info.Transparency ? GPU_DEPTH_READONLY : GPU_DEPTH_READWRITE;

	g_State.texture = pMat->info.Texturized ? (GPU_TEXTURE *)pMat->texture[0].handle : NULL;
	g_State.sampler = (g_pRLX->pV3X->Client->Capabilities & GXSPEC_ENABLEFILTERING) ? GPU_SAMPLER_REPEAT_MIPMAP : GPU_SAMPLER_REPEAT;

	// Constant colour for unshaded materials.  The GL renderer passed the
	// diffuse as (r, b, g); kept for identical looks.
	if (pMat->info.Shade == 0)
	{
		g_FlatColor[0] = pMat->diffuse.r;
		g_FlatColor[1] = pMat->diffuse.b;
		g_FlatColor[2] = pMat->diffuse.g;
		g_FlatColor[3] = pMat->alpha;
	}
}

static void FillVertex(GPU_VERTEX *v, const V3XPOLY *fce, int i, int textured, int perspective, int smooth)
{
	v->x = fce->dispTab[i].x;
	v->y = fce->dispTab[i].y;
	v->z = fce->dispTab[i].z * g_fInvZFar;
	if (textured)
	{
		if (perspective)
		{
			v->u = fce->ZTab[i].uow;
			v->v = fce->ZTab[i].vow;
			v->q = fce->ZTab[i].oow;
		}
		else
		{
			v->u = fce->uvTab[0][i].u;
			v->v = fce->uvTab[0][i].v;
			v->q = 1.f;
		}
	}
	else
	{
		v->u = v->v = 0.f;
		v->q = 1.f;
	}
	if (smooth)
		LitColor(fce, i, v->color);
	else
		memcpy(v->color, g_FlatColor, 4);
}

static void V3XAPI RenderPoly(V3XPOLY **fe, int count)
{
	g_pMat = NULL;
	for (; count != 0; fe++, count--)
	{
		V3XPOLY *fce = *fe;
		const V3XMATERIAL *pMat = (const V3XMATERIAL *)fce->Mat;
		int textured, perspective, smooth, i;

		if (pMat->Render == 255)
		{
			// Software helper (wireframe, points, 2D sprites); it draws
			// through the 2D interface.
			SYS_ASSERT(pMat->render_clip);
			pMat->render_clip(fce);
			continue;
		}
		if (pMat != g_pMat)
			ChangeMaterial(pMat);
		if (pMat->RenderID == V3XRCLASS_shadow)
			continue;

		textured = pMat->info.Texturized && g_State.texture;
		perspective = textured && pMat->info.Perspective;
		smooth = pMat->info.Shade > 1;
		if (pMat->info.Shade == 1)
			LitColor(fce, 0, g_FlatColor);	// flat: first vertex lights the face

		if (g_State.prim == GPU_PRIM_LINES)
		{
			GPU_VERTEX *v = GPU_AddVertices(&g_State, fce->numEdges * 2);
			for (i = 0; i < fce->numEdges; i++)
			{
				FillVertex(v++, fce, i, textured, perspective, smooth);
				FillVertex(v++, fce, (i + 1) % fce->numEdges, textured, perspective, smooth);
			}
		}
		else if (fce->numEdges >= 3)
		{
			GPU_VERTEX *v = GPU_AddVertices(&g_State, (fce->numEdges - 2) * 3);
			for (i = 1; i < fce->numEdges - 1; i++)
			{
				FillVertex(v++, fce, 0, textured, perspective, smooth);
				FillVertex(v++, fce, i, textured, perspective, smooth);
				FillVertex(v++, fce, i + 1, textured, perspective, smooth);
			}
		}
	}
}

static void V3XAPI StartList(void)
{
	g_fInvZFar = 1.f / g_pRLX->pV3X->Clip.Far;
	if (g_pRLX->pV3X->Client->Capabilities & GXSPEC_ENABLEZBUFFER)
		GPU_ClearDepth();
	g_pRLX->pGX->View.State |= GX_STATE_SCENEBEGUN;
}

static void V3XAPI EndList(void)
{
	g_pRLX->pGX->View.State &= ~GX_STATE_SCENEBEGUN;
}

static void V3XAPI RenderDisplay(void)
{
	int n = (int)g_pRLX->pV3X->Buffer.MaxFaces;
	V3XPOLY **f = g_pRLX->pV3X->Buffer.RenderedFaces;
	if (n)
		RenderPoly(f, n);
}

static void V3XAPI DrawPrimitives(V3XVECTOR *vertexes, u_int16_t *indexTab, unsigned NumIndexes, unsigned NumVertexes, int option, rgb32_t *color)
{
	// Disabled in the OpenGL renderer as well.
	UNUSED(vertexes); UNUSED(indexTab); UNUSED(NumIndexes); UNUSED(NumVertexes); UNUSED(option); UNUSED(color);
}

V3X_GXSystem V3X_GPU =
{
	NULL,
	RenderDisplay,
	UploadTexture,
	FreeTexture,
	TextureModify,
	SetState,
	ZbufferClear,
	RenderPoly,
	StartList,
	EndList,
	DrawPrimitives,
	"SDL_GPU",
	256 + 10,
	GXSPEC_ZBUFFER |
	GXSPEC_OPACITYTRANSPARENT |
	GXSPEC_SPRITEAREPOLY |
	GXSPEC_UVNORMALIZED |
	GXSPEC_FULLHWSPRITE |
	GXSPEC_RESIZABLEMAP |
	GXSPEC_ALPHABLENDING_ADD |
	GXSPEC_HARDWAREBLENDING |
	GXSPEC_ANTIALIASEDLINE |
	GXSPEC_ENABLEFILTERING |
	GXSPEC_HARDWARE |
	GXSPEC_ENABLEPERSPECTIVE |
	GXSPEC_NONPOWOF2,
	0x000000,
	1,
	12,
	{0, 0}
};
