//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#ifndef _WIN32
#include <stddef.h>
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* _WIN32 */

#include <assert.h>
// APPLICATION INCLUDES
#include <utl/UtlInit.h>
#include "tapi/sipXtapiEvents.h"
#include "tapi/SipXEvents.h"
#include "tapi/SipXEventDispatcher.h"
#include "os/OsDefs.h"
#include "utl/UtlSList.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsMutex.h"
#include "utl/UtlString.h"
#include "os/OsLock.h"
#include "net/Url.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlString.h"

// DEFINES
// GLOBAL VARIABLES
// EXTERNAL VARIABLES
// EXTERNAL FUNCTIONS
// STRUCTURES
// FUNCTION DECLARATIONS

/* ============================ FUNCTIONS ================================= */


/*********************************************************************/
/*       Conversion of events to strings                             */
/*********************************************************************/


static const char* convertEventCategoryToString(SIPX_EVENT_CATEGORY category)
{
   const char* str = "Unknown";

   switch (category)
   {
   case EVENT_CATEGORY_CALLSTATE:
      str = MAKESTR(EVENT_CATEGORY_CALLSTATE);
      break;
   case EVENT_CATEGORY_LINESTATE:
      str = MAKESTR(EVENT_CATEGORY_LINESTATE);
      break;
   case EVENT_CATEGORY_INFO_STATUS:
      str = MAKESTR(EVENT_CATEGORY_INFO_STATUS);
      break;
   case EVENT_CATEGORY_INFO:
      str = MAKESTR(EVENT_CATEGORY_INFO);
      break;
   case EVENT_CATEGORY_SUB_STATUS:
      str = MAKESTR(EVENT_CATEGORY_SUB_STATUS);
      break;
   case EVENT_CATEGORY_NOTIFY:
      str = MAKESTR(EVENT_CATEGORY_NOTIFY);
      break;
   case EVENT_CATEGORY_CONFIG:
      str = MAKESTR(EVENT_CATEGORY_CONFIG);
      break;
   case EVENT_CATEGORY_SECURITY:
      str = MAKESTR(EVENT_CATEGORY_SECURITY);
      break;
   case EVENT_CATEGORY_MEDIA:
      str = MAKESTR(EVENT_CATEGORY_MEDIA);
      break;
   case EVENT_CATEGORY_PIM:
      str = MAKESTR(EVENT_CATEGORY_PIM);
      break;
   case EVENT_CATEGORY_KEEPALIVE:
      str = MAKESTR(EVENT_CATEGORY_KEEPALIVE);
      break;
   case EVENT_CATEGORY_RTP_REDIRECT:
      str = MAKESTR(EVENT_CATEGORY_RTP_REDIRECT);
      break;
   case EVENT_CATEGORY_CONFERENCE:
	   str = MAKESTR(EVENT_CATEGORY_CONFERENCE);
	   break;
   default:
      break;
   }

   return str;
}

const char* convertCallstateEventToString(SIPX_CALLSTATE_EVENT eMajor)
{
   const char* str = "Unknown";

   switch (eMajor)
   {
   case CALLSTATE_UNKNOWN:
      str = MAKESTR(CALLSTATE_UNKNOWN);
      break;
   case CALLSTATE_NEWCALL:
      str = MAKESTR(CALLSTATE_NEWCALL);
      break;
   case CALLSTATE_DIALTONE:
      str = MAKESTR(CALLSTATE_DIALTONE);
      break;
   case CALLSTATE_REMOTE_OFFERING:
      str = MAKESTR(CALLSTATE_REMOTE_OFFERING);
      break;
   case CALLSTATE_REMOTE_ALERTING:
      str = MAKESTR(CALLSTATE_REMOTE_ALERTING);
      break;
   case CALLSTATE_CONNECTED:
      str = MAKESTR(CALLSTATE_CONNECTED);
      break;
   case CALLSTATE_BRIDGED:
      str = MAKESTR(CALLSTATE_BRIDGED);
      break;
   case CALLSTATE_HELD:
      str = MAKESTR(CALLSTATE_HELD);
      break;
   case CALLSTATE_REMOTE_HELD:
      str = MAKESTR(CALLSTATE_REMOTE_HELD);
      break;
   case CALLSTATE_DISCONNECTED:
      str = MAKESTR(CALLSTATE_DISCONNECTED);
      break;
   case CALLSTATE_OFFERING:
      str = MAKESTR(CALLSTATE_OFFERING);
      break;
   case CALLSTATE_ALERTING:
      str = MAKESTR(CALLSTATE_ALERTING);
      break;
   case CALLSTATE_DESTROYED:
      str = MAKESTR(CALLSTATE_DESTROYED);
      break;
   case CALLSTATE_TRANSFER_EVENT:
      str = MAKESTR(CALLSTATE_TRANSFER_EVENT);
      break;
   case CALLSTATE_QUEUED:
      str = MAKESTR(CALLSTATE_QUEUED);
      break;
   case CALLSTATE_REMOTE_QUEUED:
      str = MAKESTR(CALLSTATE_REMOTE_QUEUED);
      break;
   default:
      break;
   }

   return str;
}


