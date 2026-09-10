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

#define MAX_V3XA_AUDIO_VOLUME			1.f
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
    V3XA_FMTPCM       = 0x4,        // PCM format
    V3XA_FMTPACKED    = 0x8,        // Compressed sample
    V3XA_FMTMULTI     = 0x10,       // Reserved
    V3XA_FMTVOLATILE  = 0x20,       // Reserved
    V3XA_FMTBIGENDIAN = 0x40,       // Big endian samples
    V3XA_FMT3D        = 0x80,       // 3D Samples
    V3XA_FMTIEEE      = 0x100,      // 32 bit float samples
    V3XA_FMTUNKSIZE   = 0x200,      // Unknown size
    V3XA_PANSURROUND  = 100         // Surround panning value for Surround play.
};

// A sample loaded in memory
typedef struct _v3xa_handle
{
    void             *sample;            // Raw data sample
    u_int32_t         length;            // Sample length in byte
    u_int32_t         loopstart;         // Loop start offset
    u_int32_t         loopend;           // Loop end offset (0: no loop)
    u_int32_t         samplingRate;      // Sample sampling rate in hz
    u_int32_t         chunkLength;       // Reserved
	u_int32_t         sampleID;          // Reserved
    u_int16_t         sampleFormat;      // Sample format (see before).
    u_int8_t          mode;              // Reserved
    u_int8_t          priority;          // Play priority 0..255 (MAX)
}V3XA_HANDLE;

// Streaming handle (music); 0 means "no stream"
typedef int V3XA_STREAM;

// Channel infos
typedef struct _v3xa_channelInfo
{
    u_int8_t              Volume;
    u_int8_t              filler[3];
}V3XA_CHANNELINFO;

// Environment preset
enum V3XA_ENVIRONMENT
{
    V3XA_ENVIRONMENT_GENERIC,
    V3XA_ENVIRONMENT_PADDEDCELL,
    V3XA_ENVIRONMENT_ROOM,
    V3XA_ENVIRONMENT_BATHROOM,
    V3XA_ENVIRONMENT_LIVINGROOM,
    V3XA_ENVIRONMENT_STONEROOM,
    V3XA_ENVIRONMENT_AUDITORIUM,
    V3XA_ENVIRONMENT_CONCERTHALL,
    V3XA_ENVIRONMENT_CAVE,
    V3XA_ENVIRONMENT_ARENA,
    V3XA_ENVIRONMENT_HANGAR,
    V3XA_ENVIRONMENT_CARPETEDHALLWAY,
    V3XA_ENVIRONMENT_HALLWAY,
    V3XA_ENVIRONMENT_STONECORRIDOR,
    V3XA_ENVIRONMENT_ALLEY,
    V3XA_ENVIRONMENT_FOREST,
    V3XA_ENVIRONMENT_CITY,
    V3XA_ENVIRONMENT_MOUNTAINS,
    V3XA_ENVIRONMENT_QUARRY,
    V3XA_ENVIRONMENT_PLAIN,
    V3XA_ENVIRONMENT_PARKINGLOT,
    V3XA_ENVIRONMENT_SEWERPIPE,
    V3XA_ENVIRONMENT_UNDERWATER,
    V3XA_ENVIRONMENT_DRUGGED,
    V3XA_ENVIRONMENT_DIZZY,
    V3XA_ENVIRONMENT_PSYCHOTIC,
    V3XA_ENVIRONMENT_COUNT,
	V3XA_ENVIRONMENT_FORCEDWORD = 0xffff
};

// Reverb properties
typedef struct _v3xa_reverbproperties
{
    u_int32_t		environment;
    float		fVolume;
    float		fDecayTime_sec;
    float		fDamping;
}V3XA_REVERBPROPERTIES;

