//-------------------------------------------------------------------------
/*
Copyright (C) 1996, 2005 - realtech VR

This file is part of No Gravity

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
SDL3 audio backend: 2026 - Peter Lemenkov
*/
//-------------------------------------------------------------------------
//
// Sound backend on top of SDL3 audio streams and SDL3_sound.
//
// Sound effects: every mixer channel is an SDL_AudioStream bound to the
// playback device.  Samples are decoded once at load time into 32 bit
// float stereo at their native rate; a stream "get" callback copies the
// sample into the stream on demand, applying volume and panning, and
// SDL does the resampling and mixing.
//
// Music: an SDL3_sound decoder feeds an SDL_AudioStream chunk by chunk
// from V3XAStream_Poll(), which the game calls from its music thread.
//
//-------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <SDL3/SDL.h>
#include <SDL3_sound/SDL_sound.h>

#include "_rlx32.h"
#include "_rlx.h"
#include "systools.h"
#include "sysresmx.h"
#include "iss_defs.h"

struct V3XAUDIO V3XA;

#define MAX_CHANNELS		MAX_V3XA_AUDIO_MIX
#define MAX_STREAMS		(MAX_V3XA_AUDIO_STREAM + 1)	// slot 0 is never handed out
#define BASE_FREQUENCY		44100				// reference rate for pitch changes
#define FRAME_BYTES		(2 * sizeof(float))		// stereo float
#define STREAM_QUEUE_SECONDS	1.0f				// music read-ahead
#define STREAM_DECODE_BYTES	(64 * 1024)

typedef struct
{
	SDL_AudioStream	*stream;
	V3XA_HANDLE	*handle;	// sample being played
	size_t		frame;		// play position, in frames
	int		playing;
	int		loop;
	float		volume;		// 0..1
	float		pan;		// -1..1
} SND_CHANNEL;

typedef struct
{
	int		used;
	int		playing;
	int		loop;
	int		eof;
	Uint8		*filedata;
	Uint32		filesize;
	Sound_Sample	*sample;
	SDL_AudioStream	*stream;
} SND_STREAM;

static SDL_AudioDeviceID	g_Device = 0;
static SDL_AudioSpec		g_DeviceSpec;
static SND_CHANNEL		g_Channels[MAX_CHANNELS];
static int			g_nChannels = 0;
static SND_STREAM		g_Streams[MAX_STREAMS];

//-------------------------------------------------------------------------
// Helpers
//-------------------------------------------------------------------------

// Read a whole file from the current file system (usually the RMX archive).
static Uint8 *ReadFile(const char *filename, Uint32 *size)
{
	SYS_FILEHANDLE fp = FIO_cur->fopen(filename, "rb");
	Uint8 *data;
	int len;
	if (!fp)
		return NULL;
	len = FIO_cur->fsize(fp);
	if (len <= 0)
	{
		FIO_cur->fclose(fp);
		return NULL;
	}
	data = (Uint8 *)SDL_malloc((size_t)len);
	if (data && (FIO_cur->fread(data, 1, (size_t)len, fp) != (size_t)len))
	{
		SDL_free(data);
		data = NULL;
	}
	FIO_cur->fclose(fp);
	*size = (Uint32)len;
	return data;
}

static const char *FileExtension(const char *filename)
{
	const char *dot = strrchr(filename, '.');
	return dot ? dot + 1 : NULL;
}

static void ChannelGains(const SND_CHANNEL *ch, float *left, float *right)
{
	float pan = ch->pan;
	if (pan < -1.f) pan = -1.f;
	if (pan >  1.f) pan =  1.f;
	*left  = ch->volume * ((pan > 0.f) ? (1.f - pan) : 1.f);
	*right = ch->volume * ((pan < 0.f) ? (1.f + pan) : 1.f);
}

//-------------------------------------------------------------------------
// Sample channels
//-------------------------------------------------------------------------