const char* convertCallstateCauseToString(SIPX_CALLSTATE_CAUSE eMinor)
{
   const char* str = "Unknown";

   switch (eMinor)
   {
   case CALLSTATE_CAUSE_UNKNOWN:
      str = MAKESTR(CALLSTATE_CAUSE_UNKNOWN);
      break;
   case CALLSTATE_CAUSE_NORMAL:
      str = MAKESTR(CALLSTATE_CAUSE_NORMAL);
      break;
   case CALLSTATE_CAUSE_TRANSFERRED:
      str = MAKESTR(CALLSTATE_CAUSE_TRANSFERRED);
      break;
   case CALLSTATE_CAUSE_TRANSFER:
      str = MAKESTR(CALLSTATE_CAUSE_TRANSFER);
      break;
   case CALLSTATE_CAUSE_CONFERENCE:
      str = MAKESTR(CALLSTATE_CAUSE_CONFERENCE);
      break;
   case CALLSTATE_CAUSE_EARLY_MEDIA:
      str = MAKESTR(CALLSTATE_CAUSE_EARLY_MEDIA);
      break;
   case CALLSTATE_CAUSE_REQUEST_NOT_ACCEPTED:
      str = MAKESTR(CALLSTATE_CAUSE_REQUEST_NOT_ACCEPTED);
      break;
   case CALLSTATE_CAUSE_BAD_ADDRESS:
      str = MAKESTR(CALLSTATE_CAUSE_BAD_ADDRESS);
      break;
   case CALLSTATE_CAUSE_BUSY:
      str = MAKESTR(CALLSTATE_CAUSE_BUSY);
      break;
   case CALLSTATE_CAUSE_RESOURCE_LIMIT:
      str = MAKESTR(CALLSTATE_CAUSE_RESOURCE_LIMIT);
      break;
   case CALLSTATE_CAUSE_NETWORK:
      str = MAKESTR(CALLSTATE_CAUSE_NETWORK);
      break;
   case CALLSTATE_CAUSE_REJECTED:
      str = MAKESTR(CALLSTATE_CAUSE_REJECTED);
      break;
   case CALLSTATE_CAUSE_REDIRECTED:
      str = MAKESTR(CALLSTATE_CAUSE_REDIRECTED);
      break;
   case CALLSTATE_CAUSE_NO_RESPONSE:
      str = MAKESTR(CALLSTATE_CAUSE_NO_RESPONSE);
      break;
   case CALLSTATE_CAUSE_AUTH:
      str = MAKESTR(CALLSTATE_CAUSE_AUTH);
      break;
   case CALLSTATE_CAUSE_TRANSFER_INITIATED:
      str = MAKESTR(CALLSTATE_CAUSE_TRANSFER_INITIATED);
      break;
   case CALLSTATE_CAUSE_TRANSFER_ACCEPTED:
      str = MAKESTR(CALLSTATE_CAUSE_TRANSFER_ACCEPTED);
      break;
   case CALLSTATE_CAUSE_TRANSFER_TRYING:
      str = MAKESTR(CALLSTATE_CAUSE_TRANSFER_TRYING);
      break;
   case CALLSTATE_CAUSE_TRANSFER_RINGING:
      str = MAKESTR(CALLSTATE_CAUSE_TRANSFER_RINGING);
      break;
   case CALLSTATE_CAUSE_TRANSFER_SUCCESS:
      str = MAKESTR(CALLSTATE_CAUSE_TRANSFER_SUCCESS);
      break;
   case CALLSTATE_CAUSE_TRANSFER_FAILURE:
      str = MAKESTR(CALLSTATE_CAUSE_TRANSFER_FAILURE);
      break;
   case CALLSTATE_CAUSE_REMOTE_SMIME_UNSUPPORTED:
      str = MAKESTR(CALLSTATE_CAUSE_REMOTE_SMIME_UNSUPPORTED);
      break;
   case CALLSTATE_CAUSE_SMIME_FAILURE:
      str = MAKESTR(CALLSTATE_CAUSE_SMIME_FAILURE);
      break;
   case CALLSTATE_CAUSE_BAD_REFER:
      str = MAKESTR(CALLSTATE_CAUSE_BAD_REFER);
      break;
   case CALLSTATE_CAUSE_NO_KNOWN_INVITE:
      str = MAKESTR(CALLSTATE_CAUSE_NO_KNOWN_INVITE);
      break;
   case CALLSTATE_CAUSE_BYE_DURING_IDLE:
      str = MAKESTR(CALLSTATE_CAUSE_BYE_DURING_IDLE);
      break;
   case CALLSTATE_CAUSE_UNKNOWN_STATUS_CODE:
      str = MAKESTR(CALLSTATE_CAUSE_UNKNOWN_STATUS_CODE);
      break;
   case CALLSTATE_CAUSE_BAD_REDIRECT:
      str = MAKESTR(CALLSTATE_CAUSE_BAD_REDIRECT);
      break;
   case CALLSTATE_CAUSE_TRANSACTION_DOES_NOT_EXIST:
      str = MAKESTR(CALLSTATE_CAUSE_TRANSACTION_DOES_NOT_EXIST);
      break;
   case CALLSTATE_CAUSE_CANCEL:
      str = MAKESTR(CALLSTATE_CAUSE_CANCEL);
      break;
   case CALLSTATE_CAUSE_CLIENT_ERROR:
      str = MAKESTR(CALLSTATE_CAUSE_CLIENT_ERROR);
      break;
   case CALLSTATE_CAUSE_SERVER_ERROR:
      str = MAKESTR(CALLSTATE_CAUSE_SERVER_ERROR);
      break;
   case CALLSTATE_CAUSE_GLOBAL_ERROR:
      str = MAKESTR(CALLSTATE_CAUSE_GLOBAL_ERROR);
      break;
   default:
      break;
   }
   return str;
}


SIPXTAPI_API const char* sipxMediaEventToString(SIPX_MEDIA_EVENT event)
{
   const char* str = "Unknown";

   switch (event)
   {
   case MEDIA_UNKNOWN:
      str = MAKESTR(MEDIA_UNKNOWN);
      break;
   case MEDIA_LOCAL_START:
      str = MAKESTR(MEDIA_LOCAL_START);
      break;
   case MEDIA_LOCAL_STOP:
      str = MAKESTR(MEDIA_LOCAL_STOP);
      break;
   case MEDIA_REMOTE_START:
      str = MAKESTR(MEDIA_REMOTE_START);
      break;
   case MEDIA_REMOTE_STOP:
      str = MAKESTR(MEDIA_REMOTE_STOP);
      break;
   case MEDIA_REMOTE_SILENT:
      str = MAKESTR(MEDIA_REMOTE_SILENT);
      break;
   case MEDIA_PLAYFILE_START:
      str = MAKESTR(MEDIA_PLAYFILE_START);
      break;
   case MEDIA_PLAYFILE_STOP:
      str = MAKESTR(MEDIA_PLAYFILE_STOP);
      break;
   case MEDIA_PLAYBUFFER_START:
      str = MAKESTR(MEDIA_PLAYBUFFER_START);
      break;
   case MEDIA_PLAYBUFFER_STOP:
      str = MAKESTR(MEDIA_PLAYBUFFER_STOP);
      break;
   case MEDIA_PLAYBACK_PAUSED:
      str = MAKESTR(MEDIA_PLAYBACK_PAUSED);
      break;
   case MEDIA_PLAYBACK_RESUMED:
      str = MAKESTR(MEDIA_PLAYBACK_RESUMED);
      break;
   case MEDIA_REMOTE_DTMF:
      str = MAKESTR(MEDIA_REMOTE_DTMF);
      break;
   case MEDIA_DEVICE_FAILURE:
      str = MAKESTR(MEDIA_DEVICE_FAILURE);
      break;
   case MEDIA_REMOTE_ACTIVE:
      str = MAKESTR(MEDIA_REMOTE_ACTIVE);
      break;
   case MEDIA_RECORDING_START:
      str = MAKESTR(MEDIA_RECORDING_START);
      break;
   case MEDIA_RECORDING_STOP:
      str = MAKESTR(MEDIA_RECORDING_STOP);
      break;
   default:
      break;
   }
   return str;
}


static const char* convertMediaTypeToString(SIPX_MEDIA_TYPE type)
{
   const char* str = "Unknown";

   switch (type)
   {
   case MEDIA_TYPE_AUDIO:
      str = MAKESTR(MEDIA_TYPE_AUDIO);
      break;
   case MEDIA_TYPE_VIDEO:
      str = MAKESTR(MEDIA_TYPE_VIDEO);
      break;
   default:
      break;
   }
   return str;
}


SIPXTAPI_API const char* sipxMediaCauseToString(SIPX_MEDIA_CAUSE cause)
{
   const char* str = "Unknown";

   switch (cause)
   {
   case MEDIA_CAUSE_NORMAL:
      str = MAKESTR(MEDIA_CAUSE_NORMAL);
      break;
   case MEDIA_CAUSE_HOLD:
      str = MAKESTR(MEDIA_CAUSE_HOLD);
      break;
   case MEDIA_CAUSE_UNHOLD:
      str = MAKESTR(MEDIA_CAUSE_UNHOLD);
      break;
   case MEDIA_CAUSE_FAILED:
      str = MAKESTR(MEDIA_CAUSE_FAILED);
      break;
   case MEDIA_CAUSE_DEVICE_UNAVAILABLE:
      str = MAKESTR(MEDIA_CAUSE_DEVICE_UNAVAILABLE);
      break;
   case MEDIA_CAUSE_INCOMPATIBLE:
      str = MAKESTR(MEDIA_CAUSE_INCOMPATIBLE);
      break;
   case MEDIA_CAUSE_DTMF_INBAND:
      str = MAKESTR(MEDIA_CAUSE_DTMF_INBAND);
      break;
   case MEDIA_CAUSE_DTMF_RFC2833:
      str = MAKESTR(MEDIA_CAUSE_DTMF_RFC2833);
      break;
   case MEDIA_CAUSE_DTMF_SIPINFO:
      str = MAKESTR(MEDIA_CAUSE_DTMF_SIPINFO);
      break;
   default:
      break;
   }
   return str;
}


static const char* convertPIMEventToString(SIPX_PIM_EVENT event)
{
   const char* str = "Unknown";

   switch (event)
   {
   case PIM_INCOMING_MESSAGE:
      str = MAKESTR(PIM_INCOMING_MESSAGE);
      break;
   default:
      break;
   }
   return str;
}


