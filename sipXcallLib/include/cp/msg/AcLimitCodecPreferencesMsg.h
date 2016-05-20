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

#ifndef AcLimitCodecPreferencesMsg_h__
#define AcLimitCodecPreferencesMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/AcCommandMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Abstract call command message. Configures codecs used for future calls.
*/
class AcLimitCodecPreferencesMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcLimitCodecPreferencesMsg(const UtlString& sAudioCodecs,
                              const UtlString& sVideoCodecs);

   virtual ~AcLimitCodecPreferencesMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   UtlString getAudioCodecs() const { return m_sAudioCodecs; }
   UtlString getVideoCodecs() const { return m_sVideoCodecs; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcLimitCodecPreferencesMsg(const AcLimitCodecPreferencesMsg& rMsg);

   /** Private assignment operator */
   AcLimitCodecPreferencesMsg& operator=(const AcLimitCodecPreferencesMsg& rhs);

   UtlString m_sAudioCodecs;
   UtlString m_sVideoCodecs;
};

#endif // AcLimitCodecPreferencesMsg_h__
