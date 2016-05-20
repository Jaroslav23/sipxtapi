//  
// Copyright (C) 2006-2007 SIPez LLC. 
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

#define DEBUG_DECODING
#undef DEBUG_DECODING

// SYSTEM INCLUDES
#include <assert.h>

#ifdef WIN32 /* [ */
#include <winsock2.h>
#endif /* WIN32 ] */

#if defined(_VXWORKS) || defined(__pingtel_on_posix__) /* [ */
#include <sys/types.h>
#include <netinet/in.h>
#endif /* _VXWORKS || __pingtel_on_posix__ ] */

#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include <utl/UtlSListIterator.h>
#include "mp/MpResNotification.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MpRtpInputAudioConnection.h"
#include "mp/MprDecode.h"
#include "mp/MprDejitter.h"
#include "mp/MpDecoderBase.h"
#include "mp/NetInTask.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCodecFactory.h"
#include "mp/MpDecodeBuffer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprDecode::MprDecode(const UtlString& rName, MpRtpInputAudioConnection* pConn,
                     int samplesPerFrame, int samplesPerSec)
:  MpAudioResource(rName, 0, 0, 1, 1, samplesPerFrame, samplesPerSec),
   m_pDecodeBuffer(NULL),
   m_pMyDejitter(NULL),
   m_pCurrentDecoders(NULL),
   m_nCurrentDecoders(0),
   mpPrevCodecs(NULL),
   mNumPrevCodecs(0),
   mpConnection(pConn)
{
}

