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
#ifndef __V3XMAPS_H
#define __V3XMAPS_H

#include "gx_csp.h"

// Texture & material cache system

enum {
   V3XMAT_LOADIMAGE = 0x1,
   V3XMAT_LOADRGB = 0x2
};

// Advanced 2D sprites system

typedef struct _v3xspriteinfo
{
    unsigned    flags;	   // Material
    rgb32_t  *	color;                  // RGB Color
    V3XSCALAR    x, y, z;            // Screen positions, Z, Capabilities (see below)
    V3XSCALAR    ROTcos, ROTsin;
    union {
       V3XSCALAR    x2;
       int32_t       lx;
    };
    union {
       V3XSCALAR    y2;
       int32_t       ly;
    };    // Angle and size
    V3XPOLY     poly;                   // Pipeline polygone
    V3XMATERIAL mat;
}V3XSPRITEINFO;

typedef struct _v3xsprite
{
    V3XSPRITEINFO	_sp;
    GXSPRITE		sp;
}V3XSPRITE;

typedef struct _v3xspritegroup
{
    GXSPRITEGROUP    spg;
    V3XSPRITEINFO  * item;
    V3XMATERIAL		 mat;
}V3XSPRITEGROUP;

enum {
    V3XRESOURCETYPE_TEXTURE = 0x1,
    V3XRESOURCETYPE_STREAM = 0x2,
    V3XRESOURCETYPE_MATERIAL = 0x3,
    V3XRESOURCETYPE_TEXTURE2 = 0x4
};


__extern_c

// Texture cache
_RLXEXPORTFUNC    int            RLXAPI    V3XResources_Del(V3XRESOURCE *bm, const char *filename);
_RLXEXPORTFUNC    void           RLXAPI    V3XResources_Animated(V3XRESOURCE *bm);

// Dynamic materials
_RLXEXPORTFUNC    void           RLXAPI    V3XCache_Mesh(V3XMESH *Obj);

// Material upload mechanism
_RLXEXPORTFUNC    void           RLXAPI    V3XMaterials_LoadFromMesh(V3XMESH *Obj);

_RLXEXPORTFUNC    void           RLXAPI    V3XMaterial_LoadTextures(V3XMATERIAL *Mat);
_RLXEXPORTFUNC    void           RLXAPI    V3XMaterial_Release(V3XMATERIAL *map, V3XMESH *Obj);
_RLXEXPORTFUNC    void           RLXAPI    V3XMaterial_Register(V3XMATERIAL *mat);

// V3X'98 GXSPRITE upload methods (singled GXSPRITE)
_RLXEXPORTFUNC    void           RLXAPI    V3X_CSP_GetFn(char *filename, GXSPRITE *item, int load);
_RLXEXPORTFUNC    void           RLXAPI    V3X_CSP_Prepare(GXSPRITE *sp, int flags);
_RLXEXPORTFUNC    int            RLXAPI    V3X_CSP_Set3D(GXSPRITE *item, V3XVECTOR *pos, V3XSCALAR s, unsigned mode);
_RLXEXPORTFUNC    int            RLXAPI    V3X_CSP_Draw(GXSPRITE *sp, int clip);
_RLXEXPORTFUNC    void           RLXAPI    V3X_CSP_Unload(GXSPRITE *item);


__end_extern_c

#endif
