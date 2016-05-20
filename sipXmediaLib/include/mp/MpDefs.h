//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpDefs_h__
#define MpDefs_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES

#if defined(ENABLE_WIDEBAND_AUDIO) && !defined(HAVE_SPEEX)
#error "Wideband is not supported without Speex library"
#endif

/**
 * Maximum number of media connections to flowgraph. Limits the number of participants in a conference.
 * We may have multiple flowgraphs, so this value doesn't limit the total number of calls.
 */
#define MAX_CONNECTIONS 32

/**
 * Sampling rate of flowgraph in Hz. We don't allow custom sampling rate for each flowgraph, since
 * then we would need to do much more resampling which is already quite expensive. MpMediaTask,
 * MpAudioDriverManager will use this sampling rate.
 */
#ifdef ENABLE_WIDEBAND_AUDIO
#define SAMPLES_PER_SECOND 48000
#else
#define SAMPLES_PER_SECOND 8000
#endif
/**
 * Use in code where value 8000 is explicitly required.
 */
#define SAMPLES_PER_SECOND_8KHZ 8000

/**
* Number of audio frames that we use per second.
*
* Keep this value at 100, so that we use 10ms frames.
*/
#define FRAMES_PER_SECOND 100

/**
* Number of samples per frame. We use 10ms frames in flowgraph. For 8000Hz sampling rate, should be 80.
* For 16Khz, should be 160. Samples are 16bit - 2 bytes.
*
*/
#define SAMPLES_PER_FRAME (SAMPLES_PER_SECOND/FRAMES_PER_SECOND)

/**
 * Used by RFC2833 jitter buffer for timestamp step. We run into a problem with timestamp steps in wideband
 * when 8Khz and 16Khz codecs are offered. Then we don't know the timestamp step, and use 80 anyway.
 * It shouldn't cause a problem, since the default jitter buffer doesn't do any prefetch for RFC2833 - it just
 * orders frames.
 */
#define SAMPLES_PER_FRAME_8KHZ 80

/**
 * Noise power level in dBmO. -67 seems to give good results. -80 will be quieter, -50 louder.
 * For noise generation we use Span DSP Hoth noise generator. It is generated in decoder, when 
 */
#define NOISE_LEVEL -67

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

#endif // MpDefs_h__
