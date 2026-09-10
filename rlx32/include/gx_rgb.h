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
#ifndef __GX_RGB_H
#define __GX_RGB_H

// Color format macros
#define RGB_PixelFormat(r, g, b) \
(( (  (r)>>(8-GX.View.ColorMask.RedMaskSize  )) << GX.View.ColorMask.RedFieldPosition   )\
| ( ((g)>>(8-GX.View.ColorMask.GreenMaskSize)) << GX.View.ColorMask.GreenFieldPosition )\
| ( ((b)>>(8-GX.View.ColorMask.BlueMaskSize )) << GX.View.ColorMask.BlueFieldPosition  ))

#define RGBA_PixelFormat(r, g, b, a) \
(( (  (r)>>(8-GX.View.ColorMask.RedMaskSize  )) << GX.View.ColorMask.RedFieldPosition   )\
| ( ((g)>>(8-GX.View.ColorMask.GreenMaskSize)) << GX.View.ColorMask.GreenFieldPosition )\
| ( ((b)>>(8-GX.View.ColorMask.BlueMaskSize )) << GX.View.ColorMask.BlueFieldPosition  )\
| ( ((a)>>(8-GX.View.ColorMask.RsvdMaskSize )) << GX.View.ColorMask.RsvdFieldPosition  ))\

#define RGB_332(x) (  ((unsigned)(x).r>>5) + (((unsigned)(x).g>>5)<<3) + (((unsigned)(x).b>>6)<<6)  )
#define RGB_ToGray(r, g, b)    ((((unsigned)(b)*29L)+((unsigned)(g)*150L)+((unsigned)(r)*77L))>>8)
#define RGB_Make32bit(r, g, b, a) (unsigned)(b)+((unsigned)(g)<<8)+((unsigned)(r)<<16)+((unsigned)(a)<<24)
#define RGB_Alpha50(a, b)        ((((a)&GX.View.RGB_Magic)+((b)&GX.View.RGB_Magic))>>1)
#define RGB_Specular(a, ext)   ((unsigned)a.ext+(unsigned)GX.AmbientColor.ext>255 ? 255 : a.ext+GX.AmbientColor.ext)
#define RGB_Set(pal, xr, xg, xb) { (pal).r=(u_int8_t)(xr); (pal).g=(u_int8_t)(xg);  (pal).b=(u_int8_t)(xb); }
#define RGB32_Set(pal, xr, xg, xb, xa) { (pal).r=(u_int8_t)(xr); (pal).g=(u_int8_t)(xg);  (pal).b=(u_int8_t)(xb); (pal).a=(u_int8_t)(xa);}

// 48 bit color structure
typedef struct{
    unsigned rouge;
    unsigned vert;
    unsigned bleu;
} rgb48_t;

// Mix method for 8bit blending table
enum {
    REALRGBMIX_Alpha, // Alpha blending
    REALRGBMIX_Mul,  // Mul blending
    REALRGBMIX_Add,  // Additive blending
    REALRGBMIX_Sub,  // Substractive blending
    REALRGBMIX_Neg,  // Inverse blending
    REALCOLOR_RgbReal
};
// 8bit fade method

// Functions

__extern_c

    // Pixel format
_RLXEXPORTFUNC    void    RLXAPI  RGB_GetPixelFormat(rgb24_t *rgb, u_int32_t c);    //
_RLXEXPORTFUNC    unsigned   RLXAPI  RGB_SetPixelFormat(int r, int g, int b);    //

    // Operation Palette

    // Load color Table
_RLXEXPORTFUNC    void    RLXAPI  ACT_LoadFn(rgb24_t *pal, char *filename2);

    // Color converters
_RLXEXPORTFUNC    u_int32_t   RLXAPI  RGB_convert(int c, rgb24_t *palette);
_RLXEXPORTFUNC    u_int32_t   RLXAPI  RGB_PixelFormatEx(rgb24_t *p);
_RLXEXPORTFUNC    u_int8_t   RLXAPI *RGB_SmartConverter(void *tgt, rgb24_t *target_pal, int target_bpp,
      void *source, const rgb24_t *source_pal, int source_bpp, u_int32_t size);
_RLXEXPORTFUNC    u_int32_t  RLXAPI   RGB_findNearestColor(const rgb24_t *col, const rgb24_t *pal);

    // RGB filtering
_RLXEXPORTFUNC    void    RLXAPI  PAL_SetRedCyanPalette(void);

    // 8bit blend palette

    // Fast macro for color fading.

#define RASTER_CMDRESET       0xFF
#define RASTER_CMDBORDER      0x31
#define RASTER_CMDBACK        0x00

__end_extern_c

#endif
