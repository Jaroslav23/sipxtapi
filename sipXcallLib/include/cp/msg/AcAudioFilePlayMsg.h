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

#ifndef AcAudioFilePlayMsg_h__
#define AcAudioFilePlayMsg_h__

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
* Abstract call command message. Instructs call to start file playback.
*/
class AcAudioFilePlayMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcAudioFilePlayMsg(const UtlString& sAudioFile,
                      UtlBoolean bRepeat,
                      UtlBoolean bLocal,
                      UtlBoolean bRemote,
                      UtlBoolean bMixWithMic,
                      int iDownScaling,
                      void* pCookie);

   virtual ~AcAudioFilePlayMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   UtlString getAudioFile() const { return m_sAudioFile; }
   UtlBoolean getRepeat() const { return m_bRepeat; }
   UtlBoolean getLocal() const { return m_bLocal; }
   UtlBoolean getRemote() const { return m_bRemote; }
   UtlBoolean getMixWithMic() const { return m_bMixWithMic; }
   int getDownScaling() const { return m_iDownScaling; }
   void* getCookie() const { return m_pCookie; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcAudioFilePlayMsg(const AcAudioFilePlayMsg& rMsg);

   /** Private assignment operator */
   AcAudioFilePlayMsg& operator=(const AcAudioFilePlayMsg& rhs);

   UtlString m_sAudioFile;
   UtlBoolean m_bRepeat;
   UtlBoolean m_bLocal;
   UtlBoolean m_bRemote;
   UtlBoolean m_bMixWithMic;
   int m_iDownScaling;
   void* m_pCookie;
};

#endif // AcAudioFilePlayMsg_h__