SIPXTAPI_API const char* sipxKeepaliveEventToString(SIPX_KEEPALIVE_EVENT event)
{
   const char* str = "Unknown";

   switch (event)
   {
   case KEEPALIVE_START:
      str = MAKESTR(KEEPALIVE_START);
      break;
   case KEEPALIVE_FEEDBACK:
      str = MAKESTR(KEEPALIVE_FEEDBACK);
      break;
   case KEEPALIVE_FAILURE:
      str = MAKESTR(KEEPALIVE_FAILURE);
      break;
   case KEEPALIVE_STOP:
      str = MAKESTR(KEEPALIVE_STOP);
      break;
   default:
      break;
   }
   return str;
}


SIPXTAPI_API const char* sipxKeepaliveCauseToString(SIPX_KEEPALIVE_CAUSE event)
{
   const char* str = "Unknown";

   switch (event)
   {
   case KEEPALIVE_CAUSE_NORMAL:
      str = MAKESTR(KEEPALIVE_CAUSE_NORMAL);
      break;
   default:
      break;
   }
   return str;
}


const char* convertKeepaliveTypeToString(SIPX_KEEPALIVE_TYPE type)
{
   const char* str = "Unknown";

   switch (type)
   {
   case SIPX_KEEPALIVE_CRLF:
      str = MAKESTR(SIPX_KEEPALIVE_CRLF);
      break;
   case SIPX_KEEPALIVE_STUN:
      str = MAKESTR(SIPX_KEEPALIVE_STUN);
      break;
   case SIPX_KEEPALIVE_SIP_OPTIONS:
      str = MAKESTR(SIPX_KEEPALIVE_SIP_OPTIONS);
      break;
   default:
      break;
   }
   return str;
}

SIPXTAPI_API const char* sipxRtpRedirectEventToString(SIPX_RTP_REDIRECT_EVENT event)
{
   const char* str = "Unknown";

   switch (event)
   {
   case RTP_REDIRECT_REQUESTED:
      str = MAKESTR(RTP_REDIRECT_REQUESTED);
      break;
   case RTP_REDIRECT_ACTIVE:
      str = MAKESTR(RTP_REDIRECT_ACTIVE);
      break;
   case RTP_REDIRECT_ERROR:
      str = MAKESTR(RTP_REDIRECT_ERROR);
      break;
   case RTP_REDIRECT_STOP:
      str = MAKESTR(RTP_REDIRECT_STOP);
      break;
   default:
      break;
   }
   return str;
}

SIPXTAPI_API const char* sipxRtpRedirectCauseToString(SIPX_RTP_REDIRECT_CAUSE cause)
{
   const char* str = "Unknown";

   switch (cause)
   {
   case RTP_REDIRECT_CAUSE_NORMAL:
      str = MAKESTR(RTP_REDIRECT_CAUSE_NORMAL);
      break;
   case RTP_REDIRECT_CAUSE_SDP_CODEC_MISMATCH:
      str = MAKESTR(RTP_REDIRECT_CAUSE_SDP_CODEC_MISMATCH);
      break;
   case RTP_REDIRECT_CAUSE_CALL_NOT_READY:
      str = MAKESTR(RTP_REDIRECT_CAUSE_CALL_NOT_READY);
      break;
   case RTP_REDIRECT_CAUSE_SETUP_FAILED:
      str = MAKESTR(RTP_REDIRECT_CAUSE_SETUP_FAILED);
      break;
   default:
      break;
   }
   return str;
}

SIPXTAPI_API const char* sipxConferenceEventToString(SIPX_CONFERENCE_EVENT event)
{
	const char* str = "Unknown";

	switch (event)
	{
	case CONFERENCE_CREATED:
		str = MAKESTR(CONFERENCE_CREATED);
		break;
	case CONFERENCE_DESTROYED:
		str = MAKESTR(CONFERENCE_DESTROYED);
		break;
	case CONFERENCE_CALL_ADDED:
		str = MAKESTR(CONFERENCE_CALL_ADDED);
		break;
   case CONFERENCE_CALL_ADD_FAILURE:
      str = MAKESTR(CONFERENCE_CALL_ADD_FAILURE);
      break;
	case CONFERENCE_CALL_REMOVED:
		str = MAKESTR(CONFERENCE_CALL_REMOVED);
		break;
   case CONFERENCE_CALL_REMOVE_FAILURE:
      str = MAKESTR(CONFERENCE_CALL_REMOVE_FAILURE);
      break;
	default:
		break;
	}
	return str;
}

SIPXTAPI_API const char* sipxConferenceCauseToString(SIPX_CONFERENCE_CAUSE cause)
{
	const char* str = "Unknown";

	switch (cause)
	{
	case CONFERENCE_CAUSE_NORMAL:
		str = MAKESTR(CONFERENCE_CAUSE_NORMAL);
		break;
   case CP_CONFERENCE_CAUSE_INVALID_STATE:
      str = MAKESTR(CP_CONFERENCE_CAUSE_INVALID_STATE);
      break;
   case CONFERENCE_CAUSE_NOT_FOUND:
      str = MAKESTR(CONFERENCE_CAUSE_NOT_FOUND);
      break;
   case CONFERENCE_CAUSE_LIMIT_REACHED:
      str = MAKESTR(CONFERENCE_CAUSE_LIMIT_REACHED);
      break;
   case CONFERENCE_CAUSE_UNEXPECTED_ERROR:
      str = MAKESTR(CONFERENCE_CAUSE_UNEXPECTED_ERROR);
      break;
	default:
		break;
	}
	return str;
}

static const char* convertInfoStatusEventToString(SIPX_INFOSTATUS_EVENT event)
{
   const char* str = "Unknown";

   switch (event)
   {
   case INFOSTATUS_UNKNOWN:
      str = MAKESTR(INFOSTATUS_UNKNOWN);
      break;
   case INFOSTATUS_RESPONSE:
      str = MAKESTR(INFOSTATUS_RESPONSE);
      break;
   case INFOSTATUS_NETWORK_ERROR:
      str = MAKESTR(INFOSTATUS_NETWORK_ERROR);
      break;
   default:
      break;
   }

   return str;
}


static const char* convertMessageStatusToString(SIPX_MESSAGE_STATUS status)
{
   const char* str = "Unknown";

   switch (status)
   {
   case SIPX_MESSAGE_OK:
      str = MAKESTR(SIPX_MESSAGE_OK);
      break;
   case SIPX_MESSAGE_FAILURE:
      str = MAKESTR(SIPX_MESSAGE_FAILURE);
      break;
   case SIPX_MESSAGE_SERVER_FAILURE:
      str = MAKESTR(SIPX_MESSAGE_SERVER_FAILURE);
      break;
   case SIPX_MESSAGE_GLOBAL_FAILURE:
      str = MAKESTR(SIPX_MESSAGE_GLOBAL_FAILURE);
      break;
   default:
      break;
   }

   return str;
}


SIPXTAPI_API const char* sipxConfigEventToString(SIPX_CONFIG_EVENT event)
{
   const char* str = "Unknown";

   switch (event)
   {
   case CONFIG_UNKNOWN:
      str = MAKESTR(CONFIG_UNKNOWN);
      break;
   case CONFIG_STUN_SUCCESS:
      str = MAKESTR(CONFIG_STUN_SUCCESS);
      break;
   case CONFIG_STUN_FAILURE:
      str = MAKESTR(CONFIG_STUN_FAILURE);
      break;
   default:
      break;
   }

   return str;
}


