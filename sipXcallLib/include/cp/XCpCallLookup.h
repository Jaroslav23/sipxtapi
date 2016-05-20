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

#ifndef XCpCallLookup_h__
#define XCpCallLookup_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
template <class T>
class OsPtrLock; // forward template class declaration
class UtlString;
class SipDialog;
class XCpCall;

/**
 * XCpCallLookup is meant to be used by conference to perform operations on call for join/split.
 */
class XCpCallLookup
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

  /**
   * Finds and returns a XCpCall according to given id.
   * Returned OsPtrLock unlocks XCpCall automatically, and the object should not
   * be used outside its scope.
   *
   * @return TRUE if a call was found, FALSE otherwise.
   */
   virtual UtlBoolean findCall(const UtlString& sId, OsPtrLock<XCpCall>& ptrLock) const = 0;

   /**
   * Finds and returns a XCpCall according to given SipDialog.
   * Returned OsPtrLock unlocks XCpCall automatically, and the object should not
   * be used outside its scope.
   *
   * @return TRUE if a call was found, FALSE otherwise.
   */
   virtual UtlBoolean findCall(const SipDialog& sSipDialog, OsPtrLock<XCpCall>& ptrLock) const = 0;

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // XCpCallLookup_h__