// Called by SDL from the audio thread whenever the stream needs data.
static void SDLCALL ChannelFeed(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
	SND_CHANNEL *ch = (SND_CHANNEL *)userdata;
	float buffer[1024 * 2];
	size_t frames_total, frames_left;
	float left, right;
	UNUSED(total_amount);

	if (!ch->playing || !ch->handle || !ch->handle->sample)
		return;

	frames_total = ch->handle->length / FRAME_BYTES;
	frames_left = (size_t)(additional_amount + FRAME_BYTES - 1) / FRAME_BYTES;
	ChannelGains(ch, &left, &right);

	while (frames_left > 0)
	{
		const float *src;
		size_t n, i;

		if (ch->frame >= frames_total)
		{
			if (!ch->loop)
			{
				ch->playing = 0;
				break;
			}
			ch->frame = 0;
		}
		n = frames_total - ch->frame;
		if (n > frames_left) n = frames_left;
		if (n > sizeof(buffer) / FRAME_BYTES) n = sizeof(buffer) / FRAME_BYTES;

		src = (const float *)ch->handle->sample + ch->frame * 2;
		for (i = 0; i < n; i++)
		{
			buffer[i * 2]     = src[i * 2]     * left;
			buffer[i * 2 + 1] = src[i * 2 + 1] * right;
		}
		SDL_PutAudioStreamData(stream, buffer, (int)(n * FRAME_BYTES));
		ch->frame += n;
		frames_left -= n;
	}
}

static int RLXAPI Enum(void)
{
	return 1;
}

static int RLXAPI Detect(void)
{
	return 0;
}

static int RLXAPI Initialize(void *hwnd)
{
	SDL_AudioSpec spec;
	UNUSED(hwnd);

	if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		SYS_Msg("Audio: %s", SDL_GetError());
		return -1;
	}
	if (!Sound_Init())
	{
		SYS_Msg("Audio: %s", Sound_GetError());
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return -1;
	}
	spec.format = SDL_AUDIO_F32;
	spec.channels = 2;
	spec.freq = BASE_FREQUENCY;
	g_Device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
	if (!g_Device)
	{
		SYS_Msg("Audio: %s", SDL_GetError());
		Sound_Quit();
		SDL_QuitSubSystem(SDL_INIT_AUDIO);
		return -1;
	}
	SDL_GetAudioDeviceFormat(g_Device, &g_DeviceSpec, NULL);
	memset(g_Channels, 0, sizeof(g_Channels));
	memset(g_Streams, 0, sizeof(g_Streams));
	g_nChannels = 0;
	V3XA.numChannel = 0;
	V3XA.samplingRate = g_DeviceSpec.freq;
	return 0;
}

static void ChannelClose(void)
{
	int i;
	for (i = 0; i < g_nChannels; i++)
	{
		if (g_Channels[i].stream)
		{
			SDL_DestroyAudioStream(g_Channels[i].stream);
			g_Channels[i].stream = NULL;
		}
	}
	g_nChannels = 0;
}

