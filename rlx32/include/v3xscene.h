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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

Original Source: 1996 - Stephane Denis
Prepared for public release: 02/24/2004 - Stephane Denis, realtech VR
*/
//-------------------------------------------------------------------------
#ifndef __V3XSCENE_H
#define __V3XSCENE_H

// Object categories

enum {
 V3XOBJ_VIEWPORT = 0x1, // 1 viewport object
 V3XOBJ_CAMERA, // 2 camera object
 V3XOBJ_LIGHT, // 3 light object
 V3XOBJ_DUMMY, // 4 dummy object
 V3XOBJ_MESH, // 5 mesh object
 V3XOBJ_PORTAL, // 6 Portal object (visibility)
 V3XOBJ_NURBS, 	 // 7 Nurbs object
 V3XOBJ_NONE = 0xff // free slot
};

enum {
	V3XORI_DUPLICATED = 0x1,
 V3XORI_CSDUPLICATED = 0x2
};


// Keyframe state

enum {
 V3XKF_NONE, // No keyframe
 V3XKF_VALID = 0x1, // Keyframe valid
 V3XKF_HIDDEN = 0x2, // Keyframe orders than object is hidden
 V3XKF_KEYEX = 0x4, // Has extended keyframe (V3X'98 Spec)
 V3XKF_LOOP = 0x8, // Loop (restart animation when finished)
 V3XKF_UNROLL = 0x10, // Unroll (move object after the animation has relooped)
 V3XKF_REVERSE = 0x20, // Reverse playing
 V3XKF_ES = 0x40, // Gold Keyframer informations
 V3XKF_LOCKED = 0x80
};


// Object Pipeline Processing flags

enum {
 V3XSTATE_VALIDPOINTER = 0x1, // object pointers are valid in structures and ready to use.
 V3XSTATE_HIDDEN = 0x2, // object is frozen (not processed).
 V3XSTATE_HIDDENDISPLAY = 0x4, // object is hidden (processed but not displayed).
 V3XSTATE_CULLED = 0x8, // object was culled after processing.
 V3XSTATE_CULLEDOFR = 0x10, // object was culled because it wasn out of range.
 V3XSTATE_MATRIXUPDATE = 0x20, // object matrix should be updated.
 V3XSTATE_PROCESSED = 0x40, // object was processed
 V3XSTATE_TOPROCESS = 0x80, // object was processed
 V3XSTATE_PRSCOLLISION = 0x100, // object should be processed in collision pipeline.
 V3XSTATE_CULLNEVER = 0x400, // object should be never culled (except if outside viewport).
 V3XSTATE_GIANT = 0x800, //
 V3XSTATE_DISPLAYCOL = 0x1000, // display collision mesh
 V3XSTATE_BSPCULLED = 0x2000,
 V3XSTATE_INSTANCED = 0x8000 // FULL
};
enum {
 V3XSUBDEF_PSECTOR = 0x1
};

// Internal : reset pipeline

#define V3XSTATE_RESETMASK ~(V3XSTATE_CULLEDOFR|V3XSTATE_CULLED|V3XSTATE_TOPROCESS|V3XSTATE_PROCESSED|V3XSTATE_PRSCOLLISION|V3XSTATE_BSPCULLED)

typedef struct _v3x_morph_frame{

 V3XVECTOR *normal; // Normal
 V3XVECTOR *vertex; // Vertex
}V3XTWEENFRAME;

typedef struct _v3x_morph{

 V3XTWEENFRAME *frame; // Morphing frame
 u_int32_t numFrames; // Number of frame
 u_int32_t numVerts; // Number of vertex
 u_int32_t numFaces; // Number of face
}V3XTWEEN;


typedef struct _v3x_dummy{
 u_int32_t pad[6];
#ifdef __LP64__
 u_int32_t alignTK_lp64[6];
#endif
 V3XMATRIX matrix;
 V3XKEY Tk;
#ifndef __LP64__
 u_int32_t pad0[3+4];
#else
 u_int32_t pad0[3+4+2];
#endif
}V3XDUMMY;


