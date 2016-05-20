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
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipDialog.h>
#include <net/SipMessage.h>
#include <utl/UtlHashMapIterator.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipDialog::SipDialog(const SipMessage* initialMessage)
: m_dialogState(DIALOG_STATE_INITIAL)
, m_dialogSubState(DIALOG_SUBSTATE_UNKNOWN)
, m_localRequestUri(NULL, TRUE)
, m_remoteRequestUri(NULL, TRUE)
{
   if(initialMessage)
   {
      UtlBoolean isFromLocal = initialMessage->isFromThisSide();
      UtlString callId;
      initialMessage->getCallIdField(&callId);
      append(callId);
      updateSecureFlag(initialMessage);

      // The transaction was initiated from this side
      if((!initialMessage->isResponse() && isFromLocal) ||
         (initialMessage->isResponse() && !isFromLocal))
      {
         // either local request or remote response (to our previous request)
         m_bLocalInitiatedDialog = TRUE;
         initialMessage->getFromUrl(m_localField);
         m_localField.getFieldParameter("tag", m_sLocalTag);
         initialMessage->getToUrl(m_remoteField);
         m_remoteField.getFieldParameter("tag", m_sRemoteTag);
         initialMessage->getCSeqField(&m_iInitialLocalCseq, &m_sInitialMethod);
         m_iLastLocalCseq = m_iInitialLocalCseq;
         m_iLastRemoteCseq = -1;
         m_iInitialRemoteCseq = -1;
      }
      // The transaction was initiated from the other side
      else
      {
         // local response or remote inbound request
         m_bLocalInitiatedDialog = FALSE;
         initialMessage->getFromUrl(m_remoteField);
         m_remoteField.getFieldParameter("tag", m_sRemoteTag);
         initialMessage->getToUrl(m_localField);
         m_localField.getFieldParameter("tag", m_sLocalTag);
         initialMessage->getCSeqField(&m_iInitialRemoteCseq, &m_sInitialMethod);
         m_iLastRemoteCseq = m_iInitialRemoteCseq;
         // Start local CSeq's at 1, because some UAs cannot handle 0.
         m_iLastLocalCseq = 0;
         m_iInitialLocalCseq = 0;
      }

      if(initialMessage->isRequest())
      {
         // this is a request
         Url requestUri;
         initialMessage->getRequestUri(requestUri);
         if(isFromLocal)
         {
            // request from us
            m_remoteRequestUri = requestUri;
         }
         else
         {
            // Incoming initial Request, we need to set the Route set here
            if(initialMessage->isRecordRouteAccepted())
            {
               initialMessage->buildRouteField(&m_sRouteSet);
            }
            m_localRequestUri = requestUri;
         }
      }

      UtlString contactField;
      initialMessage->getContactField(0, contactField);
      if(isFromLocal)
      {
         m_localContactField.fromString(contactField);
      }
      else
      {
         m_remoteContactField.fromString(contactField);
      }
   }
   else
   {
      // Insert dummy values into fields that aren't automatically initialized.
      m_bLocalInitiatedDialog = TRUE;
      // Start local CSeq's at 1, because some UAs cannot handle 0.
      m_iLastLocalCseq = 0;
      m_iLastRemoteCseq = -1;
      m_iInitialLocalCseq = 0;
      m_iInitialRemoteCseq = -1;
      m_bSecure = FALSE;
   }

   updateDialogState(initialMessage);
}

SipDialog::SipDialog(const UtlString& sSipCallId,
                     const UtlString& sLocalTag,
                     const UtlString& sRemoteTag,
                     UtlBoolean isFromLocal)
: UtlString(sSipCallId)
, m_dialogState(DIALOG_STATE_INITIAL)
, m_dialogSubState(DIALOG_SUBSTATE_UNKNOWN)
{
   m_sLocalTag = sLocalTag;
   m_sRemoteTag = sRemoteTag;

   m_bLocalInitiatedDialog = isFromLocal;
   m_iInitialLocalCseq = 0;
   m_iInitialRemoteCseq = -1;
   m_iLastLocalCseq = 0;
   m_iLastRemoteCseq = -1;
   m_bSecure = FALSE;

   updateDialogState();
}

// Copy constructor
SipDialog::SipDialog(const SipDialog& rSipDialog)
: UtlString(rSipDialog)
{
   *this = rSipDialog;
}

