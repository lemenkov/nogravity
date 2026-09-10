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
#include <stdio.h>
#include <SDL3/SDL.h>
#include "_rlx32.h"
#include "systime.h"

// Internal clock runs in microseconds.
static const int64_t g_iFreq = 1000000;
#define GET_TICK(tmp) *tmp = (int64_t)(SDL_GetTicksNS() / 1000)

// stop timer
void timer_Stop(SYS_TIMER *tm)
{
    tm->flags &= ~SYS_TIMER_FLAGS_START;
    return;
}

// clear timer
void timer_Reset(SYS_TIMER *tm)
{
    tm->iCounter = 0;
    tm->fCounter = 0.f;
    return;
}

void timer_Update(SYS_TIMER *tm)
{
    int64_t ticks_to_wait = tm->iMinFrame ? (int64_t)tm->iMinFrame * g_iFreq / (int64_t)tm->iFreq : (int64_t)0; // in microseconds
    int64_t ticks_passed;
	int64_t ticks_left;
	int64_t ticks_min = g_iFreq * (int64_t)10 / (int64_t)1000;

    do
    {
        GET_TICK(&tm->tEnd);
    	ticks_passed = tm->tEnd - tm->tStart;
		ticks_left = (int64_t)ticks_to_wait - (int64_t)ticks_passed;
		if (ticks_left > ticks_min)
			SDL_Delay(1);
		else if (ticks_left > 0)
			SDL_DelayNS(10000); // Release the CPU briefly instead of spinning.
    }while(ticks_left>=0);

	tm->fFrameDelta = (float)((double)ticks_passed / (double)g_iFreq);
	tm->fCounter = tm->fFrameDelta * (float) tm->iFreq;
    tm->iCounter = (int32_t)(tm->fCounter * 65535.f);
    GET_TICK(&tm->tStart);
    return;
}

void timer_Start(SYS_TIMER *tm, int iFreq, int iMinFrame)
{
	tm->iFreq = iFreq;
	tm->iMinFrame = iMinFrame;
    GET_TICK(&tm->tStart);
    timer_Update(tm);
    return;
}
