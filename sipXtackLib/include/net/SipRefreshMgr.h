//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef SIPREFRESHMGR_H
#define SIPREFRESHMGR_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "utl/UtlRandom.h"
#include "utl/UtlHashMap.h"
#include "net/SipMessage.h"
#include "os/OsMutex.h"
#include "net/SipTcpServer.h"
#include "net/SipUdpServer.h"
#include "net/SipMessageList.h"
#include "net/SipMessageEvent.h"
#include "net/SipCallIdGenerator.h"
#include "net/SipLine.h"


// DEFINES
#define DEFAULT_PERCENTAGE_TIMEOUT 48 //48%
#define FAILED_PERCENTAGE_TIMEOUT 24 //24%

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class SipLineMgr;
class SipLineStateEventListener;

/**
 * Class responsible for line registration.
 */
class SipRefreshMgr : public OsServerTask
{
public:
   /** Constructor. */
   SipRefreshMgr(SipLineStateEventListener *pListener = NULL);

   /** Destructor */
   virtual ~SipRefreshMgr();

   /** Sets SipUserAgent that should be used by SipRefreshMgr */
   void setSipUserAgent(SipUserAgent *pSipUserAgent);

   /** Starts the SipRefreshMgr thread */
   void startRefreshMgr();

   /** Sets default registration period in seconds */
   void setRegistryPeriod(int periodInSeconds);

   /** Creates and sends new register message for given line. */
   UtlBoolean newRegisterMsg(const Url& fromUrl,
                             const Url& contactUri,
                             UtlBoolean bAllowContactOverride,
                             SIP_TRANSPORT_TYPE preferredTransport,
                             int registryPeriodSeconds = -1);

   void reRegister(const Url& fromUrl);

   void unRegisterUser(const Url& fromUrl);
   void deleteUser(const Url& fromUrl);

   void setLineMgr(SipLineMgr* lineMgr);

   void dumpMessageLists(UtlString& results) ;
   //:Appends the message contents of both the mRegisterList and 
   // mSubscribeList

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);

protected:
   void rescheduleAfterTime(SipMessage* message, int percentage = DEFAULT_PERCENTAGE_TIMEOUT );

   OsStatus sendRequest(SipMessage& registerRequest, const char *method);

   void rescheduleRequest(SipMessage* registerRequest,
                          int secondsFromNow,
                          const char* method,
                          int percentage = DEFAULT_PERCENTAGE_TIMEOUT,
                          UtlBoolean sendImmediate = FALSE);

   void processOKResponse(SipMessage* registerResponse,
                          SipMessage* registerRequest);

   void parseContactFields(SipMessage* message,
                           SipMessage* sipRequest,
                           int& expireVal);

   void processResponse(const OsMsg& eventMessage,
                        SipMessage* registerRequest);

   void createTagNameValuePair(UtlString& tagNamevaluePair);
   UtlString createTagValue();

   // register
   void registerUrl(const Url& fromUrl,
                    const Url& toUrl,
                    const Url& requestUri,
                    const UtlString& contactUrl,
                    UtlBoolean bAllowContactOverride,
                    SIP_TRANSPORT_TYPE preferredTransport,
                    const UtlString& callId,
                    int registerPeriod = -1);

   UtlBoolean isDuplicateRegister(const Url& url,
                                  SipMessage& oldMessage);

   UtlBoolean isDuplicateRegister(const Url& url);

   void addToRegisterList(SipMessage* message);

   UtlBoolean removeFromRegisterList(SipMessage* message);
   //: Returns TRUE if message was found and removed from list.
   //: Message is NOT deleted.  

   UtlString buildContactField(const Url& registerToField,
                               const UtlString& lineId = "",
                               const Url* pPreferredContactUri = NULL);

   void removeAllFromRequestList(SipMessage* response);
   //: Removes all prior request records for this response
   //: from the SipMessageLists (mRegisterList & mSubscribeList)

   void removeAllFromRequestList(SipMessage* response, SipMessageList* pRequestList);
   //: Removes all prior request records for this response
   //: from the passed-in SipMessageList

   UtlBoolean isExpiresZero(SipMessage* pRequest) ;
   //: Is the expires field set to zero for the specified msg?

   SipLineMgr* m_pLineMgr;
   // register
   int m_defaultRegistryPeriod;
   SipMessageList m_registerList;
   mutable OsMutex m_mutex; ///< mutex for concurrent access

   // events
   SipLineStateEventListener* m_pLineListener;

   // common
   SipCallIdGenerator m_callIdGenerator;
   SipUserAgent* m_pSipUserAgent;
   UtlRandom m_randomNumGenerator;
   UtlHashBag m_timerBag;
};

#endif // SIPREFRESHMGR_H
