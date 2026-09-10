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
#ifndef __SYSRESMX_H
#define __SYSRESMX_H


#ifndef _MAX_PATH

#if defined __MACOS__
#define _MAX_PATH 1024
#elif (defined _WIN32 || defined _WIN64)
#define _MAX_PATH 260
#else
#define _MAX_PATH 256
#endif

#endif

enum SYS_WAD_STATUS
{
	SYS_WAD_STATUS_ENABLED		= 0x1,	// Resource stream is enabled
};

// Stream class (remapped from the standard STDIO.H functions).
typedef struct _sys_fileio
{
	SYS_FILEHANDLE 	(CALLING_C *fopen )(const char *filename, const char *mode);
	int 			(CALLING_C *fclose)(SYS_FILEHANDLE  stream);
	int 			(CALLING_C *fseek )(SYS_FILEHANDLE  stream, long offset, int whence);
	size_t			(CALLING_C *fread )(void *ptr, size_t size, size_t n, SYS_FILEHANDLE  stream);
	int 			(CALLING_C *fgetc )(SYS_FILEHANDLE  stream);
	size_t			(CALLING_C *fwrite)(const void *ptr, size_t size, size_t n, SYS_FILEHANDLE  stream);
	long				(CALLING_C *ftell )(SYS_FILEHANDLE  stream);
	int 			(CALLING_C *eof )(SYS_FILEHANDLE  stream);
	char *			(CALLING_C *fgets )(char *s, int n, SYS_FILEHANDLE  stream);
	int				(CALLING_C *fsize )(SYS_FILEHANDLE  stream);
	int 			(CALLING_C *exists)(const char *filename);
}SYS_FILEIO;

// Single file structure in a resource

// Resource structures
typedef struct _sys_wad
{
	char					s_Root[_MAX_PATH];		//  Data directory
	char					s_Path[_MAX_PATH];		//  Current subdirectory within it
	int32_t 				mode;					//	current mode (SYS_WAD_STATUS_ENABLED, off)
}SYS_WAD;

__extern_c

_RLXEXPORTFUNC		int32_t				RLXAPI	file_length(const char *filename);

_RLXEXPORTFUNC		SYS_WAD			*	RLXAPI	filewad_open(const char *filename, int flags);
_RLXEXPORTFUNC		void				RLXAPI	filewad_close(SYS_WAD *resource);
_RLXEXPORTFUNC		void				RLXAPI	filewad_chdir(SYS_WAD *resource, const char *newpath);
_RLXEXPORTFUNC		void				RLXAPI	filewad_resolve(char *dest, const char *filename);

_RLXEXPORTFUNC		void				RLXAPI	sysInitZlib();
_RLXEXPORTFUNC		void				RLXAPI	sysInitFS();

extern				SYS_FILEIO			FIO_std,
										FIO_res,
										FIO_gzip;

extern				SYS_FILEIO		*	FIO_cur;
extern				SYS_WAD			*	FIO_wad;

__end_extern_c

#define filewad_setcurrent(a) FIO_wad = a
#define filewad_getcurrent() FIO_wad

#endif
