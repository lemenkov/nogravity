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
// Display driver: window, GPU device, per-frame batching and submission,
// texture uploads.
//
//-------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

#include "gpu_render.h"
#include "gx_init.h"
#include "v3xdefs.h"
#include "v3x_2.h"
#include "v3xrend.h"

struct RLXSYSTEM *g_pRLX;

#define ISFULLSCREEN() (!(g_pRLX->Video.Config&RLXVIDEO_Windowed))

// Embedded shaders (generated from shaders/ by tools/embed.py).
extern const unsigned char nogravity_metal[];
extern const unsigned int  nogravity_metal_len;
#ifdef HAVE_SPIRV
extern const unsigned char nogravity_vert_spv[];
extern const unsigned int  nogravity_vert_spv_len;
extern const unsigned char nogravity_frag_spv[];
extern const unsigned int  nogravity_frag_spv_len;
#endif

// The window is shared with the mouse driver.
SDL_Window *g_pSDLWindow = NULL;

static SDL_GPUDevice		*g_Device = NULL;
static SDL_GPUShader		*g_VertexShader = NULL;
static SDL_GPUShader		*g_FragmentShader = NULL;
static SDL_GPUGraphicsPipeline	*g_Pipelines[GPU_PRIM_COUNT][GPU_BLEND_COUNT][GPU_DEPTH_COUNT];
static SDL_GPUSampler		*g_Samplers[GPU_SAMPLER_COUNT];
static SDL_GPUTextureFormat	g_DepthFormat;
static SDL_GPUTextureFormat	g_ColorFormat = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
static SDL_GPUTexture		*g_ColorTarget = NULL;	// the frame is rendered here, then blitted to the swapchain
static SDL_GPUTexture		*g_DepthTarget = NULL;
static Uint32			g_TargetW = 0, g_TargetH = 0;
static GPU_TEXTURE		*g_WhiteTexture = NULL;
static SDL_GPUBuffer		*g_VertexBuffer = NULL;
static SDL_GPUTransferBuffer	*g_VertexTransfer = NULL;
static Uint32			g_VertexBufferSize = 0;
static int			g_VSync = -1;

static GXDISPLAYMODEINFO	*g_pDisplays = NULL;
static int			g_Mode = -1;

// ---- per frame recording ------------------------------------------------

enum { CMD_DRAW, CMD_PASS };

typedef struct
{
	int		type;
	GPU_STATE	state;		// CMD_DRAW
	int		first, count;	// CMD_DRAW: vertex range
	int		clear_color;	// CMD_PASS
	int		clear_depth;	// CMD_PASS
} GPU_COMMAND;

static GPU_VERTEX	*g_Vertices = NULL;
static int		g_nVertices = 0, g_maxVertices = 0;
static GPU_COMMAND	*g_Commands = NULL;
static int		g_nCommands = 0, g_maxCommands = 0;

// Textures released during a frame stay alive until the frame has been
// submitted, since recorded draws may still reference them.
static GPU_TEXTURE	**g_Garbage = NULL;
static int		g_nGarbage = 0, g_maxGarbage = 0;

// Screenshot request: NOGRAVITY_SCREENSHOT=file.png writes frame
// NOGRAVITY_SCREENSHOT_FRAME (default 120); with a %d in the name a
// screenshot is written every that many frames instead.
static const char	*g_ScreenshotPath = NULL;
static int		g_ScreenshotFrame = 120;
static int		g_FrameCount = 0;

int GPU_IsReady(void)
{
	return g_Device != NULL;
}

static int SameState(const GPU_STATE *a, const GPU_STATE *b)
{
	return (a->prim == b->prim) && (a->blend == b->blend) && (a->depth == b->depth) &&
	       (a->sampler == b->sampler) && (a->texture == b->texture);
}

// Reserve `count` vertices drawn with `state`; merges into the previous
// draw when the state matches.
GPU_VERTEX *GPU_AddVertices(const GPU_STATE *state, int count)
{
	GPU_COMMAND *cmd;
	GPU_VERTEX *v;

	if (g_nVertices + count > g_maxVertices)
	{
		int n = g_maxVertices ? g_maxVertices * 2 : 16384;
		while (n < g_nVertices + count) n *= 2;
		g_Vertices = (GPU_VERTEX *)SDL_realloc(g_Vertices, (size_t)n * sizeof(GPU_VERTEX));
		g_maxVertices = n;
	}
	cmd = g_nCommands ? &g_Commands[g_nCommands - 1] : NULL;
	if (!cmd || (cmd->type != CMD_DRAW) || !SameState(&cmd->state, state) ||
	    (cmd->first + cmd->count != g_nVertices))
	{
		if (g_nCommands == g_maxCommands)
		{
			g_maxCommands = g_maxCommands ? g_maxCommands * 2 : 1024;
			g_Commands = (GPU_COMMAND *)SDL_realloc(g_Commands, (size_t)g_maxCommands * sizeof(GPU_COMMAND));
		}
		cmd = &g_Commands[g_nCommands++];
		memset(cmd, 0, sizeof(*cmd));
		cmd->type = CMD_DRAW;
		cmd->state = *state;
		cmd->first = g_nVertices;
		cmd->count = 0;
	}
	v = g_Vertices + g_nVertices;
	g_nVertices += count;
	cmd->count += count;
	return v;
}

static void AddPass(int clear_color, int clear_depth)
{
	GPU_COMMAND *cmd;
	if (g_nCommands == g_maxCommands)
	{
		g_maxCommands = g_maxCommands ? g_maxCommands * 2 : 1024;
		g_Commands = (GPU_COMMAND *)SDL_realloc(g_Commands, (size_t)g_maxCommands * sizeof(GPU_COMMAND));
	}
	cmd = &g_Commands[g_nCommands++];
	memset(cmd, 0, sizeof(*cmd));
	cmd->type = CMD_PASS;
	cmd->clear_color = clear_color;
	cmd->clear_depth = clear_depth;
}