// Destructor
SipDialog::~SipDialog()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipDialog& SipDialog::operator=(const SipDialog& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   UtlString::operator=(rhs);  // assign fields for parent class

   m_localField = rhs.m_localField;
   m_sLocalTag = rhs.m_sLocalTag;
   m_remoteField = rhs.m_remoteField;
   m_sRemoteTag = rhs.m_sRemoteTag;
   m_localContactField = rhs.m_localContactField;
   m_remoteContactField = rhs.m_remoteContactField;
   m_sRouteSet = rhs.m_sRouteSet;
   m_sInitialMethod = rhs.m_sInitialMethod;
   m_bLocalInitiatedDialog = rhs.m_bLocalInitiatedDialog;
   m_iInitialLocalCseq = rhs.m_iInitialLocalCseq;
   m_iInitialRemoteCseq = rhs.m_iInitialRemoteCseq;
   m_iLastLocalCseq = rhs.m_iLastLocalCseq;
   m_iLastRemoteCseq = rhs.m_iLastRemoteCseq;
   m_localRequestUri = rhs.m_localRequestUri;
   m_remoteRequestUri = rhs.m_remoteRequestUri;
   m_bSecure = rhs.m_bSecure;
   m_dialogState = rhs.m_dialogState;
   m_dialogSubState = rhs.m_dialogSubState;

   return *this;
}

SipDialog& SipDialog::operator<<(const SipMessage& message)
{
   updateDialog(message);
   return *this;
}

