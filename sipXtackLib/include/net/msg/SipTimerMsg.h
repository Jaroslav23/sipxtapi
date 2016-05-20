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

#ifndef SipTimerMsg_h__
#define SipTimerMsg_h__

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
 * SipTimerMsg represents message which gets sent when a timer fires in sipxtacklib.
 * Never use directly, but subclass to supply msgSubType automatically
 * and transport any user data in subclass.
 */
class SipTimerMsg : public OsTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   typedef enum
   {
      SIP_TIMER_FIRST = 0, ///< Add your own timer ids here
      SIP_TIMER_LAST = 255 ///< Keep lower than 255
   } SubTypeEnum;

   /**
    * Constructor.
    */
   SipTimerMsg(SubTypeEnum msgSubType);

   /** Copy constructor */
   SipTimerMsg(const SipTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~SipTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   SipTimerMsg& operator=(const SipTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // SipTimerMsg_h__