void GPU_ClearColor(void)
{
	AddPass(1, 0);
}

void GPU_ClearDepth(void)
{
	AddPass(0, 1);
}

// ---- pixel conversion ---------------------------------------------------

// 8 bit indexed to 32 bit.  Bytes come out as r,g,b,a like the old
// converter produced for the OpenGL path, which read them as BGRA.
u_int8_t *GPU_Expand8(const u_int8_t *src, const rgb24_t *pal, int n, int colorkey, SDL_GPUTextureFormat *fmt)
{
	u_int8_t *dst = (u_int8_t *)SDL_malloc((size_t)n * 4);
	u_int8_t *d = dst;
	int i;
	for (i = 0; i < n; i++, src++, d += 4)
	{
		const rgb24_t *p = pal + *src;
		d[0] = p->r;
		d[1] = p->g;
		d[2] = p->b;
		d[3] = (colorkey && (*src == 0)) ? 0 : 255;
	}
	*fmt = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;
	return dst;
}

// 24 bit triplets to 32 bit, keeping the byte order; the caller says how
// OpenGL used to read them (GL_RGB -> RGBA8, GL_BGR -> BGRA8).
u_int8_t *GPU_Expand24(const u_int8_t *src, int n, SDL_GPUTextureFormat fmt)
{
	u_int8_t *dst = (u_int8_t *)SDL_malloc((size_t)n * 4);
	u_int8_t *d = dst;
	int i;
	UNUSED(fmt);
	for (i = 0; i < n; i++, src += 3, d += 4)
	{
		d[0] = src[0];
		d[1] = src[1];
		d[2] = src[2];
		d[3] = 255;
	}
	return dst;
}

// ---- textures -----------------------------------------------------------

static int MipLevels(int w, int h)
{
	int n = 1;
	while ((w > 1) || (h > 1))
	{
		w = (w > 1) ? w / 2 : 1;
		h = (h > 1) ? h / 2 : 1;
		n++;
	}
	return n;
}

static void UploadPixels(GPU_TEXTURE *tex, const void *pixels)
{
	SDL_GPUCommandBuffer *cmd;
	SDL_GPUCopyPass *copy;
	SDL_GPUTransferBuffer *tb;
	SDL_GPUTransferBufferCreateInfo tbinfo;
	SDL_GPUTextureTransferInfo src;
	SDL_GPUTextureRegion dst;
	void *map;
	Uint32 size = (Uint32)tex->width * (Uint32)tex->height * 4;

	memset(&tbinfo, 0, sizeof(tbinfo));
	tbinfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	tbinfo.size = size;
	tb = SDL_CreateGPUTransferBuffer(g_Device, &tbinfo);
	if (!tb)
		return;
	map = SDL_MapGPUTransferBuffer(g_Device, tb, false);
	if (!map)
	{
		SDL_ReleaseGPUTransferBuffer(g_Device, tb);
		return;
	}
	memcpy(map, pixels, size);
	SDL_UnmapGPUTransferBuffer(g_Device, tb);

	memset(&src, 0, sizeof(src));
	src.transfer_buffer = tb;
	memset(&dst, 0, sizeof(dst));
	dst.texture = tex->texture;
	dst.w = (Uint32)tex->width;
	dst.h = (Uint32)tex->height;
	dst.d = 1;

	cmd = SDL_AcquireGPUCommandBuffer(g_Device);
	copy = SDL_BeginGPUCopyPass(cmd);
	SDL_UploadToGPUTexture(copy, &src, &dst, false);
	SDL_EndGPUCopyPass(copy);
	if (tex->levels > 1)
		SDL_GenerateMipmapsForGPUTexture(cmd, tex->texture);
	SDL_SubmitGPUCommandBuffer(cmd);
	SDL_ReleaseGPUTransferBuffer(g_Device, tb);
}

GPU_TEXTURE *GPU_CreateTexture(int w, int h, SDL_GPUTextureFormat format, int mipmaps, const void *pixels)
{
	GPU_TEXTURE *tex;
	SDL_GPUTextureCreateInfo info;

	if (!g_Device || (w <= 0) || (h <= 0))
		return NULL;
	tex = (GPU_TEXTURE *)SDL_calloc(1, sizeof(GPU_TEXTURE));
	tex->width = w;
	tex->height = h;
	tex->format = format;
	tex->levels = mipmaps ? MipLevels(w, h) : 1;

	memset(&info, 0, sizeof(info));
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.format = format;
	info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | ((tex->levels > 1) ? SDL_GPU_TEXTUREUSAGE_COLOR_TARGET : 0);
	info.width = (Uint32)w;
	info.height = (Uint32)h;
	info.layer_count_or_depth = 1;
	info.num_levels = (Uint32)tex->levels;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	tex->texture = SDL_CreateGPUTexture(g_Device, &info);
	if (!tex->texture)
	{
		SDL_Log("Video: cannot create a %dx%d texture: %s", w, h, SDL_GetError());
		SDL_free(tex);
		return NULL;
	}
	if (pixels)
		UploadPixels(tex, pixels);
	return tex;
}

void GPU_UpdateTexture(GPU_TEXTURE *tex, const void *pixels)
{
	if (tex && tex->texture && pixels)
		UploadPixels(tex, pixels);
}

static void FreeTexture(GPU_TEXTURE *tex)
{
	if (tex->texture && g_Device)
		SDL_ReleaseGPUTexture(g_Device, tex->texture);
	SDL_free(tex->tmpbuf);
	SDL_free(tex);
}

static void FlushGarbage(void)
{
	int i;
	for (i = 0; i < g_nGarbage; i++)
		FreeTexture(g_Garbage[i]);
	g_nGarbage = 0;
}

