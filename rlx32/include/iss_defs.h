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
#ifndef _ISS_DEFS_HH
#define _ISS_DEFS_HH

#include "v3xtypes.h"

#define MAX_V3XA_CLIENT_DRIVER_NAME		128
#define MAX_V3XA_AUDIO_PATH				256
#define MAX_V3XA_AUDIO_MIX				32
#define MAX_V3XA_AUDIO_STREAM			8

// Channel handle
typedef int V3XA_CHANNEL;

// Sample format flags
enum {
    V3XA_FMT16BIT     = 0x1,        // 16Bit samples
    V3XA_FMTSTEREO    = 0x2,        // Stereo 2-Channels sample
    V3XA_FMTIEEE      = 0x100,      // 32 bit float samples
};

// A sample loaded in memory
typedef struct _v3xa_handle
{
    void             *sample;            // Raw data sample
    u_int32_t         length;            // Sample length in byte
    u_int32_t         loopend;           // Loop end offset (0: no loop)
    u_int32_t         samplingRate;      // Sample sampling rate in hz
    u_int16_t         sampleFormat;      // Sample format (see before).
    u_int8_t          mode;              // Reserved
    u_int8_t          priority;          // Play priority 0..255 (MAX)
}V3XA_HANDLE;

// Streaming handle (music); 0 means "no stream"
typedef int V3XA_STREAM;

// Channel infos

// Wave driver
typedef struct _v3xa_wave_client_driver
{
// Init Functions
	int 			(*Initialize)(void *);	// Initialize previously detected device (0: ok)
	void			(*Release)(void);		// Release driver
	void			(*SetVolume)(float volume);// Set master volume
// Sample Functions
	void			(*ChannelOpen)(int gain, int numbersOfchannel); // Open multichannel mixer
	int 			(*ChannelPlay)(V3XA_CHANNEL channel, int frequency, float volume, float panning, V3XA_HANDLE *handle);
	void			(*ChannelStop)(V3XA_CHANNEL channel);
	void			(*ChannelSetVolume)(V3XA_CHANNEL channel, float volume);
	void			(*ChannelSetPanning)(V3XA_CHANNEL channel, float panning);
	void			(*ChannelSetSamplingRate)(V3XA_CHANNEL channel, int frequency);
	int 			(*ChannelGetStatus)(V3XA_CHANNEL channel);
	V3XA_CHANNEL 	(*ChannelGetFree)(V3XA_HANDLE *handle);
	void			(*ChannelFlushAll)(int mode);
	char			s_DrvName[MAX_V3XA_CLIENT_DRIVER_NAME];
}V3XA_WaveClientDriver;

// V3X Audio object
struct V3XAUDIO
{
	V3XA_WaveClientDriver	*	Client;
	char					**	p_driverList;
	char						samplePath[MAX_V3XA_AUDIO_PATH];
	u_int32_t						State;
	int32_t						nStreamBufferSizeKb;
	int32_t 						numChannel;
	int32_t 						samplingRate;
	int32_t						deviceID;
	int32_t						driverID;
};

struct RLXSYSTEM;


// Samples
int V3XA_Handle_LoadFromFn(V3XA_HANDLE *pHandle, char *szFilename);

// Streams (music)
int V3XAStream_GetFn(V3XA_STREAM *stream, const char *szFilename, int loop);
void V3XAStream_SetVolume(V3XA_STREAM handle, V3XA_CHANNEL channel, float volume);
int V3XAStream_Poll(V3XA_STREAM handle);
int V3XAStream_PollAll(void);
void V3XAStream_Release(V3XA_STREAM handle);

void V3XA_EntryPoint(struct RLXSYSTEM *pRlx);
extern struct V3XAUDIO  V3XA;


#endif
