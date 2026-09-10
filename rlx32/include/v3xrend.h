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
#ifndef __V3XREND_H
#define __V3XREND_H


enum {
    V3XCSPDRAW_CLIP = 0x1,
    V3XCSPDRAW_INSTANCE = 0x2,
    V3XCSPDRAW_INSTMATERIAL = 0x4
};


// Rasterizer caps

enum {
      V3XTEXDWNOPTION_COLORKEY = 0x4, // Apply colorkeying
	  V3XTEXDWNOPTION_DYNAMIC = 0x1000, // Download a dynamic texture
};

enum {
      GXSPEC_ENABLECOMPRESSION =0x200, // Enable tex compression
      GXSPEC_ENABLEFILTERING =0x1000, // Enable bilinear filtering
      GXSPEC_ENABLEZBUFFER =0x40000, // Z-Buffer enable
      GXSPEC_ENABLEWBUFFER =0x2000000

};

// Render global categories
enum {
      V3XRCLASS_point,
      V3XRCLASS_wired,
      V3XRCLASS_flat,
      V3XRCLASS_gouraud,
      V3XRCLASS_transp,
      V3XRCLASS_normal_mapping,
      V3XRCLASS_opacity_mapping,
      V3XRCLASS_transp_mapping,
      V3XRCLASS_transp_flat,
      V3XRCLASS_dualtex_mapping,
      V3XRCLASS_blur_mapping,
      V3XRCLASS_shadow,
      V3XRCLASS_double_mapping,
      V3XRCLASS_bitmap,
      V3XRCLASS_bitmap_transp,
      V3XRCLASS_bitmap_any,
      V3XRCLASS_bump_mapping
};

enum {
     V3XCMD_SETZBUFFERSTATE = 0x30,
     V3XCMD_SETZBUFFERCOMP = 0x40,
     V3XCMD_SETBACKGROUNDCOLOR= 0x80

};



    // Render objects
void V3XMesh_SetRender(V3XMESH *obj);

    // Additional Primitives


    // Primitive
int V2XVector_Clip( V3XVECTOR2 *a, V3XVECTOR2 *b );
int GX_ClippedLine(V3XVECTOR2 *a, V3XVECTOR2 *b, u_int32_t cc);
void GX_ClippedLine3D(V3XVECTOR *a, V3XVECTOR *b, u_int32_t c);

    // RGB calculations

    // Sprite render

 #ifdef _GX_CSP_H
 #endif

#endif