void GPU_DestroyTexture(GPU_TEXTURE *tex)
{
	if (!tex)
		return;
	if (g_nGarbage == g_maxGarbage)
	{
		g_maxGarbage = g_maxGarbage ? g_maxGarbage * 2 : 256;
		g_Garbage = (GPU_TEXTURE **)SDL_realloc(g_Garbage, (size_t)g_maxGarbage * sizeof(GPU_TEXTURE *));
	}
	g_Garbage[g_nGarbage++] = tex;
}

// ---- device -------------------------------------------------------------

static SDL_GPUShader *CreateShader(SDL_GPUShaderStage stage, SDL_GPUShaderFormat formats)
{
	SDL_GPUShaderCreateInfo info;
	memset(&info, 0, sizeof(info));
	info.stage = stage;
	info.num_samplers = (stage == SDL_GPU_SHADERSTAGE_FRAGMENT) ? 1 : 0;
	info.num_uniform_buffers = (stage == SDL_GPU_SHADERSTAGE_VERTEX) ? 1 : 0;
#ifdef HAVE_SPIRV
	if (formats & SDL_GPU_SHADERFORMAT_SPIRV)
	{
		info.format = SDL_GPU_SHADERFORMAT_SPIRV;
		info.entrypoint = "main";
		info.code = (stage == SDL_GPU_SHADERSTAGE_VERTEX) ? nogravity_vert_spv : nogravity_frag_spv;
		info.code_size = (stage == SDL_GPU_SHADERSTAGE_VERTEX) ? nogravity_vert_spv_len : nogravity_frag_spv_len;
		return SDL_CreateGPUShader(g_Device, &info);
	}
#endif
	if (formats & SDL_GPU_SHADERFORMAT_MSL)
	{
		info.format = SDL_GPU_SHADERFORMAT_MSL;
		info.entrypoint = (stage == SDL_GPU_SHADERSTAGE_VERTEX) ? "vs_main" : "fs_main";
		info.code = nogravity_metal;
		info.code_size = nogravity_metal_len;
		return SDL_CreateGPUShader(g_Device, &info);
	}
	SDL_SetError("no shader available for this GPU backend");
	return NULL;
}

