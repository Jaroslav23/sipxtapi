//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprFromFile_h_
#define _MprFromFile_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpAudioResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief The "Audio from file" media processing resource
*/
class MprFromFile : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     //:Constructor
   MprFromFile(const UtlString& rName, int samplesPerFrame, int samplesPerSec);

     //:Destructor
   virtual
   ~MprFromFile();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

      /// Play sound from buffer w/repeat option
    OsStatus playBuffer(const void* audioBuffer, size_t bufSize, 
                        int type, UtlBoolean repeat, void* pCookie = NULL);
      /**<
      *  @param type - can be one of following:  (need a OsSoundType)<br>
      *  0 = RAW<br>
      *  1 = muLaw
      *
      *  @param repeat - TRUE/FALSE after the fromFile reaches the end of
      *   the file, go back to the beginning and continue to play.  Note this
      *   assumes that the file was opened for read.
      *
      *  @returns the result of attempting to queue the message to this
      *  resource and/or opening the named file.
      */

      /// Play sound from buffer w/repeat option

     /// Old Play from file w/file name and repeat option
   OsStatus playFile(const char* fileName, UtlBoolean repeat, void* pCookie = NULL);
     /**<
     *  Note: if this resource is deleted before <I>stopFile</I> is called, it
     *  will close the file.
     *
     *  @param fileName - name of file from which to read raw audio data in
     *   exact format of the flowgraph (sample size, rate & number of channels).
     *
     *  @param repeat - TRUE/FALSE after the fromFile reaches the end of
     *   the file, go back to the beginning and continue to play.  Note this
     *   assumes that the file was opened for read.
     *
     *  @returns the result of attempting to queue the message to this
     *  resource and/or opening the named file.
     */

     /// Stop playing from file
   OsStatus stopPlayback(void);
     /**<
     *  Sends a STOP_FILE message to this resource to stop playing audio
     *  from file
     *
     *  @returns the result of attempting to queue the message to this
     *  resource.
     */

     /// @brief Sends an PAUSE_PLAYBACK message to this resource to pause playing audio
    OsStatus pausePlayback(void);
     /**<
     *  Sends an PAUSE_PLAYBACK message to this resource to pause playing audio
     *
     *  @returns the result of attempting to queue the message to this resource.
     */

    /// @brief Sends an RESUME_PLAYBACK message to this resource to resume playing audio
    OsStatus resumePlayback(void);
    /**<
    *  Sends an RESUME_PLAYBACK message to this resource to resume playing audio
    *
    *  @returns the result of attempting to queue the message to this resource.
    */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   typedef enum
   {
      PLAY_FILE = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      STOP_FILE,
  	   PLAY_BUFFER,
	   STOP_BUFFER,
      PAUSE_PLAYBACK,
      RESUME_PLAYBACK
   } AddlMsgTypes;

   typedef enum
   {
       PLAY_ONCE,
       PLAY_REPEAT
   } MessageAttributes;

   static const unsigned int sFromFileReadBufferSize;

   UtlString* mpFileBuffer;
   void* m_pCookie;
   int mFileBufferIndex;
   UtlBoolean mFileRepeat;
   bool m_playingFromFile;

     /// @brief Convert generic audio data into flowgraph audio data.
   static OsStatus genericAudioBufToFGAudioBuf(
                                             UtlString*& fgAudioBuf,
                                             const void* audioBuffer, 
                                             size_t bufSize, 
                                             int type);
     /**<
     *  Method to convert a generic char* audio buffer, in one of several
     *  acceptable formats, to a format that can be passed through the 
     *  flowgraph.
     *
     *  @param type - can be one of following:  (need a OsSoundType)<br>
     *  0 = RAW<br>
     *  1 = muLaw
     *
     *  @param repeat - TRUE/FALSE after the fromFile reaches the end of
     *   the file, go back to the beginning and continue to play.  Note this
     *   assumes that the file was opened for read.
     *
     *  @returns the result of attempting to queue the message to this
     *  resource and/or opening the named file.
     */

     /// Read in an audio file into a new UtlString audio buffer.
   OsStatus readAudioFile(UtlString*& audioBuffer,
                          const char* audioFileName);
     /**<
     *  @param audioBuffer - a reference to a pointer that will be filled
     *   with a new buffer holding the audio data.  Ownership will then
     *   transfer to the caller.
     *
     *  @param audioFileName - the name of a file to read flowgraph
     *   audio data from.  (exact format that the FG will accept -
     *   sample size, rate, & number of channels)
     *
     *  @param event - an event to signal when state changes.  If NULL,
     *  nothing will be signaled.
     *
     *  @returns OS_INVALID_ARGUMENT if the filename was null,
     *  the file was unopenable, or the file contained less than one sample.
     *  @returns OS_SUCCESS if the file was read successfully.
     */

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

     /// Perform resetting of state, etc. upon receiving request to stop playing.
   virtual UtlBoolean handleStop(MpFlowGraphMsg& rMsg);

   virtual UtlBoolean handlePause(void);

   virtual UtlBoolean handleResume(void);

   virtual UtlBoolean handleSetup(MpFlowGraphMsg& rMsg);

     /// Handle flowgraph messages for this resource (old messaging model).
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Handle resource messages for this resource (new messaging model - 2007).
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);

   /**
    * Resamples buffer to new sampler rate. Function will allocate required amount of memory
    * and pass the buffer out in outBuffer.
    *
    * @return number of bytes in output buffer.
    */
   int resample(char* inBuffer,
                int numBytes,
                int currentSampleRate,
                int newSampleRate,
                char*& outBuffer);

     /// Copy constructor (not implemented for this class)
   MprFromFile(const MprFromFile& rMprFromFile);

     /// Assignment operator (not implemented for this class)
   MprFromFile& operator=(const MprFromFile& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromFile_h_
