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

#include <stdio.h>
#include <stdlib.h>
#include "_rlx32.h"
#include "_rlx.h"
#include "systools.h"
#include "sysresmx.h"
#include "fixops.h"
#include "gx_struc.h"
#include "gx_init.h"
#include "gx_tools.h"
#include "gx_csp.h"
#include "gx_rgb.h"
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  u_int32_t static *CreateSquareArray(void)
*
* DESCRIPTION :
*
*/
u_int32_t static *CreateSquareArray(void)
{
    static u_int32_t  *pTable=NULL;
    if (pTable)
       return pTable + 255;
    else
    {
        int32_t i;
        //  Should be initialized only once
        pTable = (u_int32_t*)malloc(512*sizeof(u_int32_t));
        for (i=0;i<512;i++)
        {
            int32_t j=i-255;
            pTable[i]=j*j;
        }
        return pTable + 255;
    }
}

#define SGN(a)       ((a)==0 ? 0 : (( (a) >0) ? (1) : (-1)))

/*------------------------------------------------------------------------
*
* PROTOTYPE  : void ACT_LoadFn(char *filename2, char *mode2, FIO_cur->fopen f)
*
* DESCRIPTION :
*
*/
static void ACT_LoadFp(rgb24_t *pal, SYS_FILEHANDLE in)
{
    int i, j;
    u_int8_t *tmp;
    tmp = (u_int8_t*) MM_heap.malloc(256*sizeof(rgb24_t));
    FIO_cur->fread(tmp, sizeof(rgb24_t), 256, in);
    for(i=0;i<256;i++)
    {
        j=255-i;
        pal[i].r=tmp[j*3+0];
        pal[i].g=tmp[j*3+1];
        pal[i].b=tmp[j*3+2];
    }
    MM_heap.free(tmp);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void ACT_LoadFn(rgb24_t *pal, char *filename2, char *mode2)
*
* DESCRIPTION :
*
*/
void ACT_LoadFn(rgb24_t *pal, char *filename2)
{
    SYS_FILEHANDLE in;
    in = FIO_cur->fopen(filename2, "rb");
    if (in!=NULL)
    {
        ACT_LoadFp(pal, in);
        FIO_cur->fclose(in);
    }
    return;
}

/*------------------------------------------------------------------------
*
* PROTOTYPE  : u_int32_t RGBconvert(int c, u_int8_t *palette)
*
* DESCRIPTION :
*
*/
u_int32_t RGB_convert(int c, rgb24_t *palette)
{
    rgb24_t *p = palette+c;
    switch(GX.View.BytePerPixel){
        case 1:
        return c;
        default:
        return RGB_PixelFormat((u_int32_t)p->r, (u_int32_t)p->g, (u_int32_t)p->b);
    }
}
void RGB_GetPixelFormat(rgb24_t *rgb, u_int32_t c)
{
    rgb->r = (u_int8_t)(((c>>GX.View.ColorMask.RedFieldPosition  ) & ((1<<GX.View.ColorMask.RedMaskSize)  -1)) << (8-GX.View.ColorMask.RedMaskSize  ));
    rgb->g = (u_int8_t)(((c>>GX.View.ColorMask.GreenFieldPosition) & ((1<<GX.View.ColorMask.GreenMaskSize)-1)) << (8-GX.View.ColorMask.GreenMaskSize));
    rgb->b = (u_int8_t)(((c>>GX.View.ColorMask.BlueFieldPosition ) & ((1<<GX.View.ColorMask.BlueMaskSize) -1)) << (8-GX.View.ColorMask.BlueMaskSize ));
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  #define RGB565(c) ((c->r>>3) + ((c->g>>2)<<5) + ((c->b>>3)<<11))
*
* Description :
*
*/
#define RGB565(c) ((c->r>>3) + ((c->g>>2)<<5) + ((c->b>>3)<<11))
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  u_int8_t *RGB_SmartConverter(void *tgt, rgb24_t *target_pal, int target_bpp, void *source, rgb24_t *source_pal, int source_bpp, u_int32_t size)
*
* DESCRIPTION :   RGB Color format converter.
*
*/
u_int8_t *RGB_SmartConverter(void *tgt, rgb24_t *target_pal, int target_bpp, void *source, const rgb24_t *source_pal, int source_bpp, u_int32_t size)
{
    u_int8_t *target=NULL;
    int i=size;
    switch(source_bpp){
        case 1: // ============================ SOURCE 8bit ==================
        {
            u_int8_t *a=(u_int8_t*)source;
            switch(target_bpp) {
                case 1:  // remap
                {
                    if (target_pal!=source_pal)
                    {
                        u_int8_t map[256];
                        for (i=0;i<256;i++) map[i]=(u_int8_t)RGB_findNearestColor(source_pal+i, target_pal);
                        for (i=size;i!=0;a++, i--) *a=map[*a];
                    }
                    return (u_int8_t*)source;
                }
                case 2:  // posterize
                {
                    u_int16_t *t=tgt ? (u_int16_t*)tgt : (u_int16_t*)MM_heap.malloc(sizeof(short)*size);
                    u_int16_t sz =  GX.View.ColorMask.RedMaskSize +
                    GX.View.ColorMask.GreenMaskSize +
                    GX.View.ColorMask.BlueMaskSize;
                    target = (u_int8_t*)t;
                    if ((sz==15)||(sz==16))
                    {
                        for (;i!=0;t++, a++, i--)
                        {
                            const rgb24_t *mp = source_pal + (*a);
                            *t=(u_int16_t)(RGB_PixelFormat(mp->r, mp->g, mp->b));
                        }
                    }
                    else
                    {
                        for (;i!=0;t++, a++, i--)
                        {
                            const rgb24_t *mp=source_pal + (*a);
                            *t=(u_int16_t)RGB565(mp);
                        }
                    }
                }
                break;
                case 3:  // posterize
                {
                    rgb24_t *t=tgt ? (rgb24_t*)tgt : (rgb24_t*)MM_heap.malloc(sizeof(rgb24_t)*size);
                    target = (u_int8_t*)t;

					SYS_ASSERT(source);
					SYS_ASSERT(source_pal);

                    for (;i!=0;t++, a++, i--)
                    {
                        const rgb24_t *mp=source_pal + (*a);
                        t->r = mp->r;
                        t->g = mp->g;
                        t->b = mp->b;
                    }
                }
                break;
                case 4:  // posterize
                {
                    rgb32_t *t=tgt ? (rgb32_t*)tgt : (rgb32_t*)MM_heap.malloc(sizeof(rgb32_t)*size);
                    target = (u_int8_t*)t;
#ifdef __BIG_ENDIAN__
					if (GX.Client->Capabilities & 0x10)
					{
					for (;i!=0;t++, a++, i--)
                    {
                        const rgb24_t *mp = source_pal + (*a);
                        *(u_int32_t*)t =
                        ((u_int32_t)mp->r << (24-GX.View.ColorMask.RedFieldPosition)) |
                        ((u_int32_t)mp->g << (24-GX.View.ColorMask.GreenFieldPosition)) |
                        ((u_int32_t)mp->b << (24-GX.View.ColorMask.BlueFieldPosition)) |
                        ((u_int32_t)(*a ? 255 : 0) << (24-GX.View.ColorMask.RsvdFieldPosition));
                    }
					}
					else
#endif

					for (;i!=0;t++, a++, i--)
                    {
                        const rgb24_t *mp = source_pal + (*a);
                        *(u_int32_t*)t =
                        ((u_int32_t)mp->r << GX.View.ColorMask.RedFieldPosition) |
                        ((u_int32_t)mp->g << GX.View.ColorMask.GreenFieldPosition) |
                        ((u_int32_t)mp->b << GX.View.ColorMask.BlueFieldPosition) |
                        ((u_int32_t)(*a ? 255 : 0) << GX.View.ColorMask.RsvdFieldPosition);
                    }
					                }
                break;
            }
        }
        break;
        case 2: // ============================ SOURCE 16bit ==================
        {
            u_int16_t *a=(u_int16_t*)source;
            switch(target_bpp) {
                case 1:  // redusize
                {
                    u_int8_t *t;
                    target = tgt ?  (u_int8_t*)tgt : (u_int8_t*)MM_heap.malloc(size);
                    t = target;
                    if (target_pal)
                    {
                        for (;i!=0;a++, t++, i--)
                        {
                            rgb24_t x;
                            RGB_GetPixelFormat(&x, *a);
                            *t = (u_int8_t)RGB_findNearestColor(&x, source_pal);
                        }
                    }
                    else
                    {
                        for (;i!=0;a++, t++, i--)
                        {
                            rgb24_t x;
                            RGB_GetPixelFormat(&x, *a);
                            *t=(u_int8_t)RGB_332(x);
                        }
                    }
                }
                break;
                case 2:  //
                return (u_int8_t*)source;
                case 3:  // posterize special
                {
                    rgb24_t *t;
                    target = tgt ? (u_int8_t*)tgt : (u_int8_t*)MM_heap.malloc(size*sizeof(rgb24_t));
                    t = (rgb24_t*)target;
                    for (;i!=0;a++, t++, i--)
                    {
                        RGB_GetPixelFormat(t, *a);
                    }
                }
                break;
                case 4:  // posterize special
                {
                    rgb32_t *t;
                    target = tgt ? (u_int8_t*)tgt : (u_int8_t*)MM_heap.malloc(size*sizeof(rgb32_t));
                    t = (rgb32_t*)target;
                    for (;i!=0;a++, t++, i--)      {
                        RGB_GetPixelFormat((rgb24_t*)t, *a);
                        t->a = (u_int8_t)RGB_ToGray(t->r, t->b, t->g);
                    }
                }
                break;
            }
        }
        break;
        case 3: // ============================ SOURCE 24bit ==================
        {
            rgb24_t *a=(rgb24_t *)source;
            switch(target_bpp) {
                case 1:  // redusize
                {
                    u_int8_t *t;
                    target = tgt ?  (u_int8_t*)tgt : (u_int8_t*)MM_heap.malloc(size);
                    t = target;
                    if (target_pal)
                    {
                        for (;i!=0;a++, t++, i--) *t=(u_int8_t)RGB_findNearestColor(a, target_pal);
                    }
                    else
                    {
                        for (;i!=0;a++, t++, i--) *t=(u_int8_t)RGB_332(*a);
                    }
                }
                break;
                case 2:  // posterize special
                {
                    u_int16_t *t;
                    target = tgt ?  (u_int8_t*)tgt : (u_int8_t*)MM_heap.malloc(size*2);
                    t = (u_int16_t*)target;
                    for (;i!=0;a++, t++, i--)
                    {
                        *t = (u_int16_t)RGB_PixelFormat(a->r, a->g, a->b);
                    }
                }
                break;
                case 3:  //
                return (u_int8_t*)source;
                case 4:  // posterize special
                {
                    RGBENDIAN *t;
                    target = tgt ?  (u_int8_t*)tgt : (u_int8_t*)MM_heap.malloc(size*sizeof(rgb32_t));
                    t = (RGBENDIAN*)target;
                    for (;i!=0;a++, t++, i--)
                    {
						t->r = GX.View.ColorMask.RedFieldPosition ? a->b : a->r;
                        t->g = a->g;
						t->b = GX.View.ColorMask.RedFieldPosition ? a->r : a->b;
						t->a = a->r>1 || a->b>1 || a->g>1 ? 255 : 0;
                    }
                }
                break;
            }
        }
        break;
        case 4: // ============================ SOURCE 32bit ==================
        {
            rgb32_t *a=(rgb32_t *)source;
            switch(target_bpp) {
                case 1:
                {
                    u_int8_t *t;
                    target = tgt ?  (u_int8_t*)tgt : (u_int8_t*)MM_heap.malloc(size);
                    t = target;
                    if (target_pal)
                    {
                        for (;i!=0;a++, t++, i--) *t=(u_int8_t)RGB_findNearestColor((rgb24_t*)a, target_pal);
                    }
                    else
                    {
                        for (;i!=0;a++, t++, i--) *t=(u_int8_t)RGB_332(*a);
                    }
                }
                break;
                case 2:  // posterize special
                {
                    u_int16_t *t;
                    target = tgt ?  (u_int8_t*)tgt : (u_int8_t*)MM_heap.malloc(size*2);
                    t = (u_int16_t*)target;
                    for (;i!=0;a++, t++, i--)
                    {
                        *t = (u_int16_t)RGBA_PixelFormat(
							GX.View.ColorMask.RedFieldPosition ? a->b : a->r,
							a->g,
							GX.View.ColorMask.RedFieldPosition ? a->r : a->b,
							a->a);
                    }
                }
                break;
                case 3:  // posterize special
                {
                    rgb24_t *t;
                    target = tgt ?  (u_int8_t*)tgt : (u_int8_t*)MM_heap.malloc(size*sizeof(rgb24_t));
                    t = (rgb24_t*)target;
                    for (;i!=0;a++, t++, i--)
                    {
                        t->r = a->r;
                        t->g = a->g;
                        t->b = a->b;
                    }
                }
                break;
                case 4:
                return (u_int8_t*)source;
            }
        }
        break;
        default:
        return (u_int8_t*)source;
    }
    if (!tgt) MM_heap.free(source);
    return target;
}
#define MAT_R(n) (u_int32_t) (Mat[n]->r)
#define MAT_G(n) (u_int32_t) (Mat[n]->g)
#define MAT_B(n) (u_int32_t) (Mat[n]->b)
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  u_int32_t RGB_findNearestColor(rgb24_t *col, rgb24_t *pal)
*
* DESCRIPTION :
*
*/
u_int32_t RGB_findNearestColor(const rgb24_t *col, const rgb24_t *pal)
{
    unsigned d, best_dist=0x7eFFFFFF;
    int i, c=0;
    u_int32_t r, g, b;
    u_int32_t *carre = CreateSquareArray();
    for (i=0;i<256;i++, pal++)
    {
        r = (u_int32_t)pal->r-(u_int32_t)col->r;
        g = (u_int32_t)pal->g-(u_int32_t)col->g;
        b = (u_int32_t)pal->b-(u_int32_t)col->b;
        d = RGB_ToGray(carre[r], carre[g], carre[b]);
        if (d<best_dist)
        {
            best_dist=d;
            c = i;
        }
    }
    return c;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  u_int32_t RGB_PixelFormatEx(rgb24_t *p)
*
* Description :
*
*/
u_int32_t RGB_PixelFormatEx(rgb24_t *p)
{
    switch(GX.View.BytePerPixel){
        case 1:
        return RGB_findNearestColor(p, GX.ColorTable);
        default:
        return RGB_PixelFormat((u_int32_t)p->r, (u_int32_t)p->g, (u_int32_t)p->b);
    }
}
// pal: palette de la map, clr : couleur … mixer  mode, alpha, quantize
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void PAL_SetRedCyanPalette(void)
*
* DESCRIPTION :
*
*/

__extern_c
static u_int8_t      *StereoRed, *StereoBlue;
__end_extern_c

void PAL_SetRedCyanPalette(void)
{
    int i, j;
    char bid[768];
    rgb24_t *pnew=GX.ColorTable, *pold=(rgb24_t*)bid;
    memcpy(pold, pnew, 768);
    if (StereoRed==NULL)
    {
        StereoRed = (u_int8_t*)malloc(256);
        StereoBlue = (u_int8_t*)malloc(256);
    }
    // Nouvelle Palette
    for (i=0;i<16;i++)
    {
        for (j=0;j<16;j++, pnew++)
        {
            pnew->r = (u_int8_t)(j<<4);
            pnew->b = (u_int8_t)(i<<4);
            pnew->g = (u_int8_t)(i<<4);
        }
    }
    for (i=0;i<256;i++, pold++)
    {
        StereoRed[i] = (u_int8_t)(RGB_ToGray(pold->r, pold->g, pold->b)>>4);
        StereoBlue[i] = (u_int8_t)(StereoRed[i]<<4);
    }
    return;
}