static SDL_GPUGraphicsPipeline *GetPipeline(const GPU_STATE *state)
{
	SDL_GPUGraphicsPipeline **slot = &g_Pipelines[state->prim][state->blend][state->depth];
	SDL_GPUGraphicsPipelineCreateInfo info;
	SDL_GPUVertexBufferDescription vb;
	SDL_GPUVertexAttribute attr[3];
	SDL_GPUColorTargetDescription color;

	if (*slot)
		return *slot;

	memset(&info, 0, sizeof(info));
	memset(&vb, 0, sizeof(vb));
	memset(attr, 0, sizeof(attr));
	memset(&color, 0, sizeof(color));

	vb.slot = 0;
	vb.pitch = sizeof(GPU_VERTEX);
	vb.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	attr[0].location = 0; attr[0].buffer_slot = 0; attr[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;      attr[0].offset = 0;
	attr[1].location = 1; attr[1].buffer_slot = 0; attr[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;      attr[1].offset = 12;
	attr[2].location = 2; attr[2].buffer_slot = 0; attr[2].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM; attr[2].offset = 24;

	info.vertex_shader = g_VertexShader;
	info.fragment_shader = g_FragmentShader;
	info.vertex_input_state.vertex_buffer_descriptions = &vb;
	info.vertex_input_state.num_vertex_buffers = 1;
	info.vertex_input_state.vertex_attributes = attr;
	info.vertex_input_state.num_vertex_attributes = 3;
	info.primitive_type = (state->prim == GPU_PRIM_LINES) ? SDL_GPU_PRIMITIVETYPE_LINELIST :
			      (state->prim == GPU_PRIM_POINTS) ? SDL_GPU_PRIMITIVETYPE_POINTLIST :
			      SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	info.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;
	info.rasterizer_state.cull_mode = SDL_GPU_CULLMODE_NONE;
	info.rasterizer_state.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;
	info.rasterizer_state.enable_depth_clip = true;
	info.multisample_state.sample_count = SDL_GPU_SAMPLECOUNT_1;

	info.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL;
	info.depth_stencil_state.enable_depth_test = (state->depth != GPU_DEPTH_OFF);
	info.depth_stencil_state.enable_depth_write = (state->depth == GPU_DEPTH_READWRITE);

	color.format = g_ColorFormat;
	color.blend_state.enable_blend = (state->blend != GPU_BLEND_NONE);
	color.blend_state.color_blend_op = SDL_GPU_BLENDOP_ADD;
	color.blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
	switch (state->blend)
	{
		case GPU_BLEND_ALPHA:
			color.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			color.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			color.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
			color.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		case GPU_BLEND_ADD:
			color.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
			color.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
			color.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
			color.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
			break;
		case GPU_BLEND_SUB:	// glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR)
			color.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
			color.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
			color.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
			color.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
			break;
		default:
			color.blend_state.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
			color.blend_state.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
			color.blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
			color.blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ZERO;
			break;
	}
	info.target_info.color_target_descriptions = &color;
	info.target_info.num_color_targets = 1;
	info.target_info.depth_stencil_format = g_DepthFormat;
	info.target_info.has_depth_stencil_target = true;

	*slot = SDL_CreateGPUGraphicsPipeline(g_Device, &info);
	if (!*slot)
		SDL_Log("Video: cannot create pipeline: %s", SDL_GetError());
	return *slot;
}

static SDL_GPUSampler *CreateSampler(int kind)
{
	SDL_GPUSamplerCreateInfo info;
	memset(&info, 0, sizeof(info));
	info.min_filter = SDL_GPU_FILTER_LINEAR;
	info.mag_filter = SDL_GPU_FILTER_LINEAR;
	info.mipmap_mode = (kind == GPU_SAMPLER_REPEAT_MIPMAP) ? SDL_GPU_SAMPLERMIPMAPMODE_LINEAR : SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
	info.address_mode_u = info.address_mode_v = info.address_mode_w =
		(kind == GPU_SAMPLER_CLAMP) ? SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE : SDL_GPU_SAMPLERADDRESSMODE_REPEAT;
	info.max_lod = (kind == GPU_SAMPLER_REPEAT_MIPMAP) ? 1000.f : 0.f;
	return SDL_CreateGPUSampler(g_Device, &info);
}

static void ReleaseTargets(void)
{
	if (g_ColorTarget) { SDL_ReleaseGPUTexture(g_Device, g_ColorTarget); g_ColorTarget = NULL; }
	if (g_DepthTarget) { SDL_ReleaseGPUTexture(g_Device, g_DepthTarget); g_DepthTarget = NULL; }
	g_TargetW = g_TargetH = 0;
}

static int EnsureTargets(Uint32 w, Uint32 h)
{
	SDL_GPUTextureCreateInfo info;
	if ((w == g_TargetW) && (h == g_TargetH) && g_ColorTarget && g_DepthTarget)
		return 1;
	ReleaseTargets();

	memset(&info, 0, sizeof(info));
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.format = g_ColorFormat;
	info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
	info.width = w;
	info.height = h;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	g_ColorTarget = SDL_CreateGPUTexture(g_Device, &info);

	info.format = g_DepthFormat;
	info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	g_DepthTarget = SDL_CreateGPUTexture(g_Device, &info);

	if (!g_ColorTarget || !g_DepthTarget)
	{
		SDL_Log("Video: cannot create render targets: %s", SDL_GetError());
		ReleaseTargets();
		return 0;
	}
	g_TargetW = w;
	g_TargetH = h;
	return 1;
}

static int CreateDevice(void)
{
	SDL_GPUShaderFormat formats;
	static const SDL_GPUTextureFormat depth_formats[] = {
		SDL_GPU_TEXTUREFORMAT_D24_UNORM, SDL_GPU_TEXTUREFORMAT_D32_FLOAT, SDL_GPU_TEXTUREFORMAT_D16_UNORM };
	int i;
	u_int8_t white[4] = {255, 255, 255, 255};

	formats = SDL_GPU_SHADERFORMAT_MSL;
#ifdef HAVE_SPIRV
	formats |= SDL_GPU_SHADERFORMAT_SPIRV;
#endif
	g_Device = SDL_CreateGPUDevice(formats, SDL_GetHintBoolean("NOGRAVITY_GPU_DEBUG", false), NULL);
	if (!g_Device)
	{
		SDL_Log("Video: cannot create a GPU device: %s", SDL_GetError());
		return 0;
	}
	formats = SDL_GetGPUShaderFormats(g_Device);
	g_VertexShader = CreateShader(SDL_GPU_SHADERSTAGE_VERTEX, formats);
	g_FragmentShader = CreateShader(SDL_GPU_SHADERSTAGE_FRAGMENT, formats);
	if (!g_VertexShader || !g_FragmentShader)
	{
		SDL_Log("Video: cannot create shaders: %s", SDL_GetError());
		return 0;
	}
	g_DepthFormat = SDL_GPU_TEXTUREFORMAT_D16_UNORM;
	for (i = 0; i < 3; i++)
	{
		if (SDL_GPUTextureSupportsFormat(g_Device, depth_formats[i], SDL_GPU_TEXTURETYPE_2D, SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET))
		{
			g_DepthFormat = depth_formats[i];
			break;
		}
	}
	for (i = 0; i < GPU_SAMPLER_COUNT; i++)
		g_Samplers[i] = CreateSampler(i);
	g_WhiteTexture = GPU_CreateTexture(1, 1, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, 0, white);
	return g_WhiteTexture != NULL;
}

static void ResetFrame(void);

static void DestroyDevice(void)
{
	int a, b, c;
	if (!g_Device)
		return;
	SDL_WaitForGPUIdle(g_Device);
	ResetFrame();
	ReleaseTargets();
	if (g_WhiteTexture)
	{
		FreeTexture(g_WhiteTexture);
		g_WhiteTexture = NULL;
	}
	for (a = 0; a < GPU_PRIM_COUNT; a++)
		for (b = 0; b < GPU_BLEND_COUNT; b++)
			for (c = 0; c < GPU_DEPTH_COUNT; c++)
				if (g_Pipelines[a][b][c])
				{
					SDL_ReleaseGPUGraphicsPipeline(g_Device, g_Pipelines[a][b][c]);
					g_Pipelines[a][b][c] = NULL;
				}
	for (a = 0; a < GPU_SAMPLER_COUNT; a++)
		if (g_Samplers[a]) { SDL_ReleaseGPUSampler(g_Device, g_Samplers[a]); g_Samplers[a] = NULL; }
	if (g_VertexBuffer) { SDL_ReleaseGPUBuffer(g_Device, g_VertexBuffer); g_VertexBuffer = NULL; }
	if (g_VertexTransfer) { SDL_ReleaseGPUTransferBuffer(g_Device, g_VertexTransfer); g_VertexTransfer = NULL; }
	g_VertexBufferSize = 0;
	if (g_VertexShader) { SDL_ReleaseGPUShader(g_Device, g_VertexShader); g_VertexShader = NULL; }
	if (g_FragmentShader) { SDL_ReleaseGPUShader(g_Device, g_FragmentShader); g_FragmentShader = NULL; }
	if (g_pSDLWindow)
		SDL_ReleaseWindowFromGPUDevice(g_Device, g_pSDLWindow);
	SDL_DestroyGPUDevice(g_Device);
	g_Device = NULL;
}

// ---- screenshot ---------------------------------------------------------

static void WritePNG(const char *path, const u_int8_t *rgba, int w, int h)
{
	FILE *fp = fopen(path, "wb");
	png_structp png;
	png_infop info;
	png_bytep *rows;
	int y;
	if (!fp)
		return;
	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info = png_create_info_struct(png);
	if (setjmp(png_jmpbuf(png)))
	{
		png_destroy_write_struct(&png, &info);
		fclose(fp);
		return;
	}
	png_init_io(png, fp);
	// The target's alpha channel is meaningless (opaque materials write 0),
	// so drop it: viewers would otherwise show those pixels as transparent.
	png_set_IHDR(png, info, (png_uint_32)w, (png_uint_32)h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png, info);
	png_set_filler(png, 0, PNG_FILLER_AFTER);
	rows = (png_bytep *)SDL_malloc((size_t)h * sizeof(png_bytep));
	for (y = 0; y < h; y++)
		rows[y] = (png_bytep)(rgba + (size_t)y * (size_t)w * 4);
	png_write_image(png, rows);
	png_write_end(png, NULL);
	SDL_free(rows);
	png_destroy_write_struct(&png, &info);
	fclose(fp);
	SDL_Log("Video: frame %d at %u ms: screenshot written to %s", g_FrameCount, (unsigned)SDL_GetTicks(), path);
}

static void Screenshot(void)
{
	SDL_GPUTransferBufferCreateInfo tbinfo;
	SDL_GPUTransferBuffer *tb;
	SDL_GPUCommandBuffer *cmd;
	SDL_GPUCopyPass *copy;
	SDL_GPUTextureRegion src;
	SDL_GPUTextureTransferInfo dst;
	SDL_GPUFence *fence;
	u_int8_t *map;

	memset(&tbinfo, 0, sizeof(tbinfo));
	tbinfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD;
	tbinfo.size = g_TargetW * g_TargetH * 4;
	tb = SDL_CreateGPUTransferBuffer(g_Device, &tbinfo);
	if (!tb)
		return;
	memset(&src, 0, sizeof(src));
	src.texture = g_ColorTarget;
	src.w = g_TargetW;
	src.h = g_TargetH;
	src.d = 1;
	memset(&dst, 0, sizeof(dst));
	dst.transfer_buffer = tb;
	cmd = SDL_AcquireGPUCommandBuffer(g_Device);
	copy = SDL_BeginGPUCopyPass(cmd);
	SDL_DownloadFromGPUTexture(copy, &src, &dst);
	SDL_EndGPUCopyPass(copy);
	fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd);
	if (fence)
	{
		SDL_WaitForGPUFences(g_Device, true, &fence, 1);
		SDL_ReleaseGPUFence(g_Device, fence);
	}
	map = (u_int8_t *)SDL_MapGPUTransferBuffer(g_Device, tb, false);
	if (map)
	{
		char path[1024];
		if (strchr(g_ScreenshotPath, '%'))
			snprintf(path, sizeof(path), g_ScreenshotPath, g_FrameCount);
		else
			snprintf(path, sizeof(path), "%s", g_ScreenshotPath);
		WritePNG(path, map, (int)g_TargetW, (int)g_TargetH);
		SDL_UnmapGPUTransferBuffer(g_Device, tb);
	}
	SDL_ReleaseGPUTransferBuffer(g_Device, tb);
}

// ---- frame submission ---------------------------------------------------

static void ResetFrame(void)
{
	g_nVertices = 0;
	g_nCommands = 0;
	FlushGarbage();
}

static int UploadVertices(SDL_GPUCommandBuffer *cmd)
{
	SDL_GPUCopyPass *copy;
	SDL_GPUTransferBufferLocation loc;
	SDL_GPUBufferRegion region;
	Uint32 size = (Uint32)g_nVertices * sizeof(GPU_VERTEX);
	void *map;

	if (size == 0)
		return 1;
	if (size > g_VertexBufferSize)
	{
		SDL_GPUBufferCreateInfo binfo;
		SDL_GPUTransferBufferCreateInfo tbinfo;
		Uint32 n = g_VertexBufferSize ? g_VertexBufferSize : 256 * 1024;
		while (n < size) n *= 2;
		if (g_VertexBuffer) SDL_ReleaseGPUBuffer(g_Device, g_VertexBuffer);
		if (g_VertexTransfer) SDL_ReleaseGPUTransferBuffer(g_Device, g_VertexTransfer);
		memset(&binfo, 0, sizeof(binfo));
		binfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
		binfo.size = n;
		g_VertexBuffer = SDL_CreateGPUBuffer(g_Device, &binfo);
		memset(&tbinfo, 0, sizeof(tbinfo));
		tbinfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		tbinfo.size = n;
		g_VertexTransfer = SDL_CreateGPUTransferBuffer(g_Device, &tbinfo);
		g_VertexBufferSize = n;
		if (!g_VertexBuffer || !g_VertexTransfer)
			return 0;
	}
	map = SDL_MapGPUTransferBuffer(g_Device, g_VertexTransfer, true);
	if (!map)
		return 0;
	memcpy(map, g_Vertices, size);
	SDL_UnmapGPUTransferBuffer(g_Device, g_VertexTransfer);

	memset(&loc, 0, sizeof(loc));
	loc.transfer_buffer = g_VertexTransfer;
	memset(&region, 0, sizeof(region));
	region.buffer = g_VertexBuffer;
	region.size = size;
	copy = SDL_BeginGPUCopyPass(cmd);
	SDL_UploadToGPUBuffer(copy, &loc, &region, true);
	SDL_EndGPUCopyPass(copy);
	return 1;
}

static SDL_GPURenderPass *BeginPass(SDL_GPUCommandBuffer *cmd, int clear_color, int clear_depth)
{
	SDL_GPUColorTargetInfo color;
	SDL_GPUDepthStencilTargetInfo depth;
	SDL_GPURenderPass *pass;
	SDL_GPUViewport vp;
	SDL_GPUBufferBinding vb;
	float xform[4];

	memset(&color, 0, sizeof(color));
	color.texture = g_ColorTarget;
	color.load_op = clear_color ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
	color.store_op = SDL_GPU_STOREOP_STORE;
	color.clear_color.a = 1.f;

	memset(&depth, 0, sizeof(depth));
	depth.texture = g_DepthTarget;
	depth.clear_depth = 1.f;
	depth.load_op = clear_depth ? SDL_GPU_LOADOP_CLEAR : SDL_GPU_LOADOP_LOAD;
	depth.store_op = SDL_GPU_STOREOP_STORE;
	depth.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
	depth.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;

	pass = SDL_BeginGPURenderPass(cmd, &color, 1, &depth);
	if (!pass)
		return NULL;

	vp.x = 0.f; vp.y = 0.f;
	vp.w = (float)g_TargetW; vp.h = (float)g_TargetH;
	vp.min_depth = 0.f; vp.max_depth = 1.f;
	SDL_SetGPUViewport(pass, &vp);

	// Logical (640x480 style) coordinates to clip space.
	xform[0] = 2.f / (float)g_pRLX->pGX->View.lWidth;
	xform[1] = 2.f / (float)g_pRLX->pGX->View.lHeight;
	xform[2] = xform[3] = 0.f;
	SDL_PushGPUVertexUniformData(cmd, 0, xform, sizeof(xform));

	if (g_VertexBuffer)
	{
		vb.buffer = g_VertexBuffer;
		vb.offset = 0;
		SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);
	}
	return pass;
}