typedef struct _v3x_node{
 u_int32_t pad[6];
#ifdef __LP64__
 u_int32_t alignTK_lp64[6];
#endif
 V3XMATRIX matrix;
 V3XKEY Tk;
#ifndef __LP64__
 u_int32_t pad0[3+4];
#else
 u_int32_t pad0[3+4+2];
#endif
}V3XNODE;


typedef struct _v3x_ori{
 u_int16_t flags; // State
 u_int8_t type; // Object kind
 u_int8_t pad; // pad
 u_int32_t dataSize; // external data size
 void *data; // external data
 union {
	 V3XMESH *mesh; // mesh
	 V3XLIGHT *light; // light
	 V3XDUMMY *dummy; // dummy object
	 V3XCAMERA *camera;
	 V3XNODE *node;
 };
 V3XTWEEN *morph; // Mesh morphing handle
 V3XCL *Cs; // Collision handle
 void *dynamic; // Dynamic controller handle
 V3XVECTOR global_center; // Visibility center
 V3XSCALAR global_rayon; // Visibility radius
 char name[16]; // Name
 u_int8_t index_color; // Index color
	u_int8_t					 sub_class;	 // sub class
 u_int8_t pad2[2];
}V3XORI; // 64bytes


// Maintenant
typedef struct _v3x_tri{
 union {
 V3XKEY *keys; // animations keys (old model)
 V3XKEYEX *keyEx; // extended animation key
 };
 union {
		struct _v3x_tri *next; // Pointer to master key informations
		int index_NEXT; // Index
 };
 void *ORI_ref; // Object reminder
 union {
		struct _v3x_tri *chain; // Pointer to master key informations
		int index_CHAIN; // Index
 };
 unsigned pad;
 u_int16_t numFrames; // number of frame in animation
 u_int16_t startFrame; // first frame
 u_int16_t numKeys; // number of keys
 u_int8_t fps; // default frame rate
 u_int8_t flags; // flags
 u_int8_t pad2[4];
}V3XTRI;

typedef struct _v3x_tvi{
 union {
	V3XTRI *TRI; // Pointer to master key informations
	int index_TRI; // Index
 };
 V3XSCALAR frame; // Current frame (float value)
 u_int16_t pad; // Index (if non null)
 u_int8_t flags; // flags
 u_int8_t fps; // Overriden frame rate
 u_int32_t startTimer; // Time when the animation has started
}V3XTVI;

typedef struct _v3x_ovi{
 u_int16_t state; // state
 u_int8_t matrix_Method; // matrix calculation method
 u_int8_t subDefinition; // pad

 u_int32_t dataSize; // external data size
 void *data; // external data
 union {
	 V3XMESH *mesh; // mesh
	 V3XLIGHT *light; // light
	 V3XDUMMY *dummy; // dummy object
	 V3XCAMERA *camera;
	 V3XNODE *node;
 };
 union {
	 unsigned index_ORI; // index in scene or ORI handle
	 V3XORI *ORI; // pointer will be calculated by CreatePointers
 };
 union {
	 unsigned index_PARENT; // index of the parent (3D hierachical)
	 struct _v3x_ovi *parent;
 };
 union {
	 unsigned index_TARGET; // index of the target (rotation)
	 struct _v3x_ovi *target;
 };
 union {
	 unsigned index_INSTANCE; // index of the master object
	 struct _v3x_ovi *instance; // for multiple instance of this object
 };
 union {
	 unsigned index_TVI; // Keyframe handle
	 V3XTVI *TVI;
 };
 V3XKEY *Tk; // Shortcut to track Info
 struct _v3x_ovi **child; // children in hierchical
 struct _v3x_ovi *collisionList; // Collision object
 union {
	 unsigned index_NEXT;
	 struct _v3x_ovi *next;
 };
 V3XSCALAR distance; // distance for z sorting
 u_int32_t filler[2];
}V3XOVI; // 64bytes