SIPXTAPI_API const char* sipxSubStatusStateToString(SIPX_SUBSCRIPTION_STATE state)
{
   const char* str = "Unknown";

   switch (state)
   {
   case SIPX_SUBSCRIPTION_PENDING:
      str = MAKESTR(SIPX_SUBSCRIPTION_PENDING);
      break;
   case SIPX_SUBSCRIPTION_ACTIVE:
      str = MAKESTR(SIPX_SUBSCRIPTION_ACTIVE);
      break;
   case SIPX_SUBSCRIPTION_FAILED:
      str = MAKESTR(SIPX_SUBSCRIPTION_FAILED);
      break;
   case SIPX_SUBSCRIPTION_EXPIRED:
      str = MAKESTR(SIPX_SUBSCRIPTION_EXPIRED);
      break;
   default:
      break;
   }

   return str;
}


SIPXTAPI_API const char* sipxSubStatusCauseToString(SIPX_SUBSCRIPTION_CAUSE cause)
{
   const char* str = "Unknown";

   switch (cause)
   {
   case SUBSCRIPTION_CAUSE_UNKNOWN:
      str = MAKESTR(SUBSCRIPTION_CAUSE_UNKNOWN);
      break;
   case SUBSCRIPTION_CAUSE_NORMAL:
      str = MAKESTR(SUBSCRIPTION_CAUSE_NORMAL);
      break;
   default:
      break;
   }

   return str;
}


static const char* convertLinestateEventToString(SIPX_LINESTATE_EVENT event)
{
   const char* str = "Unknown";

   switch (event)
   {
   case LINESTATE_UNKNOWN:
      str = MAKESTR(LINESTATE_UNKNOWN);
      break;
   case LINESTATE_REGISTERING:
      str = MAKESTR(LINESTATE_REGISTERING);
      break;
   case LINESTATE_REGISTERED:
      str = MAKESTR(LINESTATE_REGISTERED);
      break;
   case LINESTATE_UNREGISTERING:
      str = MAKESTR(LINESTATE_UNREGISTERING);
      break;
   case LINESTATE_UNREGISTERED:
      str = MAKESTR(LINESTATE_UNREGISTERED);
      break;
   case LINESTATE_REGISTER_FAILED:
      str = MAKESTR(LINESTATE_REGISTER_FAILED);
      break;
   case LINESTATE_UNREGISTER_FAILED:
      str = MAKESTR(LINESTATE_UNREGISTER_FAILED);
      break;
   case LINESTATE_PROVISIONED:
      str = MAKESTR(LINESTATE_PROVISIONED);
      break;
   default:
      break;
   }

   return str;
}


static const char* convertLinestateCauseToString(SIPX_LINESTATE_CAUSE cause)
{
   const char* str = "Unknown";

   switch (cause)
   {        
   case LINESTATE_CAUSE_UNKNOWN:
      str = MAKESTR(CAUSE_UNKNOWN);
      break;
   case LINESTATE_REGISTERING_NORMAL:
      str = MAKESTR(LINESTATE_REGISTERING_NORMAL);
      break;
   case LINESTATE_REGISTERED_NORMAL:
      str = MAKESTR(LINESTATE_REGISTERED_NORMAL);
      break;
   case LINESTATE_UNREGISTERING_NORMAL:
      str = MAKESTR(LINESTATE_UNREGISTERING_NORMAL);
      break;
   case LINESTATE_UNREGISTERED_NORMAL:
      str = MAKESTR(LINESTATE_UNREGISTERED_NORMAL);
      break;
   case LINESTATE_REGISTER_FAILED_COULD_NOT_CONNECT:
      str = MAKESTR(LINESTATE_REGISTER_FAILED_COULD_NOT_CONNECT);
      break;
   case LINESTATE_REGISTER_FAILED_NOT_AUTHORIZED:
      str = MAKESTR(LINESTATE_REGISTER_FAILED_NOT_AUTHORIZED);
      break;
   case LINESTATE_REGISTER_FAILED_TIMEOUT:
      str = MAKESTR(LINESTATE_REGISTER_FAILED_TIMEOUT);
      break;
   case LINESTATE_UNREGISTER_FAILED_COULD_NOT_CONNECT:
      str = MAKESTR(LINESTATE_UNREGISTER_FAILED_COULD_NOT_CONNECT);
      break;
   case LINESTATE_UNREGISTER_FAILED_NOT_AUTHORIZED:
      str = MAKESTR(LINESTATE_UNREGISTER_FAILED_NOT_AUTHORIZED);
      break;
   case LINESTATE_UNREGISTER_FAILED_TIMEOUT:
      str = MAKESTR(LINESTATE_UNREGISTER_FAILED_TIMEOUT);
      break;
   case LINESTATE_PROVISIONED_NORMAL:
      str = MAKESTR(LINESTATE_PROVISIONED_NORMAL);
      break;
   default:
      break;
   }
   return str;
}


SIPXTAPI_API const char* sipxSecurityEventToString(SIPX_SECURITY_EVENT event) 
{
   const char* str = "Unknown";

   switch (event)
   {
   case SECURITY_UNKNOWN:
      str = MAKESTR(SECURITY_UNKNOWN);
      break;
   case SECURITY_ENCRYPT:
      str = MAKESTR(SECURITY_ENCRYPT);
      break;
   case SECURITY_DECRYPT:
      str = MAKESTR(SECURITY_DECRYPT);
      break;
   case SECURITY_TLS:
      str = MAKESTR(SECURITY_TLS);
      break;
   default:
      break;
   }

   return str;
}


SIPXTAPI_API const char* sipxSecurityCauseToString(SIPX_SECURITY_CAUSE cause) 
{
   const char* str = "Unknown";

   switch (cause)
   {
   case SECURITY_CAUSE_UNKNOWN:
      str = MAKESTR(SECURITY_CAUSE_UNKNOWN);
      break;
   case SECURITY_CAUSE_NORMAL:
      str = MAKESTR(SECURITY_CAUSE_NORMAL);
      break;
   case SECURITY_CAUSE_ENCRYPT_SUCCESS:
      str = MAKESTR(SECURITY_CAUSE_ENCRYPT_SUCCESS);
      break;
   case SECURITY_CAUSE_ENCRYPT_FAILURE_LIB_INIT:
      str = MAKESTR(SECURITY_CAUSE_ENCRYPT_FAILURE_LIB_INIT);
      break;
   case SECURITY_CAUSE_ENCRYPT_FAILURE_BAD_PUBLIC_KEY:
      str = MAKESTR(SECURITY_CAUSE_ENCRYPT_FAILURE_BAD_PUBLIC_KEY);
      break;
   case SECURITY_CAUSE_ENCRYPT_FAILURE_INVALID_PARAMETER:
      str = MAKESTR(SECURITY_CAUSE_ENCRYPT_FAILURE_INVALID_PARAMETER);
      break;
   case SECURITY_CAUSE_DECRYPT_SUCCESS:
      str = MAKESTR(SECURITY_CAUSE_DECRYPT_SUCCESS);
      break;
   case SECURITY_CAUSE_DECRYPT_FAILURE_DB_INIT:
      str = MAKESTR(SECURITY_CAUSE_DECRYPT_FAILURE_DB_INIT);
      break;
   case SECURITY_CAUSE_DECRYPT_FAILURE_BAD_DB_PASSWORD:
      str = MAKESTR(SECURITY_CAUSE_DECRYPT_FAILURE_BAD_DB_PASSWORD);
      break;
   case SECURITY_CAUSE_DECRYPT_FAILURE_INVALID_PARAMETER:
      str = MAKESTR(SECURITY_CAUSE_DECRYPT_FAILURE_INVALID_PARAMETER);
      break;
   case SECURITY_CAUSE_DECRYPT_BAD_SIGNATURE:
      str = MAKESTR(SECURITY_CAUSE_DECRYPT_BAD_SIGNATURE);
      break;
   case SECURITY_CAUSE_DECRYPT_MISSING_SIGNATURE:
      str = MAKESTR(SECURITY_CAUSE_DECRYPT_MISSING_SIGNATURE);
      break;
   case SECURITY_CAUSE_DECRYPT_SIGNATURE_REJECTED:
      str = MAKESTR(SECURITY_CAUSE_DECRYPT_SIGNATURE_REJECTED);
      break;
   case SECURITY_CAUSE_TLS_SERVER_CERTIFICATE:
      str = MAKESTR(SECURITY_CAUSE_TLS_SERVER_CERTIFICATE);
      break;
   case SECURITY_CAUSE_TLS_BAD_PASSWORD:
      str = MAKESTR(SECURITY_CAUSE_TLS_BAD_PASSWORD);
      break;
   case SECURITY_CAUSE_TLS_LIBRARY_FAILURE:
      str = MAKESTR(SECURITY_CAUSE_TLS_LIBRARY_FAILURE);
      break;
   case SECURITY_CAUSE_REMOTE_HOST_UNREACHABLE:
      str = MAKESTR(SECURITY_CAUSE_REMOTE_HOST_UNREACHABLE);
      break;
   case SECURITY_CAUSE_TLS_CONNECTION_FAILURE:
      str = MAKESTR(SECURITY_CAUSE_TLS_CONNECTION_FAILURE);
      break;
   case SECURITY_CAUSE_TLS_HANDSHAKE_FAILURE:
      str = MAKESTR(SECURITY_CAUSE_TLS_HANDSHAKE_FAILURE);
      break;
   case SECURITY_CAUSE_SIGNATURE_NOTIFY:
      str = MAKESTR(SECURITY_CAUSE_SIGNATURE_NOTIFY);
      break;
   case SECURITY_CAUSE_TLS_CERTIFICATE_REJECTED:
      str = MAKESTR(SECURITY_CAUSE_TLS_CERTIFICATE_REJECTED);
      break;
   default:
      break;
   }

   return str;
}


