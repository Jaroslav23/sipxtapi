//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_SPEEX

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsLock.h>
#include <mp/MpMisc.h>
#include <mp/MpJitterBufferSpeex.h>
#include <mp/MpRtpBuf.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpJitterBufferSpeex::MpJitterBufferSpeex(const UtlString& name,
                                         int payloadType,
                                         unsigned int frameSize)
: MpJitterBufferBase(name, payloadType, frameSize)
, m_pJitterBuffer(NULL)
, m_lastSSRC(0)
, m_bFirstFrame(true)
, m_speexLock(OsMutex::Q_FIFO)
{
   m_pJitterBuffer = jitter_buffer_init(frameSize);

   assert(m_pJitterBuffer);
}

MpJitterBufferSpeex::~MpJitterBufferSpeex()
{
   if (m_pJitterBuffer)
   {
      jitter_buffer_destroy(m_pJitterBuffer);

      m_pJitterBuffer = NULL;
   }   
}

/* ============================ MANIPULATORS ============================== */

void MpJitterBufferSpeex::reset()
{
   if (m_pJitterBuffer)
   {
      jitter_buffer_reset(m_pJitterBuffer);
   }
}

void MpJitterBufferSpeex::frameIncrement()
{
   if (m_pJitterBuffer)
   {
      jitter_buffer_tick(m_pJitterBuffer);
   }
}

JitterBufferResult MpJitterBufferSpeex::pull(MpRtpBufPtr &pOutRtp)
{
   int result = JITTER_BUFFER_INTERNAL_ERROR;

   if (m_pJitterBuffer)
   {
      OsLock lock(m_speexLock); // lock speex jitter buffer

      // TODO: replace this with char* pool!
      char buffer[2048];
      memset(buffer, 0, sizeof(buffer));
      
      // dummy packet
      JitterBufferPacket packet;
      packet.data = buffer;

      // pull packet from speex
      result = jitter_buffer_get(m_pJitterBuffer, &packet, m_samplesPerFrame, NULL);

      assert(packet.len < sizeof(buffer)); // TODO: remove when pool is implemented

      // set correct payload size
      pOutRtp->setPayloadSize(packet.len);

      if (result == JITTER_BUFFER_OK)
      {
         // assign fresh buffer to pOutRtp
         pOutRtp = MpMisc.m_pRtpPool->getBuffer();

         if (!pOutRtp.isValid())
         {
            return MP_JITTER_BUFFER_ERROR;
         }
         
         // Copy payload to RTP buffer.
         memcpy(pOutRtp->getDataWritePtr(), packet.data, packet.len);
      }
      // set some additional properties
      pOutRtp->setRtpPayloadType(m_payloadType);
      pOutRtp->setRtpTimestamp(packet.timestamp);
   }
   
   switch(result)
   {
   case JITTER_BUFFER_OK:
      if (m_bufferLength > 0)
      {
         m_bufferLength--;
      }      
      return MP_JITTER_BUFFER_OK;
   case JITTER_BUFFER_MISSING:
      return MP_JITTER_BUFFER_MISSING;
   default:
      return MP_JITTER_BUFFER_ERROR;
   }
}

void MpJitterBufferSpeex::push(const MpRtpBufPtr &pRtp)
{
   if (m_pJitterBuffer && pRtp.isValid() && pRtp->getRtpPayloadType() == m_payloadType)
   {
      OsLock lock(m_speexLock); // lock speex jitter buffer

      // for first frame, setup SSRC
      if (m_bFirstFrame)
      {
         m_lastSSRC = pRtp->getRtpSSRC();
         m_bFirstFrame = false;
      }
      else
      if (m_lastSSRC != pRtp->getRtpSSRC())
      {
         // SSRC changed, reset jitter buffer
         reset();
         m_lastSSRC = pRtp->getRtpSSRC();
      }

      // prepare a JitterBufferPacket
      JitterBufferPacket packet;
      packet.data = const_cast<char*>(pRtp->getDataPtr());
      packet.len = pRtp->getPayloadSize();
      packet.span = m_samplesPerFrame;
      packet.timestamp = pRtp->getRtpTimestamp();

      // speex copies data content of const packet
      jitter_buffer_put(m_pJitterBuffer, &packet);
      m_bufferLength++;
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

int MpJitterBufferSpeex::getBufferLength()
{
   return m_bufferLength;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif // HAVE_SPEEX
