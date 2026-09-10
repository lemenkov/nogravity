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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "rlx32.h"
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
* PROTOTYPE  :  void IMG_stretch(u_int8_t *old_buf, u_int8_t *new_buf, int old_ly, int new_ly, int old_lx, int new_lx, int bytes)
*
* DESCRIPTION :
*
*/
void IMG_stretch(u_int8_t *old_buf, u_int8_t *new_buf, int old_ly, int new_ly, int old_lx, int new_lx, int bytes)
{
    u_int8_t *v=new_buf, *w;
    u_int16_t *v0 = (u_int16_t*)new_buf, *old_buf0=(u_int16_t*)old_buf, *w0;
    u_int32_t  *v1 = (u_int32_t*)new_buf, *old_buf1=(u_int32_t*)old_buf, *w1;
    rgb24_t *v2 = (    rgb24_t*)new_buf, *old_buf2=(    rgb24_t*)old_buf, *w2;
    // u_int32_t *v1 = (u_int16_t*)new_buf, *old_buf1=(u_int16_t*)old_buf, *w1;
    int32_t i, j;
    int32_t oy, dy=VDIV(old_ly, new_ly);
    int32_t ox, dx=VDIV(old_lx, new_lx);
    if ((old_ly==new_ly) && (old_lx==new_lx))
    {
        memcpy(new_buf, old_buf, new_lx*new_ly*bytes);
        return;
    }
    switch(bytes) {
        case 1:
        for (oy=0, i=new_ly;i!=0;oy+=dy, i--)
        {
            w = old_buf + (oy>>16)*old_lx ;
            for (ox=0, j=new_lx;j!=0;ox+=dx, j--) *(v++)=w[ox>>16];
        }
        break;
        case 2:
        for (oy=0, i=new_ly;i!=0;oy+=dy, i--)
        {
            w0 = old_buf0 + (oy>>16)*old_lx ;
            for (ox=0, j=new_lx;j!=0;ox+=dx, v0++, j--)
            {
                u_int16_t *wx=w0+(ox>>16);
                *v0 = *wx;
            }
        }
        break;
        case 3:
        for (oy=0, i=new_ly;i!=0;oy+=dy, i--)
        {
            w2 = old_buf2 + (oy>>16)*old_lx ;
            for (ox=0, j=new_lx;j!=0;ox+=dx, j--) *(v2++)=w2[ox>>16];
        }
        break;
        case 4:
        for (oy=0, i=new_ly;i!=0;oy+=dy, i--)
        {
            w1 = old_buf1 + (oy>>16)*old_lx ;
            for (ox=0, j=new_lx;j!=0;ox+=dx, v1++, j--)
            {
                u_int32_t *wx = w1+(ox>>16);
                *v1 = *wx;
            }
        }
        break;
    }
    return;
}

#define MAT_R(n) (u_int32_t) (Mat[n].r)
#define MAT_G(n) (u_int32_t) (Mat[n].g)
#define MAT_B(n) (u_int32_t) (Mat[n].b)