void SipDialog::updateDialog(const SipMessage& message)
{
   // get call-id, From, To and both tags
   UtlString messageCallId;
   message.getCallIdField(messageCallId);
   Url messageFromUrl;
   message.getFromUrl(messageFromUrl);
   UtlString messageFromTag;
   messageFromUrl.getFieldParameter("tag", messageFromTag);
   Url messageToUrl;
   message.getToUrl(messageToUrl);
   UtlString messageToTag;
   messageToUrl.getFieldParameter("tag", messageToTag);

   int cSeq;
   UtlString method;
   message.getCSeqField(&cSeq, &method);
   int responseCode = message.getResponseStatusCode();

   // Figure out if the request is from the local or
   // the remote side
   if(isTransactionLocallyInitiated(messageCallId,
      messageFromTag,
      messageToTag))
   {
      // This message is part of a transaction initiated by
      // the local side of the dialog

      if(cSeq > m_iLastLocalCseq) 
      {
         // we were probably not shown some SipMessage from this dialog
         m_iLastLocalCseq = cSeq;
      }

      if(cSeq == m_iLastLocalCseq) 
      {
         // Always update the contact if it is set
         UtlString contactField;
         // Get the Contact value, but as an addr-spec.
         if(message.getContactField(0, contactField) && !contactField.isNull())
         {
            if(message.isResponse())
            {
               if (message.isTargetRefresh() &&
                   responseCode >= SIP_1XX_CLASS_CODE &&
                   responseCode < SIP_3XX_CLASS_CODE)
               {
                  m_remoteContactField.fromString(contactField);
               }
            }
            else
            {
               m_localContactField.fromString(contactField);
            }
         }

         // A successful response to an INVITE or SUBSCRIBE
         // make this early dialog a set up dialog
         if(m_bLocalInitiatedDialog && message.isResponse() && 
            responseCode >= SIP_1XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
         {
            if (m_sRemoteTag.isNull())// tag not set
            {
               message.getToUrl(m_remoteField);
               m_remoteField.getFieldParameter("tag", m_sRemoteTag);
            }
            if (m_sRouteSet.isNull())
            {
               // Need to get the route set as well
               // Make sure the Request Method is allowed to set Record-Routes
               if(message.isRecordRouteAccepted())
               {
                  message.buildRouteField(&m_sRouteSet);
               }
            }
         }

         updateDialogState(&message);
      }
   }
   else if(isTransactionRemotelyInitiated(messageCallId,
      messageFromTag,
      messageToTag))
   {
      int prevRemoteCseq = m_iLastRemoteCseq;

      // This message is part of a transaction initiated by
      // the callee/destination of the session
      if(cSeq > m_iLastRemoteCseq) 
      {
         // we were probably not shown some SipMessage from this dialog. Perhaps remote party sent it and it was lost.
         m_iLastRemoteCseq = cSeq;
      }

      if(cSeq == m_iLastRemoteCseq)
      {
         // Always update the contact if it is set
         UtlString contactField;
         // Get the Contact value, but as an addr-spec.
         if(message.getContactField(0, contactField) && !contactField.isNull())
         {
            if(message.isResponse())
            {
               m_localContactField.fromString(contactField);
            }
            else
            {
               if (message.isTargetRefresh())
               {
                  m_remoteContactField.fromString(contactField);
               }
            }
         }

         // A response (e.g. NOTIFY) can come before we get the
         // successful response to the initial transaction
         if(m_bLocalInitiatedDialog && message.isRequest() && m_sRemoteTag.isNull()) // tag not set
         {
            // Change this early dialog to a set up dialog.
            // The tag gets set in the 2xx response
            // so we need to update the URL
            message.getFromUrl(m_remoteField);
            m_remoteField.getFieldParameter("tag", m_sRemoteTag);
         }
         if(!m_bLocalInitiatedDialog && message.isResponse() && 
            responseCode >= SIP_1XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE && m_sLocalTag.isNull()) 
         {
            // Update the local tag
            message.getToUrl(m_localField);
            m_localField.getFieldParameter("tag", m_sLocalTag);
         }
         updateDialogState(&message);
      }
   }
}

void SipDialog::setRequestData(SipMessage& request, const UtlString& method, int cseqNum)
{
   Url requestUri;
   if (m_dialogState != SipDialog::DIALOG_STATE_INITIAL &&
       m_dialogSubState == SipDialog::DIALOG_SUBSTATE_CONFIRMED)
   {
      // we are in established confirmed state, request Uri must be constructed from contact
      m_remoteContactField.getUri(requestUri);
      request.setSipRequestFirstHeaderLine(method, requestUri.toString());
   }
   else
   {
      // request Uri must be constructed either from remote request uri, or remote field
      if (!m_remoteRequestUri.isNull())
      {
         // remote request Uri is know, use it. It is known only for outbound dialog
         request.setSipRequestFirstHeaderLine(method, m_remoteRequestUri.toString());
      }
      else
      {
         // use remote field - will be used for inbound dialogs that are not yet confirmed
         m_remoteField.getUri(requestUri);
         request.setSipRequestFirstHeaderLine(method, requestUri.toString());
      }
   }

   // The local field is the From field
   UtlString fromField;
   m_localField.toString(fromField);
   request.setRawFromField(fromField);

   // The remote field is the To field
   UtlString toField;
   m_remoteField.toString(toField);
   request.setRawToField(toField);

   if (cseqNum == -1)
   {
      // Get the next local Cseq, the method should already be set
      cseqNum = getNextLocalCseq();
   }
   else
   {
      if (cseqNum > m_iLastLocalCseq)
      {
         // don't overwrite local cseq num with lower value. It might happen, as somebody might be trying
         // to send request with previous seq num (for example ACK)
         m_iLastLocalCseq = cseqNum;
      }
   }
   request.setCSeqField(cseqNum, method);

   // Set the route header according to the route set
   if(!m_sRouteSet.isNull())
   {
      request.setRouteField(m_sRouteSet);
   }

   if (!m_localContactField.isNull())
   {
      // if local contact is not configured, don't set it. SipUserAgent will add one if needed.
      request.setContactField(m_localContactField.toString());
   }

   // Set the call-id
   request.setCallIdField(*this);
}

/* ============================ ACCESSORS ================================= */

void SipDialog::getDialogHandle(const SipMessage& sipMessage, UtlString& sDialogHandle)
{
   sDialogHandle.remove(0);
   sipMessage.getCallIdField(sDialogHandle);
   sDialogHandle.append(DIALOG_HANDLE_SEPARATOR);
   UtlString sLocalTag;
   UtlString sRemoteTag;
   getLocalRemoteTag(sipMessage, sLocalTag, sRemoteTag);
   sDialogHandle.append(sLocalTag);
   sDialogHandle.append(DIALOG_HANDLE_SEPARATOR);
   sDialogHandle.append(sRemoteTag);
}

void SipDialog::getDialogHandle(UtlString& sDialogHandle) const
{
   sDialogHandle = *this; // callId
   sDialogHandle.append(DIALOG_HANDLE_SEPARATOR);
   sDialogHandle.append(m_sLocalTag);
   sDialogHandle.append(DIALOG_HANDLE_SEPARATOR);
   sDialogHandle.append(m_sRemoteTag);
}

void SipDialog::getInitialDialogHandle(UtlString& initialDialogHandle) const
{
   // Do not add the tag for the side that did not initiate the dialog
   initialDialogHandle = *this; // callId
   initialDialogHandle.append(DIALOG_HANDLE_SEPARATOR);
   if(m_bLocalInitiatedDialog)
   {
      initialDialogHandle.append(m_sLocalTag);
   }
   initialDialogHandle.append(DIALOG_HANDLE_SEPARATOR);
   if(!m_bLocalInitiatedDialog)
   {
      initialDialogHandle.append(m_sRemoteTag);
   }
}

void SipDialog::parseHandle(const UtlString& dialogHandle,
                            UtlString& callId,
                            UtlString& localTag,
                            UtlString& remoteTag)
{
   callId="";
   localTag = "";
   remoteTag = "";

   // The call-id ends at the first comma
   const char* callIdEnd = strchr(dialogHandle, DIALOG_HANDLE_SEPARATOR);
   if(callIdEnd)
   {
      // Move past the first comma
      const char* localTagBegin = callIdEnd + 1;

      // Copy the call id
      callId.append(dialogHandle, callIdEnd - dialogHandle);

      // The local tag ends at the second comma
      const char* localTagEnd = strchr(localTagBegin, DIALOG_HANDLE_SEPARATOR);
      if(localTagEnd)
      {
         // Copy the local tag
         localTag.append(localTagBegin, localTagEnd - localTagBegin);

         // The remote tag begins beyond the comma
         const char* remoteTagBegin = localTagEnd + 1;

         // Copy the remote tag
         remoteTag.append(remoteTagBegin);
      }
   }
}

void SipDialog::reverseTags(const UtlString& dialogHandle,
                            UtlString& reversedHandle)
{
   UtlString tag1;
   UtlString tag2;
   parseHandle(dialogHandle, reversedHandle, tag1, tag2); // copies call-id into reversedHandle
   reversedHandle.capacity(dialogHandle.length() + 2);
   reversedHandle.append(DIALOG_HANDLE_SEPARATOR);
   reversedHandle.append(tag2);
   reversedHandle.append(DIALOG_HANDLE_SEPARATOR);
   reversedHandle.append(tag1);
}

void SipDialog::getCallId(UtlString& callId) const
{
   callId = *this;
}

UtlString SipDialog::getCallId() const
{
   return *this;
}

void SipDialog::setCallId(const char* callId)
{
   remove(0);
   append(callId ? callId : "");
   updateDialogState(); // also update dialog state, maybe dialog got established now
}

void SipDialog::getLocalField(Url& localField) const
{
   localField = m_localField;
}

void SipDialog::getLocalField(UtlString& sLocalUrl) const
{
   m_localField.toString(sLocalUrl);
}

Url SipDialog::getLocalField() const
{
   return m_localField;
}

void SipDialog::getLocalTag(UtlString& localTag) const
{
   localTag = m_sLocalTag;
}

void SipDialog::setLocalField(const Url& localField)
{
   m_localField = localField;
   m_localField.getFieldParameter("tag", m_sLocalTag); // also set tag
   updateDialogState(); // also update dialog state, maybe dialog got established now
}

void SipDialog::getRemoteField(Url& remoteField) const
{
   remoteField = m_remoteField;
}

void SipDialog::getRemoteField(UtlString& sRemoteUrl) const
{
   m_remoteField.toString(sRemoteUrl);
}

Url SipDialog::getRemoteField() const
{
   return m_remoteField;
}

void SipDialog::getRemoteTag(UtlString& remoteTag) const
{
   remoteTag = m_sRemoteTag;
}

void SipDialog::setRemoteField(const Url& remoteField)
{
   m_remoteField = remoteField;
   m_remoteField.getFieldParameter("tag", m_sRemoteTag); // also set tag
   updateDialogState(); // also update dialog state, maybe dialog got established now
}

void SipDialog::getRemoteContact(Url& remoteContact) const
{
   remoteContact = m_remoteContactField;
}

void SipDialog::getRemoteContact(UtlString& sRemoteContact) const
{
   m_remoteContactField.toString(sRemoteContact);
}

void SipDialog::setRemoteContact(const Url& remoteContact)
{
   m_remoteContactField = remoteContact;
}

void SipDialog::getLocalContact(Url& localContact) const
{
   localContact = m_localContactField;
}

void SipDialog::getLocalContact(UtlString& sLocalContact) const
{
   m_localContactField.toString(sLocalContact);
}

void SipDialog::setLocalContact(const Url& localContact)
{
   m_localContactField = localContact;
}

void SipDialog::getLocalRequestUri(Url& requestUri) const
{
   requestUri = m_localRequestUri;
}

void SipDialog::setLocalRequestUri(const Url& requestUri)
{
   m_localRequestUri = requestUri;
}

void SipDialog::getRemoteRequestUri(Url& requestUri) const
{
   requestUri = m_remoteRequestUri;
}

void SipDialog::setRemoteRequestUri(const Url& requestUri)
{
   m_remoteRequestUri = requestUri;
}

void SipDialog::terminateDialog()
{
   m_dialogState = DIALOG_STATE_TERMINATED;
}

void SipDialog::getInitialMethod(UtlString& method) const
{
   method = m_sInitialMethod;
}

void SipDialog::setInitialMethod(const char* method)
{
   m_sInitialMethod = method;
}

int SipDialog::getLastLocalCseq() const
{
   return(m_iLastLocalCseq);
}

void SipDialog::setLastLocalCseq(int lastLocalCseq)
{
   m_iLastLocalCseq = lastLocalCseq;
}

int SipDialog::getLastRemoteCseq() const
{
   return(m_iLastRemoteCseq);
}

void SipDialog::setLastRemoteCseq(int lastRemoteCseq)
{
   m_iLastRemoteCseq = lastRemoteCseq;
}

int SipDialog::getNextLocalCseq()
{
   m_iLastLocalCseq++;
   return(m_iLastLocalCseq);
}

/* ============================ INQUIRY =================================== */


UtlBoolean SipDialog::isSameDialog(const SipMessage& message,
                                   UtlBoolean strictMatch) const
{
   UtlString messageCallId;
   UtlString localTag;
   UtlString remoteTag;

   message.getCallIdField(&messageCallId);
   getLocalRemoteTag(message, localTag, remoteTag);

   return isSameDialog(messageCallId, localTag, remoteTag, strictMatch);
}

UtlBoolean SipDialog::isSameDialog(const UtlString& callId,
                                   const UtlString& localTag,
                                   const UtlString& remoteTag,
                                   UtlBoolean strictMatch) const
{
   UtlBoolean isSameDialog = FALSE;

   if (!strictMatch && isInitialDialog())
   {
      // we are initial dialog, one tag is missing. Use relaxed comparison
      isSameDialog = isInitialDialogOf(callId, localTag, remoteTag);
   }
   else
   {
      if(callId.compareTo(*this, UtlString::ignoreCase) == 0)
      {
         // call-id matches
         // we are established dialog, compare by both tags
         if(localTag.compareTo(m_sLocalTag, UtlString::ignoreCase) == 0 &&
            remoteTag.compareTo(m_sRemoteTag, UtlString::ignoreCase) == 0)
         {
            isSameDialog = TRUE;
         }
      }
   }

   return isSameDialog;
}

UtlBoolean SipDialog::isSameDialog(const UtlString& dialogHandle,
                                   UtlBoolean strictMatch) const
{
   UtlString callId;
   UtlString localTag;
   UtlString remoteTag;
   parseHandle(dialogHandle, callId, localTag, remoteTag);
   return isSameDialog(callId, localTag, remoteTag, strictMatch);
}

SipDialog::DialogMatchEnum SipDialog::compareDialogs(const SipDialog& sipDialog) const
{
   UtlString localTag;
   UtlString remoteTag;
   UtlString callId;
   sipDialog.getLocalTag(localTag);
   sipDialog.getRemoteTag(remoteTag);
   sipDialog.getCallId(callId);

   if (isInitialDialog())
   {
      // left hand dialog is initial, one tag is missing. Use relaxed comparison
      if(isInitialDialogOf(callId, localTag, remoteTag))
      {
         if (sipDialog.isInitialDialog())
         {
            // right hand side dialog is initial
            return SipDialog::DIALOG_INITIAL_INITIAL_MATCH;
         }
         else
         {
            // right hand side dialog is established
            return SipDialog::DIALOG_INITIAL_ESTABLISHED_MATCH;
         }
      }
   }
   else
   {
      // left dialog is established
      if (!sipDialog.isInitialDialog())
      {
         // right hand side dialog is established, strict comparison
         if (isSameDialog(callId, localTag, remoteTag, TRUE))
         {
            return SipDialog::DIALOG_ESTABLISHED_MATCH;
         }
      }
      else
      {
         // right hand side dialog is initial - has only 1 tag, use relaxed comparison
         if(isInitialDialogOf(callId, localTag, remoteTag))
         {
            return SipDialog::DIALOG_ESTABLISHED_INITIAL_MATCH;
         }
      }
   }

   return SipDialog::DIALOG_MISMATCH;
}

UtlBoolean SipDialog::isInitialDialogOf(const SipMessage& message) const
{
   UtlString messageCallId;
   UtlString localTag;
   UtlString remoteTag;

   message.getCallIdField(&messageCallId);
   getLocalRemoteTag(message, localTag, remoteTag);

   return isInitialDialogOf(messageCallId, localTag, remoteTag);
}

UtlBoolean SipDialog::isInitialDialogOf(const UtlString& callId,
                                        const UtlString& localTag,
                                        const UtlString& remoteTag) const
{
   UtlBoolean isSameInitialDialog = FALSE;
   // we don't check if we are currently established or initial dialog
   // for initial dialog, callId and localTag must match
   if(this->compareTo(callId, UtlString::ignoreCase) == 0)
   {
      // if dialog was initiated remotely, then remote tag gets filled first, with local tag being filled later by us
      if (m_bLocalInitiatedDialog)
      {
         if (localTag.compareTo(m_sLocalTag, UtlString::ignoreCase) == 0)
         {
            isSameInitialDialog = TRUE;
         }
      }
      else
      {
         // remotely initiated dialog
         if (remoteTag.compareTo(m_sRemoteTag, UtlString::ignoreCase) == 0)
         {
            isSameInitialDialog = TRUE;
         }
      }
   }

   return isSameInitialDialog;
}

UtlBoolean SipDialog::isTransactionLocallyInitiated(const UtlString& callId,
                                                    const UtlString& fromTag,
                                                    const UtlString& toTag) const
{
   UtlBoolean isLocalDialog = FALSE;
   if(callId.compareTo(*this, UtlString::ignoreCase) == 0)
   {
      if(fromTag.compareTo(m_sLocalTag, UtlString::ignoreCase) == 0 &&
         (toTag.compareTo(m_sRemoteTag, UtlString::ignoreCase) == 0 || toTag.isNull() || m_sRemoteTag.isNull()))
      {
         isLocalDialog = TRUE;
      }
   }

   return(isLocalDialog);
}

UtlBoolean SipDialog::isTransactionRemotelyInitiated(const UtlString& callId,
                                                     const UtlString& fromTag,
                                                     const UtlString& toTag) const
{
   UtlBoolean isRemoteDialog = FALSE;
   if(callId.compareTo(*this, UtlString::ignoreCase) == 0)
   {
      if(((toTag.compareTo(m_sLocalTag, UtlString::ignoreCase) == 0 || toTag.isNull() || m_sLocalTag.isNull()) &&
         (fromTag.compareTo(m_sRemoteTag, UtlString::ignoreCase) == 0)) ||
         // also handle special case when NOTIFY is received before response to SUBSCRIBE
         // NOTIFY will have from tag but our remote tag is null. But to tag must match local tag.
         (toTag.compareTo(m_sLocalTag, UtlString::ignoreCase) == 0 && (m_sRemoteTag.isNull() && !fromTag.isNull())))
      {
         isRemoteDialog = TRUE;
      }
   }

   return(isRemoteDialog);
}

UtlBoolean SipDialog::isInitialDialog() const
{
   return (m_dialogState == DIALOG_STATE_INITIAL);
}

UtlBoolean SipDialog::isEstablishedDialog() const
{
   return (m_dialogState == DIALOG_STATE_ESTABLISHED);
}

UtlBoolean SipDialog::isTerminatedDialog() const
{
   return (m_dialogState == DIALOG_STATE_TERMINATED);
}

UtlBoolean SipDialog::isInitialDialog(const UtlString& handle)
{
   // For now make the simple assumption that if one of
   // the tags is not set that this is an early dialog
   // Note: RFC 2543 clients only needed to optionally
   // set the tags.  I do not think we need to support
   // RFC 2543 in this class.
   UtlBoolean tagNotSet = FALSE;
   if(!handle.isNull())
   {
      UtlString callId;
      UtlString localTag;
      UtlString remoteTag;
      parseHandle(handle, callId, localTag, remoteTag);
      if(localTag.isNull() || remoteTag.isNull())
      {
         tagNotSet = TRUE;
      }
   }
   return tagNotSet;
}

UtlBoolean SipDialog::isInitialDialog(const UtlString& callId,
                                      const UtlString& localTag,
                                      const UtlString& remoteTag)
{
   UtlBoolean tagNotSet = FALSE;
   if(localTag.isNull() || remoteTag.isNull())
   {
      tagNotSet = TRUE;
   }
   return tagNotSet;
}

UtlBoolean SipDialog::isSameLocalCseq(const SipMessage& message) const
{
   int cseq;
   message.getCSeqField(&cseq, NULL);

   return(cseq == m_iLastLocalCseq);
}

UtlBoolean SipDialog::isSameRemoteCseq(const SipMessage& message) const
{
   int cseq;
   message.getCSeqField(&cseq, NULL);

   return(cseq == m_iLastRemoteCseq);
}

UtlBoolean SipDialog::isNextLocalCseq(const SipMessage& message) const
{
   int cseq;
   message.getCSeqField(&cseq, NULL);

   return(cseq > m_iLastLocalCseq);
}

UtlBoolean SipDialog::isNextRemoteCseq(const SipMessage& message) const
{
   int cseq;
   message.getCSeqField(&cseq, NULL);

   return(cseq > m_iLastRemoteCseq);
}

void SipDialog::toString(UtlString& dialogDumpString)
{
   // Serialize all the members into the dumpString
   char numberString[20];
   dialogDumpString="SipDialog: ";
   SNPRINTF(numberString, sizeof(numberString), "%p", this);
   dialogDumpString.append(numberString);
   dialogDumpString.append("\nCall-Id:");
   // The callId is stored in the UtlString base class data element
   dialogDumpString.append(*this);
   dialogDumpString.append("\nmLocalField:");
   UtlString tmpString;
   m_localField.toString(tmpString);
   dialogDumpString.append(tmpString);
   dialogDumpString.append("\nmRemoteField:");
   m_remoteField.toString(tmpString);
   dialogDumpString.append(tmpString);
   dialogDumpString.append("\nmLocalTag:");
   dialogDumpString.append(m_sLocalTag);
   dialogDumpString.append("\nmRemoteTag:");
   dialogDumpString.append(m_sRemoteTag);
   dialogDumpString.append("\nmLocalContact:");
   m_localContactField.toString(tmpString);
   dialogDumpString.append(tmpString);
   dialogDumpString.append("\nmRemoteContact:");
   m_remoteContactField.toString(tmpString);
   dialogDumpString.append(tmpString);
   dialogDumpString.append("\nmRouteSet:");
   dialogDumpString.append(m_sRouteSet);
   dialogDumpString.append("\nmInitialMethod:");
   dialogDumpString.append(m_sInitialMethod);
   dialogDumpString.append("\nmsLocalRequestUri:");
   dialogDumpString.append(m_localRequestUri.toString());
   dialogDumpString.append("\nmsRemoteRequestUri:");
   dialogDumpString.append(m_remoteRequestUri.toString());
   dialogDumpString.append("\nmLocalInitatedDialog:");
   dialogDumpString.append(m_bLocalInitiatedDialog);
   SNPRINTF(numberString, sizeof(numberString), "%d", m_iInitialLocalCseq);
   dialogDumpString.append("\nmInitialLocalCseq:");
   dialogDumpString.append(numberString);
   SNPRINTF(numberString, sizeof(numberString), "%d", m_iInitialRemoteCseq);
   dialogDumpString.append("\nmInitialRemoteCseq:");
   dialogDumpString.append(numberString);
   SNPRINTF(numberString, sizeof(numberString), "%d", m_iLastLocalCseq);
   dialogDumpString.append("\nmLastLocalCseq:");
   dialogDumpString.append(numberString);
   SNPRINTF(numberString, sizeof(numberString), "%d", m_iLastRemoteCseq);
   dialogDumpString.append("\nmLastRemoteCseq:");
   dialogDumpString.append(numberString);
   SNPRINTF(numberString, sizeof(numberString), "%d", (int)m_bSecure);
   dialogDumpString.append("\nmSecure:");
   dialogDumpString.append(numberString);
   SNPRINTF(numberString, sizeof(numberString), "%d", (int)m_dialogState);
   dialogDumpString.append("\nmDialogState:");
   dialogDumpString.append(numberString);
   SNPRINTF(numberString, sizeof(numberString), "%d", (int)m_dialogSubState);
   dialogDumpString.append("\nmDialogSubState:");
   dialogDumpString.append(numberString);
}

void SipDialog::getLocalRemoteTag(const SipMessage& sipMessage, UtlString& sLocalTag, UtlString& sRemoteTag)
{
   sLocalTag.remove(0);
   sRemoteTag.remove(0);
   Url messageFromUrl;
   sipMessage.getFromUrl(messageFromUrl);
   Url messageToUrl;
   sipMessage.getToUrl(messageToUrl);
   bool swapTags = isTagSwapNeeded(sipMessage);

   if (swapTags)
   {
      messageFromUrl.getFieldParameter("tag", sRemoteTag);
      messageToUrl.getFieldParameter("tag", sLocalTag);
   }
   else
   {
      messageFromUrl.getFieldParameter("tag", sLocalTag);
      messageToUrl.getFieldParameter("tag", sRemoteTag);
   }
}

bool SipDialog::isTagSwapNeeded(const SipMessage& sipMessage)
{
   return !((bool)sipMessage.isRequest()) ^ ((bool)!sipMessage.isFromThisSide()); // bool XOR, don't change to UtlBoolean!
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipDialog::updateDialogState(const SipMessage* pSipMessage)
{
   // only update state if not terminated
   if (m_dialogState != DIALOG_STATE_TERMINATED)
   {
      if (!isNull())
      {
         // call-id is not null
         if (!m_sLocalTag.isNull())
         {
            if (!m_sRemoteTag.isNull())
            {
               m_dialogState = DIALOG_STATE_ESTABLISHED;
            }
            else
            {
               m_dialogState = DIALOG_STATE_INITIAL;
            }
         }
         else
         {
            m_dialogState = DIALOG_STATE_INITIAL;
         }
      }
      else
      {
         m_dialogState = DIALOG_STATE_INITIAL;
      }

      if (m_dialogState == DIALOG_STATE_ESTABLISHED)
      {
         // check if SipMessage is 200 Ok
         if (pSipMessage && pSipMessage->isResponse())
         {
            int statusCode = pSipMessage->getResponseStatusCode();
            // 100 Trying cannot establish early dialog
            if (statusCode > SIP_1XX_CLASS_CODE && statusCode < SIP_2XX_CLASS_CODE)
            {
               // we have both tags and 200 response, this is a confirmed dialog
               m_dialogSubState = DIALOG_SUBSTATE_EARLY;
            }
            else if (statusCode >= SIP_2XX_CLASS_CODE && statusCode < SIP_3XX_CLASS_CODE)
            {
               // we have both tags and 200 response, this is a confirmed dialog
               m_dialogSubState = DIALOG_SUBSTATE_CONFIRMED;
            }
         }
      }
      else if (m_dialogState == DIALOG_STATE_INITIAL)
      {
         // if dialog is in initial state and we get 5xx or 6xx, we terminate it
         if (pSipMessage && pSipMessage->isResponse())
         {
            int statusCode = pSipMessage->getResponseStatusCode();
            if (statusCode >= SIP_5XX_CLASS_CODE)
            {
               m_dialogState = DIALOG_STATE_TERMINATED;
            }
         }
      }
   }
}

void SipDialog::updateSecureFlag(const SipMessage* pSipMessage /*= NULL*/)
{
   if (pSipMessage && pSipMessage->isRequest())
   {
      // init secure flag
      Url requestUri;
      pSipMessage->getRequestUri(requestUri);
      if (requestUri.getScheme() == Url::SipsUrlScheme)
      {
         m_bSecure = TRUE;
         return;
      }
   }

   m_bSecure = FALSE;
}

/* ============================ FUNCTIONS ================================= */

