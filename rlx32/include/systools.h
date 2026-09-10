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
#ifndef __SYSTOOLS_H
#define __SYSTOOLS_H

#include <stdlib.h>
#include <string.h>

// Memory manager: MM_heap is a bump allocator over one block while a level
// is loaded (push/pop/reset), and plain malloc/free otherwise.
typedef struct _sys_memory
{
    void    	*(*malloc)(size_t size);
    void    	 (*free)(void *block);
    void    	*(*realloc)(void *block, size_t size);
    void    	 (*heapalloc)(void *block, size_t size);
    unsigned	 (*push)(void);
    void    	 (*pop)(int32_t id);
    void    	 (*reset)(void);
    u_int8_t     *heapAddress;
    u_int32_t     Size;
    u_int32_t     PreviousAddress;
    u_int32_t     CurrentAddress;
    u_int32_t     TotalAllocated;
    u_int32_t     Stack;
    unsigned	  active;
}SYS_MEMORYMANAGER;



    // Strings operations
void sysStrExtChg(char *nouvo, const char *old, const char *ext);

    // Array operations
int array_size(const char **tt);
void array_free(char **tt);
char **array_loadtext(SYS_FILEHANDLE in, int maxy, int maxx);

    // File operations
char *file_searchpathES(char *fileName, const char *pathSearch);
char *file_name(char *a);

extern    SYS_MEMORYMANAGER MM_heap;


    // Byte order conversion of arrays: the data files are little endian.
static inline void BSWAP16(u_int16_t *pValue, int n)
{
    while (n--)
    {
        *pValue = SDL_Swap16(*pValue);
        pValue++;
    }
}

static inline void BSWAP32(u_int32_t *pValue, int n)
{
    while (n--)
    {
        *pValue = SDL_Swap32(*pValue);
        pValue++;
    }
}

#ifndef min
#define min(a,b) ((a)<(b) ? a : b)
#define max(a,b) ((a)>(b) ? a : b)
#endif

// Macros
#define SETBITFIELD(cond, dest, value) if (cond) dest|=value; else dest&=~value;
#define XCHG(a, b, type) {type x=a;a=b;b=x;}
#define MM_CALLOC(n, type) (type*)MM_heap.malloc( (n) * sizeof(type))

#endif
