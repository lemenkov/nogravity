// SPDX-FileCopyrightText: 1996-2004 realtech VR
// SPDX-FileCopyrightText: 2005 Matt Williams
// SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later
//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2004 - realtech VR

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
Linux/SDL Port: 2005 - Matt Williams
*/
//-------------------------------------------------------------------------
#include <SDL3/SDL.h>
#include "_rlx32.h"
#include "systime.h"

// stop timer
void timer_Stop(SYS_TIMER *tm)
{
    tm->flags &= ~SYS_TIMER_FLAGS_START;
}

// clear timer
void timer_Reset(SYS_TIMER *tm)
{
    tm->iCounter = 0;
    tm->fCounter = 0.f;
}

// Wait until iMinFrame ticks of the iFreq Hz clock have passed since the
// previous update, then measure the frame time.
void timer_Update(SYS_TIMER *tm)
{
    Uint64 wait = tm->iMinFrame ? (Uint64)tm->iMinFrame * SDL_NS_PER_SECOND / (Uint64)tm->iFreq : 0;
    Uint64 now = SDL_GetTicksNS();
    if (now < tm->tStart + wait)
    {
        SDL_DelayPrecise(tm->tStart + wait - now);
        now = SDL_GetTicksNS();
    }
    tm->fFrameDelta = (float)((double)(now - tm->tStart) / (double)SDL_NS_PER_SECOND);
    tm->fCounter = tm->fFrameDelta * (float)tm->iFreq;
    tm->iCounter = (int32_t)(tm->fCounter * 65535.f);
    tm->tStart = now;
}

void timer_Start(SYS_TIMER *tm, int iFreq, int iMinFrame)
{
    tm->iFreq = iFreq;
    tm->iMinFrame = iMinFrame;
    tm->tStart = SDL_GetTicksNS();
    timer_Update(tm);
}
