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

#ifndef MpStopDTMFTimerMsg_h__
#define MpStopDTMFTimerMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <mp/MpTimerMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * MpStopDTMFTimerMsg is sent when Stop DTMF timer fires.
 */
class MpStopDTMFTimerMsg : public MpTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   MpStopDTMFTimerMsg();

   /** Copy constructor */
   MpStopDTMFTimerMsg(const MpStopDTMFTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~MpStopDTMFTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   MpStopDTMFTimerMsg& operator=(const MpStopDTMFTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // MpStopDTMFTimerMsg_h__
