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

#ifndef CpCodecInfo_h__
#define CpCodecInfo_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <cp/CpAudioCodecInfo.h>
#include <cp/CpVideoCodecInfo.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Information about codecs being used by a call. Used in event system.
*/
class CpCodecInfo
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   CpAudioCodecInfo m_audioCodec;
   CpVideoCodecInfo m_videoCodec;
   bool m_bIsEncrypted;

   /* ============================ CREATORS ================================== */

   CpCodecInfo() : m_bIsEncrypted(false)
      , m_audioCodec()
      , m_videoCodec()
   {
   }

   ~CpCodecInfo()
   {

   }

   CpCodecInfo(const CpCodecInfo& rhs)
   {
      *this = rhs;
   }

   CpCodecInfo& operator=(const CpCodecInfo& rhs)
   {
      if (&rhs == this)
      {
         return *this;
      }

      m_audioCodec = rhs.m_audioCodec;
      m_videoCodec = rhs.m_videoCodec;
      m_bIsEncrypted = rhs.m_bIsEncrypted;

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

#endif // CpCodecInfo_h__
