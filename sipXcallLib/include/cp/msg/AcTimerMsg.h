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

#ifndef AcTimerMsg_h__
#define AcTimerMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cp/msg/CpTimerMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* AcTimerMsg represents message which gets sent when a timer fires in sipxcalllib.
* The timer message is meant to be processed by abstract call.
*
* Subclass for custom timer message.
 */
class AcTimerMsg : public CpTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   typedef enum
   {
      PAYLOAD_TYPE_FIRST = 0, ///< Add your own payload ids here
   } PayloadTypeEnum;

   /**
    * Constructor.
    */
   AcTimerMsg(PayloadTypeEnum payloadType);

   /** Copy constructor */
   AcTimerMsg(const AcTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~AcTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   AcTimerMsg& operator=(const AcTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   AcTimerMsg::PayloadTypeEnum getPayloadType() const { return m_payloadType; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   PayloadTypeEnum m_payloadType; ///< type of payload
   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // AcTimerMsg_h__
