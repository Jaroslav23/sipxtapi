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

#ifndef OsTimerMsg_h__
#define OsTimerMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMsg.h>
#include <os/OsTime.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * OsTimerMsg represents message which gets sent when a timer fires.
 * Never use directly, but subclass to supply msgSubType automatically
 * and transport any user data in subclass.
 */
class OsTimerMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor. Creates OsTimerMsg with given sub type and timestamp.
    */
   OsTimerMsg(const unsigned char msgSubType);

   /** Copy constructor */
   OsTimerMsg(const OsTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~OsTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   OsTimerMsg& operator=(const OsTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   /** Return the timestamp associated with this event */
   OsTime getTimestamp() const { return m_timestamp; }

   /** Sets timestamp associated with this event */
   void setTimestamp(const OsTime& val) { m_timestamp = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsTime m_timestamp;
};

#endif // OsTimerMsg_h__
