//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#ifndef _SipDialog_h_
#define _SipDialog_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include <net/Url.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DIALOG_HANDLE_SEPARATOR ','

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;

//! Class for containing SIP dialog state information
/*! In SIP a dialog is defined by the SIP Call-Id
*  header and the tag parameter from the SIP To
*  and From header fields.
*
*  Dialog state is split into state and substate. The reason
*  is we can construct SipDialog from call-id, tags, but have
*  no idea if this is an early or confirmed dialog according
*  to RFC2833. Dialog is established if it contains both tags
*  and is not terminated. Dialog is early after 1xx response with
*  to tag. Dialog is confirmed after 200 OK response with to tag.
*  Dialog termination of established dialog is not automatically
*  detected, and must be explicitly set by user.
*
* \par Local and Remote
*  As the To and From fields get swapped depending
*  upon which side initiates a transaction (i.e.
*  sends a request) local and remote are used in
*  SipDialog to label tags, fields and information.
*  Local and Remote are unabiquous when used in
*  an end point.  In a proxy context the SipDialog
*  can still be used.  One can visualize the
*  sides of the dialog by thinking Left and Right
*  instead of local and remote.
*
*  This class makes a strict difference between client and server
*  SipDialog. SipDialog on client will have the same call-id like
*  server SipDialog, but local/remote tags will be reversed. When
*  trying to match server dialog against a client dialog, matching will
*  fail. Client and server dialog are considered separate dialogs.
*  This is possible because SipMessage records if it was received through
*  socket. Thanks to strict difference between client and server SipDialog
*  we can make a call to ourselves or subscribe and send notifications to
*  ourselves.
*
*  Divergence from RFC3261:
*  - we track localField and remoteField not local URI and remote URI. This also allows
*    us to track display name.
*  - remoteContact corresponds to "remote target"
*
*  This class is intended to deprecate the SipSession
*  class.
*/
class SipDialog : public UtlString
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /**
    * State of SipDialog:
    * - initial - when SipDialog is constructed and one of tags is missing
    * - established - when SipDialog is constructed with both tags, or as result of updateDialogData
    * - terminated - after 4xx, 5xx, 6xx responses
    */
   typedef enum
   {
      DIALOG_STATE_INITIAL,
      DIALOG_STATE_ESTABLISHED,
      DIALOG_STATE_TERMINATED
   } DialogState;

   /**
    * Substate of SipDialog:
    * - unknown - the default state
    * - early - when updateDialogData can see 1xx response with remote tag
    * - confirmed - after final 200 OK response to dialog initiation, with both tags
    */
   typedef enum
   {
      DIALOG_SUBSTATE_UNKNOWN,
      DIALOG_SUBSTATE_EARLY,
      DIALOG_SUBSTATE_CONFIRMED
   } DialogSubState;

   /**
    * Enum for dialog match results. Typically the best match is returned.
    */
   typedef enum
   {
      DIALOG_ESTABLISHED_MATCH = 0, /**< both compared dialogs are established and tags match */
      DIALOG_INITIAL_INITIAL_MATCH, /**< Right hand side initial dialog matches current initial dialog. */
      DIALOG_INITIAL_ESTABLISHED_MATCH, /**< Right hand side established dialog matches current initial dialog. */
      DIALOG_ESTABLISHED_INITIAL_MATCH, /**< Right hand side dialog which is initial matches current
                                         *   established dialog with single tag.
                                         *   In other words, right hand side initial dialog was the initial
                                         *   dialog for current established dialog, before 2nd tag
                                         *   was added.
                                         */
      DIALOG_MISMATCH /**< dialog mismatch, either tags or call-id is different */
   } DialogMatchEnum;

   /* ============================ CREATORS ================================== */

   //! Default Dialog constructor
   /*! Optionally construct a dialog from the given message
   *
   * \param initialMessage - message to initiate the dialog, typically this
   *        is a request.
   */
   SipDialog(const SipMessage* initialMessage = NULL);

   //! Constructor accepting the basic pieces of a session callId, toUrl, and from Url.
   /*! Optionally construct a dialog from the given message
   *
   * \param callId - sip message call-id header value
   * \param localField - sip message To or From field value representing the 
   *        local side of the dialog.
   * \param remoteField - sip message To or From field value representing the 
   *        remote side of the dialog.
   */
   SipDialog(const UtlString& sSipCallId,
             const UtlString& sLocalTag = NULL,
             const UtlString& sRemoteTag = NULL,
             UtlBoolean isFromLocal = TRUE); 

   //! Copy constructor
   SipDialog(const SipDialog& rSipDialog);

   //! Destructor
   virtual
      ~SipDialog();


   /* ============================ MANIPULATORS ============================== */

   //! Assignment operator
   SipDialog& operator=(const SipDialog& rhs);

   //! Update dialog operator
   SipDialog& operator<<(const SipMessage& message);

   //! update the dialog information based upon the given message
   /*! Typically this updates things like the Contact, CSeq headers and 
   *  tag information for the dialog.
   *  \param message - SIP message which is assumed to be part of this
   *         dialog.
   */
   void updateDialog(const SipMessage& message);

   //! Set fields in next SIP request for this dialog
   /*! Set the request URI, call-id, To, From, Route and Cseq headers
   *  fields for the given request to be sent in the given dialog.  The
   *  last local cseq of the dialog is incremented and set in the request.
   *  \param method - the sip request method for this request.
   *  \param request - the request which is to be part of this dialog
   *         and sent as orginating from the local side of the dialog.
   *  \param cseqNum - allows to specify cseq number from outside.
   */
   void setRequestData(SipMessage& request, const UtlString& method, int cseqNum = -1);

   /* ============================ ACCESSORS ================================= */

   /**
    * Constructs dialog handle for given SipMessage. Tags are swapped if needed.
    */
   static void getDialogHandle(const SipMessage& sipMessage, UtlString& sDialogHandle);

   /** Gets a string handle that can be used to uniquely identify this dialog */
   void getDialogHandle(UtlString& dialogHandle) const;

   /**
    * Get the initial dialog handle for this dialog. Initial dialog is dialog
    * that has not yet been established. Early and confirmed dialogs are considered
    * established dialogs.
    */
   void getInitialDialogHandle(UtlString& initialDialogHandle) const;

   //! Gets the call-id, and tags from the dialogHandle
   static void parseHandle(const UtlString& dialogHandle,
                           UtlString& callId,
                           UtlString& localTag,
                           UtlString& remoteTag);

   //! Reverse the order of the tags in the handle
   static void reverseTags(const UtlString& dialogHandle,
                           UtlString& reversedHandle);

   //! Get the SIP call-id header value for this dialog
   void getCallId(UtlString& callId) const;
   //! Get the SIP call-id header value for this dialog
   UtlString getCallId() const;
   //! Set the SIP call-id header value for this dialog
   void setCallId(const char* callId);

   //! Get the SIP To/From header value for the local side of this dialog
   void getLocalField(Url& localUrl) const;
   //! Get the SIP To/From header value for the local side of this dialog
   void getLocalField(UtlString& sLocalUrl) const;
   //! Get the SIP To/From header value for the local side of this dialog
   Url getLocalField() const;
   //! Get the tag from the SIP To/From header value for the local side of this dialog
   void getLocalTag(UtlString& localTag) const;
   //! Set the SIP To/From header value for the local side of this dialog
   void setLocalField(const Url& localUrl);

   //! Get the SIP To/From header value for the remote side of this dialog
   void getRemoteField(Url& remoteUrl) const;
   //! Get the SIP To/From header value for the remote side of this dialog
   void getRemoteField(UtlString& sRemoteUrl) const;
   //! Get the SIP To/From header value for the remote side of this dialog
   Url getRemoteField() const;
   //! Get the tag from the SIP To/From header value for the remote side of this dialog
   void getRemoteTag(UtlString& remoteTag) const;
   //! Set the SIP To/From header value for the remote side of this dialog
   void setRemoteField(const Url& remoteUrl);

   //! Get the SIP Contact header value for the remote side of this dialog
   void getRemoteContact(Url& remoteContact) const;
   //! Get the SIP Contact header value for the remote side of this dialog
   void getRemoteContact(UtlString& sRemoteContact) const;
   //! Set the SIP Contact header value for the remote side of this dialog
   void setRemoteContact(const Url& remoteContact);

   //! Get the SIP Contact header value for the local side of this dialog
   void getLocalContact(Url& localContact) const;
   //! Get the SIP Contact header value for the local side of this dialog
   void getLocalContact(UtlString& sLocalContact) const;
   //! Get the SIP Contact header value for the remote side of this dialog
   void setLocalContact(const Url& localContact);

   //! Get the SIP method of the request that initiated this dialog
   void getInitialMethod(UtlString& method) const;
   //! Set the SIP method of the request that initiated this dialog
   void setInitialMethod(const char* method);

   //! Get the next (incremented) SIP Cseq number for the local side
   int getNextLocalCseq();
   //! Get the last used SIP Cseq number for the local side
   int getLastLocalCseq() const;
   //! Set the last used SIP Cseq number for the local side
   void setLastLocalCseq(int seqNum);

   //! Get the last used SIP Cseq number for the remote side
   int getLastRemoteCseq() const;
   //! Set the last used SIP Cseq number for the remote side
   void setLastRemoteCseq(int seqNum);

   /**
    * Get the request URI for the local side.
    *
    * This may be different than the local contact.  This is
    * what was received in the last request from the remote 
    * side.
    */
   void getLocalRequestUri(Url& requestUri) const;

   /** Set the request URI for the local side */
   void setLocalRequestUri(const Url& requestUri);

   /**
    *  Gets the requestUri we used in the initial request.
    */
   void getRemoteRequestUri(Url& requestUri) const;
   //! Set the request URI for the remote side
   void setRemoteRequestUri(const Url& requestUri);

   /** Retrieves state of SipDialog. */
   SipDialog::DialogState getDialogState() const { return m_dialogState; }
   /** Sets state of SipDialog. Avoid using it if possible. */
   void setDialogState(SipDialog::DialogState val) { m_dialogState = val; }

   /** Retrieves substate of SipDialog - unknown, early, confirmed */
   SipDialog::DialogSubState getDialogSubState() const { return m_dialogSubState; }
   void setDialogSubState(SipDialog::DialogSubState val) { m_dialogSubState = val; }

   /** Marks dialog as terminated */
   void terminateDialog();

   /** Gets the secure flag of sip dialog */
   UtlBoolean getSecure() const { return m_bSecure; }
   /** Sets the secure flag of sip dialog */
   void setSecure(UtlBoolean val) { m_bSecure = val; }

   //! Debug method to dump the contents of this SipDialog into a string
   void toString(UtlString& dialogDumpString);

   /* ============================ INQUIRY =================================== */

   /**
   * Checks if SipMessage is part of this SipDialog. strictMatch
   * affects if we want initial SipDialog to match SipMessage with
   * established dialog.
   * @param strictMatch If true then both tags must match.
   */
   UtlBoolean isSameDialog(const SipMessage& message,
                           UtlBoolean strictMatch = FALSE) const;

   /**
   * Checks if given dialog is part of this SipDialog. strictMatch
   * affects if we want initial SipDialog to match given established
   * dialog.
   * @param strictMatch If true then both tags must match.
   */
   UtlBoolean isSameDialog(const UtlString& callId,
                           const UtlString& localTag,
                           const UtlString& remoteTag,
                           UtlBoolean strictMatch = FALSE) const;

   /**
    * Compare the given dialog handle with that of this dialog.
    * Dialog handle must of the form: "callid,localtag,remotetag".
    *
    * Localtag doesn't always equal to fromtag. When constructing
    * dialogHandle from SipMessage, tag swap is done automatically.
    * @param strictMatch If true then both tags must match.
    */
   UtlBoolean isSameDialog(const UtlString& dialogHandle,
                           UtlBoolean strictMatch = FALSE) const;

   /**
   * Compares dialogs according to call-id and tags, returning type of match
   * that was detected.
   */
   DialogMatchEnum compareDialogs(const SipDialog& sipDialog) const;

   /** Returns TRUE if dialog was started locally. */
   UtlBoolean isLocalInitiatedDialog() const { return m_bLocalInitiatedDialog; }

   /** Determine if this is an initial (not yet established) dialog. Initial dialog has only 1 tag. */
   UtlBoolean isInitialDialog() const;

   /** Determine if this is an established dialog. This is NOT a negation of isInitialDialog! */
   UtlBoolean isEstablishedDialog() const;

   /** Determine if this is a terminated dialog */
   UtlBoolean isTerminatedDialog() const;

   /** Determine if this is an initial (not yet established) dialog */
   static UtlBoolean isInitialDialog(const UtlString& dialogHandle);

   /** Determine if this is an initial (not yet established) dialog */
   static UtlBoolean isInitialDialog(const UtlString& callId,
                                     const UtlString& localTag,
                                     const UtlString& remoteTag);

   /**
    * Check if current SipDialog could have been the initial dialog of
    * given SipMessage dialog. Compares callid and 1 tag.
    */
   UtlBoolean isInitialDialogOf(const SipMessage& message) const;

   /**
    * Check if current SipDialog could have been the initial dialog of
    * given dialog. Compares callid and 1 tag.
    */
   UtlBoolean isInitialDialogOf(const UtlString& callId,
                                const UtlString& localTag,
                                const UtlString& remoteTag) const;

   //! Query if the transaction request was sent from the local side
   /*! If the request was sent from the local side, the fromTag will
   *  match the local tag.
   */
   UtlBoolean isTransactionLocallyInitiated(const UtlString& callId,
                                            const UtlString& fromTag,
                                            const UtlString& toTag) const;

   //! Query if the transaction request was sent from the remote side
   /*! If the request was sent from the local side, the fromTag will
   *  match the remote tag.
   */
   UtlBoolean isTransactionRemotelyInitiated(const UtlString& callId,
                                             const UtlString& fromTag,
                                             const UtlString& toTag) const;

   //! Check if message and SIP local Cseq match
   UtlBoolean isSameLocalCseq(const SipMessage& message) const;

   //! Check if message and SIP remote Cseq match
   UtlBoolean isSameRemoteCseq(const SipMessage& message) const;

   //! Check if mesage cseq is after the last local transaction
   UtlBoolean isNextLocalCseq(const SipMessage& message) const;

   //! Check if mesage cseq is after the last remote transaction
   UtlBoolean isNextRemoteCseq(const SipMessage& message) const;

   /**
    * Gets local and remote tag for given sip message for usage in dialog. Tag swap operation
    * is performed if needed.
    */
   static void getLocalRemoteTag(const SipMessage& sipMessage, UtlString& sLocalTag, UtlString& sRemoteTag);

   /**
    * Determines if we need to swap tags when transforming from/to tag into
    * local/remote tag. Normally from=local and to/remote, but if this function
    * returns true, they need to be swapped. This allows more strict sip dialog
    * matching.
    * Depends on if message is request/response and if its inbound/outbound.
    */
   static bool isTagSwapNeeded(const SipMessage& sipMessage);

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /**
    * Updates state of dialog according to given SipMessage. This is used to update state
    * to established state, and determine if we are early or confirmed dialog.
    */
   void updateDialogState(const SipMessage* pSipMessage = NULL);

   /**
    * Updates the secure flag of SipDialog.
    */
   void updateSecureFlag(const SipMessage* pSipMessage = NULL);

   // The callId is stored in the UtlString base class data element
   Url m_localField; // To or From depending on who initiated the transaction
   Url m_remoteField; // To or From depending on who initiated the transaction
   UtlString m_sLocalTag;
   UtlString m_sRemoteTag;
   Url m_localContactField; // our contact url, we use in outbound messages (including display name)
   Url m_remoteContactField; // In RFC-3261 described as "remote target", contact url of remote party (including display name)
   UtlString m_sRouteSet; // route set for building Record-Route header
   UtlString m_sInitialMethod; // INVITE etc
   Url m_localRequestUri; // request URI used for first inbound request
   Url m_remoteRequestUri; // request URI used for first outbound request
   UtlBoolean m_bLocalInitiatedDialog; // TRUE if dialog was started locally
   int m_iInitialLocalCseq;
   int m_iInitialRemoteCseq;
   int m_iLastLocalCseq; // last used Cseq for new outbound transactions
   int m_iLastRemoteCseq; // last used Cseq for inbound transactions
   UtlBoolean m_bSecure; // TRUE if dialog is secure
   DialogState m_dialogState;
   DialogSubState m_dialogSubState;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipDialog_h_
