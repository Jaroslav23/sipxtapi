//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_ILBC // [

// WIN32: Add libilbc to linker input.
#ifdef WIN32 // [
#   ifdef _DEBUG // [
#      pragma comment(lib, "libilbcd.lib")
#   else // _DEBUG ][
#      pragma comment(lib, "libilbc.lib")
#   endif // _DEBUG ]
#endif // WIN32 ]

// SYSTEM INCLUDES
#include <limits.h>

// APPLICATION INCLUDES
#include "mp/MpdSipxILBC.h"
extern "C" {
#include <iLBC_define.h>
#include <iLBC_decode.h>
}

const MpCodecInfo MpdSipxILBC::smCodecInfo30ms(
    SdpCodec::SDP_CODEC_ILBC_30MS,   // codecType
    "iLBC",                     // codecVersion
    8000,                       // samplingRate
    16,                          // numBitsPerSample (not used)
    1,                          // numChannels
    13334,                      // bitRate
    NO_OF_BYTES_30MS * 8,       // minPacketBits
    NO_OF_BYTES_30MS * 8,       // maxPacketBits
    240);                        // numSamplesPerFrame

const MpCodecInfo MpdSipxILBC::smCodecInfo20ms(
   SdpCodec::SDP_CODEC_ILBC_20MS,   // codecType
   "iLBC",                     // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                          // numChannels
   13334,                      // bitRate
   NO_OF_BYTES_20MS * 8,       // minPacketBits
   NO_OF_BYTES_20MS * 8,       // maxPacketBits
   160);                        // numSamplesPerFrame

MpdSipxILBC::MpdSipxILBC(int payloadType, int mode)
: MpDecoderBase(payloadType, mode == 20 ? &smCodecInfo20ms : &smCodecInfo30ms)
, m_mode(mode)
, m_pState20(NULL)
, m_pState30(NULL)
{
}

MpdSipxILBC::~MpdSipxILBC()
{
   freeDecode();
}

OsStatus MpdSipxILBC::initDecode()
{
   if (!m_pState20)
   {
      m_pState20 = new iLBC_Dec_Inst_t();
      memset(m_pState20, 0, sizeof(*m_pState20));
      ::initDecode(m_pState20, 20, 1);
   }
   if (!m_pState30)
   {
      m_pState30 = new iLBC_Dec_Inst_t();
      memset(m_pState30, 0, sizeof(*m_pState30));
      ::initDecode(m_pState30, 30, 1);
   }
   return OS_SUCCESS;
}

OsStatus MpdSipxILBC::freeDecode(void)
{
   delete m_pState20;
   m_pState20 = NULL;
   delete m_pState30;
   m_pState30 = NULL;
   return OS_SUCCESS;
}

int MpdSipxILBC::decode(const MpRtpBufPtr &rtpPacket,
                        unsigned decodedBufferLength,
                        MpAudioSample *samplesBuffer,
                        UtlBoolean bIsPLCFrame)
{
   if (!rtpPacket.isValid())
      return 0;

   unsigned payloadSize = rtpPacket->getPayloadSize();
   iLBC_Dec_Inst_t_* pState = NULL;
   unsigned int samplesPerFrame = getInfo()->getNumSamplesPerFrame();

   // Decode incoming packet to temp buffer. If no packet - do PLC.
   if (payloadSize == NO_OF_BYTES_30MS)
   {
      pState = m_pState30;
      samplesPerFrame = 240;
   }
   else if (payloadSize == NO_OF_BYTES_20MS)
   {
      pState = m_pState20;
      samplesPerFrame = 160;
   }
   else
   {
      return 0; // generate noise for weird frame sizes
   }

   unsigned char* pInputBuffer = (unsigned char*)rtpPacket->getDataPtr();
   int packetType = 1;
   if (bIsPLCFrame)
   {
      pInputBuffer = NULL;
      packetType = 0;
   }

   // with NULL input buffer codec does PLC
   iLBC_decode(m_tmpOutBuffer, pInputBuffer, pState, packetType);

   for (unsigned int i = 0; i < samplesPerFrame; ++i)
   {
      float tmp = m_tmpOutBuffer[i];
      if (tmp > SHRT_MAX)
         tmp = SHRT_MAX;
      if (tmp < SHRT_MIN)
         tmp = SHRT_MIN;

      samplesBuffer[i] = MpAudioSample(tmp + 0.5f);
   }

   return samplesPerFrame;
}

#endif // HAVE_ILBC ]