static void Flip(void)
{
	SDL_GPUCommandBuffer *cmd;
	SDL_GPUTexture *swapchain = NULL;
	Uint32 w = 0, h = 0;
	SDL_GPURenderPass *pass = NULL;
	SDL_GPUGraphicsPipeline *pipeline = NULL;
	GPU_TEXTURE *texture = NULL;
	int sampler = -1;
	int i, take_shot;

	if (!g_Device || !g_pSDLWindow)
	{
		ResetFrame();
		return;
	}
	cmd = SDL_AcquireGPUCommandBuffer(g_Device);
	if (!cmd)
	{
		ResetFrame();
		return;
	}
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, g_pSDLWindow, &swapchain, &w, &h) || !swapchain ||
	    !EnsureTargets(w, h) || !UploadVertices(cmd))
	{
		// Minimised, or nothing to draw to: drop the frame.
		SDL_SubmitGPUCommandBuffer(cmd);
		ResetFrame();
		return;
	}

	for (i = 0; i < g_nCommands; i++)
	{
		const GPU_COMMAND *c = &g_Commands[i];
		if (c->type == CMD_PASS)
		{
			if (pass)
				SDL_EndGPURenderPass(pass);
			pass = BeginPass(cmd, c->clear_color, c->clear_depth);
			pipeline = NULL;
			texture = NULL;
			sampler = -1;
			continue;
		}
		if (!pass)
		{
			pass = BeginPass(cmd, 0, 0);
			pipeline = NULL;
			texture = NULL;
			sampler = -1;
		}
		if (!pass)
			break;
		{
			SDL_GPUGraphicsPipeline *p = GetPipeline(&c->state);
			GPU_TEXTURE *t = c->state.texture ? c->state.texture : g_WhiteTexture;
			if (!p)
				continue;
			if (p != pipeline)
			{
				SDL_BindGPUGraphicsPipeline(pass, p);
				pipeline = p;
			}
			if ((t != texture) || (c->state.sampler != sampler))
			{
				SDL_GPUTextureSamplerBinding bind;
				bind.texture = t->texture;
				bind.sampler = g_Samplers[c->state.sampler];
				SDL_BindGPUFragmentSamplers(pass, 0, &bind, 1);
				texture = t;
				sampler = c->state.sampler;
			}
			SDL_DrawGPUPrimitives(pass, (Uint32)c->count, 1, (Uint32)c->first, 0);
		}
	}
	if (pass)
		SDL_EndGPURenderPass(pass);

	// Present: copy the frame to the swapchain image.
	{
		SDL_GPUBlitInfo blit;
		memset(&blit, 0, sizeof(blit));
		blit.source.texture = g_ColorTarget;
		blit.source.w = g_TargetW;
		blit.source.h = g_TargetH;
		blit.destination.texture = swapchain;
		blit.destination.w = w;
		blit.destination.h = h;
		blit.load_op = SDL_GPU_LOADOP_DONT_CARE;
		blit.filter = SDL_GPU_FILTER_LINEAR;
		SDL_BlitGPUTexture(cmd, &blit);
	}
	++g_FrameCount;
	take_shot = g_ScreenshotPath && (g_ScreenshotFrame > 0) &&
		(strchr(g_ScreenshotPath, '%') ? (g_FrameCount % g_ScreenshotFrame == 0) : (g_FrameCount == g_ScreenshotFrame));
	if (take_shot)
	{
		SDL_GPUFence *fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd);
		if (fence)
		{
			SDL_WaitForGPUFences(g_Device, true, &fence, 1);
			SDL_ReleaseGPUFence(g_Device, fence);
		}
		Screenshot();
	}
	else
	{
		SDL_SubmitGPUCommandBuffer(cmd);
	}
	ResetFrame();
}

