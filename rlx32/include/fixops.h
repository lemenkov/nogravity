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

#ifndef __FIXOPCODE
#define __FIXOPCODE

#define xMUL8(a, b) ((u_int32_t)(((u_int32_t)(a)*(u_int32_t)(b))>>8))
#define SHL16(x)         ((int32_t)(x)<<16)
#define SHR16(x)         ((int32_t)(x)>>16)
#define SHLD(x)          ((int32_t)(x)<<16)
#define SHRD(x)          ((int32_t)(x)>>16)

  #define fMUL(x, y)        (((float)(x)*(float)(y))*(1.f/65536.f))
  #define fDIV(x, y)        (((float)(x)*65536.f)/(float)(y))
  #define VMUL(x, y)        (int32_t)fMUL(x, y)
  #define VDIV(x, y)        (int32_t)fDIV(x, y)
  #define VMUL2(x)         VMUL(x, x)
  #define VMUL_DIV(x, y, z)  ((((float)(x)*(float)(y))/(float)(z)))


#endif