//---------------------------------------------------------


typedef struct _v3xlayer_timer{
 u_int32_t flags;
 u_int32_t numFrames; // numbers of frames in the animation
 V3XSCALAR firstFrame; // firstFrame
 u_int32_t fps; // frame Rate
 u_int32_t startTimer; // internal : startup timer
}V3XLAYER_TIMER; //SizeOf: 20b


enum {
 V3XBG_NONE = 0x0, // no background (not cleared)
 V3XBG_BLACK = 0x1, // black background
 V3XBG_COLOR = 0x2, // color background
	V3XBG_SIDE = 0x3, // barre side
 V3XBG_GRAD = 0x4, // gradient background
 V3XBG_IMG = 0x8, // picture
 V3XBG_CALCINDEX = 0x20, // Recalc IndexColor
 V3XBG_STEREO = 0x40, // Stereo rendering mixed
};


typedef struct _v3x_layerbg{
	u_int32_t flags; // type of background
	char filename[16]; // file name's of the bitmap
	rgb32_t UP_color; // up color (for gradient)
	union {
		rgb32_t BG_color; // Background color
		u_int32_t BG_index;
	};
	rgb32_t DW_color; // down color (for gradient)
	rgb32_t AMB_color; // Ambient color
	GXSPRITE bitmap; // internal : bitmap structure
	V3XSPRITEINFO _bitmap; // internal : extended bitmap
	u_int32_t index_color; // internal : Color (indexed value)
	V3XSCALAR stereoPitch;
}V3XLAYER_BG; //SizeOf: 60b

enum V3XFG{
	V3XFG_LIN = 0x1, // fog linear
};

typedef struct _v3x_layerfog{
	u_int32_t flags; // fog informations
	rgb32_t color; // fog color
}V3XLAYER_FOG; //SizeOf: 20b



typedef struct _v3xlayer{
 V3XLAYER_TIMER 	tm; // timers and keyframe 20b
 V3XLAYER_BG 	bg; // background 64b
 V3XLAYER_FOG 	fg; // fog 20b
 V3XLAYER_CLUT 	lt; // color tables 120b
}V3XLAYER; 	 // SizeOf 256b

typedef struct _v3x_scene{
 char Signature[4]; // version signature
 u_int16_t numOVI; // number of OVI object
 u_int16_t numTVI;
 u_int16_t numTRI;
 u_int16_t numORI;
 V3XORI *ORI;
 V3XOVI *OVI;
 V3XTRI *TRI;
 V3XTVI *TVI;
 V3XLAYER Layer;
}V3XSCENE;

struct _gx_viewport;

//
V3XOVI *V3XScene_Camera_GetByName(V3XSCENE *Scene, const char *name);
void V3XScene_Camera_Select(V3XOVI *OVI);
V3XOVI *V3XScene_OVI_GetByName(V3XSCENE *Scene, const char *name);
//
int V3XScene_Verify(V3XSCENE *Scene);
void V3XScene_Release(V3XSCENE *Scene);
//
V3XSCENE *V3XScene_GetFromFile(const char *filename);

//
void V3XScene_LoadTextures(V3XSCENE *Scene, void (*callback)(void *));//
void V3XScene_Viewport_Build(V3XSCENE *Scene, struct _gx_viewport *viewport);
void V3XScene_Viewport_Clear(V3XSCENE *Scene);
void V3XScene_Viewport_Render(V3XSCENE *Scene);

//
void V3XScene_ObjectBuild(V3XOVI *OVI, int spec);
void V3XScene_MatrixBuild(V3XOVI *OVI);
//
int V3XVECTOR_IsVisible(V3XSCENE *Scene, V3XVECTOR *start, V3XVECTOR *end, unsigned hint, V3XOVI **who_hide);
#endif
