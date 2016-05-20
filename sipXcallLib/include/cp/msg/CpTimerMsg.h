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

#ifndef CpTimerMsg_h__
#define CpTimerMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsTimerMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * CpTimerMsg represents message which gets sent when a timer fires in sipxcalllib.
 * Never use directly, but subclass to supply msgSubType automatically
 * and transport any user data in subclass.
 */
class CpTimerMsg : public OsTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   typedef enum
   {
      CP_TIMER_FIRST = 0, ///< Add your own timer ids here
      CP_ABSTRACT_CALL_TIMER, ///< Timer which is meant to be processed by abstract call
      CP_SIP_CONNECTION_TIMER, ///< Timer which is meant to be processed by sip connection state machine
      CP_TIMER_LAST = 255 ///< Keep lower than 255
   } SubTypeEnum;

   /**
    * Constructor.
    */
   CpTimerMsg(SubTypeEnum msgSubType);

   /** Copy constructor */
   CpTimerMsg(const CpTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~CpTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   CpTimerMsg& operator=(const CpTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // CpTimerMsg_h__
