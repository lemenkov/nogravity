// SPDX-FileCopyrightText: 1996-2005 realtech VR
// SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
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
//
// Game data access.  The data is a directory tree; the engine and the
// game ask for files with DOS style names (".\\voix\\LASER0.WAV") which
// are mapped onto "<root>/voix/laser0.wav".
//
//-------------------------------------------------------------------------

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "_rlx32.h"
#include "systools.h"
#include "sysresmx.h"

SYS_FILEIO  *	FIO_cur = &FIO_std;
SYS_WAD		*	FIO_wad;

static void MakePathUUU(char *dest, const char *path, const char *lpFilename);

// path + "/" + name, with the name's backslashes turned into slashes,
// a leading "./" dropped and lower cased (that is how the files are
// stored); path itself is copied as is.
static void MakePathUUU(char *dest, const char *path, const char *lpFilename)
{
	size_t n = 0;
	const char *s = lpFilename;

	if (path && *path)
	{
		n = strlen(path);
		if (n > _MAX_PATH - 2)
			n = _MAX_PATH - 2;
		memcpy(dest, path, n);
		if ((dest[n - 1] != '/') && (dest[n - 1] != '\\'))
			dest[n++] = '/';
	}
	while ((s[0] == '.') && ((s[1] == '\\') || (s[1] == '/')))
		s += 2;
	while ((*s == '\\') || (*s == '/'))
		s++;
	for (; *s && (n < _MAX_PATH - 1); s++)
	{
		char c = *s;
		if (c == '\\')
			c = '/';
		else
			c = (char)tolower((unsigned char)c);
		if ((c == '/') && n && (dest[n - 1] == '/'))
			continue;
		dest[n++] = c;
	}
	dest[n] = 0;
}

// Full path of a data file: root + current subdirectory + name.
void filewad_resolve(char *dest, const char *lpFilename)
{
	SYS_WAD *pWad = filewad_getcurrent();
	char rel[_MAX_PATH];
	if (!pWad)
	{
		MakePathUUU(dest, "", lpFilename);
		return;
	}
	MakePathUUU(rel, "", lpFilename);
	if (pWad->s_Path[0])
	{
		char tmp[_MAX_PATH];
		MakePathUUU(tmp, pWad->s_Path, rel);
		MakePathUUU(dest, pWad->s_Root, tmp);
	}
	else
		MakePathUUU(dest, pWad->s_Root, rel);
}

// Open a data directory.
SYS_WAD *filewad_open(const char *lpDirectory, int flags)
{
	SDL_PathInfo info;
	SYS_WAD *pWad;

	if (!lpDirectory || !*lpDirectory)
		return NULL;
	if (!SDL_GetPathInfo(lpDirectory, &info) || (info.type != SDL_PATHTYPE_DIRECTORY))
		return NULL;
	pWad = MM_CALLOC(1, SYS_WAD);
	SDL_strlcpy(pWad->s_Root, lpDirectory, _MAX_PATH);
	pWad->s_Path[0] = 0;
	pWad->mode = flags & ~SYS_WAD_STATUS_ENABLED;
	return pWad;
}

void filewad_close(SYS_WAD *pWad)
{
	if (pWad)
		MM_heap.free(pWad);
}


// Set the current subdirectory ("" for the root); names given to
// FIO_res are then relative to it.
void filewad_chdir(SYS_WAD *pWad, const char *szNewPath)
{
	size_t n;
	if (!pWad)
		pWad = filewad_getcurrent();
	if (!pWad)
		return;
	MakePathUUU(pWad->s_Path, "", szNewPath ? szNewPath : "");
	n = strlen(pWad->s_Path);
	while (n && (pWad->s_Path[n - 1] == '/'))
		pWad->s_Path[--n] = 0;
}

// Plain files: the C library, plus size and existence helpers.
static int CALLING_C file_size(SYS_FILEHANDLE stream)
{
	long curpos = ftell(stream), length;
	fseek(stream, 0L, SEEK_END);
	length = ftell(stream);
	fseek(stream, curpos, SEEK_SET);
	return (int)length;
}

static int CALLING_C file_exists(const char *filename)
{
	SDL_PathInfo info;
	return SDL_GetPathInfo(filename, &info) && (info.type == SDL_PATHTYPE_FILE);
}

SYS_FILEIO FIO_std = {fopen, fclose, fseek, fread, fgetc, fwrite, ftell, feof, fgets, file_size, file_exists};

// Data files: names resolve inside the data directory.
static int CALLING_C filewad_fexist(const char *szFilename)
{
	char name[_MAX_PATH];
	if (!szFilename || !*szFilename)
		return 0;
	filewad_resolve(name, szFilename);
	return file_exists(name);
}

static SYS_FILEHANDLE CALLING_C filewad_fopen(const char *lpFilename, const char *mode)
{
	char name[_MAX_PATH];
	SYS_FILEHANDLE fp;
	filewad_resolve(name, lpFilename);
	fp = fopen(name, mode);
	if (!fp && getenv("NOGRAVITY_TRACE_FILES"))
		fprintf(stderr, "data: %s (%s) not found\n", lpFilename, name);
	return fp;
}

static int CALLING_C filewad_fclose(SYS_FILEHANDLE fp)
{
	return fp ? fclose(fp) : 0;
}

SYS_FILEIO FIO_res = {filewad_fopen, filewad_fclose, fseek, fread, fgetc, NULL, ftell, feof, fgets, file_size, filewad_fexist};

