//-------------------------------------------------------------------------
/*
Copyright (C) 2005 - Matt Williams

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

Linux/SDL Port: 2005 - Matt Williams
*/
//-------------------------------------------------------------------------

#include "_rlx32.h"

/* The SDL_mixer backend targets SDL 1.2 and is being replaced; until then
   only the silent driver builds. */
#include "snd_none.c"