// Destructor
MprDecode::~MprDecode()
{
   // Clean up decoder object
   int i;

   handleDeselectCodecs();

   // Delete the list of decoders used in the past.
   {
      OsLock lock(mLock);
      if (mNumPrevCodecs > 0)
      {
         for (i = 0; i < mNumPrevCodecs; i++)
         {
            mpPrevCodecs[i]->freeDecode();
            delete mpPrevCodecs[i];
            mpPrevCodecs[i] = NULL;
         }
         delete[] mpPrevCodecs;
         mNumPrevCodecs = 0;
      }
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus MprDecode::selectCodecs(const UtlSList& codecList)
{
   OsStatus ret = OS_SUCCESS;
   SdpCodec** codecArray;
   int audioCodecsNum = 0;
   UtlString codecMediaType;
   MpFlowGraphMsg msg(SELECT_CODECS, this, NULL, NULL, 0, 0);
   int numCodecs = (int)codecList.entries();

   codecArray = new SdpCodec*[numCodecs];
   SdpCodec* pCodec = NULL;
   UtlSListIterator itor(codecList);
   while (itor())
   {
      pCodec = (SdpCodec*)itor.item();
      if (pCodec)
      {
         pCodec->getMediaType(codecMediaType);
         if (codecMediaType.compareTo("audio") == 0)
         {
            codecArray[audioCodecsNum] = new SdpCodec(*pCodec);
            audioCodecsNum++;
         }
      }
   }

   msg.setPtr1(codecArray);
   msg.setInt1(audioCodecsNum);
   ret = postMessage(msg);

   return ret;
}

OsStatus MprDecode::deselectCodec()
{
   MpFlowGraphMsg msg(DESELECT_CODECS, this, NULL, NULL, 0, 0);
   OsStatus ret = OS_SUCCESS;

//   osPrintf("MprDecode::deselectCodec\n");
   ret = postMessage(msg);

   return ret;
}

void MprDecode::setMyDejitter(MprDejitter* pDJ)
{
   m_pMyDejitter = pDJ;
}

void MprDecode::onNotify(UtlObservable* subject, int code, intptr_t userData)
{
   MpResNotificationType type = (MpResNotificationType)code;

   switch(type)
   {
   case MP_RES_DTMF_2833_NOTIFICATION:
      // DTMF notification received from MpdPtAVT
      // send notification up to parent
      notify(MP_RES_DTMF_2833_NOTIFICATION, userData);
      break;
   default:
      ;
   }
}

/* ============================ ACCESSORS ================================= */

MpDecodeBuffer* MprDecode::getDecodeBuffer(UtlBoolean optional)
{
   if ((NULL == m_pDecodeBuffer) && (!optional))
   {
      m_pDecodeBuffer = new MpDecodeBuffer(m_pMyDejitter, getSamplesPerFrame(), getSamplesPerSec());
      assert(NULL != m_pDecodeBuffer);
   }
   return m_pDecodeBuffer;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

MprDejitter* MprDecode::getMyDejitter(void)
{
   assert(NULL != m_pMyDejitter);
   return m_pMyDejitter;
}

UtlBoolean MprDecode::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   if (outBufsSize == 0)
      return FALSE;

   if (!isEnabled)
   {
      return TRUE;
   }

   // Not sure this is a good idea to do in doProcessFrame, 
   // but the use of this lock is meaningless
   // unless we lock all access of the mpCurrentCodecs
   OsLock lock(mLock);

   // Get new audio buffer for decoded sound
   MpAudioBufPtr out = MpMisc.m_pRawAudioPool->getBuffer();
   if (!out.isValid())
   {
      return FALSE;
   }

   out->setSamplesNumber(samplesPerFrame);
   MpAudioSample* pSamples = out->getSamplesWritePtr();
   memset(pSamples, 0, out->getSamplesNumber() * sizeof(MpAudioSample));
   out->setSpeechType(MP_SPEECH_SILENT);

   // Decode one packet from Jitter Buffer
   MpDecodeBuffer* pDecodeBuffer = getDecodeBuffer();
   if (pDecodeBuffer)
   {
      // This should be a JB_something or other.  However the only
      // current choices is a short or long equivalent and this needs
      // to be a plain old int:
      MpSpeechType speechType;
      pDecodeBuffer->getSamples(pSamples, samplesPerFrame, speechType);
      assert(samplesPerFrame == (int)out->getSamplesNumber());
      out->setSpeechType(speechType);
   }
   
   // Push decoded audio packet downstream
   outBufs[0] = out;

   return TRUE;
}

// Handle messages for this resource.
UtlBoolean MprDecode::handleMessage(MpFlowGraphMsg& rMsg)
{
   UtlBoolean ret = FALSE;

   switch (rMsg.getMsg()) {
   case DESELECT_CODECS:
      handleDeselectCodecs();
      ret = TRUE;
      break;
   case SELECT_CODECS:
      handleSelectCodecs((SdpCodec**) rMsg.getPtr1(), rMsg.getInt1());
      ret = TRUE;
      break;
   default:
      ret = MpAudioResource::handleMessage(rMsg);
      break;
   }
   return ret;
}

UtlBoolean MprDecode::handleSelectCodecs(SdpCodec* pCodecs[], int numCodecs)
{
   int i;
   SdpCodec* pCodec;
   int payloadFormat;
   SdpCodec::SdpCodecTypes oldInternalCodecId = SdpCodec::SDP_CODEC_UNKNOWN;
   OsStatus ret;
   MpDecoderBase* pNewDecoder;
   MpCodecFactory* pFactory = MpCodecFactory::getMpCodecFactory();
   int allReusable = 1;
   int canReuse;
   if (OsSysLog::willLog(FAC_MP, PRI_DEBUG))
   {
      for (i=0; i<numCodecs; i++) {
         pCodec = pCodecs[i];
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprDecode::handleSelectCodecs "
                       "pCodecs[%d]->getCodecType() = %d, "
                       "pCodecs[%d]->getCodecPayloadFormat() = %d",
                       i, pCodec->getCodecType(),
                       i, pCodec->getCodecPayloadId());
            }
   }

   // Check to see if all codecs in pCodecs can be handled by codecs
   // in mpCurrentCodecs.
   SdpCodec::SdpCodecTypes internalCodecId;
   MpDecoderBase* pOldDecoder;
   for (i=0; i<numCodecs; i++)
   {
      pCodec = pCodecs[i];
      internalCodecId = pCodec->getCodecType();
      payloadFormat = pCodec->getCodecPayloadId();
      pOldDecoder = mpConnection->mapPayloadType(payloadFormat);
      if (pOldDecoder)
      {
         oldInternalCodecId = pOldDecoder->getInfo()->getCodecType();
         canReuse = (internalCodecId == oldInternalCodecId);
      }
      else
      {
         canReuse = 0;
      }
      allReusable &= canReuse;
   }

   // If the new list is not a subset of the old list, we have to copy
   // pCodecs into mpCurrentCodecs.
   if (!allReusable)
   {
      // Lock the m*Codecs members.
      OsLock lock(mLock);

      // Delete the current codecs.
      handleDeselectCodecs(FALSE);

      m_nCurrentDecoders = numCodecs;
      m_pCurrentDecoders = new MpDecoderBase*[numCodecs];

      for (i=0; i<numCodecs; i++)
      {
         pCodec = pCodecs[i];
         payloadFormat = pCodec->getCodecPayloadId();
         ret = pFactory->createDecoder(*pCodec, pNewDecoder);
         assert(OS_SUCCESS == ret);
         assert(NULL != pNewDecoder);
         pNewDecoder->initDecode();
         // Add this codec to mpConnection's payload type decoding table.
         mpConnection->addPayloadType(payloadFormat, pNewDecoder);
         m_pCurrentDecoders[i] = pNewDecoder;
      }

      // Go back and add any signaling codecs to Jitter Buffer.
      for (i=0; i<numCodecs; i++)
      {
         if (m_pCurrentDecoders[i]->getInfo()->isSignalingCodec())
         {
            m_pCurrentDecoders[i]->initDecode();
            // start observing this decoder, used for DTMF notifications
            m_pCurrentDecoders[i]->registerObserver(this);
         }
      }
   }

   MpDecodeBuffer* pJBState = getDecodeBuffer();   
   pJBState->setCodecList(m_pCurrentDecoders,numCodecs);

   // Delete the list pCodecs.
   for (i=0; i<numCodecs; i++)
   {
      delete pCodecs[i];
   }
   delete[] pCodecs;
   return TRUE;
}

UtlBoolean MprDecode::handleDeselectCodec(MpDecoderBase* pDecoder)
{
   int payload;

   if (NULL != pDecoder) {
      payload = pDecoder->getPayloadType();
      mpConnection->deletePayloadType(payload);
   }
   return TRUE;
}

UtlBoolean MprDecode::handleDeselectCodecs(UtlBoolean shouldLock)
{
   int i;
   MpDecoderBase** pCurrentCodecs;
   MpDecoderBase** pPrevCodecs;
   int newN;
   if(shouldLock)
   {
       mLock.acquire();
   }
   if (0 < m_nCurrentDecoders) {

      newN = m_nCurrentDecoders + mNumPrevCodecs;
      pPrevCodecs = new MpDecoderBase*[newN];
      if (mNumPrevCodecs > 0) {
         for (i=0; i<mNumPrevCodecs; i++) {
            pPrevCodecs[i] = mpPrevCodecs[i];
         }
         delete[] mpPrevCodecs;
      }

      i = m_nCurrentDecoders;
      m_nCurrentDecoders = 0;
      pCurrentCodecs = m_pCurrentDecoders;
      m_pCurrentDecoders = NULL;
      while (i>0) {
         i--;
         handleDeselectCodec(pCurrentCodecs[i]);
         pPrevCodecs[i+mNumPrevCodecs] = pCurrentCodecs[i];
         pCurrentCodecs[i] = NULL;
      }
      delete[] pCurrentCodecs;
      mpPrevCodecs = pPrevCodecs;
      mNumPrevCodecs = newN;
   }

   delete m_pDecodeBuffer;
   m_pDecodeBuffer = NULL;

   if(shouldLock)
   {
       mLock.release();
   }
   return TRUE;
}


/* ============================ FUNCTIONS ================================= */
