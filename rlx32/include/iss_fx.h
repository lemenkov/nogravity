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
#ifndef __ISS_FXINCLUDED
#define __ISS_FXINCLUDED

#include "systime.h"

typedef struct {
    char     name[32];        // file name
    u_int32_t    samplingRate;    // Overriden sampling rate
    u_int32_t    randomPitch;     // Random pitch value
    u_int32_t    defaultVol;      // Default volume
    u_int32_t    Flags;           // Download features (see before)
    int32_t     loopStart;       // Loop end, start
    int32_t     loopEnd;
    int32_t     priority;        // Priority
}SND_DWHANDLE;

#endif
