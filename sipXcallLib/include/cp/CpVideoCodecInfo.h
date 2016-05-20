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

#ifndef CpVideoCodecInfo_h__
#define CpVideoCodecInfo_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef enum CP_VIDEO_BANDWIDTH_ID
{
   CP_VIDEO_CODEC_BW_VARIABLE = 0,
   CP_VIDEO_CODEC_BW_LOW,
   CP_VIDEO_CODEC_BW_NORMAL,
   CP_VIDEO_CODEC_BW_HIGH,
   CP_VIDEO_CODEC_BW_CUSTOM,
   CP_VIDEO_CODEC_BW_DEFAULT
} CP_VIDEO_BANDWIDTH_ID;

// MACROS
// FORWARD DECLARATIONS

/**
* Information about video codec being used by a call. Used in event system.
*/
class CpVideoCodecInfo
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   UtlString m_codecName;
   CP_VIDEO_BANDWIDTH_ID m_iBandWidth;
   int m_iPayloadType;

   /* ============================ CREATORS ================================== */

   CpVideoCodecInfo() : m_codecName()
      , m_iBandWidth(CP_VIDEO_CODEC_BW_DEFAULT)
      , m_iPayloadType(0)
   {
   }

   ~CpVideoCodecInfo()
   {

   }

   CpVideoCodecInfo(const CpVideoCodecInfo& rhs)
   {
      *this = rhs;
   }

   CpVideoCodecInfo& operator=(const CpVideoCodecInfo& rhs)
   {
      if (&rhs == this)
      {
         return *this;
      }

      m_codecName = rhs.m_codecName;
      m_iBandWidth = rhs.m_iBandWidth;
      m_iPayloadType = rhs.m_iPayloadType;

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

#endif // CpVideoCodecInfo_h__
