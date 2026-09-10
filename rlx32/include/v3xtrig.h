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

#ifndef __V3XTRIG_H
#define __V3XTRIG_H

// Basic PI values
#ifndef M_PI
    #define M_PI        3.14159265358979323846
#endif

#define M_PI180           (M_PI/180.f)

// Precision format

// Conversion
#define SIN_Trunc(x)      ((unsigned)(x)&4095)

// Short Integer table

// Floating point table
#define TRG_cos(x)        (*((float*)TRG_Table + SIN_Trunc(x       )))
#define TRG_sin(x)        (*((float*)TRG_Table + SIN_Trunc((x)-1024)))



void TRG_Generate(void);
extern void           *TRG_Table;


#endif