// ---- GX client driver ---------------------------------------------------

static int HasMode(const GXDISPLAYMODEINFO *list, int n, int w, int h)
{
	int i;
	for (i = 0; i < n; i++)
		if ((list[i].lWidth == w) && (list[i].lHeight == h))
			return 1;
	return 0;
}

// Every fullscreen mode of the primary display, largest first.  The
// renderer always works in 32-bit colour.
static GXDISPLAYMODEINFO *EnumDisplayList(void)
{
	GXDISPLAYMODEINFO *displays;
	int n;

	if (g_pDisplays == NULL)
	{
		int count = 0, i;
		SDL_DisplayID display = SDL_GetPrimaryDisplay();
		SDL_DisplayMode **modes = display ? SDL_GetFullscreenDisplayModes(display, &count) : NULL;

		g_pDisplays = (GXDISPLAYMODEINFO *)g_pRLX->mm_heap->malloc((count + 3) * sizeof(GXDISPLAYMODEINFO));
		n = 0;
		for (i = 0; modes && (i < count); i++)
		{
			if (HasMode(g_pDisplays, n, modes[i]->w, modes[i]->h))
				continue;
			g_pDisplays[n].lWidth = (u_int16_t)modes[i]->w;
			g_pDisplays[n].lHeight = (u_int16_t)modes[i]->h;
			g_pDisplays[n].BitsPerPixel = 32;
			g_pDisplays[n].mode = (short)n;
			n++;
		}
		SDL_free(modes);
		if (n == 0)
		{
			static const int fallback[2][2] = {{800, 600}, {640, 480}};
			for (i = 0; i < 2; i++, n++)
			{
				g_pDisplays[n].lWidth = (u_int16_t)fallback[i][0];
				g_pDisplays[n].lHeight = (u_int16_t)fallback[i][1];
				g_pDisplays[n].BitsPerPixel = 32;
				g_pDisplays[n].mode = (short)n;
			}
		}
		g_pDisplays[n].lWidth = 0;
		g_pDisplays[n].lHeight = 0;
		g_pDisplays[n].BitsPerPixel = 0;
		g_pDisplays[n].mode = 0;
	}
	for (n = 0; g_pDisplays[n].BitsPerPixel != 0; n++)
	{
		// Count entries.
	}
	displays = (GXDISPLAYMODEINFO *)g_pRLX->mm_heap->malloc((n + 1) * sizeof(GXDISPLAYMODEINFO));
	memcpy(displays, g_pDisplays, (n + 1) * sizeof(GXDISPLAYMODEINFO));
	return displays;
}

