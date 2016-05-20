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

#ifndef XCpCallIdUtil_h__
#define XCpCallIdUtil_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <net/SipCallIdGenerator.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * This class represents a generic call/conference identifier.
 */
class XCpCallIdUtil : public UtlString
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   typedef enum
   {
      ID_TYPE_CALL,
      ID_TYPE_CONFERENCE,
      ID_TYPE_UNKNOWN
   } ID_TYPE;

   /* ============================ MANIPULATORS ============================== */

   /** Gets new call id. */
   static UtlString getNewCallId();

   /** Gets new conference id. */
   static UtlString getNewConferenceId();

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /** Checks if this Id identifies a call instance */
   static UtlBoolean isCallId(const UtlString& sId);

   /** Checks if this Id identifies a conference instance */
   static UtlBoolean isConferenceId(const UtlString& sId);

   /** Gets the type of Id. Can be call, conference or unknown. */
   static XCpCallIdUtil::ID_TYPE getIdType(const UtlString& sId);

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static SipCallIdGenerator ms_callIdGenerator; ///< generates string ids for calls
   static SipCallIdGenerator ms_conferenceIdGenerator; ///< generates string ids for conferences
};

#endif // XCpCallIdUtil_h__
