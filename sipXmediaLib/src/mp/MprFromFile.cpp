//  
// Copyright (C) 2005-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <os/fstream>
#include <stdio.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsEvent.h"
#include "mp/MpTypes.h"
#include "mp/MpBuf.h"
#include "mp/MprFromFile.h"
#include "mp/MpAudioAbstract.h"
#include "mp/MpAudioFileOpen.h"
#include "mp/MpAudioUtils.h"
#include "mp/MpAudioWaveFileRead.h"
#include "mp/MpFromFileStartResourceMsg.h"
#include "mp/mpau.h"
#include "mp/MpMisc.h"
#include "mp/MpFlowGraphBase.h"
#include <mp/MpResamplerBase.h>
#include <mp/MpResamplerFactory.h>
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
const unsigned int MprFromFile::sFromFileReadBufferSize = 8192;

static const unsigned int MAXFILESIZE = 50000000;
static const unsigned int MINFILESIZE = 8000;

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromFile::MprFromFile(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 0, 1, 1, 1, samplesPerFrame, samplesPerSec),
   mpFileBuffer(NULL),
   mFileRepeat(FALSE),
   m_playingFromFile(false),
   m_pCookie(NULL)
{
}

// Destructor
MprFromFile::~MprFromFile()
{
   if(mpFileBuffer) delete mpFileBuffer;
   mpFileBuffer = NULL;
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprFromFile::playBuffer(const void* audioBuffer, size_t bufSize, 
                                 int type, UtlBoolean repeat, void* pCookie)
{
   UtlString* fgAudBuffer = NULL;
   OsStatus res = genericAudioBufToFGAudioBuf(fgAudBuffer, audioBuffer, 
                                              bufSize, type);

   if(res == OS_SUCCESS)
   {
      m_playingFromFile = false;
      m_pCookie = pCookie;

      // send message to this resource to start playing
      // message will be processed in audio thread at the start of next audio frame
      MpFlowGraphMsg msg(PLAY_BUFFER, this, NULL, fgAudBuffer,
                         repeat ? PLAY_REPEAT : PLAY_ONCE, 0);
      res = postMessage(msg);
   }

   return res;
}

OsStatus MprFromFile::playFile(const char* audioFileName, 
                               UtlBoolean repeat,
                               void* pCookie)
{
   UtlString* audioBuffer = NULL;
   OsStatus stat = readAudioFile(audioBuffer, audioFileName);

   //create a msg from the buffer
   if (audioBuffer && audioBuffer->length())
   {
      m_playingFromFile = true;
      m_pCookie = pCookie;

	   MpFlowGraphMsg msg(PLAY_FILE, this, NULL, audioBuffer,
                         repeat ? PLAY_REPEAT : PLAY_ONCE, 0);

      //now post the msg (with the audio data) to be played
      stat = postMessage(msg);
   }

   return stat;
}

// stop file play
OsStatus MprFromFile::stopPlayback(void)
{
   MpFlowGraphMsg msg((m_playingFromFile ? STOP_FILE : STOP_BUFFER), this, NULL, NULL, 0, 0);
   return postMessage(msg);
}

OsStatus MprFromFile::pausePlayback(void)
{
   MpFlowGraphMsg msg(PAUSE_PLAYBACK, this, NULL, NULL, 0, 0);
   return postMessage(msg);
}

OsStatus MprFromFile::resumePlayback(void)
{
   MpFlowGraphMsg msg(RESUME_PLAYBACK, this, NULL, NULL, 0, 0);
   return postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsStatus MprFromFile::genericAudioBufToFGAudioBuf(UtlString*& fgAudioBuf, 
                                                  const void* audioBuffer, 
                                                  size_t bufSize, 
                                                  int type)
{
   OsStatus stat = OS_SUCCESS;
   char* convertedBuffer = NULL;

   assert(fgAudioBuf == NULL); // assume UtlString buffer pointer is null.
   fgAudioBuf = new UtlString();

   if (fgAudioBuf)
   {
      switch(type)
      {
      case 0 : fgAudioBuf->append((char*)audioBuffer,bufSize);
         break;

      case 1 : convertedBuffer = new char[bufSize*2];
         //NOTE by Keith Kyzivat - this code was pulled directly from
         // the old implementation of playBuffer... obviously broken here.
         //TODO: actually convert the buffer
         fgAudioBuf->append(convertedBuffer,bufSize);
         delete[] convertedBuffer; 
         break;
      }
   }
   else
   {
      stat = OS_INVALID_ARGUMENT;
   }

   return stat;
}

OsStatus MprFromFile::readAudioFile(UtlString*& audioBuffer,
                                    const char* audioFileName)
{
   char* pAudioBuffer = NULL;
   FILE* audioFilePtr = NULL;
   int iTotalChannels = 1;
   unsigned long filesize;
   unsigned long trueFilesize;
   int samplesRead;
   int compressionType = 0;
   int channelsMin = 1, channelsMax = 2, channelsPreferred = 0;
   long rateMin = 8000, rateMax = 48000, ratePreferred = 22050;
   UtlBoolean bDetectedFormatIsOk = TRUE;
   MpAudioAbstract *audioFile = NULL;
   int requiredSamplesPerSec = getSamplesPerSec();

   // Assume audioBuffer passed in is NULL..
   assert(audioBuffer == NULL);
   audioBuffer = NULL;


   if (!audioFileName)
      return OS_INVALID_ARGUMENT;

   ifstream inputFile(audioFileName,ios::in|ios::binary);

   if (!inputFile.good())
   {
      return OS_INVALID_ARGUMENT;
   }

   //get file size
   inputFile.seekg(0, ios::end);
   filesize = trueFilesize = inputFile.tellg();
   inputFile.seekg(0);

   //we have to have at least one sample to play
   if (trueFilesize < sizeof(AudioSample))  
   {
      osPrintf("WARNING: %s contains less than one sample to play. "
         "Skipping play.\n", audioFileName);
      return OS_INVALID_ARGUMENT;
   }

   if (trueFilesize > MAXFILESIZE)
   {
      osPrintf("playFile('%s') WARNING:\n"
         "    length (%lu) exceeds size limit (%d)\n",
         audioFileName, trueFilesize, MAXFILESIZE);
      filesize = MAXFILESIZE;
   }

   if (trueFilesize < MINFILESIZE)
   {
      osPrintf("playFile('%s') WARNING:\n"
         "    length (%lu) is suspiciously short!\n",
         audioFileName, trueFilesize);
   }


   audioFile = MpOpenFormat(inputFile);
   //if we have an audioFile object, then it must be a known file type
   //otherwise, lets treat it as RAW
   if (audioFile)
   {
      if (audioFile->isOk())
      {
         audioFile->minMaxChannels(&channelsMin,
            &channelsMax, &channelsPreferred);

         if (channelsMin > channelsMax) 
         {
            osPrintf("Couldn't negotiate channels.\n");
            bDetectedFormatIsOk = FALSE;
         }

         audioFile->minMaxSamplingRate(&rateMin,&rateMax,&ratePreferred);
         if (rateMin > rateMax) 
         {
            osPrintf("Couldn't negotiate rate.\n");
            bDetectedFormatIsOk = FALSE;
         }
      }
      else
         bDetectedFormatIsOk = FALSE;

      if (bDetectedFormatIsOk)
      {
         iTotalChannels = channelsPreferred;
         compressionType = audioFile->getDecompressionType();
      }
      else
      {
         osPrintf("\nERROR: Could not detect format correctly. "
            "Should be AU or WAV or RAW\n");
      }

      // First, figure out which kind of file it is
      if (bDetectedFormatIsOk && 
         audioFile->getAudioFormat() == AUDIO_FORMAT_WAV)
      {

         // Get actual data size without header.
         filesize = audioFile->getBytesSize();

         switch(compressionType) 
         {
         case MpAudioWaveFileRead::DePcm8Unsigned: //8
            // We'll convert it to 16 bit
            filesize *= sizeof(AudioSample);
            pAudioBuffer = (char*)malloc(filesize);
            samplesRead = audioFile->getSamples((AudioSample*)pAudioBuffer,
                                                  filesize);

            if (samplesRead) 
            {
               assert(samplesRead*sizeof(AudioSample) == filesize);

               // Convert to mono if needed
               if (channelsPreferred > 1)
                  filesize = mergeChannels(pAudioBuffer, filesize, iTotalChannels);

               char* pOutputBuffer = NULL;
               // Resample if needed
               if (ratePreferred != requiredSamplesPerSec)
                  filesize = resample(pAudioBuffer, filesize, ratePreferred, requiredSamplesPerSec, pOutputBuffer);
               if (pOutputBuffer)
               {
                  free(pAudioBuffer);
                  pAudioBuffer = pOutputBuffer;
               }
            }
            break;

         case MpAudioWaveFileRead::DePcm16LsbSigned: // 16
            pAudioBuffer = (char*)malloc(filesize);
            samplesRead = audioFile->getSamples((AudioSample*)pAudioBuffer,
                                                  filesize/sizeof(AudioSample));
            if (samplesRead)
            {
               assert(samplesRead*sizeof(AudioSample) == filesize);

               // Convert to mono if needed
               if (iTotalChannels > 1)
                  filesize = mergeChannels(pAudioBuffer, filesize, iTotalChannels);

               char* pOutputBuffer = NULL;
               // Resample if needed
               if (ratePreferred != requiredSamplesPerSec)
                  filesize = resample(pAudioBuffer, filesize, ratePreferred, requiredSamplesPerSec, pOutputBuffer);
               if (pOutputBuffer)
               {
                  free(pAudioBuffer);
                  pAudioBuffer = pOutputBuffer;
               }
            }
            break;
         }
      }
      else
         if (bDetectedFormatIsOk && 
            audioFile->getAudioFormat() == AUDIO_FORMAT_AU)
         {

            // Get actual data size without header.
            filesize = audioFile->getBytesSize();

            switch(compressionType)
            {
            case MpAuRead::DePcm8Unsigned:
               break; //do nothing for this format

            case MpAuRead::DeG711MuLaw:
               pAudioBuffer = (char*)malloc(filesize*2);
               samplesRead = audioFile->getSamples((AudioSample*)pAudioBuffer, filesize);
               if (samplesRead) 
               {

                  //it's now 16 bit so it's twice as long
                  filesize *= sizeof(AudioSample);

                  // Convert to mono if needed
                  if (channelsPreferred > 1)
                     filesize = mergeChannels(pAudioBuffer, filesize, iTotalChannels);
                  
                  char* pOutputBuffer = NULL;
                  // Resample if needed
                  if (ratePreferred != requiredSamplesPerSec)
                     filesize = resample(pAudioBuffer, filesize, ratePreferred, requiredSamplesPerSec, pOutputBuffer);
                  if (pOutputBuffer)
                  {
                     free(pAudioBuffer);
                     pAudioBuffer = pOutputBuffer;
                  }
               }
               break;

            case MpAuRead::DePcm16MsbSigned:
               pAudioBuffer = (char*)malloc(filesize);
               samplesRead = audioFile->getSamples((AudioSample*)pAudioBuffer,
                                                     filesize/sizeof(AudioSample));
               if (samplesRead) 
               {
                  assert(samplesRead*sizeof(AudioSample) == filesize);

                  // Convert to mono if needed
                  if (channelsPreferred > 1)
                     filesize = mergeChannels(pAudioBuffer, filesize, iTotalChannels);

                  char* pOutputBuffer = NULL;
                  // Resample if needed
                  if (ratePreferred != requiredSamplesPerSec)
                     filesize = resample(pAudioBuffer, filesize, ratePreferred, requiredSamplesPerSec, pOutputBuffer);
                  if (pOutputBuffer)
                  {
                     free(pAudioBuffer);
                     pAudioBuffer = pOutputBuffer;
                  }
               }
               break;
            }
         } 
         else 
         {
            OsSysLog::add(FAC_MP, PRI_ERR, 
               "ERROR: Detected audio file is bad.  "
               "Must be MONO, 16bit signed wav or u-law au");
         }

         //remove object used to determine rate, compression, etc.
         delete audioFile;
         audioFile = NULL;
   }
   else
   {
#if 0
      osPrintf("AudioFile: raw file\n");
#endif

      // if we cannot determine the format of the audio file,
      // and if the ext of the file is .ulaw, we assume it is a no-header
      // raw file of ulaw audio, 8 bit, 8kHZ.
      if (strstr(audioFileName, ".ulaw"))
      {
         ratePreferred = 8000;
         channelsPreferred = 1;
         audioFile = new MpAuRead(inputFile, 1);
         if (audioFile)
         {
            filesize *= sizeof(AudioSample);
            pAudioBuffer = (char*)malloc(filesize);

            samplesRead = audioFile->getSamples((AudioSample*)pAudioBuffer,
                                                  filesize/sizeof(AudioSample));
         }
      }
      else // the file extension is not .ulaw ... 
      {
         if (0 != (audioFilePtr = fopen(audioFileName, "rb")))
         {
            unsigned int cbIdx = 0;
            int bytesRead = 0;
            pAudioBuffer = (char*)malloc(filesize);
            assert(pAudioBuffer != NULL); // Assume malloc succeeds.

            // Read in the unknown audio file a chunk at a time.
            // (specified by sFromFileReadBufferSize)
            while((cbIdx < filesize) &&
               ((bytesRead = fread(pAudioBuffer+cbIdx, 1, 
               sFromFileReadBufferSize, 
               audioFilePtr)) > 0))
            {
               cbIdx += bytesRead;
            }

            // Now that we're done with the unknown raw audio file
            // close it up.
            fclose(audioFilePtr);
         }
      }
   }

   // Now we copy over the char buffer data to UtlString for use in
   // messages.
   if (pAudioBuffer)
   {
      audioBuffer = new UtlString();
      if (audioBuffer)
      {
         audioBuffer->append(pAudioBuffer, filesize);
      }
      free(pAudioBuffer);
   }

   return OS_SUCCESS;
}

UtlBoolean MprFromFile::doProcessFrame(MpBufPtr inBufs[],
                                       MpBufPtr outBufs[],
                                       int inBufsSize,
                                       int outBufsSize,
                                       UtlBoolean isEnabled,
                                       int samplesPerFrame,
                                       int samplesPerSecond)
{
   MpAudioBufPtr out;
   MpAudioSample *outbuf;
   int count;
   int bytesLeft;

   // There's nothing to do if the output buffers or the number
   // of samples per frame are zero, so just return.
   if (outBufsSize == 0 || samplesPerFrame == 0)
   {
       return FALSE;
   }

   if (isEnabled) 
   {
      if (mpFileBuffer)
      {
         // Get new buffer
         out = MpMisc.m_pRawAudioPool->getBuffer();
         if (!out.isValid())
         {
            return FALSE;
         }
         out->setSpeechType(MP_SPEECH_TONE);
         out->setSamplesNumber(samplesPerFrame);
         count = out->getSamplesNumber();
         outbuf = out->getSamplesWritePtr();

         int bytesPerFrame = count * sizeof(MpAudioSample);
         int bufferLength = mpFileBuffer->length();
         int totalBytesRead = 0;

         if(mFileBufferIndex < bufferLength)
         {
            totalBytesRead = bufferLength - mFileBufferIndex;
            totalBytesRead = min(totalBytesRead, bytesPerFrame);
            memcpy(outbuf, &(mpFileBuffer->data()[mFileBufferIndex]),
                   totalBytesRead);
            mFileBufferIndex += totalBytesRead;
         }

         if (mFileRepeat) 
         {
            bytesLeft = 1;
            while((totalBytesRead < bytesPerFrame) && (bytesLeft > 0))
            {
               mFileBufferIndex = 0;
               bytesLeft = min(bufferLength - mFileBufferIndex,
                               bytesPerFrame - totalBytesRead);
               memcpy(&outbuf[(totalBytesRead/sizeof(MpAudioSample))],
                      &(mpFileBuffer->data()[mFileBufferIndex]), bytesLeft);
               totalBytesRead += bytesLeft;
               mFileBufferIndex += bytesLeft;
            }
         }
         else 
         {
            if (mFileBufferIndex >= bufferLength) 
            {
               // We're done playing..
               // zero out the remaining bytes in the frame after the end
               // of the real data before sending it off - it could be garbage.
               bytesLeft = bytesPerFrame - totalBytesRead;
               memset(&outbuf[(totalBytesRead/sizeof(MpAudioSample))], 0, bytesLeft);

               // Send a message to tell this resource to stop playing the file
               // this resets some state, and sends a notification.
               stopPlayback();
            }
         }
      }
      else
      {
         // No audio buffer, pass through input data. It could occur if somebody wants to play file
         // but supplies wrong filename. Then resource would be enabled but buffer would be NULL.
         out.swap(inBufs[0]);
      }
   }
   else
   {
      // Resource is disabled. Pass through input data
      out.swap(inBufs[0]);
   }

   // Push audio data downstream
   outBufs[0] = out;

   return TRUE;
}

// Handle messages for this resource.

UtlBoolean MprFromFile::handleSetup(MpFlowGraphMsg& rMsg)
{
   // replace internal buffer
   if(mpFileBuffer) delete mpFileBuffer;
   mpFileBuffer = (UtlString*) rMsg.getPtr2();

   if(mpFileBuffer)
   {
      mFileBufferIndex = 0;
      mFileRepeat = (rMsg.getInt1() == PLAY_ONCE) ? FALSE : TRUE;

      switch (rMsg.getMsg()) 
      {
      case PLAY_BUFFER:
         sendInterfaceNotification(MP_NOTIFICATION_START_PLAY_BUFFER, (intptr_t)m_pCookie, mFileBufferIndex);
         break;
      case PLAY_FILE:
         sendInterfaceNotification(MP_NOTIFICATION_START_PLAY_FILE, (intptr_t)m_pCookie, mFileBufferIndex);
         break;
      }
   }

   return TRUE;
}

// this is used in both old and new messaging schemes to do reset state
// and send notification when stop is requested.
UtlBoolean MprFromFile::handleStop(MpFlowGraphMsg& rMsg)
{
   // Send a notification -- we don't really care at this level if
   // it succeeded or not.
   if (mpFileBuffer)
   {
      switch (rMsg.getMsg()) 
      {
      case STOP_FILE:
         sendInterfaceNotification(MP_NOTIFICATION_STOP_PLAY_FILE, (intptr_t)m_pCookie, mFileBufferIndex);
         break;
      case STOP_BUFFER:
         sendInterfaceNotification(MP_NOTIFICATION_STOP_PLAY_BUFFER, (intptr_t)m_pCookie, mFileBufferIndex);
         break;
      }
   }

   delete mpFileBuffer;
   mpFileBuffer = NULL;
   mFileBufferIndex = 0;
   disable();
   return TRUE;
}

UtlBoolean MprFromFile::handlePause()
{
   if (mpFileBuffer)
   {
      sendInterfaceNotification(MP_NOTIFICATION_PAUSE_PLAYBACK, (intptr_t)m_pCookie, mFileBufferIndex);
      disable();
   }

   return TRUE;
}

UtlBoolean MprFromFile::handleResume()
{
   if (mpFileBuffer)
   {
      sendInterfaceNotification(MP_NOTIFICATION_RESUME_PLAYBACK, (intptr_t)m_pCookie, mFileBufferIndex);
      enable();
   }

   return TRUE;
}

UtlBoolean MprFromFile::handleMessage(MpFlowGraphMsg& rMsg)
{
   switch (rMsg.getMsg()) 
   {
   case PLAY_BUFFER:
   case PLAY_FILE:
      return handleSetup(rMsg);
      break;
   case STOP_BUFFER:
   case STOP_FILE:
      return handleStop(rMsg);
      break;
   case PAUSE_PLAYBACK:
      return handlePause();

   case RESUME_PLAYBACK:
      return handleResume();

   default:
      return MpAudioResource::handleMessage(rMsg);
      break;
   }
   return TRUE;
}


// New resource message handling.  This is part of the new
// messaging infrastructure (2007).
UtlBoolean MprFromFile::handleMessage(MpResourceMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   MpFromFileStartResourceMsg* ffsRMsg = NULL;
   switch (rMsg.getMsg()) 
   {
   case MpResourceMsg::MPRM_FROMFILE_START:
      ffsRMsg = (MpFromFileStartResourceMsg*)&rMsg;

      // Enable this resource - as it's disabled automatically when the last file ends.
      enable();
      if(mpFileBuffer) delete mpFileBuffer;
      mpFileBuffer = (UtlString*) ffsRMsg->getAudioBuffer();
      if(mpFileBuffer) 
      {
         mFileBufferIndex = 0;
         mFileRepeat = ffsRMsg->isRepeating();
      }
      msgHandled = TRUE;
      break;

   /*case MpResourceMsg::MPRM_FROMFILE_STOP:
      handleStop(); // this will never be called as this msg handling is disabled
      msgHandled = TRUE;
      break;*/

   default:
      // If we don't handle the message here, let our parent try.
      msgHandled = MpResource::handleMessage(rMsg); 
      break;
   }
   return msgHandled;
}

int MprFromFile::resample(char* inBuffer,
                          int numBytes,
                          int currentSampleRate,
                          int newSampleRate,
                          char*& outBuffer)
{
   if (inBuffer && numBytes > 0)
   {
      MpResamplerBase* pResampler = MpResamplerFactory::createResampler(currentSampleRate, newSampleRate);
      uint32_t inBufferLength = numBytes / sizeof(MpAudioSample); // size of buffer in samples
      uint32_t inSamplesProcessed;
      uint32_t outSamplesWritten;
      // calculate size of output buffer in samples
      uint32_t outBufferLength = inBufferLength * newSampleRate / currentSampleRate;
      MpAudioSample* pOutputBuffer = (MpAudioSample*)malloc(outBufferLength * sizeof(MpAudioSample));
      memset(pOutputBuffer, 0, outBufferLength * sizeof(MpAudioSample));
      // resample
      pResampler->resample((MpAudioSample*)inBuffer, inBufferLength, inSamplesProcessed,
         (MpAudioSample*)pOutputBuffer, outBufferLength, outSamplesWritten);

      outBuffer = (char*)pOutputBuffer;

      delete pResampler;
      pResampler = NULL;

      return outSamplesWritten * sizeof(MpAudioSample);
   }
   else
   {
      return 0;
   }
}

/* ============================ FUNCTIONS ================================= */