static void SetPrimitive(void)
{
	g_pRLX->pGX->View.Flip = Flip;
	g_pRLX->pGX->gi = GI_GPU;
	g_pRLX->pGX->csp = CSP_GPU;
	g_pRLX->pGX->csp_cfg.put.fonct = g_pRLX->pGX->csp.put;
}

// The engine renders in a fixed logical resolution (640x480, or 768x480
// on wide screens) which the vertex shader scales to the window; mouse
// coordinates are scaled back the same way.
typedef unsigned long (*PFMOUSECALLBACK)(void *);
static PFMOUSECALLBACK g_pMouseCallback;
static int g_WindowW, g_WindowH;

static unsigned long MouseUpdate(void *device)
{
	MSE_ClientDriver *m = g_pRLX->Control.mouse;
	unsigned long ret = g_pMouseCallback(device);
	if (g_WindowW > 0) m->x = m->x * g_pRLX->pGX->View.lWidth / g_WindowW;
	if (g_WindowH > 0) m->y = m->y * g_pRLX->pGX->View.lHeight / g_WindowH;
	return ret;
}

static void GPU_FakeViewPort(void)
{
	MSE_ClientDriver *m = g_pRLX->Control.mouse;
	g_WindowW = g_pRLX->pGX->View.lWidth;
	g_WindowH = g_pRLX->pGX->View.lHeight;
	g_pRLX->pGX->View.lWidth = 640;
	g_pRLX->pGX->View.lHeight = 480;
	if ((float)g_WindowW / (float)g_WindowH >= 1.6f)
		g_pRLX->pGX->View.lWidth = 768;
	g_pRLX->pGX->View.xmax = g_pRLX->pGX->View.lWidth - 1;
	g_pRLX->pGX->View.ymax = g_pRLX->pGX->View.lHeight - 1;
	if (m->Update != MouseUpdate)
	{
		g_pMouseCallback = m->Update;
		m->Update = MouseUpdate;
	}
}

static void GetDisplayInfo(GXDISPLAYMODEHANDLE mode)
{
	SYS_ASSERT(g_pDisplays != NULL);
	if ((mode < 0) && (g_pDisplays[0].BitsPerPixel != 0))
		mode = 0;
	GX_SetupViewport(&g_pRLX->pGX->View, g_pDisplays[mode].lWidth, g_pDisplays[mode].lHeight, 32);
	g_pRLX->pGX->View.ColorMask.RedMaskSize = 8;
	g_pRLX->pGX->View.ColorMask.GreenMaskSize = 8;
	g_pRLX->pGX->View.ColorMask.BlueMaskSize = 8;
	g_pRLX->pGX->View.ColorMask.RsvdMaskSize = 8;
	g_pRLX->pGX->View.ColorMask.RedFieldPosition = 0;
	g_pRLX->pGX->View.ColorMask.GreenFieldPosition = 8;
	g_pRLX->pGX->View.ColorMask.BlueFieldPosition = 16;
	g_pRLX->pGX->View.ColorMask.RsvdFieldPosition = 24;
	SetPrimitive();
	GPU_FakeViewPort();
}

static int SetDisplayMode(GXDISPLAYMODEHANDLE mode)
{
	if ((mode < 0) && (g_pDisplays[0].BitsPerPixel != 0))
		mode = 0;
	g_Mode = mode;
	return 0;
}

static GXDISPLAYMODEHANDLE SearchDisplayMode(int lx, int ly)
{
	GXDISPLAYMODEHANDLE mode;
	SYS_ASSERT(g_pDisplays != NULL);
	for (mode = 0; g_pDisplays[mode].BitsPerPixel != 0; mode++)
	{
		if ((g_pDisplays[mode].lWidth == lx) && (g_pDisplays[mode].lHeight == ly))
			break;
	}
	if (g_pDisplays[mode].BitsPerPixel == 0)
		mode = (g_pDisplays[0].BitsPerPixel != 0) ? 0 : -1;
	return mode;
}