static void RLXAPI Release(void)
{
	V3XAStream_ReleaseAll();
	ChannelClose();
	if (g_Device)
	{
		SDL_CloseAudioDevice(g_Device);
		g_Device = 0;
	}
	Sound_Quit();
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static void RLXAPI SetVolume(float volume)
{
	if (g_Device)
		SDL_SetAudioDeviceGain(g_Device, volume);
}

static void RLXAPI ChannelStop(V3XA_CHANNEL channel)
{
	SND_CHANNEL *ch;
	if ((channel < 0) || (channel >= g_nChannels))
		return;
	ch = &g_Channels[channel];
	SDL_LockAudioStream(ch->stream);
	ch->playing = 0;
	ch->handle = NULL;
	SDL_UnlockAudioStream(ch->stream);
	SDL_ClearAudioStream(ch->stream);
}

static void RLXAPI Start(void)
{
	// Channels play as soon as something is queued on them.
}

static void RLXAPI Stop(void)
{
	int i;
	for (i = 0; i < g_nChannels; i++)
		ChannelStop(i);
}

static int32_t RLXAPI Poll(int32_t v)
{
	return v;
}

static void RLXAPI Render(void)
{
}

static void RLXAPI UserSetParms(V3XMATRIX *lpMAT, V3XVECTOR *lpVEL, float *lpDistanceF, float *lpDopplerF, float *lpRolloffF)
{
	UNUSED(lpMAT); UNUSED(lpVEL); UNUSED(lpDistanceF); UNUSED(lpDopplerF); UNUSED(lpRolloffF);
}

static void RLXAPI ChannelOpen(int nGain, int numChannels)
{
	int i;
	SDL_AudioSpec spec;
	UNUSED(nGain);

	ChannelClose();
	if (numChannels > MAX_CHANNELS)
		numChannels = MAX_CHANNELS;

	spec.format = SDL_AUDIO_F32;
	spec.channels = 2;
	spec.freq = BASE_FREQUENCY;
	for (i = 0; i < numChannels; i++)
	{
		SND_CHANNEL *ch = &g_Channels[i];
		memset(ch, 0, sizeof(*ch));
		ch->stream = SDL_CreateAudioStream(&spec, &g_DeviceSpec);
		if (!ch->stream)
			break;
		SDL_SetAudioStreamGetCallback(ch->stream, ChannelFeed, ch);
		SDL_BindAudioStream(g_Device, ch->stream);
	}
	g_nChannels = i;
	V3XA.numChannel = i;
}

static int RLXAPI ChannelPlay(V3XA_CHANNEL channel, int frequency, float volume, float pan, V3XA_HANDLE *handle)
{
	SND_CHANNEL *ch;
	SDL_AudioSpec spec;
	if ((channel < 0) || (channel >= g_nChannels) || !handle || !handle->sample)
		return -1;
	ch = &g_Channels[channel];

	spec.format = SDL_AUDIO_F32;
	spec.channels = 2;
	spec.freq = (int)handle->samplingRate;

	SDL_LockAudioStream(ch->stream);
	ch->playing = 0;
	SDL_ClearAudioStream(ch->stream);
	SDL_SetAudioStreamFormat(ch->stream, &spec, NULL);
	SDL_SetAudioStreamFrequencyRatio(ch->stream, (frequency > 0) ? (float)frequency / (float)BASE_FREQUENCY : 1.f);
	ch->handle = handle;
	ch->frame = 0;
	ch->loop = (handle->loopend != 0);
	ch->volume = volume;
	ch->pan = pan;
	ch->playing = 1;
	SDL_UnlockAudioStream(ch->stream);
	return 0;
}

static void RLXAPI ChannelSetVolume(V3XA_CHANNEL channel, float volume)
{
	if ((channel < 0) || (channel >= g_nChannels))
		return;
	SDL_LockAudioStream(g_Channels[channel].stream);
	g_Channels[channel].volume = volume;
	SDL_UnlockAudioStream(g_Channels[channel].stream);
}

static void RLXAPI ChannelSetPanning(V3XA_CHANNEL channel, float pan)
{
	if ((channel < 0) || (channel >= g_nChannels))
		return;
	SDL_LockAudioStream(g_Channels[channel].stream);
	g_Channels[channel].pan = pan;
	SDL_UnlockAudioStream(g_Channels[channel].stream);
}

static void RLXAPI ChannelSetSamplingRate(V3XA_CHANNEL channel, int frequency)
{
	if ((channel < 0) || (channel >= g_nChannels) || (frequency <= 0))
		return;
	SDL_SetAudioStreamFrequencyRatio(g_Channels[channel].stream, (float)frequency / (float)BASE_FREQUENCY);
}

static int RLXAPI ChannelGetStatus(V3XA_CHANNEL channel)
{
	if ((channel < 0) || (channel >= g_nChannels))
		return 0;
	return g_Channels[channel].playing;
}

static void RLXAPI ChannelSetParms(V3XA_CHANNEL channel, V3XVECTOR *pos, V3XVECTOR *velocity, V3XRANGE *fRange)
{
	// 3D positioning is done by the game (volume + panning).
	UNUSED(channel); UNUSED(pos); UNUSED(velocity); UNUSED(fRange);
}

static int RLXAPI ChannelSetEnvironment(V3XA_CHANNEL channel, V3XA_REVERBPROPERTIES *cfg)
{
	UNUSED(channel); UNUSED(cfg);
	return 0;
}

// A free channel, or failing that one playing a lower priority sample.
static V3XA_CHANNEL RLXAPI ChannelGetFree(V3XA_HANDLE *handle)
{
	int i, best = -1;
	for (i = 0; i < g_nChannels; i++)
	{
		if (!g_Channels[i].playing)
			return i;
	}
	if (!handle)
		return -1;
	for (i = 0; i < g_nChannels; i++)
	{
		const V3XA_HANDLE *cur = g_Channels[i].handle;
		if (cur && (cur->priority < handle->priority) &&
		    ((best < 0) || (cur->priority < g_Channels[best].handle->priority)))
			best = i;
	}
	if (best >= 0)
		ChannelStop(best);
	return best;
}

static void RLXAPI ChannelFlushAll(int mode)
{
	UNUSED(mode);
	Stop();
}

static void RLXAPI ChannelInvalidate(V3XA_HANDLE *handle)
{
	int i;
	for (i = 0; i < g_nChannels; i++)
	{
		if (g_Channels[i].handle == handle)
			ChannelStop(i);
	}
}

static V3XA_HANDLE *RLXAPI ChannelGetSample(V3XA_CHANNEL channel)
{
	if ((channel < 0) || (channel >= g_nChannels))
		return NULL;
	return g_Channels[channel].handle;
}

//-------------------------------------------------------------------------
// Samples
//-------------------------------------------------------------------------

// Decode a sound file into 32 bit float stereo at its native rate.
int V3XA_Handle_LoadFromFn(V3XA_HANDLE *pHandle, char *szFilename)
{
	Uint32 size = 0;
	Uint8 *data;
	Sound_Sample *sample;
	SDL_AudioSpec dst;
	Uint8 *pcm = NULL;
	int pcmlen = 0;

	memset(pHandle, 0, sizeof(*pHandle));
	data = ReadFile(szFilename, &size);
	if (!data)
	{
		SYS_Msg("Audio: cannot read %s", szFilename);
		return 0;
	}
	sample = Sound_NewSampleFromMem(data, size, FileExtension(szFilename), NULL, STREAM_DECODE_BYTES);
	if (!sample)
	{
		SYS_Msg("Audio: cannot decode %s: %s", szFilename, Sound_GetError());
		SDL_free(data);
		return 0;
	}
	Sound_DecodeAll(sample);

	dst.format = SDL_AUDIO_F32;
	dst.channels = 2;
	dst.freq = sample->actual.freq;
	if (!SDL_ConvertAudioSamples(&sample->actual, (const Uint8 *)sample->buffer, (int)sample->buffer_size, &dst, &pcm, &pcmlen))
	{
		SYS_Msg("Audio: cannot convert %s: %s", szFilename, SDL_GetError());
		Sound_FreeSample(sample);
		SDL_free(data);
		return 0;
	}
	Sound_FreeSample(sample);
	SDL_free(data);

	pHandle->sample = pcm;
	pHandle->length = (u_int32_t)pcmlen;
	pHandle->samplingRate = (u_int32_t)dst.freq;
	pHandle->sampleFormat = V3XA_FMT16BIT | V3XA_FMTSTEREO | V3XA_FMTIEEE;
	return 1;
}

void V3XA_Handle_Release(V3XA_HANDLE *pHandle)
{
	if (pHandle->sample)
	{
		ChannelInvalidate(pHandle);
		SDL_free(pHandle->sample);
		pHandle->sample = NULL;
	}
	pHandle->length = 0;
}

//-------------------------------------------------------------------------
// Streams (music)
//-------------------------------------------------------------------------

static SND_STREAM *StreamGet(V3XA_STREAM handle)
{
	if ((handle <= 0) || (handle >= MAX_STREAMS) || !g_Streams[handle].used)
		return NULL;
	return &g_Streams[handle];
}

static void StreamClose(SND_STREAM *st)
{
	if (st->stream)
	{
		SDL_UnbindAudioStream(st->stream);
		SDL_DestroyAudioStream(st->stream);
	}
	if (st->sample)
		Sound_FreeSample(st->sample);
	SDL_free(st->filedata);
	memset(st, 0, sizeof(*st));
}

int V3XAStream_GetFn(V3XA_STREAM *stream, const char *szFilename, int loop)
{
	SND_STREAM *st = NULL;
	int i;

	*stream = 0;
	if (!g_Device)
		return -1;
	for (i = 1; i < MAX_STREAMS; i++)
	{
		if (!g_Streams[i].used)
		{
			st = &g_Streams[i];
			break;
		}
	}
	if (!st)
		return -20;	// no free slot

	memset(st, 0, sizeof(*st));
	st->filedata = ReadFile(szFilename, &st->filesize);
	if (!st->filedata)
		return -21;	// file not found
	st->sample = Sound_NewSampleFromMem(st->filedata, st->filesize, FileExtension(szFilename), NULL, STREAM_DECODE_BYTES);
	if (!st->sample)
	{
		SYS_Msg("Audio: cannot decode %s: %s", szFilename, Sound_GetError());
		SDL_free(st->filedata);
		st->filedata = NULL;
		return -22;	// codec error
	}
	st->stream = SDL_CreateAudioStream(&st->sample->actual, &g_DeviceSpec);
	if (!st->stream || !SDL_BindAudioStream(g_Device, st->stream))
	{
		StreamClose(st);
		return -23;
	}
	st->used = 1;
	st->loop = loop;
	st->playing = 1;
	*stream = i;
	return 0;
}

// Keep the stream fed; returns 1 while playing, 0 once finished.
int V3XAStream_Poll(V3XA_STREAM handle)
{
	SND_STREAM *st = StreamGet(handle);
	int frame_bytes, target;
	if (!st || !st->playing)
		return 0;

	frame_bytes = SDL_AUDIO_FRAMESIZE(st->sample->actual);
	target = (int)(STREAM_QUEUE_SECONDS * (float)st->sample->actual.freq) * frame_bytes;

	while (!st->eof && (SDL_GetAudioStreamQueued(st->stream) < target))
	{
		Uint32 n = Sound_Decode(st->sample);
		if (n > 0)
			SDL_PutAudioStreamData(st->stream, st->sample->buffer, (int)n);
		if (st->sample->flags & SOUND_SAMPLEFLAG_ERROR)
		{
			st->eof = 1;
		}
		else if (st->sample->flags & SOUND_SAMPLEFLAG_EOF)
		{
			if (st->loop && Sound_Rewind(st->sample))
				continue;
			st->eof = 1;
		}
	}
	if (st->eof && (SDL_GetAudioStreamQueued(st->stream) == 0))
		st->playing = 0;
	return st->playing;
}

int V3XAStream_PollAll(void)
{
	int i, s = 0;
	for (i = 1; i < MAX_STREAMS; i++)
		s += V3XAStream_Poll(i);
	return s;
}

void V3XAStream_SetVolume(V3XA_STREAM handle, V3XA_CHANNEL channel, float volume)
{
	SND_STREAM *st = StreamGet(handle);
	UNUSED(channel);
	if (st)
		SDL_SetAudioStreamGain(st->stream, volume);
}

void V3XAStream_Release(V3XA_STREAM handle)
{
	SND_STREAM *st = StreamGet(handle);
	if (st)
		StreamClose(st);
}

void V3XAStream_ReleaseAll(void)
{
	int i;
	for (i = 1; i < MAX_STREAMS; i++)
	{
		if (g_Streams[i].used)
			StreamClose(&g_Streams[i]);
	}
}

void V3XAStream_Rewind(V3XA_STREAM handle)
{
	SND_STREAM *st = StreamGet(handle);
	if (!st)
		return;
	SDL_ClearAudioStream(st->stream);
	if (Sound_Rewind(st->sample))
	{
		st->eof = 0;
		st->playing = 1;
	}
}

void V3XAStream_Stop(V3XA_STREAM handle)
{
	SND_STREAM *st = StreamGet(handle);
	if (st)
		SDL_UnbindAudioStream(st->stream);
}

int V3XAStream_Start(V3XA_STREAM handle)
{
	SND_STREAM *st = StreamGet(handle);
	if (!st)
		return 0;
	return SDL_BindAudioStream(g_Device, st->stream) ? 1 : 0;
}

//-------------------------------------------------------------------------
// Driver entry point
//-------------------------------------------------------------------------

void RLXAPI V3XA_EntryPoint(struct RLXSYSTEM *rlx)
{
	static V3XA_WaveClientDriver SDL3_Client =
	{
		Enum,
		Detect,
		Initialize,
		Release,
		SetVolume,
		Start,
		Stop,
		Poll,
		Render,
		UserSetParms,
		ChannelOpen,
		ChannelPlay,
		ChannelStop,
		ChannelSetVolume,
		ChannelSetPanning,
		ChannelSetSamplingRate,
		ChannelGetStatus,
		ChannelSetParms,
		ChannelSetEnvironment,
		ChannelGetFree,
		ChannelFlushAll,
		ChannelInvalidate,
		ChannelGetSample,
		"SDL3",
		NULL
	};
	UNUSED(rlx);
	V3XA.Client = &SDL3_Client;
}