SIPXTAPI_API void sipxCallEventToString(SIPX_CALLSTATE_EVENT event,
                                        SIPX_CALLSTATE_CAUSE cause,
                                        char*  szBuffer,
                                        size_t nBuffer)
{
   SNPRINTF(szBuffer, nBuffer, "%s::%s",  convertCallstateEventToString(event),
      convertCallstateCauseToString(cause));
}


SIPXTAPI_API void sipxLineEventToString(SIPX_LINESTATE_EVENT event,
                                        SIPX_LINESTATE_CAUSE cause,
                                        char*  szBuffer,
                                        size_t nBuffer)
{
   SNPRINTF(szBuffer, nBuffer, "%s::%s", convertLinestateEventToString(event),
      convertLinestateCauseToString(cause));
}


SIPXTAPI_API void sipxEventToString(const SIPX_EVENT_CATEGORY category,
                                    const void* pEvent,
                                    char*  szBuffer,
                                    size_t nBuffer)
{
   switch (category)
   {
   case EVENT_CATEGORY_CALLSTATE:
      {
         SIPX_CALLSTATE_INFO* pCallEvent = (SIPX_CALLSTATE_INFO*)pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s::%s", 
            convertEventCategoryToString(category),
            convertCallstateEventToString(pCallEvent->event), 
            convertCallstateCauseToString(pCallEvent->cause));
      }
      break;
   case EVENT_CATEGORY_LINESTATE:
      {
         SIPX_LINESTATE_INFO* pLineEvent = (SIPX_LINESTATE_INFO*)pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s::%s", 
            convertEventCategoryToString(category),
            convertLinestateEventToString(pLineEvent->event), 
            convertLinestateCauseToString(pLineEvent->cause));
      }
      break;
   case EVENT_CATEGORY_INFO_STATUS:
      {
         SIPX_INFOSTATUS_INFO* pInfoEvent = (SIPX_INFOSTATUS_INFO*)pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s::%s", 
            convertEventCategoryToString(category),
            convertInfoStatusEventToString(pInfoEvent->event),
            convertMessageStatusToString(pInfoEvent->status));

      }
      break;
   case EVENT_CATEGORY_INFO:
      {
         SNPRINTF(szBuffer, nBuffer, "%s", 
            convertEventCategoryToString(category));
      }
      break;
   case EVENT_CATEGORY_SUB_STATUS:
      {
         SIPX_SUBSTATUS_INFO* pStatusInfo = (SIPX_SUBSTATUS_INFO*) pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s::%s", 
            convertEventCategoryToString(category),
            sipxSubStatusStateToString(pStatusInfo->state),
            sipxSubStatusCauseToString(pStatusInfo->cause));
      }
      break;
   case EVENT_CATEGORY_NOTIFY:
      {
         SNPRINTF(szBuffer, nBuffer, "%s", 
            convertEventCategoryToString(category));
      }
      break;
   case EVENT_CATEGORY_CONFIG:
      {
         SIPX_CONFIG_INFO* pConfigEvent = (SIPX_CONFIG_INFO*)pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s", 
            convertEventCategoryToString(category),
            sipxConfigEventToString(pConfigEvent->event));
      }
      break;
   case EVENT_CATEGORY_SECURITY:
      {
         SIPX_SECURITY_INFO* pEventData = (SIPX_SECURITY_INFO*)pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s::%s", 
            convertEventCategoryToString(category),
            sipxSecurityEventToString(pEventData->event),
            sipxSecurityCauseToString(pEventData->cause));
      }
      break;
   case EVENT_CATEGORY_MEDIA:
      {
         SIPX_MEDIA_INFO* pEventData = (SIPX_MEDIA_INFO*)pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s::%s::%s", 
            convertEventCategoryToString(category),
            sipxMediaEventToString(pEventData->event),
            convertMediaTypeToString(pEventData->mediaType),
            sipxMediaCauseToString(pEventData->cause));
      }
      break;
   case EVENT_CATEGORY_PIM:
      {
         SIPX_PIM_INFO* pEventData = (SIPX_PIM_INFO*)pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s", 
            convertEventCategoryToString(category),
            convertPIMEventToString(pEventData->event));
      }
      break;
   case EVENT_CATEGORY_KEEPALIVE:
      {
         SIPX_KEEPALIVE_INFO* pEventData = (SIPX_KEEPALIVE_INFO*)pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s::%s::%s", 
            convertEventCategoryToString(category),
            sipxKeepaliveEventToString(pEventData->event),
            convertKeepaliveTypeToString(pEventData->type),
            sipxKeepaliveCauseToString(pEventData->cause));
      }
      break;
   case EVENT_CATEGORY_RTP_REDIRECT:
      {
         SIPX_RTP_REDIRECT_INFO* pEventData = (SIPX_RTP_REDIRECT_INFO*)pEvent;
         SNPRINTF(szBuffer, nBuffer, "%s::%s::%s", 
            convertEventCategoryToString(category),
            sipxRtpRedirectEventToString(pEventData->event),
            sipxRtpRedirectCauseToString(pEventData->cause));
      }
      break;
   case EVENT_CATEGORY_CONFERENCE:
	   {
		   SIPX_CONFERENCE_INFO* pEventData = (SIPX_CONFERENCE_INFO*)pEvent;
		   SNPRINTF(szBuffer, nBuffer, "%s::%s::%s", 
			   convertEventCategoryToString(category),
			   sipxConferenceEventToString(pEventData->event),
			   sipxConferenceCauseToString(pEventData->cause));
	   }
	   break;
   default:
      assert(false);
      break;
   }
}

/*********************************************************************/
/*       Event duplication & freeing                                 */
/*********************************************************************/


