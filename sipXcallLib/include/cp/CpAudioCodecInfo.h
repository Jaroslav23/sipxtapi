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

#ifndef CpAudioCodecInfo_h__
#define CpAudioCodecInfo_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef enum CP_AUDIO_BANDWIDTH_ID
{
   CP_AUDIO_CODEC_BW_VARIABLE = 0,
   CP_AUDIO_CODEC_BW_LOW,
   CP_AUDIO_CODEC_BW_NORMAL,
   CP_AUDIO_CODEC_BW_HIGH,
   CP_AUDIO_CODEC_BW_CUSTOM,
   CP_AUDIO_CODEC_BW_DEFAULT
} CP_AUDIO_BANDWIDTH_ID;

// MACROS
// FORWARD DECLARATIONS

/**
* Information about audio codec being used by a call. Used in event system.
*/
class CpAudioCodecInfo
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   UtlString m_codecName;
   CP_AUDIO_BANDWIDTH_ID m_iBandWidth;
   int m_iPayloadId; ///< payload id from SDP

   /* ============================ CREATORS ================================== */

   CpAudioCodecInfo() : m_codecName()
      , m_iBandWidth(CP_AUDIO_CODEC_BW_DEFAULT)
      , m_iPayloadId(0)
   {
   }

   ~CpAudioCodecInfo()
   {

   }

   CpAudioCodecInfo(const CpAudioCodecInfo& rhs)
   {
      *this = rhs;
   }

   CpAudioCodecInfo& operator=(const CpAudioCodecInfo& rhs)
   {
      if (&rhs == this)
      {
         return *this;
      }

      m_codecName = rhs.m_codecName;
      m_iBandWidth = rhs.m_iBandWidth;
      m_iPayloadId = rhs.m_iPayloadId;

      return *this;
   }

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // CpAudioCodecInfo_h__
