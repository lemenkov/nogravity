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
#ifndef _GX_CSP_H
#define _GX_CSP_H

// Download GXSPRITE method
enum {
    CSPLOAD_POSTERIZE = 0x1,
    CSPLOAD_SURFACE = 0x2,
    CSPLOAD_HWSPRITE = 0x4,
    CSPLOAD_FORCE8 = 0x8
};


// Get Area definition (SPC file item structures)
typedef struct{
    short   a, b, c, d;
}GXSPRITEFORMAT;

// Sprite group definition
typedef struct {
    GXSPRITE           *item;        // single GXSPRITE
    short int         maxItem;     // numbers of GXSPRITE in group
    u_int16_t            speed;       // internal
    u_int8_t             HSpacing;    // Horizontal spacing
    u_int8_t             VSpacing;    // Vertical spacing
    u_int8_t             Caps;        // Capabilities
    u_int8_t             filler;
}GXSPRITEGROUP;

// Sprite function pointer
typedef void (* CSP_STDFUNCTION)(int32_t x, int32_t y, GXSPRITE *sp);
typedef void (* CSP_STRFUNCTION)(GXSPRITE *sp, int32_t x, int32_t y, int32_t lx, int32_t ly);

typedef union {
	CSP_STDFUNCTION fonct;
	CSP_STRFUNCTION zoomf;
}CSP_FUNCTION;

typedef struct {
    u_int32_t             flags;      // Flags
    u_int32_t             color;      // Color replaced by white
    u_int32_t             alpha;      // Alpha component
    void             *table;      // Table
    CSP_FUNCTION      put;        // Main operator
    char              ext[4];     // Extension for bitmap GXSPRITE file
}CSP_Config;

// Sprite class
typedef struct {
    u_int32_t           caps;
    // Normal GXSPRITE drawer
    void (*put)(      int32_t x, int32_t y, GXSPRITE *sp);
    void (*pset)(     int32_t x, int32_t y, GXSPRITE *sp);

    // Blended GXSPRITE drawer (in the current depth color)
    void (*TrspADD)(  int32_t x, int32_t y, GXSPRITE *sp);
    void (*TrspSUB)(  int32_t x, int32_t y, GXSPRITE *sp);
    void (*TrspALPHA)(int32_t x, int32_t y, GXSPRITE *sp);

    // Scalable GXSPRITE drawer
    void (*zoom_pset)(   GXSPRITE *sp, int32_t x, int32_t y, int32_t new_lx, int32_t new_ly);
    void (*zoom_put)(      GXSPRITE *sp, int32_t x, int32_t y, int32_t new_lx, int32_t new_ly);
    void (*zoom_TrspADD)( GXSPRITE *sp, int32_t x, int32_t y, int32_t new_lx, int32_t new_ly);
    void (*zoom_TrspSUB)( GXSPRITE *sp, int32_t x, int32_t y, int32_t new_lx, int32_t new_ly);
    void (*zoom_TrspALPHA)( GXSPRITE *sp, int32_t x, int32_t y, int32_t new_lx, int32_t new_ly);

}GXSPRITEINTERFACE;

struct _sys_fileio;

    // Sprite
void CSP_Resize(GXSPRITE *sp, int lx, int ly, int bpp);

    // Sprite familly
void CSPG_Release(GXSPRITEGROUP *pSpriteGroup);
GXSPRITEGROUP *CSPG_GetFn(char *filename, struct _sys_fileio *f, unsigned option);

int32_t CSPG_TxLen(const char *texte, const GXSPRITEGROUP *Fonte);

    // Graphic Text writer
void CSP_DrawText(const char *texte, int32_t xx, int32_t yy, const GXSPRITEGROUP *Fonte, CSP_FUNCTION sp);
void CSP_DrawTextC(const char *str, int x, int y, int attr1, int attr2, const GXSPRITEGROUP *Fonte, CSP_FUNCTION spz);
    //

#define CSP_Color(c)     GX.csp_cfg.color = c
#define CSP_Alpha(c)  GX.csp_cfg.alpha = c

#define CSP_DrawCenterText(texte, yy, Fonte, sp)     CSP_DrawText(texte, GX.View.xmin+(((GX.View.xmax-GX.View.xmin)-CSPG_TxLen(texte, Fonte))>>1), yy, Fonte, sp)
#define CSP_WriteText(texte, xx, yy, Fonte)          CSP_DrawText(texte, xx, yy, Fonte, GX.csp_cfg.put)
#define CSP_WriteCenterText(texte, yy, Fonte)       CSP_DrawCenterText(texte, yy, Fonte, GX.csp_cfg.put)
#endif
