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

#ifndef AcAudioToneStartMsg_h__
#define AcAudioToneStartMsg_h__

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
* Abstract call command message. Instructs call to carry out some action.
*/
class AcAudioToneStartMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcAudioToneStartMsg(int iToneId,
                       UtlBoolean bLocal,
                       UtlBoolean bRemote,
                       int duration);

   virtual ~AcAudioToneStartMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   int getToneId() const { return m_iToneId; }
   UtlBoolean getLocal() const { return m_bLocal; }
   UtlBoolean getRemote() const { return m_bRemote; }
   int getDuration() const { return m_duration; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcAudioToneStartMsg(const AcAudioToneStartMsg& rMsg);

   /** Private assignment operator */
   AcAudioToneStartMsg& operator=(const AcAudioToneStartMsg& rhs);

   int m_iToneId;
   UtlBoolean m_bLocal;
   UtlBoolean m_bRemote;
   int m_duration;
};

#endif // AcAudioToneStartMsg_h__
