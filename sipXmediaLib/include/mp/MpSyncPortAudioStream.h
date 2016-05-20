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

#ifndef MpSyncPortAudioStream_h__
#define MpSyncPortAudioStream_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpPortAudioStreamBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * This class represents synchronous port audio stream.
 */
class MpSyncPortAudioStream : public MpPortAudioStreamBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor */
   MpSyncPortAudioStream(MpAudioStreamId streamId,
                         int outputChannelCount,
                         int inputChannelCount,
                         MpAudioDriverSampleFormat outputSampleFormat,
                         MpAudioDriverSampleFormat inputSampleFormat,
                         double sampleRate,
                         unsigned long framesPerBuffer);

   /** Destructor */
   virtual ~MpSyncPortAudioStream();

   /* ============================ MANIPULATORS ============================== */

   /**
   * Reads given number of frames into buffer. Buffer must be allocated memory.
   *
   * @param buffer Allocated memory to read data into
   * @param frames Number of frames to read
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus readStream(void *buffer,
                               unsigned long frames);

   /**
   * Writes given number of frames into stream from buffer.
   *
   * @param buffer Buffer with samples
   * @param frames Number of frames to write to stream
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus writeStream(const void *buffer,
                                unsigned long frames);

   /* ============================ ACCESSORS ================================= */

   /**
    * Returns input latency in seconds of the stream.
    */
   virtual double getInputLatency() const;

   /**
    * Returns output latency in seconds of the stream.
    */
   virtual double getOutputLatency() const;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // MpSyncPortAudioStream_h__
