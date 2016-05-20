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

#ifndef CpSessionTimerState_h__
#define CpSessionTimerState_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <cp/CpDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * This class keeps various negotiated properties of session timer according to RFC4028.
 */
class CpSessionTimerProperties
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor */
   CpSessionTimerProperties();

   /** Destructor */
   ~CpSessionTimerProperties();

   /* ============================ MANIPULATORS ============================== */

   /**
    * Resets session timer properties to initial configuration. If full reset,
    * then also session expiration is reset. Otherwise only refresher is reset.
    *
    * After reset, getRefresher() will return value for SIP request.
    */
   void reset(UtlBoolean bFullReset = FALSE);

   /** Must be called after re-INVITE or UPDATE succeeds */
   void onSessionRefreshed();

   /* ============================ ACCESSORS ================================= */

   /** 
    * Gets refresher value for given transaction direction. Refresher value depends on direction
    * of transaction.
    */
   UtlString getRefresher(UtlBoolean bIsOutboundTransaction) const;

   /** Gets negotiated refresher in transaction direction independent way */
   CP_SESSION_TIMER_REFRESH getRefresher() const { return m_sRefresher; }

   /**
    * Reconfigures current refresher, and stores the value independent of INVITE/UPDATE transaction direction.
    * Refresher must be configured before getRefresher() will return a valid value for SIP response.
    *
    * Calling reset() also clears refresher, so that initial value of refresher can be sent in SIP request
    * and negotiated if empty.
    */
   void configureRefresher(const UtlString& refresher, UtlBoolean bIsOutboundTransaction);

   int getSessionExpires() const { return m_sessionExpires; }
   /** Sets new value for session expires. This value can only be increased or reset. */
   void setSessionExpires(int sessionExpires);

   int getMinSessionExpires() const { return m_minSessionExpires; }
   /** Sets new value for minSe. This value can only be increased or reset. */
   void setMinSessionExpires(int val);

   /** Default values to use after reset. */
   void setInitialSessionExpires(int val) { m_initialSessionExpires = val; }
   void setInitialRefresher(CP_SESSION_TIMER_REFRESH sessionTimerRefresh) { m_sInitialRefresher = sessionTimerRefresh; }

   long getLastRefreshTimestamp() const { return m_lastRefreshTimestamp; }

   /* ============================ INQUIRY =================================== */

   /** Returns TRUE if session hasn't been refreshed for too long and should be terminated */
   UtlBoolean isSessionStale() const;

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   // initial properties, and properties after reset
   int m_initialSessionExpires; ///< initial time in seconds when session expires
   CP_SESSION_TIMER_REFRESH m_sInitialRefresher; ///< configuration of initial refresher, independent of call direction
   UtlBoolean m_bIsLocallyInitiatedCall; ///< TRUE if call is locally initiated

   long m_lastRefreshTimestamp; ///< timestamp in seconds of last successful refresh (re-INVITE or UPDATE)

   CP_SESSION_TIMER_REFRESH m_sRefresher; ///< current refresher
   int m_sessionExpires; ///< time in seconds when session expires
   int m_minSessionExpires; ///< minimum session expiration time that might be specified in future INVITE/UPDATE messages
};

#endif // CpSessionTimerState_h__
