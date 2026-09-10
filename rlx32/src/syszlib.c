// SPDX-FileCopyrightText: 2026 Peter Lemenkov <lemenkov@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later
//
// Gzip compressed data files (the .vmx scenes) read through zlib's gz
// functions, exposed with the engine's SYS_FILEIO interface. Names are
// resolved against the data directory like every other data file.
//
//-------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "_rlx32.h"
#include "sysresmx.h"
#include "systools.h"

static SYS_FILEHANDLE CALLING_C fzip_fopen(const char *filename, const char *mode)
{
	char name[_MAX_PATH];
	filewad_resolve(name, filename);
	return (SYS_FILEHANDLE)gzopen(name, mode);
}

static int CALLING_C fzip_fclose(SYS_FILEHANDLE file)
{
	return file ? gzclose((gzFile)file) : 0;
}

static int CALLING_C fzip_fseek(SYS_FILEHANDLE file, long offset, int whence)
{
	return gzseek((gzFile)file, offset, whence) < 0 ? -1 : 0;
}

static size_t CALLING_C fzip_fread(void *ptr, size_t size, size_t n, SYS_FILEHANDLE file)
{
	int ret = gzread((gzFile)file, ptr, (unsigned)(size * n));
	return (ret > 0 && size) ? (size_t)ret / size : 0;
}

static int CALLING_C fzip_fgetc(SYS_FILEHANDLE file)
{
	return gzgetc((gzFile)file);
}

static size_t CALLING_C fzip_fwrite(const void *ptr, size_t size, size_t n, SYS_FILEHANDLE file)
{
	int ret = gzwrite((gzFile)file, ptr, (unsigned)(size * n));
	return (ret > 0 && size) ? (size_t)ret / size : 0;
}

static long CALLING_C fzip_ftell(SYS_FILEHANDLE file)
{
	return (long)gztell((gzFile)file);
}

static int CALLING_C fzip_feof(SYS_FILEHANDLE file)
{
	return gzeof((gzFile)file);
}

static char * CALLING_C fzip_fgets(char *buf, int len, SYS_FILEHANDLE file)
{
	return gzgets((gzFile)file, buf, len);
}

static int CALLING_C fzip_fsize(SYS_FILEHANDLE file)
{
	// The uncompressed size is not known up front.
	UNUSED(file);
	return -1;
}

static int CALLING_C fzip_exists(const char *filename)
{
	return FIO_cur->exists(filename);
}

static int CALLING_C fzip_fputc(int c, SYS_FILEHANDLE file)
{
	return gzputc((gzFile)file, c);
}

static int CALLING_C fzip_fflush(SYS_FILEHANDLE file)
{
	return gzflush((gzFile)file, Z_SYNC_FLUSH);
}

SYS_FILEIO FIO_gzip =
{
	fzip_fopen,
	fzip_fclose,
	fzip_fseek,
	fzip_fread,
	fzip_fgetc,
	fzip_fwrite,
	fzip_ftell,
	fzip_feof,
	fzip_fgets,
	fzip_fsize,
	fzip_exists,
	NULL,
	NULL,
	fzip_fputc,
	fzip_fflush,
};