// Wave driver
typedef struct _v3xa_wave_client_driver
{
// Init Functions
	int 			(RLXAPI *Enum)(void);			// Enumerate devices
	int 			(RLXAPI *Detect)(void);			// Detect devices (0: found)
	int 			(RLXAPI *Initialize)(void *);	// Initialize previously detected device (0: ok)
	void			(RLXAPI *Release)(void);		// Release driver
	void			(RLXAPI *SetVolume)(float volume);// Set master volume
	void			(RLXAPI *Start)(void); 		// Start to play
	void			(RLXAPI *Stop)(void);			// Stop to play
	int32_t			(RLXAPI *Poll)(int32_t param); // poller
	void			(RLXAPI *Render)(void);		// Render 3D environment sound (optional)
	void			(RLXAPI *UserSetParms)(V3XMATRIX *lpMAT, V3XVECTOR *lpVEL,
										   float *lpDistanceF, float *lpDopplerF,
										   float *lpRolloff);
// Sample Functions
	void			(RLXAPI *ChannelOpen)(int gain, int numbersOfchannel); // Open multichannel mixer
	int 			(RLXAPI *ChannelPlay)(V3XA_CHANNEL channel, int frequency, float volume, float panning, V3XA_HANDLE *handle);
	void			(RLXAPI *ChannelStop)(V3XA_CHANNEL channel);
	void			(RLXAPI *ChannelSetVolume)(V3XA_CHANNEL channel, float volume);
	void			(RLXAPI *ChannelSetPanning)(V3XA_CHANNEL channel, float panning);
	void			(RLXAPI *ChannelSetSamplingRate)(V3XA_CHANNEL channel, int frequency);
	int 			(RLXAPI *ChannelGetStatus)(V3XA_CHANNEL channel);
	void			(RLXAPI *ChannelSetParms)(V3XA_CHANNEL channel, V3XVECTOR *pos,
																    V3XVECTOR *velocity,
																	V3XRANGE *fRange);
	int 			(RLXAPI *ChannelSetEnvironment)(V3XA_CHANNEL channel, V3XA_REVERBPROPERTIES *cfg);
	V3XA_CHANNEL 	(RLXAPI *ChannelGetFree)(V3XA_HANDLE *handle);
	void			(RLXAPI *ChannelFlushAll)(int mode);
	void			(RLXAPI *ChannelInvalidate)(V3XA_HANDLE *handle);
	V3XA_HANDLE* 	(RLXAPI *ChannelGetSample)(V3XA_CHANNEL channel);
	char			s_DrvName[MAX_V3XA_CLIENT_DRIVER_NAME];
	char		**	p_DriverList;
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

__extern_c

// Samples
_RLXEXPORTFUNC    int    V3XA_Handle_LoadFromFn(V3XA_HANDLE *pHandle, char *szFilename);
_RLXEXPORTFUNC    void   V3XA_Handle_Release(V3XA_HANDLE *pHandle);

// Streams (music)
_RLXEXPORTFUNC    int	 V3XAStream_GetFn(V3XA_STREAM *stream, const char *szFilename, int loop);
_RLXEXPORTFUNC    void   V3XAStream_SetVolume(V3XA_STREAM handle, V3XA_CHANNEL channel, float volume);
_RLXEXPORTFUNC    int    V3XAStream_Poll(V3XA_STREAM handle);
_RLXEXPORTFUNC    int    V3XAStream_PollAll(void);
_RLXEXPORTFUNC    void   V3XAStream_Release(V3XA_STREAM handle);
_RLXEXPORTFUNC    void   V3XAStream_ReleaseAll(void);
_RLXEXPORTFUNC    void   V3XAStream_Rewind(V3XA_STREAM handle);
_RLXEXPORTFUNC    void   V3XAStream_Stop(V3XA_STREAM handle);
_RLXEXPORTFUNC    int    V3XAStream_Start(V3XA_STREAM handle);

_RLXEXPORTFUNC	  void	 V3XA_EntryPoint(struct RLXSYSTEM *pRlx);
_RLXEXPORTDATA    extern struct V3XAUDIO  V3XA;

__end_extern_c

#endif
