//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef SipXCore_h__
#define SipXCore_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "tapi/sipXtapi.h"
#include "utl/UtlDefs.h"
#include "utl/UtlContainable.h"
#include "os/OsRWMutex.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsMutex.h"

// DEFINES

/** sipXtapi can be configured to expire after a certain date */
//add SIPXTAPI_EVAL_EXPIRATION to preprocessor definitions

#ifdef SIPXTAPI_EVAL_EXPIRATION
#  define EVAL_EXPIRE_MONTH     0       // zero based
#  define EVAL_EXPIRE_DAY       31
#  define EVAL_EXPIRE_YEAR      2008
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class Url;
class OsNotification;
class SipSubscribeServer;
class SipSubscribeClient;
class XCpCallManager;
class SipLineMgr;
class SdpCodec;
class SdpCodecList;
class SipUserAgent;
class SipRefreshMgr;
class SipDialogMgr;
class SipRefreshManager;
class SipXKeepaliveEventListener;
class SipXRtpRedirectEventListener;
class SipXConferenceEventListener;
class SipXEventDispatcher;
class SipPimClient;
class SipXMessageObserver;
class SipXLineEventListener;
class SipXCallEventListener;
class SipXInfoStatusEventListener;
class SipXInfoEventListener;
class SipXSecurityEventListener;
class SipXMediaEventListener;
class OsSharedServerTaskMgr;
class SipContact;

// STRUCTS
// TYPEDEFS

class AEC_SETTING
{
public:
   UtlBoolean bInitialized;/**< Is the data valid */
   SIPX_AEC_MODE mode;     /**< Is AEC enabled? */

   AEC_SETTING() : bInitialized(FALSE),
      mode(SIPX_AEC_DISABLED)
   {

   }
};

class AGC_SETTING
{
public:
   UtlBoolean bInitialized;     /**< Is the data valid */
   UtlBoolean bEnabled;         /**< Is AGC enabled? */

   AGC_SETTING() : bInitialized(FALSE),
      bEnabled(FALSE)
   {

   }
};

class NOISE_REDUCTION_SETTING
{
public:
   UtlBoolean bInitialized;           /**< Is the data valid */
   SIPX_NOISE_REDUCTION_MODE mode;    /**< Is NR enabled? */

   NOISE_REDUCTION_SETTING() : bInitialized(FALSE),
      mode(SIPX_NOISE_REDUCTION_DISABLED)
   {

   }
};

class SIPX_INSTANCE_DATA
{
public:
   size_t nSize;     /**< Size of the structure */
   SipUserAgent*    pSipUserAgent;
   SipPimClient*    pSipPimClient;
   // codec settings
   SdpCodecList* pSelectedCodecList; // shared with XCpCallManager, but not XCpAbstractCall. Only for new calls.
   SdpCodecList* pAvailableCodecList;
   UtlString selectedAudioCodecNames;
   UtlString selectedVideoCodecNames;
   SIPX_VIDEO_FORMAT videoFormat; // selected video format

   XCpCallManager*     pCallManager;
   SipLineMgr*      pLineManager;
   SipRefreshMgr*   pRefreshManager;
   SipSubscribeServer* pSubscribeServer;
   SipSubscribeClient* pSubscribeClient;
   SipRefreshManager* pSipRefreshManager;
   SipXLineEventListener* pLineEventListener;
   SipXCallEventListener* pCallEventListener;
   SipXInfoStatusEventListener* pInfoStatusEventListener;
   SipXInfoEventListener* pInfoEventListener;
   SipXSecurityEventListener* pSecurityEventListener;
   SipXMediaEventListener* pMediaEventListener;
   SipXKeepaliveEventListener* pKeepaliveEventListener;
   SipXRtpRedirectEventListener* pRtpRedirectEventListener;
   SipXConferenceEventListener* pConferenceEventListener;
   SipDialogMgr* pDialogManager;
   OsSharedServerTaskMgr* pSharedTaskMgr;

