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
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include "_rlx32.h"
#include "_rlx.h"
#include "sysresmx.h"
#include "systools.h"
#include "gx_struc.h"
#include "gx_csp.h"
#include "gx_flc.h"
#include "gx_tools.h"
#include "gx_init.h"
#include "gx_rgb.h"
#include "v3xdefs.h"
#include "v3x_1.h"
#include "v3x_2.h"
#include "v3xtrig.h"
#include "v3xcoll.h"
#include "v3xmaps.h"
#include "v3xrend.h"
#include "v3xscene.h"

#include "v3x_old.h"

/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void V3x_Create_ORI(V3XORI *ORI, int i)
*
* DESCRIPTION :
*
*/
void RLXAPI static V3x_Create_ORI(V3XORI *ORI, int i)
{
    memset(ORI, 0, sizeof(V3XORI));
    sprintf(ORI->name, "object.%d", i);
    ORI->type = V3XOBJ_NONE;
    ORI->node = NULL;
    ORI->sub_class = 0;
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void V3x_Create_OVI(V3XOVI *OVI, int i)
*
* DESCRIPTION :
*
*/
void RLXAPI static V3x_Create_OVI(V3XOVI *OVI, int al)
{
    memset(OVI, 0, sizeof(V3XOVI));
    if (al)
    {
        OVI->node = (V3XNODE*)MM_heap.malloc(sizeof(V3XNODE));
        OVI->Tk = &OVI->node->Tk;
        OVI->Tk->vinfo.target.x = CST_ONE;
        OVI->Tk->vinfo.target.y = CST_ONE;
        OVI->Tk->vinfo.target.z = CST_ONE;
    }
    OVI->state|=V3XSTATE_VALIDPOINTER;
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  V3XOVI RLXAPI *V3XScene_OVI_GetByName(V3XSCENE *pScene, char *name)
*
* DESCRIPTION :
*
*/
V3XOVI RLXAPI *V3XScene_OVI_GetByName(V3XSCENE *pScene, const char *name)
{
    int i, f=0;
    V3XOVI *OVI, *OVIf=NULL;
    for ( i=pScene->numOVI, OVI=pScene->OVI; (i!=0)&&(f!=1); OVI++, i--)
    {
        V3XORI *ORI = (OVI->index_ORI<0xffff) ? pScene->ORI + OVI->index_ORI : OVI->ORI;
        if (ORI)
        {

            if (SDL_strcasecmp(ORI->name, name)==0)
            {
                f = 1;
                OVIf = OVI;
            }
        }
    }
    return OVIf;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void RLXAPI V3XScene_Camera_Select(V3XOVI *OVI)
*
* DESCRIPTION :
*
*/
void RLXAPI V3XScene_Camera_Select(V3XOVI *OVI)
{
    if (OVI==NULL) return;

    V3X.Camera.matrix_Method = V3XMATRIX_Vector;

    V3X.Camera.Tk = OVI->mesh->Tk;
    V3XMatrix_MeshTransform(&V3X.Camera);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  V3XOVI RLXAPI *V3XScene_Camera_GetByName(V3XSCENE *pScene, char *name)
*
* DESCRIPTION :
*
*/
V3XOVI RLXAPI *V3XScene_Camera_GetByName(V3XSCENE *pScene, const char *name)
{
    int i;
    V3XOVI *OVI, *OVIf=NULL;
    for (i=pScene->numOVI, OVI=pScene->OVI;i!=0;OVI++, i--)
    {
        if ((OVI->ORI)&&(OVI->ORI->type==V3XOBJ_CAMERA))
        {
            if (SDL_strcasecmp(OVI->ORI->name, name)==0)
            {
                OVI->state&=~V3XSTATE_HIDDEN;
                OVIf = OVI;
            }
            else
            OVI->state|=V3XSTATE_HIDDEN;
        }
    }
    return OVIf;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void RLXAPI V3XScene_CRC_Check(V3XSCENE* pScene)
*
* Description :
*
*/
void RLXAPI V3XScene_CRC_Check(V3XSCENE* pScene)
{
    V3XOVI *OVI;
    V3XORI *nORI, *cORI;
    int i;
    for (OVI=pScene->OVI, i=0;i<pScene->numOVI;i++, OVI++)
    {
        if ((i)&&(OVI->ORI == pScene->ORI)) OVI->ORI = NULL;
        nORI = OVI->ORI;
        if (nORI)
        if (nORI->type!=V3XOBJ_NONE)
        {
            if (OVI->index_INSTANCE>0)
            {
                cORI = pScene->ORI + OVI->index_INSTANCE;
				if (cORI!=nORI)
				{
					nORI->mesh = (V3XMESH*) MM_heap.malloc(sizeof(V3XNODE));
					if (nORI->type == V3XOBJ_MESH)
					{
						if (cORI->mesh)
	                    V3XMesh_Duplicate(nORI->mesh, cORI->mesh);
						nORI->flags|=V3XORI_DUPLICATED;
		            }
					else
					{
	                    *nORI->node = *cORI->node;
						nORI->flags|=V3XORI_DUPLICATED;
					}
				}
            }
            // Calcul Matrice
            switch( nORI->type) {
                case V3XOBJ_LIGHT:
				{
					nORI->light->Tk = *OVI->Tk;
					nORI->light->pos = OVI->Tk->vinfo.pos;
					*OVI->light = *nORI->light;
				}
                break;
                case V3XOBJ_MESH:
                {
                    V3XMATRIX pMat = OVI->mesh->matrix;
                    V3XKEY Tk = OVI->mesh->Tk;
                    if ((OVI->state&V3XSTATE_INSTANCED)&&(!OVI->index_INSTANCE))
                    {
                        V3XMesh_Duplicate(OVI->mesh, nORI->mesh);
                    }
                    else
					{
						if (nORI->mesh)
							*OVI->mesh = *nORI->mesh;
					}
                    if ((OVI->state&V3XSTATE_INSTANCED)==0)
                    {
                        OVI->mesh->matrix = pMat;
                        OVI->mesh->Tk = Tk;
                    }
                }
                break;
            }
        }
    }
    if (!pScene->Layer.lt.palette.lut) pScene->Layer.lt.palette.lut = (rgb24_t*) MM_heap.malloc(768);
    return;
}
/* 1 */
void RLXAPI V3XOVI_BuildChildren(V3XOVI *OVI, V3XSCENE *pScene)
{
    int i, n=0, j=0;
    for (i=0;i<pScene->numOVI;i++)
    {
        if (pScene->OVI[i].parent == OVI) n++;
    }
    if (OVI->child) MM_heap.free(OVI->child);
    OVI->child = V3X_CALLOC(n+1, V3XOVI*);
    for (i=0;i<pScene->numOVI;i++)
    {
        if (pScene->OVI[i].parent == OVI)
        {
            OVI->child[j] = pScene->OVI + i;
            j++;
        }
    }
    OVI->child[n]=NULL;
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void RLXAPI V3XScene_Validate(V3XSCENE* pScene)
*
* DESCRIPTION :
*
*/
void RLXAPI V3XScene_Validate(V3XSCENE* pScene)
{
    V3XTVI *TVI;
    V3XTRI *TRI;
    V3XOVI *OVI;
    unsigned i;
    for (OVI=pScene->OVI, i=0;i<pScene->numOVI;i++, OVI++)
    {
        if ((OVI->state&V3XSTATE_VALIDPOINTER)==0)
        {
			SYS_ASSERT(OVI->index_ORI>=0 && OVI->index_ORI<pScene->numORI);
            OVI->ORI = pScene->ORI + OVI->index_ORI;

			if (OVI->index_TVI)
            {
				SYS_ASSERT(OVI->index_TVI>=0 && OVI->index_TVI<=pScene->numTVI);
                TVI = pScene->TVI + OVI->index_TVI;
                if (TVI->index_TRI < pScene->numTRI)
                {
                    TRI = pScene->TRI + TVI->index_TRI;

					if (0!=TRI->index_NEXT)
						TRI->next = pScene->TRI + TRI->index_NEXT;

					if (0!=TRI->index_CHAIN)
						TRI->chain = pScene->TRI + TRI->index_CHAIN;

					TVI->TRI = TRI;
                    OVI->TVI = TVI;
                }
                else
                {
                    OVI->TVI = NULL;
                }
            }
            else
            {
                OVI->TVI = NULL;
            }
            if (0!=OVI->index_NEXT)
			{
				SYS_ASSERT(OVI->index_NEXT>=0 && OVI->index_NEXT<pScene->numOVI);
				OVI->next = pScene->OVI + OVI->index_NEXT;
			}
		}
        OVI->Tk = &OVI->mesh->Tk;

    }
    for (OVI=pScene->OVI, i=0;i<pScene->numOVI;i++, OVI++)
    {
        if ((OVI->state&V3XSTATE_VALIDPOINTER)==0)
        {
            unsigned p;
            p = OVI->index_PARENT;
            if ((p>0)&&(p<0xffff)&&(p!=i))
            {
                OVI->parent = pScene->OVI + p;
            }
            else OVI->parent = NULL;
            p = OVI->index_TARGET;
            if ((p>0)&&(p<0xffff)&&(p!=i))
            {
                OVI->target = pScene->OVI + p;
            }
            else OVI->target = NULL;
        }
    }
    for (OVI=pScene->OVI, i=0;i<pScene->numOVI;i++, OVI++)
    {
        if (!OVI->child) V3XOVI_BuildChildren(OVI, pScene);
        OVI->state|=V3XSTATE_VALIDPOINTER;
    }
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  : void Destroy_ORI(V3XSCENE *pScene, int i)
*
* DESCRIPTION :
*
*/
void RLXAPI static v3x_freeMorph3D(V3XTWEEN *mo)
{
    unsigned int i;
    for (i=0;i<mo->numFrames;i++) MM_heap.free(mo->frame[i].vertex);
    MM_heap.free(mo->frame);
    MM_heap.free(mo);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void V3XLight_Release(V3XLIGHT *light)
*
* Description :
*
*/
static void V3XLight_Release(V3XLIGHT *light)
{
    if ((light->material)&&(light->flags&V3XLIGHTCAPS_LENZFLARE))
    {
        V3XMaterial_Release(light->material, NULL);
        MM_heap.free(light->material);
    }
    memset(light, 0, sizeof(V3XLIGHT));
    MM_heap.free(light);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void v3x_Destroy_ORI(V3XSCENE *pScene, int i)
*
* DESCRIPTION :
*
*/
void RLXAPI static v3x_Destroy_ORI(V3XSCENE *pScene, int i)
{
    V3XORI *ORI = pScene->ORI + i;
    switch(ORI->type) {
        case V3XOBJ_LIGHT:
        if (ORI->light)
		{
			if ((ORI->flags&V3XORI_DUPLICATED)==0)
			{
				V3XLight_Release(ORI->light);
			}
			else
			{
				MM_heap.free(ORI->light);
			}
		}
        break;
        case V3XOBJ_VIEWPORT:
        case V3XOBJ_DUMMY:
        MM_heap.free(ORI->mesh);
        break;
        case V3XOBJ_MESH:
        if (ORI->mesh)
        {

            if ((ORI->flags&V3XORI_DUPLICATED)==0)
            {

                V3XMesh_Release(ORI->mesh);
            }
			else
			{

				V3XMesh_ReleaseDup(ORI->mesh);
			}
        }
        break;
    }
    ORI->node = NULL;
    if ((ORI->flags & V3XORI_CSDUPLICATED)==0)
    {
        if (ORI->Cs) V3XCL_Release(ORI->Cs);
    }
    if (ORI->morph) v3x_freeMorph3D(ORI->morph);
    V3x_Create_ORI(ORI, i);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void v3x_Destroy_TRI(V3XSCENE *pScene, int i)
*
* DESCRIPTION :
*
*/
void RLXAPI static v3x_Destroy_TRI(V3XSCENE *pScene, int i)
{
    V3XTRI *TRI = pScene->TRI + i;
    MM_heap.free(TRI->keys);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void v3x_Destroy_OVI(V3XSCENE *pScene, int mp)
*
* DESCRIPTION :
*
*/
void RLXAPI static v3x_Destroy_OVI(V3XSCENE *pScene, int mp)
{
    V3XOVI *OVI = pScene->OVI + mp;
    if (OVI->index_INSTANCE)
    {
		if (OVI->ORI)
        if ((OVI->ORI->type == V3XOBJ_MESH)&&(OVI->mesh)&&(OVI->ORI->mesh))
        {
            if (OVI->mesh->face!=OVI->ORI->mesh->face)
            {
                MM_heap.free(OVI->mesh->face);
                OVI->mesh->face = NULL;
            }
        }
    }
    if (OVI->child) MM_heap.free(OVI->child);
    if (OVI->mesh)
    if (OVI->mesh!=OVI->ORI->mesh) MM_heap.free(OVI->mesh);
    V3x_Create_OVI(OVI, FALSE);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void v3x_Destroy_TVI(V3XSCENE *pScene, int i)
*
* DESCRIPTION :
*
*/
static void RLXAPI v3x_Destroy_TVI(V3XSCENE *pScene, int i)
{
    pScene = pScene;
    i = i;
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  : V3XSCENE *load_V3XSCENE(char *mode, FlushClass *f);
*
* DESCRIPTION :
*
*/
static void v3xtx_load(V3XLAYER_CLITEM *item)
{
    if (item->filename[0])
    {
		char tex[256];
        sprintf(tex, "%s.png", item->filename);
        item->table = REALCOLOR_LoadFn(tex);
        REALCOLOR_Simply(item->table);
    }
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void v3xtx_free(V3XLAYER_CLITEM *item)
*
* DESCRIPTION :
*
*/
static void v3xtx_free(V3XLAYER_CLITEM *item)
{
    if (item->table)
    {
        REALCOLOR_Free(item->table);
        item->table = NULL;
    }
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void RLXAPI V3XScene_Release(V3XSCENE *pScene)
*
* DESCRIPTION :
*
*/
void RLXAPI V3XScene_Release(V3XSCENE *pScene)
{
    int i;
    V3XLAYER *layer = &pScene->Layer;
    if (pScene==NULL) return;
    for (i=0;i<pScene->numOVI;i++) v3x_Destroy_OVI(pScene, i);
    for (i=0;i<pScene->numTVI;i++) v3x_Destroy_TVI(pScene, i);
    for (i=0;i<pScene->numORI;i++)
    {

        v3x_Destroy_ORI(pScene, i);
    }
    for (i=0;i<pScene->numTRI;i++) v3x_Destroy_TRI(pScene, i);
    MM_heap.free(pScene->ORI);
    MM_heap.free(pScene->OVI);
    MM_heap.free(pScene->TRI);
    MM_heap.free(pScene->TVI);
    if (GX.View.BytePerPixel==1)
    {
        v3xtx_free(&layer->lt.alpha50);
        v3xtx_free(&layer->lt.additive);
        v3xtx_free(&layer->lt.blur);
        if (layer->lt.gouraud.table)
        {
            REALCOLOR_Free(layer->lt.gouraud.table);
            MM_heap.free(layer->lt.phong.table);
            layer->lt.gouraud.table = NULL;
        }
    }
    if (layer->bg.bitmap.handle) V3X_CSP_Unload(&layer->bg.bitmap);
    if (layer->lt.palette.table)
    MM_heap.free(layer->lt.palette.table);
    MM_heap.free(pScene);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  : static void V3x_GetRenderBackground(V3XSCENE *pScene, char *res)
*
* DESCRIPTION :
*
*/
static void RLXAPI V3x_GetRenderBackground(V3XSCENE *pScene)
{
    V3XLAYER *layer = &pScene->Layer;
    if (FIO_cur->exists(layer->bg.filename))
    {
        layer->bg.bitmap.handle = &layer->bg._bitmap;
        V3X_CSP_GetFn(layer->bg.filename, &layer->bg.bitmap, 1);
    }
    else
    {
        layer->bg.bitmap.handle = NULL;
        switch(layer->bg.flags&(0x10-1)) {
            case V3XBG_IMG:
            layer->bg.flags&=~V3XBG_IMG;
            layer->bg.flags|= V3XBG_COLOR;
            break;
        }
    }
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void V3XScene_LoadTextures(V3XSCENE *pScene, void (*callback)(void *))
*
* DESCRIPTION :
*
*/
void V3XScene_LoadTextures(V3XSCENE *pScene, void (*callback)(void *))
{
    int i, x, m=MM_heap.active;
    V3XLAYER *layer = &pScene->Layer;
    V3XORI    *ORI;
    V3X.Setup.warnings &=~ V3XWARN_NOENOUGHSurfaces;
    /*
    *  Palette
    */
    if (FIO_cur->exists(layer->lt.palette.filename))
    {
        ACT_LoadFn(layer->lt.palette.lut, layer->lt.palette.filename);
        GX.ColorClut = layer->lt.palette.lut;
    }
    else
    {
        GX.ColorClut = GX.ColorTable;
    }
    /*
    BackGround
    */
    switch(layer->bg.flags&15) {
        case V3XBG_BLACK:
        case V3XBG_NONE:
        break;
        case V3XBG_COLOR:
        layer->bg.index_color = (GX.View.BytePerPixel==1)
        ? RGB_findNearestColor((rgb24_t*)&layer->bg.BG_color, GX.ColorClut)
        : RGB_PixelFormat(layer->bg.BG_color.r, layer->bg.BG_color.g, layer->bg.BG_color.b);
        break;
        case V3XBG_IMG:
        V3x_GetRenderBackground(pScene);
        break;
        default:
        break;
    }
    /*
    Charge Les Maps
    */
    for (i=0, ORI=pScene->ORI;i<pScene->numORI;i++, ORI++)
    {
        switch(ORI->type) {
            case V3XOBJ_MESH:
			{
				if (ORI->mesh)
				V3XMaterials_LoadFromMesh(ORI->mesh);
			}
			break;
            case V3XOBJ_LIGHT:
            {
                if (ORI->light->flare)
                {
                    V3XSPRITE *lt = (V3XSPRITE *)ORI->light->flare;
                    V3XMaterial_LoadTextures(&lt->_sp.mat);
                    V3X_CSP_Prepare(&lt->sp, 0x20);
                }
            }
            break;
        }
    }

    /*
    *   Charge les Tables realColor
    */
    if (!(V3X.Setup.flags&V3XOPTION_TRUECOLOR))
    {
        if( GX.View.BytePerPixel==1)
        {
            v3xtx_load(&layer->lt.alpha50);
            v3xtx_load(&layer->lt.additive);
            v3xtx_load(&layer->lt.blur);
            v3xtx_load(&layer->lt.gouraud);
            if (layer->lt.gouraud.table)
            {
                if (layer->lt.shift)
					REALCOLOR_Reduce(layer->lt.gouraud.table, layer->lt.shift);
                layer->lt.phong.table = (u_int8_t**) MM_heap.malloc(256*sizeof(u_int8_t*));
                for (i=0;i<128;i++)
                {
                    x = (int)sin16(i<<3);
                    x = MULF32_SQR(MULF32_SQR(MULF32_SQR(x)));
                    layer->lt.phong.table[i]=layer->lt.gouraud.table[x>>8];
                }
            }
        }
    }
    GX.ColorClut=NULL;
    V3XViewport_Setup(&V3X.Camera, GX.View);
    V3XMatrix_MeshTransform(&V3X.Camera);
    UNUSED(callback);
    MM_heap.active = m;
    return;
}

/*------------------------------------------------------------------------
*
* PROTOTYPE  : V3XSCENE *load_V3XSCENE(char *mode, FlushClass *f);
*
* DESCRIPTION :
*
*/
static void RLXAPI *v3x_read_alloc(int32_t sz, int32_t n, int32_t n2, SYS_FILEHANDLE in)
{
    u_int8_t *tmp;
    int s;
    if (n2<n)
		n2 = n;
    s = sz * n2;
    if (!s)
		return NULL;
	tmp = (u_int8_t*)MM_heap.malloc(s);
    s = FIO_gzip.fread(tmp, sz, n, in);
	SYS_ASSERT(n == s);
    return tmp;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  V3XMATERIAL *V3XMaterials_GetFp(SYS_FILEHANDLE in, int mt)
*
* DESCRIPTION :
*
*/

#ifdef __BIG_ENDIAN__
static u_int32_t BGETFIELD(u_int32_t bf, int base, int length)
{
	return (bf >> base) & (( 1<<length ) - 1);
}
#endif

V3XMATERIAL *V3XMaterials_GetFp(SYS_FILEHANDLE in, int numMaterial)
{
    V3XMATERIAL *Mat;
    u_int32_t *raw;
    int i;
	SYS_ASSERT(numMaterial < 255);

    /* Unfortunately we cannot directly read the material struct from disk as
       it contains (not used on disk) pointers, which on disk are 32 bit, but
       may in reality be different (64 bits) see v3x_VMX_unpack_morph3D() for
       a simpler example of the same problem. */
    raw = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), numMaterial * 32, -1, in);
    Mat = (V3XMATERIAL*)MM_heap.malloc(numMaterial * sizeof(V3XMATERIAL));

    /* copy the raw data to the material structs */
    for (i=0;i<numMaterial;i++)
    {
      /* copy everything up to the textures */
      memcpy(&Mat[i], raw, 14 * sizeof(u_int32_t));
      /* each texture first has 2 ints, then 2 pointers */
      memcpy(&Mat[i].texture[0], raw + 14, 2 * sizeof(u_int32_t));
      Mat[i].texture[0].data = (u_int8_t *)(uintptr_t)raw[16];
      Mat[i].texture[0].handle = (void *)(uintptr_t)raw[17];
      memcpy(&Mat[i].texture[1], raw + 18, 2 * sizeof(u_int32_t));
      Mat[i].texture[1].data = (u_int8_t *)(uintptr_t)raw[20];
      Mat[i].texture[1].handle = (void *)(uintptr_t)raw[21];
      Mat[i].reserved[0] = (void *)(uintptr_t)raw[22];
      Mat[i].reserved[1] = (void *)(uintptr_t)raw[23];
      Mat[i].fli = (struct _fli_struct *)(uintptr_t)raw[24];
      Mat[i].render_clip = (void *)(uintptr_t)raw[25];
      Mat[i].lod_near = raw[26];
      Mat[i].lod_far  = raw[27];
      /* copy the rest of the non ptr data starting at ambient */
      memcpy(&Mat[i].ambient, raw + 28, 4 * sizeof(u_int32_t));
      raw += 32;
    }
    raw -= numMaterial * 32;
    MM_heap.free(raw);

#ifdef __BIG_ENDIAN__
	{
    V3XMATERIAL *pMat = Mat;
    for (i=0;i<numMaterial;i++, pMat++)
    {
        u_int32_t info = pMat->lod;
		BSWAP32(&info, 1);
        pMat->info.TwoSide = BGETFIELD(info, 0, 1);
        pMat->info.Opacity = BGETFIELD(info, 1, 1);
        pMat->info.Perspective = BGETFIELD(info, 2, 1);
        pMat->info.Filtering = BGETFIELD(info, 3, 1);

        pMat->info.Texturized = BGETFIELD(info, 4, 2);
        pMat->info.Transparency = BGETFIELD(info, 6, 2);
        pMat->info.Shade = BGETFIELD(info, 8, 2);
		pMat->info.Sprite = BGETFIELD(info, 10, 2);

		pMat->info.Environment = BGETFIELD(info, 12, 4);

		pMat->info.Dynamic = BGETFIELD(info, 16, 1);
		pMat->info.AlphaLight = BGETFIELD(info, 15, 1);
		pMat->info.AlphaLight = BGETFIELD(info, 16, 1);
		pMat->info.Transparency2 = BGETFIELD(info, 17, 3);
		pMat->info.MultiPassBlend = BGETFIELD(info, 20, 2);

	}
	}
#endif
    return Mat;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  V3XMESH static RLXAPI *v3x_VMX_unpack_object(SYS_FILEHANDLE in)
*
* DESCRIPTION :
*
*/
static void V3XRGB_ConvertToMono(V3XSCALAR *mono, rgb32_t *rgb, unsigned n)
{
    for (;n!=0L;mono++, rgb++, n--) *mono = (V3XSCALAR)RGB_ToGray(rgb->r, rgb->g, rgb->b);
}

static void v3x_raw_to_mesh(u_int32_t *raw, V3XMESH *obj)
{
    /* copy the raw data to the in memory mesh struct */
    /* even though the first 6 items are pointers they contain relevant info */
    obj->vertex       = (V3XVECTOR *)  (uintptr_t)raw[0];
    obj->face         = (V3XPOLY *)    (uintptr_t)raw[1];
    obj->uv           = (V3XUV *)      (uintptr_t)raw[2];
    obj->normal       = (V3XVECTOR *)  (uintptr_t)raw[3];
    obj->normal_face  = (V3XVECTOR *)  (uintptr_t)raw[4];
    obj->material     = (V3XMATERIAL *)(uintptr_t)raw[5];
    memcpy(&obj->matrix, raw + 6, 27 * sizeof(u_int32_t));
    obj->rgb = (rgb32_t *)(uintptr_t)raw[33];
    memcpy(&obj->radius, raw + 34, 2 * sizeof(u_int32_t));
}

/*------------------------------------------------------------------------
*
* PROTOTYPE  :  V3XMESH static RLXAPI *v3x_VMX_unpack_object(SYS_FILEHANDLE in)
*
* Description :
*
*/
V3XNODE static RLXAPI *v3x_VMX_unpack_node(SYS_FILEHANDLE in)
{
    V3XMESH *obj;
    /* Unfortunately we cannot directly read the MESH struct from disk as
       it contains (not used on disk) pointers, which on disk are 32 bit, but
       may in reality be different (64 bits) see v3x_VMX_unpack_morph3D() for
       a simpler example of the same problem. */
    u_int32_t *raw = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), 36, -1, in);
    obj =(V3XMESH*)MM_heap.malloc(sizeof(V3XMESH));
    v3x_raw_to_mesh(raw, obj);
    MM_heap.free(raw);

#ifdef __BIG_ENDIAN__
    BSWAP32((u_int32_t*)&obj->matrix, 12);
	BSWAP32((u_int32_t*)&obj->Tk, 3+3+1);
#endif

	return (V3XNODE*)obj;
}

static V3XMESH RLXAPI *v3x_VMX_unpack_object(SYS_FILEHANDLE in)
{
    V3XMESH *obj;
    int i;
    V3XPOLY *f;

    /* Unfortunately we cannot directly read the MESH struct from disk as
       it contains (not used on disk) pointers, which on disk are 32 bit, but
       may in reality be different (64 bits) see v3x_VMX_unpack_morph3D() for
       a simpler example of the same problem. */
    u_int32_t *raw = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), 36, -1, in);
    obj =(V3XMESH*)MM_heap.malloc(sizeof(V3XMESH));
    v3x_raw_to_mesh(raw, obj);
    MM_heap.free(raw);

#ifdef __BIG_ENDIAN__
    BSWAP16((u_int16_t*)&obj->numVerts, 4);
    BSWAP32((u_int32_t *)&obj->flags, 1);
    BSWAP32((u_int32_t *)&obj->scale, 1);
    BSWAP32((u_int32_t*)&obj->matrix, 12);
	BSWAP32((u_int32_t*)&obj->Tk, 3+3+1);
#endif
    if (obj->numVerts)
    {
        obj->vertex = (V3XVECTOR*)v3x_read_alloc(sizeof(V3XVECTOR), obj->numVerts, -1, in);
#ifdef __BIG_ENDIAN__
        BSWAP32((u_int32_t*)obj->vertex , obj->numVerts*3);
#endif
        /* Unfortunately we cannot directly read the POLY struct from disk as
           it contains (not used on disk) pointers, which on disk are 32 bit, but
           may in reality be different (64 bits) see v3x_VMX_unpack_morph3D() for
           a simpler example of the same problem. */
        raw = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), 8 * obj->numFaces, -1, in);
        obj->face =(V3XPOLY*)MM_heap.malloc(sizeof(V3XPOLY) * obj->numFaces);
        /* copy the raw data to the in memory poly struct */
        for (i = 0; i < obj->numFaces; i++)
        {
            u_int32_t *raw_poly = raw + i * 8;
            V3XPOLY *poly = &obj->face[i];
            poly->matIndex = raw_poly[0];
            poly->faceTab  = (u_int32_t *)(uintptr_t)raw_poly[1];
            poly->dispTab  = (V3XPTS *)(uintptr_t)raw_poly[2];
            poly->uvTab    = (V3XUV **)(uintptr_t)raw_poly[3];
            memcpy(&poly->distance, raw_poly + 4, sizeof(u_int32_t));
            poly->rgb      = (rgb32_t*)(uintptr_t)raw_poly[5];
            poly->ZTab     = (V3XWPTS*)(uintptr_t)raw_poly[6];
            memcpy(&poly->numEdges, raw_poly + 7, sizeof(u_int32_t));
        }
        MM_heap.free(raw);

        if (obj->uv)
        {
            obj->uv =(V3XUV*) v3x_read_alloc(sizeof(V3XUV), obj->numVerts, -1, in);
#ifdef __BIG_ENDIAN__
			BSWAP32((u_int32_t*)obj->uv, obj->numVerts*2);
#endif
        }
        if (obj->normal)
        {
            obj->normal =(V3XVECTOR*)v3x_read_alloc(sizeof(V3XVECTOR), obj->numVerts, -1, in);
#ifdef __BIG_ENDIAN__
			BSWAP32((u_int32_t*)obj->normal, obj->numVerts*3);
#endif
        }
        if (obj->flags&V3XMESH_HASSHADETABLE)
        {
            unsigned nb = obj->flags&V3XMESH_FLATSHADE ? obj->numFaces : obj->numVerts;
            obj->rgb = (rgb32_t*)v3x_read_alloc(sizeof(rgb32_t), nb, -1, in);
#ifdef __BIG_ENDIAN__
            BSWAP32((u_int32_t*)obj->rgb, nb);
#endif
            if ((V3X.Client->Capabilities&GXSPEC_RGBLIGHTING)==0)
            V3XRGB_ConvertToMono(obj->shade, obj->rgb, nb);
        }
        else
			obj->rgb = NULL;
        if ((V3X.Setup.flags&V3XOPTION_97)||(!obj->scale))
		{
			obj->scale = CST_ONE;
		}

        obj->normal_face = (V3XVECTOR*)v3x_read_alloc(sizeof(V3XVECTOR), obj->numFaces, -1, in);
#ifdef __BIG_ENDIAN__
        BSWAP32((u_int32_t*)obj->normal_face, obj->numFaces*3);
#endif
        obj->material = V3XMaterials_GetFp(in, obj->numMaterial);

        for (f=obj->face, i=obj->numFaces;i!=0;f++, i--)
        {
            V3XMATERIAL *pMat;
#ifdef __BIG_ENDIAN__
            BSWAP32((u_int32_t*)&f->matIndex, 1);
#endif
            f->matIndex--;
			SYS_ASSERT(!((f->matIndex<0)||(f->matIndex>=obj->numMaterial)));
            f->Mat = obj->material + f->matIndex ;
            pMat  = (V3XMATERIAL*) f->Mat;

			if ((V3X.Client->Capabilities&GXSPEC_ENABLEPERSPECTIVE)&&(!pMat->info.Sprite)&&(pMat->info.Texturized))
				pMat->info.Perspective=1;

			f->dispTab = (V3XPTS*)MM_heap.malloc(sizeof(V3XPTS)*f->numEdges);

			f->faceTab = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), f->numEdges, -1, in);
#ifdef __BIG_ENDIAN__
            BSWAP32((u_int32_t*)f->faceTab, f->numEdges);
#endif
            f->shade = (pMat->info.Shade)  ? (V3XSCALAR *) v3x_read_alloc(sizeof(V3XSCALAR) , f->numEdges, -1, in) : NULL;
            if ((f->shade)&&((V3X.Client->Capabilities&GXSPEC_RGBLIGHTING)==0))
				V3XRGB_ConvertToMono(f->shade, f->rgb, f->numEdges);

			if (pMat->info.Texturized)
            {
				unsigned j;
                unsigned n = pMat->info.Environment&V3XENVMAPTYPE_DOUBLE ? 2 : 1;
                f->uvTab = V3X_CALLOC(V3X_MAXTMU, V3XUV*);
                f->uvTab[0] = (V3XUV *) v3x_read_alloc(sizeof(V3XUV), f->numEdges, -1, in);
#ifdef __BIG_ENDIAN__
                BSWAP32((u_int32_t*)f->uvTab[0], 2 * f->numEdges);
#endif
				for (j=0;j<f->numEdges;j++)
				{
					f->uvTab[0][j].u/=255.f;
					f->uvTab[0][j].v/=255.f;
					SYS_ASSERT(f->faceTab[j]<obj->numVerts);
				}


                if (n>1)
					f->uvTab[1] = V3X_CALLOC(f->numEdges, V3XUV);
                else
					f->uvTab[1] = NULL;
            }
			else
				f->uvTab = NULL;
            f->ZTab = (pMat->info.Perspective) ? V3X_CALLOC(f->numEdges, V3XWPTS) : NULL;
        }
    }
    return obj;
}

/*------------------------------------------------------------------------
*
* PROTOTYPE  :  V3XLIGHT static RLXAPI *v3x_VMX_unpack_light(SYS_FILEHANDLE in)
*
* Description :
*
*/
V3XLIGHT static RLXAPI *v3x_VMX_unpack_light(SYS_FILEHANDLE in)
{
    V3XLIGHT *light;
    /* Unfortunately we cannot directly read the LIGHT struct from disk as
       it contains (not used on disk) pointers, which on disk are 32 bit, but
       may in reality be different (64 bits) see v3x_VMX_unpack_morph3D() for
       a simpler example of the same problem. */
    u_int32_t *raw = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), 36, -1, in);
    light =(V3XLIGHT*)MM_heap.malloc(sizeof(V3XLIGHT));

    /* copy the raw data to the in memory mesh struct */
    memcpy(light, raw, 17 * sizeof(u_int32_t));
    light->material = (V3XMATERIAL *)(uintptr_t)raw[17];
    memcpy(&light->flaresize, raw + 18, 2 * sizeof(u_int32_t));
    light->reserved[0] = (void *)(uintptr_t)raw[20];
    light->reserved[1] = (void *)(uintptr_t)raw[21];
    memcpy(&light->Tk, raw + 22, 14 * sizeof(u_int32_t));
    MM_heap.free(raw);

#ifdef __BIG_ENDIAN__
    BSWAP32((u_int32_t*)&light->pos, 16);
#endif
    if ((light->flags&V3XLIGHTCAPS_LENZFLARE)&&(light->material))
    {
        light->material = V3XMaterials_GetFp(in, 1);
    }
    else
    {
        light->material = NULL;
        light->flags&=~V3XLIGHTCAPS_LENZFLARE;
    }
	light->flags |= V3XLIGHTCAPS_RANGE;
    return light;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  V3XCAMERA static RLXAPI *v3x_VMX_unpack_camera(SYS_FILEHANDLE in)
*
* Description :
*
*/
V3XCAMERA static RLXAPI *v3x_VMX_unpack_camera(SYS_FILEHANDLE in)
{
    V3XCAMERA *camera;
    /* Unfortunately we cannot directly read the CAMERA struct from disk as
       we've added aditional padding to compensate for 64 bits pointers in
       other structs with which we are in a union. */
    u_int32_t *raw = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), 36, -1, in);
    camera =(V3XCAMERA*)MM_heap.malloc(sizeof(V3XCAMERA));
    memcpy(camera, raw, 6 * sizeof(u_int32_t));
    memcpy(&camera->M, raw + 6, 30 * sizeof(u_int32_t));
    MM_heap.free(raw);

#ifdef __BIG_ENDIAN__
    BSWAP32((u_int32_t*)&camera->M, 16);
#endif
    return camera;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  V3XCOLLISION static *v3x_VMX_unpack_collide(SYS_FILEHANDLE in)
*
* DESCRIPTION :
*
*/
V3XCL static RLXAPI *v3x_VMX_unpack_collide(SYS_FILEHANDLE in)
{
    V3XCL *Cs =(V3XCL*)MM_heap.malloc(sizeof(V3XCL));
    int i;

    /* Unfortunately we cannot directly read the collision struct from disk as
       it contains (not used on disk) pointers, which on disk are 32 bit, but
       may in reality be different (64 bits) see v3x_VMX_unpack_morph3D() for
       a simpler example of the same problem. */
    u_int32_t *raw = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), 22, -1, in);

    /* copy the raw data to the in memory struct */
    memcpy(Cs, raw, 2 * sizeof(u_int32_t));
    Cs->item = (V3XCL_ITEM *)(uintptr_t)raw[2];
    memcpy(&Cs->global, raw + 3, 12 * sizeof(u_int32_t)); /* sphere */
    Cs->mesh_ref = (V3XMESH *)(uintptr_t)raw[15];
    memcpy(&Cs->flags, raw + 16, 2 * sizeof(u_int32_t));
    memcpy(&Cs->old, raw + 18, sizeof(u_int32_t));
    memcpy(&Cs->hitCount, raw + 19, 2 * sizeof(u_int32_t));
    Cs->last_hit = (V3XCL_ITEM *)(uintptr_t)raw[21];
    MM_heap.free(raw);

#ifdef __BIG_ENDIAN__
    BSWAP32((u_int32_t*)Cs, 2);
#endif
    Cs->item = (V3XCL_ITEM*)MM_heap.malloc(sizeof(V3XCL_ITEM) * Cs->numItem);
    /* Unfortunately we cannot directly read the collision item struct from
       disk as it contains (not used on disk) pointers, which on disk are 32
       bit, buts may in reality be different (64 bits) */
    raw = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), 16 * Cs->numItem, -1, in);

    for (i = 0; i < Cs->numItem; i++)
    {
        V3XCL_ITEM *item = &Cs->item[i];
        u_int32_t *raw_item = raw + 16 * i;

        item->type = raw_item[0];
#ifdef __BIG_ENDIAN__
        BSWAP32((u_int32_t*)&item->type, 1);
#endif
        if (item->type != V3XCTYPE_MESH)
        {
            /* disk and memory layout identical */
            memcpy(((u_int32_t*)&item->type) + 1, raw_item + 1,
                   15 * sizeof(u_int32_t));
        }
        else
        {
            /* copy the raw data to the in memory struct */
            memcpy(&item->mesh.center, raw_item + 1, 7 * sizeof(u_int32_t));
            item->mesh.face = (V3XCL_FACE *)(uintptr_t)raw_item[8];
            item->mesh.sectorList = (u_int16_t *)(uintptr_t)raw_item[9];
            item->mesh.maxsectors = raw_item[10];
            item->mesh.mesh_ref = (V3XMESH *)(uintptr_t)raw_item[11];
            /* skip 4 ints of unused padding (not there when using
               64 bits pointers) */
        }
    }

#ifdef __BIG_ENDIAN__
    BSWAP32((u_int32_t*)&Cs->global, 9);
    {
        V3XCL_ITEM *item = Cs->item;
        for (i=Cs->numItem;i!=0;item++, i--)
        {
            BSWAP32(((u_int32_t*)&item->box.type) + 1 , 15);
        }
    }
#endif
    {
		if (Cs->old[0])
        {
            Cs->flags|=V3XCMODE_INVERTCONDITION;
        }
    }
    return Cs;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  V3XMorph static *v3x_VMX_unpack_morph3D(SYS_FILEHANDLE in)
*
* DESCRIPTION :
*
*/
V3XTWEEN static RLXAPI *v3x_VMX_unpack_morph3D(SYS_FILEHANDLE in)
{
    unsigned int i;
    /* The V3XTWEEN struct looks like this:

    typedef struct _v3x_morph{

     V3XTWEENFRAME *frame; // Morphing frame
     u_int32_t numFrames; // Number of frame
     u_int32_t numVerts; // Number of vertex
     u_int32_t numFaces; // Number of face
    }V3XTWEEN;

    Unfortunately we cannot read this directly from disk as the on disk
    format contains 32 bits (unused) frame pointers, and our pointers may
    have a different size, so instead we read 4 32 bit ints and copy the
    results to a V3XTWEEN structure */

    V3XTWEEN *Mo = (V3XTWEEN *)MM_heap.malloc(sizeof(V3XTWEEN));
    u_int32_t *raw = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), 4, -1, in);
#ifdef __BIG_ENDIAN__
    BSWAP32(raw + 1, 3);
#endif
    Mo->frame     = (V3XTWEENFRAME *)(uintptr_t)raw[0];
    Mo->numFrames = raw[1];
    Mo->numVerts  = raw[2];
    Mo->numFaces  = raw[3];
    MM_heap.free(raw);
    if ((!Mo->numFrames)||(!Mo->numVerts)) { MM_heap.free(Mo); return NULL; }
    Mo->frame = (V3XTWEENFRAME*) MM_heap.malloc(Mo->numFrames*sizeof(V3XTWEENFRAME));
    for (i=0;i<Mo->numFrames;i++)
    {
        Mo->frame[i].vertex = (V3XVECTOR*) v3x_read_alloc(sizeof(V3XVECTOR), Mo->numVerts, -1, in);
#ifdef __BIG_ENDIAN__
        BSWAP32((u_int32_t*)Mo->frame[i].vertex, Mo->numVerts*3);
#endif
    }
    return Mo;
}

/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void RLXAPI v3x_VMX_unpack_ORI(V3XORI *ORI, SYS_FILEHANDLE in)
*
* Description :
*
*/
static void RLXAPI v3x_VMX_unpack_ORI(V3XORI *ORI, SYS_FILEHANDLE in, int bFormat97)
{
	if (ORI->type!=V3XOBJ_NONE)
    {
        if (ORI->node)
        {
            switch(ORI->type) {
                case V3XOBJ_LIGHT:
					ORI->light = v3x_VMX_unpack_light(in);
				break;
                case V3XOBJ_CAMERA:
					ORI->camera = v3x_VMX_unpack_camera(in);
				break;
				case V3XOBJ_MESH:
					ORI->mesh = v3x_VMX_unpack_object(in);
				break;
				default:
					ORI->node = v3x_VMX_unpack_node(in);
				break;
            }
        }

        if (ORI->morph)
			ORI->morph = v3x_VMX_unpack_morph3D(in);

        if (ORI->Cs)
			ORI->Cs = v3x_VMX_unpack_collide(in);
    }
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void RLXAPI v3x_VMX_unpack_OVI(V3XOVI *OVI, SYS_FILEHANDLE in)
*
* DESCRIPTION :
*
*/
static void RLXAPI v3x_VMX_unpack_OVI(V3XOVI *OVI, SYS_FILEHANDLE in)
{
	OVI->node = v3x_VMX_unpack_node(in);
	return;
}

/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void RLXAPI v3x_VMX_unpack_TRI(V3XTRI *TRI, SYS_FILEHANDLE in)
*
* DESCRIPTION :
*
*/
static void RLXAPI v3x_VMX_unpack_TRI(u_int32_t *rawTRI, V3XTRI *TRI,
    SYS_FILEHANDLE in)
{
    /* Translate raw (on disk, pointers 32 bits, might be different in memory)
       format to our in memory format */
    TRI->keys = (V3XKEY *)(uintptr_t)rawTRI[0];
    TRI->index_NEXT = rawTRI[1];
    TRI->ORI_ref = (void *)(uintptr_t)rawTRI[2];
    TRI->index_CHAIN = rawTRI[3];
    memcpy(&TRI->pad, rawTRI + 4, 4 * sizeof(u_int32_t));

#ifdef __BIG_ENDIAN__
	BSWAP32((u_int32_t *)&TRI->index_CHAIN, 1);
	BSWAP16((u_int16_t*)&TRI->numFrames, 3);
#endif

    if (TRI->keys)
    {
        if (TRI->flags&V3XKF_KEYEX)
        {
            /* In this rare case  we can directly read the V3XKEYEX struct from disk as
               it does not contain pointers! */
            TRI->keyEx=(V3XKEYEX*)v3x_read_alloc(sizeof(V3XKEYEX), TRI->numKeys, -1, in);
#ifdef __BIG_ENDIAN__
            {
                int i;
                V3XKEYEX *kf = TRI->keyEx;
                for (i=TRI->numFrames;i!=0;i--, kf++)
                {
                    BSWAP32((u_int32_t*)kf, (sizeof(V3XKEY)>>2)+3);
                    BSWAP16((u_int16_t*)&kf->frame, 2);
                }
            }
#endif
        }
        else
        {
            /* In this rare case we can directly read the V3XKEY struct from
               disk as it does not contain pointers! */
            TRI->keys=(V3XKEY*)v3x_read_alloc(sizeof(V3XKEY), TRI->numFrames, -1, in);
#ifdef __BIG_ENDIAN__
            BSWAP32((u_int32_t*)TRI->keys, TRI->numFrames*7);
#endif
        }
    }
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void V3XLAYER_Read(V3XLAYER *layer, SYS_FILEHANDLE in)
*
* DESCRIPTION :
*
*/

static void v3xORI_Convert97(V3XSCENE *pScene, SYS_FILEHANDLE in)
{
    V3XORI97	ori97;
    V3XORI		*ori;
    u_int32_t *rawORIs = (u_int32_t *)malloc(pScene->numORI * 16 *
                                                    sizeof(u_int32_t));
    int i;

    /* Unfortunately we cannot directly read the struct from disk as
       it contains (not used on disk) pointers, which on disk are 32 bit, but
       may in reality be different (64 bits) */
    FIO_gzip.fread(rawORIs, pScene->numORI, 16 * sizeof(u_int32_t), in);
    pScene->ORI = MM_CALLOC(pScene->numORI, V3XORI);
    for (ori = pScene->ORI, i=0;i<pScene->numORI;i++, ori++)
    {
        const u_int8_t objTable[8] = { V3XOBJ_NONE, V3XOBJ_MESH, V3XOBJ_DUMMY, V3XOBJ_LIGHT, V3XOBJ_NONE, V3XOBJ_CAMERA, V3XOBJ_VIEWPORT};
        u_int32_t *rawORI = rawORIs + 16 * i;

        /* copy the raw data to the in memory structs */
        ori97.mesh  = (V3XMESH *) (uintptr_t)rawORI[0];
        ori97.morph = (V3XTWEEN *)(uintptr_t)rawORI[1];
        ori97.Cs    = (V3XCL *)   (uintptr_t)rawORI[2];
        ori97.data  = (void *)    (uintptr_t)rawORI[3];
        memcpy(&ori97.global_rayon, rawORI + 4, 12 * sizeof(u_int32_t));

#ifdef __BIG_ENDIAN__
        BSWAP32((u_int32_t* )&ori97.global_rayon, 1);
        BSWAP32((u_int32_t* )&ori97.global_pivot, 3);
        BSWAP16((u_int16_t*)&ori97.index_Parent, 1);
#endif
        ori->flags = 0;
        SDL_strlcpy(ori->name, ori97.name, 16);
        ori->type = objTable[ori97.Type];
        ori->mesh = ori97.mesh;
        ori->morph = ori97.morph;
        ori->Cs = ori97.Cs;
        ori->global_center = ori97.global_pivot;
        ori->global_rayon = ori97.global_rayon;
        ori->dataSize = ori97.index_Parent;
        ori->pad2[0]  = ori97.matrix_Method;
        ori->index_color = ori97.index_Color;
    }
    free(rawORIs);
    return;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  static void v3xOVI_Convert97(V3XSCENE *pScene, SYS_FILEHANDLE in)
*
* DESCRIPTION :
*
*/
static void v3xOVI_Convert97(V3XSCENE *pScene, SYS_FILEHANDLE in)
{
    V3XOVI97 ovi97;
    V3XOVI *ovi;
    u_int32_t *rawOVIs = (u_int32_t *)malloc(pScene->numOVI * 16 *
                                                    sizeof(u_int32_t));
    int i;

    /* Unfortunately we cannot directly read the struct from disk as
       it contains (not used on disk) pointers, which on disk are 32 bit, but
       may in reality be different (64 bits) */
    FIO_gzip.fread(rawOVIs, pScene->numOVI, 16 * sizeof(u_int32_t), in);
    pScene->OVI = MM_CALLOC(pScene->numOVI, V3XOVI);

    for (ovi = pScene->OVI, i = 0; i < pScene->numOVI; i++, ovi++)
    {
        u_int32_t *rawOVI = rawOVIs + 16 * i;

        /* copy the raw data to the in memory structs */
        ovi97.mesh  = (V3XMESH *) (uintptr_t)rawOVI[0];
        ovi97.data  = (void *)    (uintptr_t)rawOVI[1];
        ovi97.Tk    = (V3XKEY *)  (uintptr_t)rawOVI[2];
        ovi97.ORI   = (V3XORI *)  (uintptr_t)rawOVI[3];
        ovi97.TVI   = (V3XTVI *)  (uintptr_t)rawOVI[4];
        ovi97.Parent= (struct _ovi97 *) (uintptr_t)rawOVI[5];
        ovi97.Child = (struct _ovi97 **)(uintptr_t)rawOVI[6];
        ovi97.collisionList  = (struct _ovi97 *)(uintptr_t)rawOVI[7];
        memcpy(&ovi97.distance, rawOVI + 8, 8 * sizeof(u_int32_t));

#ifdef __BIG_ENDIAN__
        BSWAP16((u_int16_t*)&ovi97.index_OVI, 3);
#endif
        ovi->state = V3XSTATE_MATRIXUPDATE;
        if (ovi97.Hide_Never)
			ovi->state|=V3XSTATE_CULLNEVER;

        if (ovi97.Hide_ByDisplay)
			ovi->state|=V3XSTATE_HIDDEN;

		ovi->mesh = ovi97.mesh;
        ovi->index_ORI = ovi97.index_ORI;
        ovi->index_INSTANCE = ovi97.index_OVI;
        ovi->index_TVI = ovi97.index_TVI;
        ovi->matrix_Method = pScene->ORI[ovi->index_ORI].pad2[0];
        ovi->index_PARENT = pScene->ORI[ovi->index_ORI].dataSize;
    }
    for (ovi = pScene->OVI, i = 0; i < pScene->numOVI; i++, ovi++)
    {
        int j = ovi->index_PARENT;
        if (j)
        {
            ovi->index_PARENT = V3XScene_OVI_GetByName(pScene, pScene->ORI[j].name) - pScene->OVI;
        }
    }
    free(rawOVIs);
    return;
}

static void ReadSceneNodes(V3XSCENE *pScene, SYS_FILEHANDLE in, int bFormat97)
{
    unsigned i;
    u_int32_t *rawTRIs;
    u_int32_t *rawTVIs;
    V3XLAYER *layer = &pScene->Layer;
    layer->tm.numFrames = 0;
    layer->tm.firstFrame = 0;

    if (!bFormat97) {
        fprintf(stderr, "Fatal error non Format97 is NOT supported\n");
        abort();
    }

    v3xORI_Convert97(pScene, in);

    for (i=0;i<pScene->numORI;i++)
    {
        if (pScene->ORI[i].type == 0)
		pScene->ORI[i].type = V3XOBJ_NONE;
    }

    v3xOVI_Convert97(pScene, in);

    /* Unfortunately we cannot directly read the structs from disk as
       tkey contain (not used on disk) pointers, which on disk are 32 bit, but
       may in reality be different (64 bits) */
    pScene->TRI = (V3XTRI*)MM_heap.malloc(pScene->numTRI * sizeof(V3XTRI));
    rawTRIs = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), pScene->numTRI * 8, -1, in);
    pScene->TVI = (V3XTVI*)MM_heap.malloc(pScene->numTVI * sizeof(V3XTVI));
    rawTVIs = (u_int32_t *)v3x_read_alloc(sizeof(u_int32_t), pScene->numTVI * 4, -1, in);
    /* copy the raw data to the in memory structs */
    for (i = 0; i < pScene->numTVI; i++)
    {
        u_int32_t *rawTVI = rawTVIs + 4 * i;
        pScene->TVI[i].index_TRI = rawTVI[0];
        memcpy(&(pScene->TVI[i].frame), rawTVI + 1, 3 * sizeof(u_int32_t));
    }
    MM_heap.free(rawTVIs);

    if (bFormat97)
    {
        V3XTVI *TVI = pScene->TVI;
        for (i=0;i<pScene->numTVI;i++, TVI++)
		{
#ifdef __BIG_ENDIAN__
			BSWAP16((u_int16_t*)&TVI->pad, 1);
#endif
			TVI->index_TRI = TVI->pad;
		}
    }
    for (i=0;i<pScene->numORI;i++)
		v3x_VMX_unpack_ORI(pScene->ORI+i, in, bFormat97);

    for (i=0;i<pScene->numOVI;i++)
		v3x_VMX_unpack_OVI(pScene->OVI+i, in);

 	for (i=0;i<pScene->numTRI;i++)
    {
        V3XTRI *TRI = pScene->TRI + i;
        u_int32_t *rawTRI = rawTRIs + 8 * i;
        v3x_VMX_unpack_TRI(rawTRI, TRI, in);
        if ( layer->tm.firstFrame < TRI->startFrame )
			layer->tm.firstFrame = TRI->startFrame;
        if ( layer->tm.numFrames < TRI->numFrames )
			layer->tm.numFrames = TRI->numFrames;
    }
    MM_heap.free(rawTRIs);
    return;
}

#define HEAD1 28
_RLXEXPORTFUNC V3XSCENE RLXAPI *V3XScene_GetFromFile_VMX(const char *filename)
{
    u_int8_t *temp, *sy;
    SYS_FILEHANDLE in = FIO_gzip.fopen(filename, "rb");
    V3XSCENE *pScene = (V3XSCENE*) MM_heap.malloc(sizeof(V3XSCENE));
    V3XLAYER97 *bk;
    V3XLAYER *layer = &pScene->Layer;
    V3X.Setup.flags|=V3XOPTION_97;

    temp = (u_int8_t*) malloc(HEAD1 + sizeof(V3XLAYER97));
    FIO_gzip.fread(temp, HEAD1 + sizeof(V3XLAYER97), 1, in);
    memcpy(pScene, temp, HEAD1);
#ifdef __BIG_ENDIAN__
	BSWAP16(&pScene->numOVI, 4);
#endif
    ReadSceneNodes(pScene, in, 1);
    sy = temp + HEAD1;
    bk = (V3XLAYER97*)sy;
    strcpy(layer->lt.palette.filename, bk->ColorTable_name);
    strcpy(layer->bg.filename, bk->background_name);
    layer->bg.BG_color = bk->SolidColor;
    layer->bg.flags  |= V3XBG_COLOR;
    layer->fg.color.r = bk->FogColor.r;
    layer->fg.color.g = bk->FogColor.g;
    layer->fg.color.b = bk->FogColor.b;
    if (bk->FogActivate)
		layer->fg.flags|= V3XFG_LIN;
    free(temp);
    FIO_gzip.fclose(in);
    return pScene;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  :  void RLXAPI V3XScene_Verify(V3XSCENE *pScene)
*
* DESCRIPTION :
*
*/
int RLXAPI V3XScene_Verify(V3XSCENE *pScene)
{
    V3XOVI *OVI;
    V3XORI    *ORI;
    V3XCL             *OCs;
    int i, j;
    V3X.Buffer.MaxObj = 0;
    V3X.Light.numSource = 0;
    V3X.Light.tables = &pScene->Layer.lt;
    for (OVI=pScene->OVI, i=0;i<pScene->numOVI;i++, OVI++)
    {
        if (!OVI->ORI)
        {
            OVI->ORI = pScene->ORI + 0;
        }
        ORI = OVI->ORI;
        OVI->state|=V3XSTATE_TOPROCESS|V3XSTATE_MATRIXUPDATE;
        if (ORI->type == V3XOBJ_MESH )
        {
			if (OVI->ORI->mesh)
			{
				V3XMesh_SetRender(OVI->mesh);
				OVI->mesh->radius = OVI->ORI->global_rayon;
				SYS_ASSERT(OVI->mesh->numVerts<V3X.Buffer.MaxPointsPerMesh);
			}
        }
        else
        if (ORI->type == V3XOBJ_LIGHT)
        {
		     if (!OVI->light->diminish)
			OVI->light->diminish=1.f/2000.f;
        }
    }
    for (OVI=pScene->OVI, i=0;i<pScene->numOVI;i++, OVI++)
    {
        ORI = OVI->ORI;
        if (ORI->type!=V3XOBJ_NONE)
        {
            if (OVI->node) OVI->Tk = &OVI->node->Tk;
            else
            {
                if (ORI->type==V3XOBJ_CAMERA) OVI->Tk = &V3X.Camera.Tk;
            }
        }
        else OVI->Tk = NULL;
        OCs = ORI->Cs;
        if (ORI->type == V3XOBJ_MESH )
        {
            if (OVI->state&V3XSTATE_CULLNEVER)
            {
                for (j=0;j<OVI->mesh->numMaterial;j++)
                OVI->mesh->material[j].render_far = OVI->mesh->material[j].render_near;
            }
        }
        if (OCs)
        {
            V3XVector_Set(&OCs->velocity, CST_ZERO, CST_ZERO, CST_ZERO);
        }
        if (ORI->type!=V3XOBJ_NONE)
        {
            if (ORI->type == V3XOBJ_MESH) OVI->mesh->flags|=V3XMESH_FULLUPDATE;
            V3XScene_MatrixBuild(OVI);
            OVI->state |=V3XSTATE_MATRIXUPDATE;
            V3XScene_ObjectBuild(OVI, TRUE);
            if (ORI->type==V3XOBJ_MESH)   OVI->mesh->flags&=~V3XMESH_FULLUPDATE;
            OVI->state |=V3XSTATE_MATRIXUPDATE;
        }
    }
    V3XViewport_Setup(&V3X.Camera, GX.View);
    i = V3X.Setup.flags;
    V3X.Setup.flags = 0;
    V3XScene_Viewport_Build(pScene, NULL);
    if (pScene->Layer.lt.palette.lut) memcpy(GX.ColorTable, pScene->Layer.lt.palette.lut, 768);
    V3X.Setup.flags = i;
    return 0;
}
/*------------------------------------------------------------------------
*
* PROTOTYPE  : V3XSCENE *V3XScene_GetFromFile(char *filename)
*
* DESCRIPTION : Charge une scene
*
*/

V3XSCENE RLXAPI *V3XScene_GetFromFile(const char *filename)
{
    V3XSCENE *pScene = V3XScene_GetFromFile_VMX(filename);

    if (pScene)
    {
        pScene->Layer.lt.palette.lut = NULL;
        V3XScene_Validate(pScene);
        V3XScene_CRC_Check(pScene);
    }
    return pScene;
}


