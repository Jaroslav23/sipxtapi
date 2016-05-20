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

#ifndef AcAudioBufferPlayMsg_h__
#define AcAudioBufferPlayMsg_h__

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
* Abstract call command message. Instructs call to start playback from buffer.
*/
class AcAudioBufferPlayMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcAudioBufferPlayMsg(const void* pAudiobuf,
                        size_t iBufSize,
                        int iType,
                        UtlBoolean bRepeat,
                        UtlBoolean bLocal,
                        UtlBoolean bRemote,
                        UtlBoolean bMixWithMic,
                        int iDownScaling,
                        void* pCookie);

   virtual ~AcAudioBufferPlayMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void* getAudiobuf() const { return m_pAudiobuf; }
   size_t getBufSize() const { return m_iBufSize; }
   int getType() const { return m_iType; }
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
   AcAudioBufferPlayMsg(const AcAudioBufferPlayMsg& rMsg);

   /** Private assignment operator */
   AcAudioBufferPlayMsg& operator=(const AcAudioBufferPlayMsg& rhs);

   void* m_pAudiobuf;
   size_t m_iBufSize;
   int m_iType;
   UtlBoolean m_bRepeat;
   UtlBoolean m_bLocal;
   UtlBoolean m_bRemote;
   UtlBoolean m_bMixWithMic;
   int m_iDownScaling;
   void* m_pCookie;
};

#endif // AcAudioBufferPlayMsg_h__