static void ConfigureWindow(SDL_Window *window, int w, int h)
{
	if (ISFULLSCREEN())
	{
		SDL_DisplayMode closest;
		SDL_DisplayID display = SDL_GetDisplayForWindow(window);
		if (SDL_GetClosestFullscreenDisplayMode(display, w, h, 0.f, false, &closest))
			SDL_SetWindowFullscreenMode(window, &closest);
		SDL_SetWindowFullscreen(window, true);
	}
	else
	{
		SDL_SetWindowFullscreen(window, false);
		SDL_SetWindowSize(window, w, h);
		SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}
	SDL_SyncWindow(window);
}

static void ApplyVSync(void)
{
	int vsync = (g_pRLX->pGX->View.Flags & GX_CAPS_VSYNC) ? 1 : 0;
	SDL_GPUPresentMode mode = SDL_GPU_PRESENTMODE_VSYNC;
	if (vsync == g_VSync)
		return;
	g_VSync = vsync;
	if (!vsync)
	{
		if (SDL_WindowSupportsGPUPresentMode(g_Device, g_pSDLWindow, SDL_GPU_PRESENTMODE_IMMEDIATE))
			mode = SDL_GPU_PRESENTMODE_IMMEDIATE;
		else if (SDL_WindowSupportsGPUPresentMode(g_Device, g_pSDLWindow, SDL_GPU_PRESENTMODE_MAILBOX))
			mode = SDL_GPU_PRESENTMODE_MAILBOX;
	}
	SDL_SetGPUSwapchainParameters(g_Device, g_pSDLWindow, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, mode);
}

static int CreateSurface(int BackBufferCount)
{
	int w, h;
	SYS_ASSERT(g_pDisplays != NULL);
	SYS_ASSERT(g_Mode != -1);
	w = g_pDisplays[g_Mode].lWidth;
	h = g_pDisplays[g_Mode].lHeight;

	if (g_pSDLWindow == NULL)
	{
		g_pSDLWindow = SDL_CreateWindow("No Gravity", w, h, ISFULLSCREEN() ? SDL_WINDOW_FULLSCREEN : 0);
		if (!g_pSDLWindow)
		{
			SDL_Log("Video: cannot create window: %s", SDL_GetError());
			return -1;
		}
		ConfigureWindow(g_pSDLWindow, w, h);
		if (!SDL_ClaimWindowForGPUDevice(g_Device, g_pSDLWindow))
		{
			SDL_Log("Video: cannot attach the window to the GPU device: %s", SDL_GetError());
			SDL_DestroyWindow(g_pSDLWindow);
			g_pSDLWindow = NULL;
			return -1;
		}
		g_VSync = -1;
		SDL_Log("Video: %dx%d %s, SDL_GPU (%s)", w, h, ISFULLSCREEN() ? "fullscreen" : "windowed", SDL_GetGPUDeviceDriver(g_Device));
	}
	else
	{
		// A mode change keeps the window, the device and every texture.
		ConfigureWindow(g_pSDLWindow, w, h);
		SDL_Log("Video: %dx%d %s", w, h, ISFULLSCREEN() ? "fullscreen" : "windowed");
	}
	ApplyVSync();
	g_pRLX->pGX->Surfaces.maxSurface = BackBufferCount;
	return 0;
}

static void ReleaseSurfaces(void)
{
	g_pRLX->pGX->Surfaces.maxSurface = 0;
}

static int RegisterMode(GXDISPLAYMODEHANDLE mode)
{
	if ((mode < 0) && (g_pDisplays[0].BitsPerPixel != 0))
		mode = 0;
	g_pRLX->pGX->View.DisplayMode = (u_int16_t)mode;
	g_pRLX->pGX->Client->GetDisplayInfo(mode);
	return g_pRLX->pGX->Client->SetDisplayMode(mode);
}

static void Shutdown(void)
{
	DestroyDevice();
	if (g_pSDLWindow)
	{
		SDL_DestroyWindow(g_pSDLWindow);
		g_pSDLWindow = NULL;
	}
	if (g_pDisplays != NULL)
	{
		g_pRLX->mm_heap->free(g_pDisplays);
		g_pDisplays = NULL;
	}
	SDL_free(g_Vertices); g_Vertices = NULL; g_maxVertices = 0;
	SDL_free(g_Commands); g_Commands = NULL; g_maxCommands = 0;
	SDL_free(g_Garbage); g_Garbage = NULL; g_maxGarbage = 0;
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

static int Open(void *hnd)
{
	const char *frame;
	UNUSED(hnd);
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
		return -1;
	if (!CreateDevice())
	{
		DestroyDevice();
		return -1;
	}
	g_ScreenshotPath = SDL_getenv("NOGRAVITY_SCREENSHOT");
	frame = SDL_getenv("NOGRAVITY_SCREENSHOT_FRAME");
	if (frame)
		g_ScreenshotFrame = atoi(frame);
	return 0;
}

GXCLIENTDRIVER GX_GPU = {
	EnumDisplayList,
	GetDisplayInfo,
	SetDisplayMode,
	SearchDisplayMode,
	CreateSurface,
	ReleaseSurfaces,
	NULL,
	NULL,
	NULL,
	RegisterMode,
	Shutdown,
	Open,
	"SDL_GPU"
};

static void GX_EntryPoint(struct RLXSYSTEM *p)
{
	g_pRLX = p;
	GPU_SetPrimitiveSprites();
	g_pRLX->pGX->Client = &GX_GPU;
}

void V3X_EntryPoint(struct RLXSYSTEM *p)
{
	GX_EntryPoint(p);
	g_pRLX->pV3X->Client = &V3X_GPU;
}