   char*            inputAudioDevices[MAX_AUDIO_DEVICES];
   int              nInputAudioDevices;
   char*            outputAudioDevices[MAX_AUDIO_DEVICES];
   int              nOutputAudioDevices;
   SipXMessageObserver* pMessageObserver;
   OsMutex         lock;
   int             nCalls;        /**< Counter for inprocess calls */
   int             nConferences;  /**< Counter for inprocess conferences */
   int             nLines;        /**< Counter for inprocess lines */
   UtlBoolean      bShortNames;   /**< short names in sip messages */
   UtlBoolean      bAllowHeader;  /**< use Allow header in sip messages */
   UtlBoolean      bSupportedHeader;  /**< use Supported header in sip messages */
   UtlBoolean      bDateHeader;   /**< use Date header in sip messages */
   char            szAcceptLanguage[16]; /**< accept language to use in sip messages>*/
   char            szLocationHeader[256]; /**< location header */
   UtlBoolean      bRtpOverTcp;   /**< allow RTP over TCP */

   SIPX_INSTANCE_DATA()
      : lock(OsMutex::Q_FIFO),
      nSize(sizeof(SIPX_INSTANCE_DATA)),
      pSipUserAgent(NULL),
      pSipPimClient(NULL),
      pSelectedCodecList(NULL),
      pAvailableCodecList(NULL),
      pCallManager(NULL),
      pLineManager(NULL),
      pRefreshManager(NULL),
      pSubscribeServer(NULL),
      pSubscribeClient(NULL),
      pSipRefreshManager(NULL),
      pLineEventListener(NULL),
      pCallEventListener(NULL),
      pInfoStatusEventListener(NULL),
      pInfoEventListener(NULL),
      pSecurityEventListener(NULL),
      pMediaEventListener(NULL),
      pKeepaliveEventListener(NULL),
      pConferenceEventListener(NULL),
      pDialogManager(NULL),
      pMessageObserver(NULL),
      nCalls(0),
      nConferences(0),
      nLines(0),
      bShortNames(FALSE),
      bAllowHeader(TRUE),
      bSupportedHeader(TRUE),
      bDateHeader(TRUE),
      bRtpOverTcp(FALSE),
      nInputAudioDevices(0),
      nOutputAudioDevices(0),
      pSharedTaskMgr(NULL)
   {
      // Clear devices
      for (int i = 0; i < MAX_AUDIO_DEVICES; i++)
      {
         inputAudioDevices[i] = NULL;
         outputAudioDevices[i] = NULL;
      }
      
      memset(szAcceptLanguage, 0, sizeof(szAcceptLanguage));
      memset(szLocationHeader, 0, sizeof(szLocationHeader));
   }

   ~SIPX_INSTANCE_DATA()
   {
      nSize = 0;
   }

   /**
    * Increments conference counter in thread safe manner.
    */
   void incrementConferenceCount()
   {
      lock.acquire();
      nConferences++;
      lock.release();
   }

   /**
    * Decrements conference counter in thread safe manner.
    */
   void decrementConferenceCount()
   {
      lock.acquire();
      nConferences--;
      assert(nConferences >= 0);
      lock.release();
   }

   /**
    * Increments call counter in thread safe manner.
    */
   void incrementCallCount()
   {
      lock.acquire();
      nCalls++;
      lock.release();
   }

   /**
    * Decrements call counter in thread safe manner.
    */
   void decrementCallCount()
   {
      lock.acquire();
      nCalls--;
      assert(nCalls >= 0);
      lock.release();
   }
};

typedef enum CONF_HOLD_STATE
{
   CONF_STATE_UNHELD = 0,
   CONF_STATE_BRIDGING_HOLD,
   CONF_STATE_NON_BRIDGING_HOLD
} CONF_HOLD_STATE;

typedef enum SIPX_LOCK_TYPE
{
   SIPX_LOCK_NONE,
   SIPX_LOCK_READ,
   SIPX_LOCK_WRITE
} SIPX_LOCK_TYPE;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Flush handles to remove peaks between test cases -- this *WILL* leak 
* memory.
*/
SIPXTAPI_API SIPX_RESULT sipxFlushHandles();

/**
* Look for leaks in internal handles
*/
SIPXTAPI_API SIPX_RESULT sipxCheckForHandleLeaks();

const char* sipxContactTypeToString(SIPX_CONTACT_TYPE type);

void sipxIncSessionCount();
void sipxDecSessionCount();
int sipxGetSessionCount();

/**
 * Converts SipContact to SIPX_CONTACT_ADDRESS.
 */
SIPX_CONTACT_ADDRESS getSipxContact(const SipContact& sipContact);

#endif // SipXCore_h__
