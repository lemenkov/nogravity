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
#ifndef __GX_STRUC_H
#define __GX_STRUC_H


// Video caps
enum {
	GX_CAPS_FBLINEAR = 0x2,      // FB is linear
	GX_CAPS_BACKBUFFERINVIDEO = 0x8,      // The backbuffer is in video memory
	GX_CAPS_VSYNC = 0x10,     // Vertical sync after page flipping
	GX_CAPS_FBINTERLEAVED = 0x20,     // FB is interleaved (1 line/2)
	GX_CAPS_3DSYSTEM = 0x200,    // 3D display buffer (for Direct3D)

	GX_CAPS_MULTISAMPLING = 0x4000      // Reserved
};

enum
{
	GX_STATE_BACKBUFFERPAGE = 0x100,    // Internal (buffer page 0/1)
	GX_STATE_LOCKED = 0x800,    // FB is locked (write is allowed)
    GX_STATE_SCENEBEGUN = 0x1000,   // 3D Scene has begun.
};


// User video driver
typedef struct _gx_colormask {
	unsigned char		RedMaskSize;       // RGB Pixel format
    unsigned char		RedFieldPosition;
    unsigned char		GreenMaskSize;
    unsigned char		GreenFieldPosition;
    unsigned char		BlueMaskSize;
    unsigned char		BlueFieldPosition;
    unsigned char		RsvdMaskSize;
    unsigned char		RsvdFieldPosition;
}GXRGBCOMPONENT;

typedef struct _gx_viewport{

    u_int32_t            lPitch;            // Pitch in byte (byte per lines)
    u_int32_t            lSurfaceSize;      // Surface size in bytes
    int32_t              lWidth;            // Width size of the screen
    int32_t              lHeight;           // Height size of the screen
    int32_t              lRatio;            // Ratio
    int32_t              xmin;              // Viewport coordinates
    int32_t              ymin;
    int32_t              xmax;
    int32_t              ymax;
    void (*Flip)(void);   // Display page function
    u_int32_t            Flags;             // Flags (see "Video Caps")
	int					 State;
    u_int16_t            DisplayMode;       // Display mode ID
    u_int8_t             BytePerPixel;      // Numbers of bytes per pixel
    u_int8_t             BitsPerPixel;      // Numbers of bits per pixel
    u_int8_t			 Multisampling;
	GXRGBCOMPONENT		 ColorMask;

}GXVIEWPORT;

// Extended GXSPRITE structure
typedef struct {
     u_int8_t U, V, Alpha, Mode;
     void *Page;
}GXSPRITEUV;

// Offscreen informations (private)
typedef struct _gx_offplain {
    unsigned          maxSurface;
    u_int8_t             flags[16];
}GXSCREENBUFFERS;


// Display mode informations (private)
typedef struct _gx_display_mode_info {
    short             mode;
    u_int16_t            lWidth, lHeight;
    u_int16_t            BitsPerPixel;
}GXDISPLAYMODEINFO;

typedef int GXDISPLAYMODEHANDLE;

// Video Driver
typedef struct {
    GXDISPLAYMODEINFO*(* EnumDisplayList)(void);
    void               (* GetDisplayInfo)(GXDISPLAYMODEHANDLE mode);
    int	               (* SetDisplayMode)(GXDISPLAYMODEHANDLE mode);
    GXDISPLAYMODEHANDLE(* SearchDisplayMode)(int lx, int ly);
    int                (* CreateSurface)(int numberOfSparePages);
    void               (* ReleaseSurfaces)(void);
    void               (* UploadSprite)(GXSPRITE *sp, rgb24_t *colorTable, int bpp);
    void               (* ReleaseSprite)(GXSPRITE *sp);
    unsigned           (* UpdateSprite)(GXSPRITE *sp, const u_int8_t *bitmap, const rgb24_t *colorTable);
    int                (* RegisterMode)(int bpp);
    void               (* Shutdown)(void);
    int                (* Open)(void *hwnd);


    char               s_DrvName[64];
	int				   Capabilities;
	int				   State;
}GXCLIENTDRIVER;

// Graphic driver
typedef struct _gx_graphic_interface
{
    void  (* drawAnyLine)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, u_int32_t colour);
    void  (* drawHorizontalLine)(int32_t x1, int32_t y1, int32_t lx, u_int32_t colour);
    void  (* drawVerticalLine)(int32_t x1, int32_t y1, int32_t lx, u_int32_t colour);
    void  (* drawWiredRect)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, u_int32_t colour);
    void  (* drawShadedRect)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, void *palette);
    void  (* drawMeshedRect)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, u_int32_t colour);
    void  (* drawFilledRect)(int32_t x1, int32_t y1, int32_t x2, int32_t y2, u_int32_t colour);
    void  (* drawPixel)(int32_t x, int32_t y, u_int32_t colour);
    void  (* clearBackBuffer)(void);
    void  (* clearVideo)(void);
}GXGRAPHICINTERFACE;

#include "gx_csp.h"

struct GXSYSTEM
{
	GXGRAPHICINTERFACE	gi;
	GXVIEWPORT		View;
	GXSCREENBUFFERS		Surfaces;
	GXCLIENTDRIVER *	Client;
	GXSPRITEINTERFACE			csp;
	CSP_Config			csp_cfg;
	rgb24_t			 *  ColorClut;
	rgb24_t				ColorTables[4][256];
	rgb24_t				ColorTable[256];
	rgb24_t				AmbientColor;
	rgb24_t				DefaultColor;
	void (*init)(void);
};


extern struct GXSYSTEM	GX;



#endif
