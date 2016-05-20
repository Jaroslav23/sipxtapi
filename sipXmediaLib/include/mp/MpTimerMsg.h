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

#ifndef MpTimerMsg_h__
#define MpTimerMsg_h__

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
 * MpTimerMsg represents message which gets sent when a timer fires in sipXmediaLib.
 * Timer message will be available to be processed in flowgraph.
 *   
 * Never use directly, but subclass to supply msgSubType automatically
 * and transport any user data in subclass.
 */
class MpTimerMsg : public OsTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   typedef enum
   {
      MP_TIMER_FIRST = 0, ///< Add your own timer ids here
      MP_STOP_DTMF_TONE_TIMER, ///< Timer which is meant to be processed by flowgraph
      MP_TIMER_LAST = 255 ///< Keep lower than 255
   } SubTypeEnum;

   /**
    * Constructor.
    */
   MpTimerMsg(SubTypeEnum msgSubType);

   /** Copy constructor */
   MpTimerMsg(const MpTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~MpTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   MpTimerMsg& operator=(const MpTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // MpTimerMsg_h__
