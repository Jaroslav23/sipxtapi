//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpAudioDriverDefs_h__
#define MpAudioDriverDefs_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES

/**
* Use for MpAudioDriverSampleFormat
*/
#define MP_AUDIO_FORMAT_FLOAT32        0x00000001
#define MP_AUDIO_FORMAT_INT32          0x00000002
#define MP_AUDIO_FORMAT_INT24          0x00000004
#define MP_AUDIO_FORMAT_INT16          0x00000008
#define MP_AUDIO_FORMAT_INT8           0x00000010
#define MP_AUDIO_FORMAT_UINT8          0x00000020
#define MP_AUDIO_FORMAT_NONINTERLEAVED 0x80000000

/**
* Use for MpAudioStreamFlags. For other drivers than portaudio, might be unsupported
*/
#define   MP_AUDIO_STREAM_NOFLAG          0
#define   MP_AUDIO_STREAM_CLIPOFF         0x00000001
#define   MP_AUDIO_STREAM_DITHEROFF       0x00000002
#define   MP_AUDIO_STREAM_NEVERDROPINPUT  0x00000004
#define   MP_AUDIO_STREAM_PRIMEOUTPUTBUFFERSUSINGSTREAMCALLBACK 0x00000008

#define MP_AUDIO_STREAM_FRAMESPERBUFFERUNSPECIFIED 0

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS

typedef enum
{
   MP_VOLUME_METER_VU = 0,
   MP_VOLUME_METER_PPM
} MP_VOLUME_METER_TYPE;

typedef unsigned long MpAudioDriverSampleFormat;
typedef unsigned long MpAudioStreamFlags;
typedef int MpAudioDeviceIndex;
typedef void* MpAudioStreamId;

typedef float MpAudioVolume; ///< 0.0 (min) --> 1.0 (max)
typedef float MpAudioBalance; ///< -1.0 (left) --> 1.0 (right)

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

#endif // MpAudioDriverDefs_h__
