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
#include "_rlx32.h"
#include "_rlx.h"
#include "sysresmx.h"
#include "systools.h"
/*****/
#include "gx_struc.h"
#include "gx_tools.h"
#include "gx_rgb.h"
#include "gx_csp.h"
/*****/
#include "v3xdefs.h"
#include "v3x_1.h"
#include "v3x_2.h"
#include "v3xtrig.h"
#include "v3xmaps.h"
#include "v3xrend.h"

void *TRG_Table;
struct V3XSYSTEM V3X;
static void V3XPoly_Release(V3XPOLY *f);
static void V3XPoly_Alloc(V3XPOLY *f, int som);
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void V3XPlugIn_Add(int id, void (*plug)(const void *))
*
* DESCRIPTION :
*
*/
void V3XPlugIn_Add(int id, void (*plug)(const void *))
{
    V3X.Plugin.plug[V3X.Plugin.maxPlug].call = plug;
    V3X.Plugin.maxPlug++;
    UNUSED(id);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  : void SinGen(void)
*
* DESCRIPTION : Construit une table de sinus
*
*/
void TRG_Generate(void)
{
    int i;
    V3XSCALAR *f;
    TRG_Table = (float *) malloc(sizeof(float)*4096);
    for (f=(float*)TRG_Table, i=0;i<4096;i++, f++)
    {
        float a = (float)M_PI*(float)i/2048.f;
        *f = (float)cos((float)a);
    }
    return;
}
/*------------------------------------------------------------------------ bc
*
* PROTOTYPE  :  void RLXAPI static *v3x_mallocopy(void *b, u_int32_t sz)
*
* DESCRIPTION :
*
*/
void RLXAPI static *v3x_mallocopy(void *b, u_int32_t sz)
{
    if (sz)
	{
		u_int8_t *m=(u_int8_t*)MM_heap.malloc(sz);
		memcpy(m, b, sz);
		return m;
	}
	else return NULL;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void V3XMesh_Duplicate(V3XMESH *mesh1, V3XMESH *mesh2)
*
* DESCRIPTION :
*
*/

static void V3XPoly_ReleaseDup(V3XPOLY *f)
{
    MM_heap.free(f->ZTab);
    MM_heap.free(f->shade);
    MM_heap.free(f->dispTab);

    if (f->uvTab)
    {
        MM_heap.free(f->uvTab[0]);
        MM_heap.free(f->uvTab[1]);
    }
    MM_heap.free(f->uvTab);
    return;
}
void V3XMesh_ReleaseDup(V3XMESH *obj)
{
	int j;
    if (obj->face)
    {
        V3XPOLY *fce;
        for (fce=obj->face, j=0;j<obj->numFaces;fce++, j++) V3XPoly_ReleaseDup(fce);
        MM_heap.free(obj->face);
    }
    memset(obj, 0, sizeof(V3XMESH));
    MM_heap.free(obj);
    return;
}

void V3XMesh_Duplicate(V3XMESH *mesh1, V3XMESH *mesh2)
{
    int j;
    V3XPOLY *fa, *fb;
    memcpy(mesh1, mesh2, sizeof(V3XMESH));
    // Copie les faces
    mesh1->face=(V3XPOLY*)v3x_mallocopy(mesh2->face, sizeof(V3XPOLY)*mesh1->numFaces);
    for ( fa = mesh1->face, fb = mesh2->face, j = 0;j < mesh1->numFaces; j++, fa++, fb++)
    {
        V3XMATERIAL *mat=(V3XMATERIAL*)fb->Mat;
        if ((!mat->info.Sprite)&&(mat->info.Texturized)) mat->info.Perspective=1;
        fa->visible = 1;
        fa->dispTab = (V3XPTS*)v3x_mallocopy(fb->dispTab, fb->numEdges * sizeof(V3XPTS));
        if (mat->info.Shade)
        fa->shade = (V3XSCALAR*)  v3x_mallocopy(fb->shade, fb->numEdges * sizeof(V3XSCALAR));
        fa->faceTab = fb->faceTab;
        if (mat->info.Texturized)
        {
            int j, kX = 2;
            fa->uvTab = (V3XUV**)MM_heap.malloc(kX * sizeof(V3XUV*));
            for (j=0;j<kX;j++)
            {
				if (fb->uvTab[j])
                fa->uvTab[j] = (V3XUV*)v3x_mallocopy(fb->uvTab[j], fb->numEdges * sizeof(V3XUV));
            }
        }
        if (mat->info.Perspective)
        fa->ZTab = (V3XWPTS*)MM_heap.malloc(fb->numEdges*sizeof(V3XWPTS));
    }
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void V3XMesh_Release(V3XMESH *obj)
*
* DESCRIPTION :
*
*/
void V3XMesh_Release(V3XMESH *obj)
{
    unsigned j;
	if (!obj) return;
    MM_heap.free(obj->vertex);
    MM_heap.free(obj->uv);
    MM_heap.free(obj->normal);
    for (j=0;j<obj->numMaterial;j++)
    {
        V3XMATERIAL *mat = obj->material+j;
        V3XMaterial_Release(mat, obj);
    }
    MM_heap.free(obj->material);
    if (obj->face)
    {
        V3XPOLY *fce;
        for (fce=obj->face, j=0;j<obj->numFaces;fce++, j++) V3XPoly_Release(fce);
        MM_heap.free(obj->face);
    }
    memset(obj, 0, sizeof(V3XMESH));
    MM_heap.free(obj);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  : void V3X_AllocEngine(void)
*
* DESCRIPTION :
*
*/
void V3XKernel_RenderClass(void)
{
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void v3x_NothingToAdd(void)
*
* DESCRIPTION :
*
*/
static void v3x_NothingToAdd(void)
{
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void v3x_NothingToDo(void *ovi)
*
* DESCRIPTION :
*
*/
static void v3x_NothingToDo(void *ovi)
{
    UNUSED(ovi);
    return;
}
int V3XKernel_Alloc(void)
{
    unsigned int k;
    V3X.Camera.focal = 600;
    V3X.Clip.Far = -CST_MAX;
    V3X.Clip.Near  = -1.f;
    V3X.Clip.Correction =  0.000005f;
    V3X.ViewPort.minVisibleRadius =  2.f/600;
    V3X.ViewPort.minTextureVisibleRadius = 4.f/600;
    V3X.Setup.MaxStartObjet =  32;
    V3X.Setup.pre_render =  v3x_NothingToAdd;
    V3X.Setup.post_render =  v3x_NothingToAdd;
    V3X.Setup.add_poly =  v3x_NothingToAdd;
    V3X.Setup.add_lights =  v3x_NothingToAdd;
    V3X.Setup.custom_ovi =  v3x_NothingToDo;
    if (!V3X.Buffer.MaxLight)
		V3X.Buffer.MaxLight =   4;
    if (!V3X.Buffer.MaxTmpMaterials)
		V3X.Buffer.MaxTmpMaterials =   64;
    if (!V3X.Buffer.MaxFacesDisplay)
		V3X.Buffer.MaxFacesDisplay = 1024;
    if (!V3X.Buffer.MaxClippedFaces)
		V3X.Buffer.MaxClippedFaces =  512;
    if (!V3X.Buffer.MaxPointsPerMesh)
		V3X.Buffer.MaxPointsPerMesh= 1024;
    if (!V3X.Buffer.MaxEdges)
		V3X.Buffer.MaxEdges = 9;
    if (!V3X.Cache.numItems)
		V3X.Cache.numItems = 256;
    if (!V3X.Buffer.MaxSceneNodes)
		V3X.Buffer.MaxSceneNodes = 512;

    k = V3X.Buffer.MaxFacesDisplay;
    V3X.Cache.item = V3X_CALLOC(V3X.Cache.numItems, V3XRESOURCE_ITEM);
    V3X.Buffer.RenderedFaces = (V3XPOLY**)     MM_heap.malloc((k+1)*sizeof(V3XPOLY*));
    V3X.Buffer.Mat = (V3XMATERIAL*)  MM_heap.malloc((V3X.Buffer.MaxTmpMaterials    +1)*sizeof(V3XMATERIAL));
    V3X.Buffer.ClippedFaces = (V3XPOLY*)      MM_heap.malloc((V3X.Buffer.MaxClippedFaces+1)*sizeof(V3XPOLY));
    V3X.Light.light = (V3XLIGHT*)     MM_heap.malloc((V3X.Buffer.MaxLight       +1)*sizeof(V3XLIGHT));
    SYS_ASSERT(V3X.Buffer.ClippedFaces);

    for (k=0;k<=V3X.Buffer.MaxClippedFaces;k++)
    {
        V3XPoly_Alloc( V3X.Buffer.ClippedFaces + k, V3X.Buffer.MaxEdges );    // Nombres de pts par objets
    }
    k = V3X.Buffer.MaxPointsPerMesh;
    V3X.Buffer.rot_vertex = (V3XVECTOR*) MM_heap.malloc(k*sizeof(V3XVECTOR));
    V3X.Buffer.prj_vertex = (V3XVECTOR*) MM_heap.malloc(k*sizeof(V3XVECTOR));
    V3X.Buffer.uv_vertex = (V3XUV*)     MM_heap.malloc(k*sizeof(V3XUV));
    V3X.Buffer.flag = (u_int8_t*)     MM_heap.malloc(k);
    V3X.Buffer.shade = (V3XSCALAR*)  MM_heap.malloc(k*sizeof(V3XSCALAR));
    V3X.Buffer.OVI = (u_int8_t**)    MM_heap.malloc((V3X.Buffer.MaxSceneNodes+1)*sizeof(u_int8_t*));
    if (!V3X.Ln.maxLines)
		V3X.Ln.maxLines = 4;
    V3X.Ln.lineBuffer = (V3XVECTOR*)MM_heap.malloc(V3X.Ln.maxLines * sizeof (V3XVECTOR) );
    V3X.Ln.lineColor = (rgb32_t*)MM_heap.malloc(V3X.Ln.maxLines * sizeof (rgb32_t) );

    return 0;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void V3XKernel_Release(void)
*
* DESCRIPTION :
*
*/
void V3XKernel_Release(void)
{
    unsigned int k;
	if (!TRG_Table)
		return;
    MM_heap.free(V3X.Ln.lineColor);
    MM_heap.free(V3X.Ln.lineBuffer);
    MM_heap.free(V3X.Buffer.OVI);
    MM_heap.free(V3X.Buffer.shade);
    MM_heap.free(V3X.Buffer.uv_vertex);
    MM_heap.free(V3X.Buffer.prj_vertex);
    MM_heap.free(V3X.Buffer.rot_vertex);

	if (V3X.Buffer.MaxClippedFaces)
		for (k=0;k<=V3X.Buffer.MaxClippedFaces;k++)
			V3XPoly_Release( V3X.Buffer.ClippedFaces + k);
    MM_heap.free(V3X.Light.light);
    MM_heap.free(V3X.Buffer.ClippedFaces);
    MM_heap.free(V3X.Buffer.Mat);
    MM_heap.free(V3X.Buffer.RenderedFaces);
    MM_heap.free(V3X.Cache.item);
    MM_heap.free(V3X.Buffer.flag);
    if (TRG_Table)
		free(TRG_Table);
	TRG_Table = 0;
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :
*
* DESCRIPTION :
*
*/
static void V3XPoly_Alloc(V3XPOLY *f, int som)
{
    f->numEdges = (u_int8_t) som;
    f->visible = 1;
    f->dispTab = V3X_CALLOC(f->numEdges, V3XPTS);
    f->uvTab = V3X_CALLOC(V3X_MAXTMU, V3XUV*);
    f->uvTab[0] = V3X_CALLOC(f->numEdges, V3XUV);
    f->uvTab[1] = V3X_CALLOC(f->numEdges, V3XUV);
    f->shade = V3X_CALLOC(f->numEdges, V3XSCALAR);
    f->faceTab = V3X_CALLOC(f->numEdges, u_int32_t);
    f->ZTab = V3X_CALLOC(f->numEdges, V3XWPTS);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void V3XPoly_Release(V3XPOLY *f)
*
* DESCRIPTION :
*
*/
static void V3XPoly_Release(V3XPOLY *f)
{
	SYS_ASSERT(f);
    MM_heap.free(f->ZTab);
    MM_heap.free(f->faceTab);
    MM_heap.free(f->shade);
    if (f->uvTab)
    {
        MM_heap.free(f->uvTab[0]);
        MM_heap.free(f->uvTab[1]);
    }
    MM_heap.free(f->uvTab);
    MM_heap.free(f->dispTab);
    return;
}

/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void V3XPoly_SpriteZoom(V3XPOLY *f, GXSPRITE *sp, V3XVECTOR *p, V3XSCALAR lx, V3XSCALAR ly)
*
* DESCRIPTION :
*
*/
void V3XPoly_SpriteZoom(V3XPOLY *f, GXSPRITE *sp, V3XVECTOR *p, V3XSCALAR lx, V3XSCALAR ly, int option)
{
    V3XPTS * pd = f->dispTab;
    V3XUV  * uv = f->uvTab[0];
    f->numEdges = 4;
    f->visible = 1;

    if (option&1)
    {
        pd[0].x = p->x-(lx/2);
        pd[0].y = p->y-(lx/2);
    }
    else
    {
        pd[0].x = p->x;
        pd[0].y = p->y;
    }

    pd[0].z = -p->z;

    pd[ 1].x = pd[0].x;    pd[1].y = pd[0].y+ly; pd[1].z = pd[0].z;
    pd[ 2].x = pd[0].x+lx; pd[2].y = pd[1].y;    pd[2].z = pd[0].z;
    pd[ 3].x = pd[2].x;    pd[3].y = pd[0].y;    pd[3].z = pd[0].z;
    uv[ 0].u = 0;
    uv[ 0].v = 0;
    uv[ 1].u = uv[ 0].u;
    uv[ 1].v = 1.f;
    uv[ 2].u = 1.f;
    uv[ 2].v = uv[ 1].v;
    uv[ 3].u = uv[ 2].u;
    uv[ 3].v = uv[ 0].v;

    f->distance = p->z + p->z;
    return;
}
// 2 nouvelles fonctions