SIPXTAPI_API SIPX_RESULT sipxDuplicateEvent(SIPX_EVENT_CATEGORY category, 
                                            const void*         pEventSource, 
                                            void**              pEventCopy) 
{
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

   assert(VALID_SIPX_EVENT_CATEGORY(category));
   assert(pEventSource);
   assert(pEventCopy);

   if (VALID_SIPX_EVENT_CATEGORY(category) && pEventSource && pEventCopy)
   {
      switch (category)
      {
      case EVENT_CATEGORY_CALLSTATE:
         {
            SIPX_CALLSTATE_INFO* pSourceInfo = (SIPX_CALLSTATE_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_CALLSTATE_INFO));

            SIPX_CALLSTATE_INFO* pInfo = new SIPX_CALLSTATE_INFO();
            memset(pInfo, 0, sizeof(SIPX_CALLSTATE_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->hCall = pSourceInfo->hCall;
            pInfo->hLine = pSourceInfo->hLine;
            pInfo->event = pSourceInfo->event;
            pInfo->cause = pSourceInfo->cause;
            pInfo->hAssociatedCall = pSourceInfo->hAssociatedCall;
            pInfo->sipResponseCode = pSourceInfo->sipResponseCode;
            pInfo->szSipResponseText = SAFE_STRDUP(pSourceInfo->szSipResponseText);
            pInfo->szReferredBy = SAFE_STRDUP(pSourceInfo->szReferredBy);
            pInfo->szReferTo = SAFE_STRDUP(pSourceInfo->szReferTo);

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_LINESTATE:
         {
            SIPX_LINESTATE_INFO* pSourceInfo = (SIPX_LINESTATE_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_LINESTATE_INFO));

            SIPX_LINESTATE_INFO* pInfo = new SIPX_LINESTATE_INFO();
            memset(pInfo, 0, sizeof(SIPX_LINESTATE_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->hLine = pSourceInfo->hLine;
            pInfo->szLineUri = SAFE_STRDUP(pSourceInfo->szLineUri);
            pInfo->event = pSourceInfo->event;
            pInfo->cause = pSourceInfo->cause;
            pInfo->sipResponseCode = pSourceInfo->sipResponseCode;
            pInfo->szSipResponseText = SAFE_STRDUP(pSourceInfo->szSipResponseText);

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_INFO_STATUS:
         {
            SIPX_INFOSTATUS_INFO* pSourceInfo = (SIPX_INFOSTATUS_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_INFOSTATUS_INFO));

            SIPX_INFOSTATUS_INFO* pInfo = new SIPX_INFOSTATUS_INFO();
            memset(pInfo, 0, sizeof(SIPX_INFOSTATUS_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->hCall = pSourceInfo->hCall;
            pInfo->hLine = pSourceInfo->hLine;
            pInfo->status = pSourceInfo->status;
            pInfo->responseCode = pSourceInfo->responseCode;
            pInfo->szResponseText = SAFE_STRDUP(pSourceInfo->szResponseText);
            pInfo->event = pSourceInfo->event;
            pInfo->pCookie = pSourceInfo->pCookie;

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_INFO:
         {
            SIPX_INFO_INFO* pSourceInfo = (SIPX_INFO_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_INFO_INFO));

            SIPX_INFO_INFO* pInfo = new SIPX_INFO_INFO();
            memset(pInfo, 0, sizeof(SIPX_INFO_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->hCall = pSourceInfo->hCall;
            pInfo->hLine = pSourceInfo->hLine;
            pInfo->szContentType = SAFE_STRDUP(pSourceInfo->szContentType);

            if (pSourceInfo->nContentLength > 0 && pSourceInfo->pContent)
            {
               pInfo->pContent = (char*)malloc(pSourceInfo->nContentLength + 1);
               ((char*)pInfo->pContent)[pSourceInfo->nContentLength] = 0; // terminate string
               assert(pInfo->pContent);
               memcpy((void*)pInfo->pContent, pSourceInfo->pContent, pSourceInfo->nContentLength);
               pInfo->nContentLength = pSourceInfo->nContentLength;
            }
            else
            {
               pInfo->pContent = NULL;
               pInfo->nContentLength = 0;
            }                    

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_SUB_STATUS:
         {
            SIPX_SUBSTATUS_INFO* pSourceInfo = (SIPX_SUBSTATUS_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_SUBSTATUS_INFO));

            SIPX_SUBSTATUS_INFO* pInfo = new SIPX_SUBSTATUS_INFO();
            memset(pInfo, 0, sizeof(SIPX_SUBSTATUS_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->hSub = pSourceInfo->hSub;
            pInfo->state = pSourceInfo->state;
            pInfo->cause = pSourceInfo->cause;
            pInfo->szSubServerUserAgent = SAFE_STRDUP(pSourceInfo->szSubServerUserAgent);

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_NOTIFY:
         {
            SIPX_NOTIFY_INFO* pSourceInfo = (SIPX_NOTIFY_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_NOTIFY_INFO));

            SIPX_NOTIFY_INFO* pInfo = new SIPX_NOTIFY_INFO();
            memset(pInfo, 0, sizeof(SIPX_NOTIFY_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->hSub = pSourceInfo->hSub;
            pInfo->szNotiferUserAgent = SAFE_STRDUP(pSourceInfo->szNotiferUserAgent);
            pInfo->szContentType = SAFE_STRDUP(pSourceInfo->szContentType);

            if (pSourceInfo->nContentLength > 0 && pSourceInfo->pContent)
            {
               pInfo->pContent = (char*)malloc(pSourceInfo->nContentLength);
               assert(pInfo->pContent);
               memcpy((void*) pInfo->pContent, (void*)pSourceInfo->pContent, pSourceInfo->nContentLength);
               pInfo->nContentLength = pSourceInfo->nContentLength;
            }
            else
            {
               pInfo->pContent = NULL;
               pInfo->nContentLength = 0;
            }

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_CONFIG:
         {
            SIPX_CONFIG_INFO* pSourceInfo = (SIPX_CONFIG_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_CONFIG_INFO));

            SIPX_CONFIG_INFO* pInfo = new SIPX_CONFIG_INFO();
            memset(pInfo, 0, sizeof(SIPX_CONFIG_INFO));

            pInfo->nSize = sizeof(SIPX_CONFIG_INFO);
            pInfo->event = pSourceInfo->event;

            if (pSourceInfo->pData)
            {
               if (pSourceInfo->event == CONFIG_STUN_SUCCESS)
               {
                  pInfo->pData = new SIPX_CONTACT_ADDRESS(*((SIPX_CONTACT_ADDRESS*)pSourceInfo->pData));
               }
               else if (pSourceInfo->event == CONFIG_STUN_FAILURE)
               {
                  const SIPX_STUN_FAILURE_INFO* pSourceStunFailureInfo = (const SIPX_STUN_FAILURE_INFO*)pSourceInfo->pData;
                  assert(pSourceStunFailureInfo->nSize == sizeof(SIPX_STUN_FAILURE_INFO));
                  // create copy
                  SIPX_STUN_FAILURE_INFO* pStunFailureInfo = new SIPX_STUN_FAILURE_INFO();
                  pStunFailureInfo->nSize = sizeof(SIPX_STUN_FAILURE_INFO);
                  pStunFailureInfo->szAdapterName = SAFE_STRDUP(pSourceStunFailureInfo->szAdapterName);
                  pStunFailureInfo->szInterfaceIp = SAFE_STRDUP(pSourceStunFailureInfo->szInterfaceIp);
                  pInfo->pData = pStunFailureInfo;
               }
            }

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_SECURITY:
         {
            SIPX_SECURITY_INFO* pSourceInfo = (SIPX_SECURITY_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_SECURITY_INFO));

            SIPX_SECURITY_INFO* pInfo = new SIPX_SECURITY_INFO();
            memset(pInfo, 0, sizeof(SIPX_SECURITY_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->szSRTPkey = SAFE_STRDUP(pSourceInfo->szSRTPkey);

            if (pSourceInfo->nCertificateSize > 0 && pSourceInfo->pCertificate)
            {
               pInfo->pCertificate = malloc(pSourceInfo->nCertificateSize);
               assert(pInfo->pCertificate);
               memcpy(pInfo->pCertificate, pSourceInfo->pCertificate, pSourceInfo->nCertificateSize);
               pInfo->nCertificateSize = pSourceInfo->nCertificateSize;
            }
            else
            {
               pInfo->pCertificate = NULL;
               pInfo->nCertificateSize = 0;
            }

            pInfo->event = pSourceInfo->event;
            pInfo->cause = pSourceInfo->cause;
            pInfo->szSubjAltName = SAFE_STRDUP(pSourceInfo->szSubjAltName);
            pInfo->callId = SAFE_STRDUP(pSourceInfo->callId);
            pInfo->hCall = pSourceInfo->hCall;
            pInfo->remoteAddress = SAFE_STRDUP(pSourceInfo->remoteAddress);

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_MEDIA:
         {
            SIPX_MEDIA_INFO* pSourceInfo = (SIPX_MEDIA_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_MEDIA_INFO));

            SIPX_MEDIA_INFO* pInfo = new SIPX_MEDIA_INFO();
            memset(pInfo, 0, sizeof(SIPX_MEDIA_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->event = pSourceInfo->event;
            pInfo->cause = pSourceInfo->cause;
            pInfo->mediaType = pSourceInfo->mediaType;
            pInfo->hCall = pSourceInfo->hCall;
            pInfo->codec = pSourceInfo->codec;
            pInfo->idleTime = pSourceInfo->idleTime;
            pInfo->toneId = pSourceInfo->toneId;
            pInfo->pCookie = pSourceInfo->pCookie;
            pInfo->playBufferIndex = pSourceInfo->playBufferIndex;

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_PIM:
         {
            SIPX_PIM_INFO* pSourceInfo = (SIPX_PIM_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_PIM_INFO));

            SIPX_PIM_INFO* pInfo = new SIPX_PIM_INFO();
            memset(pInfo, 0, sizeof(SIPX_PIM_INFO));

            pInfo->nSize = pSourceInfo->nSize;                    
            pInfo->event = pSourceInfo->event;
            pInfo->fromAddress = SAFE_STRDUP(pSourceInfo->fromAddress);

            if (pSourceInfo->textLength > 0 && pSourceInfo->textMessage)
            {
               pInfo->textMessage = (char*)malloc(pSourceInfo->textLength);
               assert(pInfo->textMessage);
               memcpy((void*)pInfo->textMessage, (void*)pSourceInfo->textMessage, pSourceInfo->textLength);
               pInfo->textLength = pSourceInfo->textLength;
            }
            else
            {
               pInfo->textLength = 0;
               pInfo->textMessage = NULL;
            }

            pInfo->subject = SAFE_STRDUP(pSourceInfo->subject);

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_KEEPALIVE:
         {
            SIPX_KEEPALIVE_INFO* pSourceInfo = (SIPX_KEEPALIVE_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_KEEPALIVE_INFO));

            SIPX_KEEPALIVE_INFO* pInfo = new SIPX_KEEPALIVE_INFO();
            memset(pInfo, 0, sizeof(SIPX_KEEPALIVE_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->event = pSourceInfo->event;
            pInfo->cause = pSourceInfo->cause;
            pInfo->type = pSourceInfo->type;
            pInfo->szRemoteAddress = SAFE_STRDUP(pSourceInfo->szRemoteAddress);
            pInfo->remotePort = pSourceInfo->remotePort;
            pInfo->keepAliveSecs = pSourceInfo->keepAliveSecs;
            pInfo->szFeedbackAddress = SAFE_STRDUP(pSourceInfo->szFeedbackAddress);
            pInfo->feedbackPort = pSourceInfo->feedbackPort;

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_RTP_REDIRECT:
         {
            SIPX_RTP_REDIRECT_INFO* pSourceInfo = (SIPX_RTP_REDIRECT_INFO*)pEventSource;
            assert(pSourceInfo->nSize == sizeof(SIPX_RTP_REDIRECT_INFO));

            SIPX_RTP_REDIRECT_INFO* pInfo = new SIPX_RTP_REDIRECT_INFO();
            memset(pInfo, 0, sizeof(SIPX_RTP_REDIRECT_INFO));

            pInfo->nSize = pSourceInfo->nSize;
            pInfo->event = pSourceInfo->event;
            pInfo->cause = pSourceInfo->cause;
            pInfo->hCall = pSourceInfo->hCall;
            pInfo->hLine = pSourceInfo->hLine;

            *pEventCopy = pInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
	  case EVENT_CATEGORY_CONFERENCE:
		  {
			  SIPX_CONFERENCE_INFO* pSourceInfo = (SIPX_CONFERENCE_INFO*)pEventSource;
			  assert(pSourceInfo->nSize == sizeof(SIPX_CONFERENCE_INFO));

			  SIPX_CONFERENCE_INFO* pInfo = new SIPX_CONFERENCE_INFO();
			  memset(pInfo, 0, sizeof(SIPX_CONFERENCE_INFO));

			  pInfo->nSize = pSourceInfo->nSize;
			  pInfo->event = pSourceInfo->event;
			  pInfo->cause = pSourceInfo->cause;
			  pInfo->hConf = pSourceInfo->hConf;
			  pInfo->hCall = pSourceInfo->hCall;

			  *pEventCopy = pInfo;

			  rc = SIPX_RESULT_SUCCESS;
		  }
		  break;
      default:
         *pEventCopy = NULL;
         break;
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxFreeDuplicatedEvent(SIPX_EVENT_CATEGORY category, 
                                                 void*               pEventCopy) 
{
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

   assert(VALID_SIPX_EVENT_CATEGORY(category));
   assert(pEventCopy);

   if (VALID_SIPX_EVENT_CATEGORY(category) && pEventCopy)
   {
      switch (category)
      {
      case EVENT_CATEGORY_CALLSTATE:
         {
            SIPX_CALLSTATE_INFO* pSourceInfo = (SIPX_CALLSTATE_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_CALLSTATE_INFO));
            free((void*)pSourceInfo->szSipResponseText);
            free((void*)pSourceInfo->szReferredBy);
            free((void*)pSourceInfo->szReferTo);
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_LINESTATE:
         {
            SIPX_LINESTATE_INFO* pSourceInfo = (SIPX_LINESTATE_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_LINESTATE_INFO));
            free((void*)pSourceInfo->szLineUri);
            free((void*)pSourceInfo->szSipResponseText);
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_INFO_STATUS:
         {
            SIPX_INFOSTATUS_INFO* pSourceInfo = (SIPX_INFOSTATUS_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_INFOSTATUS_INFO));
            free((void*)pSourceInfo->szResponseText);
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_INFO:
         {
            SIPX_INFO_INFO* pSourceInfo = (SIPX_INFO_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_INFO_INFO));
            free((void*)pSourceInfo->szContentType);
            free((void*)pSourceInfo->pContent);
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_SUB_STATUS:
         {
            SIPX_SUBSTATUS_INFO* pSourceInfo = (SIPX_SUBSTATUS_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_SUBSTATUS_INFO));
            free((void*)pSourceInfo->szSubServerUserAgent);
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_NOTIFY:
         {
            SIPX_NOTIFY_INFO* pSourceInfo = (SIPX_NOTIFY_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_NOTIFY_INFO));
            free((void*)pSourceInfo->szNotiferUserAgent);
            free((void*)pSourceInfo->szContentType);
            free((void*)pSourceInfo->pContent);
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_CONFIG:
         {
            SIPX_CONFIG_INFO* pSourceInfo = (SIPX_CONFIG_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_CONFIG_INFO));

            if (pSourceInfo->event == CONFIG_STUN_SUCCESS)
            {
               delete ((SIPX_CONTACT_ADDRESS*)pSourceInfo->pData);
            }
            else if (pSourceInfo->event == CONFIG_STUN_FAILURE)
            {
               SIPX_STUN_FAILURE_INFO* pStunFailureInfo = (SIPX_STUN_FAILURE_INFO*)pSourceInfo->pData;
               assert(pStunFailureInfo->nSize == sizeof(SIPX_STUN_FAILURE_INFO));
               free((void*)pStunFailureInfo->szAdapterName);
               free((void*)pStunFailureInfo->szInterfaceIp);
               delete pStunFailureInfo;
            }

            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_SECURITY:
         {
            SIPX_SECURITY_INFO* pSourceInfo = (SIPX_SECURITY_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_SECURITY_INFO));
            free((void*)pSourceInfo->szSRTPkey);
            free((void*)pSourceInfo->pCertificate);
            free((void*)pSourceInfo->szSubjAltName);
            free((void*)pSourceInfo->callId);
            free((void*)pSourceInfo->remoteAddress);
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_MEDIA:
         {
            SIPX_MEDIA_INFO* pSourceInfo = (SIPX_MEDIA_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_MEDIA_INFO));
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_PIM:
         {
            SIPX_PIM_INFO* pSourceInfo = (SIPX_PIM_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_PIM_INFO));
            free((void*)pSourceInfo->fromAddress);
            free((void*)pSourceInfo->textMessage);
            free((void*)pSourceInfo->subject);
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_KEEPALIVE:
         {
            SIPX_KEEPALIVE_INFO* pSourceInfo = (SIPX_KEEPALIVE_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_KEEPALIVE_INFO));
            free((void*) pSourceInfo->szRemoteAddress);
            free((void*) pSourceInfo->szFeedbackAddress);
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
      case EVENT_CATEGORY_RTP_REDIRECT:
         {
            SIPX_RTP_REDIRECT_INFO* pSourceInfo = (SIPX_RTP_REDIRECT_INFO*)pEventCopy;
            assert(pSourceInfo->nSize == sizeof(SIPX_RTP_REDIRECT_INFO));
            delete pSourceInfo;

            rc = SIPX_RESULT_SUCCESS;
         }
         break;
	  case EVENT_CATEGORY_CONFERENCE:
		  {
			  SIPX_CONFERENCE_INFO* pSourceInfo = (SIPX_CONFERENCE_INFO*)pEventCopy;
			  assert(pSourceInfo->nSize == sizeof(SIPX_CONFERENCE_INFO));
			  delete pSourceInfo;

			  rc = SIPX_RESULT_SUCCESS;
		  }
		  break;
      default:
         break;
      }
   }

   return rc;
}

/*********************************************************************/
/*       Event firing functions                                      */
/*********************************************************************/


// userData must be SIPX_INST
void sipxFirePIMEvent(void* userData,
                      const UtlString& fromAddress,
                      const char* textMessage,
                      int textLength,
                      const char* subject,
                      const SipMessage& messageRequest)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxFirePIMEvent");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxFirePIMEvent userData=%p fromAddress=%s textMessage=%s textLength=%i subject=%s",
      userData, fromAddress.data(), textMessage, textLength, subject);

   assert(userData);

   if (userData)
   {
      SIPX_PIM_INFO pimInfo;
      memset(&pimInfo, 0, sizeof(SIPX_PIM_INFO));

      pimInfo.nSize = sizeof(SIPX_PIM_INFO);
      pimInfo.event = PIM_INCOMING_MESSAGE;
      pimInfo.fromAddress = fromAddress.data();

      if (textLength > 0 && textMessage)
      {
         pimInfo.textMessage = (char*)malloc(textLength);
         assert(pimInfo.textMessage);
         memcpy((void*)pimInfo.textMessage, (void*)textMessage, textLength);
         pimInfo.textLength = textLength;
      }
      else
      {
         pimInfo.textLength = 0;
         pimInfo.textMessage = NULL;
      }
      pimInfo.subject = SAFE_STRDUP(subject);

      SipXEventDispatcher::dispatchEvent((SIPX_INST)userData, EVENT_CATEGORY_PIM, &pimInfo);

      // free memory after strdup
      free((void*)pimInfo.textMessage);
      free((void*)pimInfo.subject);
   }
}

void sipxFireConfigEvent(const SIPX_INST pInst,                                                        
                         SIPX_CONFIG_EVENT event,
                         const SIPX_CONTACT_ADDRESS* pContactAddress,
                         const SIPX_STUN_FAILURE_INFO* pStunFailureDetails)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxFireConfigEvent src=%p event=%s\n",
      pInst, 
      sipxConfigEventToString(event));

   SIPX_CONFIG_INFO eventInfo;
   memset(&eventInfo, 0, sizeof(SIPX_CONFIG_INFO));
   eventInfo.nSize = sizeof(SIPX_CONFIG_INFO);
   eventInfo.event = event;

   switch(event)
   {
   case CONFIG_STUN_SUCCESS:
      {
         if (pContactAddress)
         {
            SIPX_CONTACT_ADDRESS sipxContact(*pContactAddress);
            // Fire off an event for the STUN contact (normal)
            sipxContact.eContactType = CONTACT_NAT_MAPPED;
            eventInfo.pData = &sipxContact;
            SipXEventDispatcher::dispatchEvent(pInst, EVENT_CATEGORY_CONFIG, &eventInfo);
         }
         break;
      }
   case CONFIG_STUN_FAILURE:
      {
         eventInfo.pData = (void*)pStunFailureDetails;
         SipXEventDispatcher::dispatchEvent(pInst, EVENT_CATEGORY_CONFIG, &eventInfo);
         break;
      }
   }
}

bool sipxFireSubscriptionStatusEvent(const SIPX_INST pInst,
                                     SIPX_SUBSTATUS_INFO* pInfo)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxFireSubscriptionStatusEvent pInst=%p pInfo=%p",
      pInst, &pInfo);

   SipXEventDispatcher::dispatchEvent(pInst, EVENT_CATEGORY_SUB_STATUS, pInfo);

   return true;
}


bool sipxFireNotifyEvent(const SIPX_INST pInst,
                         SIPX_NOTIFY_INFO* pInfo)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxFireNotifyEvent pInst=%p pInfo=%p",
      pInst, &pInfo);

   SipXEventDispatcher::dispatchEvent(pInst, EVENT_CATEGORY_NOTIFY, pInfo);

   return true;
}

/*********************************************************************/
/*       Event listener management                                   */
/*********************************************************************/


SIPXTAPI_API SIPX_RESULT sipxEventListenerAdd(const SIPX_INST hInst,
                                              SIPX_EVENT_CALLBACK_PROC pCallbackProc,
                                              void* pUserData)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxEventListenerAdd hInst=%p pCallbackProc=%p pUserData=%p",
      hInst, pCallbackProc, pUserData);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   if (hInst && pCallbackProc)
   {
      UtlBoolean res = SipXEventDispatcher::addListener(hInst, pCallbackProc, pUserData);

      if (res)
      {
         rc = SIPX_RESULT_SUCCESS;
      }
      else
      {
         rc = SIPX_RESULT_FAILURE;
      }
   }

   return rc ;
}


SIPXTAPI_API SIPX_RESULT sipxEventListenerRemove(const SIPX_INST hInst, 
                                                 SIPX_EVENT_CALLBACK_PROC pCallbackProc, 
                                                 void* pUserData) 
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxEventListenerRemove hInst=%p pCallbackProc=%p pUserData=%p",
      hInst, pCallbackProc, pUserData);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   if (hInst && pCallbackProc)
   {
      UtlBoolean res = SipXEventDispatcher::removeListener(hInst, pCallbackProc, pUserData);

      if (res)
      {
         rc = SIPX_RESULT_SUCCESS;
      }
      else
      {
         rc = SIPX_RESULT_FAILURE;
      }            
   }
   return rc;
}
