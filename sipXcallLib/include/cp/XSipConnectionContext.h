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

#ifndef XSipConnectionContext_h__
#define XSipConnectionContext_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsRWMutex.h>
#include <os/OsRWSyncBase.h>
#include <net/SipDialog.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * XSipConnectionContext contains various stateful information about Sip connection,
 * like SipDialog, keeps track of ongoing transactions etc. State machine makes decisions
 * based on values saved in this context and inbound Sip message or command. This class
 * is meant to store only information between states of sip connection. If some memory
 * is required only for the duration of a single state, then it should be kept in the state
 * itself.
 *
 * Before this class can be read from, it must be locked externally. External locking is
 * used instead of internal, to avoid frequent locking when accessing multiple members.
 * Use OsReadLock or OsWriteLock to lock this class.
 *
 * This class is meant to contain public members. Destructor is responsible for freeing
 * any pointers.
 *
 * When XSipConnection is first constructed, it is immediately assigned a sip dialog.
 * Sip callId of this sip dialog cannot change during the lifetime of a XSipConnection.
 * Local tag will also be present if connection was initiated locally, and remote tag will be
 * present if connection was initiated remotely.
 */
class XSipConnectionContext : public OsRWSyncBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   SipDialog m_sipDialog; ///< contains properties of Sip dialog as defined in RFC 3261
   UtlString m_remoteUserAgent;
   UtlString m_sAbstractCallId; ///< contains Id of abstract call holding Sip connection

   // thread safe atomic
   int m_mediaConnectionId; ///< contains Id of media connection for CpMediaInterface
   /**
    * Contains Id of media connection for CpMediaInterface - used for media event routing.
    * This one is synchronized with m_mediaConnectionId, but not set to -1, when media connection
    * is deleted. This allows us to route media events that arrive after media connection has been
    * destroyed.
    */
   int m_mediaEventConnectionId;
   UtlBoolean m_bSupressCallEvents; ///< TRUE when call events should not be fired

   /* ============================ CREATORS ================================== */

   /** Constructor */
   XSipConnectionContext();

   /** Destructor */
   virtual ~XSipConnectionContext();

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   XSipConnectionContext(const XSipConnectionContext& rhs);

   XSipConnectionContext& operator=(const XSipConnectionContext& rhs);

   /**
   * Block (if necessary) until the task acquires the resource for reading.
   * Multiple simultaneous readers are allowed.
   */
   virtual OsStatus acquireRead();

   /**
   * Block (if necessary) until the task acquires the resource for writing.
   * Only one writer at a time is allowed (and no readers).
   */
   virtual OsStatus acquireWrite();

   /**
   * Conditionally acquire the resource for reading (i.e., don't block).
   * Multiple simultaneous readers are allowed.
   * Return OS_BUSY if the resource is held for writing by some other task
   */
   virtual OsStatus tryAcquireRead();

   /**
   * Conditionally acquire the resource for writing (i.e., don't block).
   * Only one writer at a time is allowed (and no readers).
   * Return OS_BUSY if the resource is held for writing by some other task
   * or if there are running readers.
   */
   virtual OsStatus tryAcquireWrite();

   /** Release the resource for reading */
   virtual OsStatus releaseRead();

   /** Release the resource for writing */
   virtual OsStatus releaseWrite();

   mutable OsRWMutex m_memberMutex; ///< mutex for guarding instance against deletion from XCpAbstractCall
};

#endif // XSipConnectionContext_h__
