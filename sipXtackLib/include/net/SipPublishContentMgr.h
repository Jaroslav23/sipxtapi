//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#ifndef _SipPublishContentMgr_h_
#define _SipPublishContentMgr_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <os/OsMutex.h>
#include <utl/UtlDefs.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlContainableAtomic.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class HttpBody;
class UtlString;
class SipPublishContentMgrDefaultConstructor;

// TYPEDEFS

/** Class for managing body content to be accepted via PUBLISH or provided in NOTIFY requests
 *
 *  This class is a database that is used to store and retrieve
 *  content (i.e. SIP Event state bodies).  This class does not
 *  actually touch or process SIP messages.  It is used by other
 *  classes and applications to store and retrieve content related to
 *  SIP SUBSCRIBE, NOTIFY and PUBLISH requests.  The usual usage is to
 *  have one instance that maintains state for an unlimited number of
 *  resources and event types.
 *
 *  The resourceId and eventTypeKey have no semantics.  Syntactically,
 *  they are restricted only by:  (1) resourceId may not be the null
 *  string, and (2) eventTypeKey may not contain a byte with the value
 *  1 (control-A), so that the concatenation of resourceId and
 *  eventTypeKey can be split unambiguously.
 *
 *  It is up to the application or event package to decide what the 
 *  resourceId and eventTypeKey look like.  In addition, there is an
 *  eventType that may provide a coarser classification than
 *  eventTypeKey.  Callback functions are registered for eventTypes
 *  rather than eventTypeKeys, so the set of eventTypeKeys is not
 *  limited at compile time.
 *  
 *  A suggested convention for the resourceId
 *  is to use:  <userId>@<hostname>[:port] as provided in the
 *  SUBSCRIBE or PUBLISH request URI.  It is suggested that host be
 *  the domainname not the specific IP address.
 *
 *  It is also suggested the SIP event type token be used (without any
 *  event header parameters) as the eventTypeKey.  Only in special
 *  cases where the content varies based upon an event parameter,
 *  should the parameter(s) be include included in the eventTypeKey.
 *  Usually, eventType is the same as eventTypeKey, or is the
 *  SIP event type alone, if eventTypeKey contains parameters.
 *
 * \par Put Event State In
 *  Applications put Event state information for a specific resourceId
 *  and eventTypeKey into the SipPublishContentMgr via the publish method.
 *
 * \par Retrieve Event State
 *  Applications retrieve published content type via the getContent
 *  method.
 *
 * \par Remove Event State
 * All event state information for a resource Id and event type key can
 * can be removed via the unpublish method.  The content associated
 * with the keys is passed back so the application can destroy the
 * content.
 *
 * \par Default Event State
 * It is possible to define a default event state for an event type key.
 * This default content is provided in the getContent method if no
 * content was provided for the specific resource Id.  Default content
 * is set via the publishDefault method.
 */
class SipPublishContentMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /** Callback used to notify interested applications when content has changed
     *  Well behaved applications that register and implement this function
     *  should not block.  They should quickly return as failure to do so
     *  may hinder timely processing and system performance.  The memory
     *  for the content provided in the arguments should not be presumed to
     *  exist beyond the point where the application returns from this function.
     *
     *  /param applicationData - provided with the callback function pointer
     *  when it was registered.
     *  The rest of the arguments in this callback have the same meaning as 
     *  the getContent method.
     */
    typedef void (*SipPublisherContentChangeCallback) (void* applicationData,
                                                       const char* resourceId,
                                                       const char* eventTypeKey,
                                                       const char* eventType,
                                                       UtlBoolean isDefaultContent);

/* ============================ CREATORS ================================== */

    /// Default publish container constructor
    SipPublishContentMgr();


    /// Destructor
    virtual
    ~SipPublishContentMgr();


/* ============================ MANIPULATORS ============================== */

    /** Provide the default content for the given event type key
     *
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain 
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no semantics enforced.  This is an opaque string used 
     *         as part of the key.
     *  \param eventType - SIP event type token
     *  \param numContentTypes - the number of bodies (each having a unique
     *         content type) provided in the eventContent array.  Multiple
     *         content types are published if the server wants to deliver
     *         different content types based upon the SUBSCRIBE Accepts
     *         header content types listed.
     *  \param eventContent - the SIP Event state content which was provided
     *         via a PUBLISH or requested via a SUBSCRIBE to be delivered
     *         via a NOTIFY.  If multiple bodies are provided and the content 
     *         types match more than one of the mime types provided in the
     *         SUBSCRIBE Accepts header, the order of the bodies in the
     *         eventContent array indicates a preference.  The bodies are
     *         NOT copied, but their memory becomes owned by the
     *         SipPublishContentMgr object and will be deleted by it when
     *         they are no longer needed.
     */
    virtual void publishDefault(const char* eventTypeKey,
                                      const char* eventType,
                                      int numContentTypes,
                                HttpBody* eventContent[]);

    /** Add a default content constructor function.
     *
     *  \param *defaultConstructor becomes owned by the SipPublishContentMgr,
     *         which will delete it when it is no longer needed.
     */
    virtual void publishDefault(const char* eventTypeKey,
                                        const char* eventType,
                                SipPublishContentMgrDefaultConstructor*
                                defaultConstructor);

    /** Remove the default content and default content constructor for
     *  eventTypeKey.
     */
    virtual void unpublishDefault(const char* eventTypeKey,
                                  const char* eventType);

    /** Provide the given content for the resource and event type key
     *  An application provides content (i.e. SIP event state bodies)
     *  through this interface for the given resourceId and eventTypeKey.
     *  The resourceId and eventTypeKey together compose a unique key which
     * identifies the provided content.  The resourceId is optained from
     * the PUBLISH or SUBSCRIBE request URI.  The eventTypeKey is obtained
     * from the SIP Event header field.
     *  \param resourceId - a unique id for the resource, typically the
     *         identity or AOR for the event type content.  There is no
     *         semantics enforced.  This is an opaque string used as part
     *         of the key.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain 
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no semantics enforced.  This is an opaque string used 
     *         as part of the key.
     *  \param eventType - SIP event type token
     *  \param numContentTypes - the number of bodies (each having a unique
     *         content type) provided in the eventContent array.  Multiple
     *         content types are published if the server wants to deliver
     *         different content types based upon the SUBSCRIBE Accepts
     *         header content types listed.
     *  \param eventContent - the SIP Event state content which was provided
     *         via a PUBLISH or requested via a SUBSCRIBE to be delivered
     *         via a NOTIFY.  If multiple bodies are provided and the content 
     *         types match more than one of the mime types provided in the
     *         SUBSCRIBE Accepts header, the order of the bodies in the
     *         eventContent array indicates a preference.  The bodies are
     *         NOT copied, but their memory becomes owned by the
     *         SipPublishContentMgr object and will be deleted by it when
     *         they are no longer needed.
     *  \param noNotify - if TRUE, do not generate any NOTIFYs for this content
     *         change.  This should only be used in generateDefaultContent
     *         methods.
     */
    virtual void publish(const char* resourceId,
                               const char* eventTypeKey,
                               const char* eventType,
                               int numContentTypes,
                               HttpBody* eventContent[],
                         UtlBoolean noNotify = FALSE);

    /** Remove the content for the given resourceId and eventTypeKey
     *  The content bodies are given back so that the application can
     *  release or delete the bodies.
     *  \param resourceId - a unique id for the resource, typically the
     *         identity or AOR for the event type content.  There is no
     *         semantics enforced.  This is an opaque string used as part
     *         of the key.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain 
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no semantics enforced.  This is an opaque string used 
     *         as part of the key.
     *  \param eventType - SIP event type token
     */
    virtual void unpublish(const char* resourceId,
                                const char* eventTypeKey,
                           const char* eventType);

    /** Fetch the published content for a given resourceId/eventTypeKey.
     *  The content body pointers point to copies of the stored
     *  bodies, and the caller is responsible for deleting them.
     *  Returns TRUE unless maxContentTypes is too small to hold the
     *  published content versions.
     *  \param resourceId - a unique id for the resource, or NULL
     *         to retrieve the default content for the eventTypeKey.
     *  \param eventTypeKey - the unique id for the event type.
     *  \param eventType - SIP event type token
     *  \param maxContentTypes - the size of the provided
     *         eventContent array.
     *  \param numContentTypes - upon return, *numContentType is set
     *         to the number of bodies (each having a unique
     *         content type) set in the eventContent array.
     *  \param eventContent - the SIP Event state content.
     *  \param pDefaultConstructor - if not NULL, *pDefaultConstructor
     *         is set to point to a copy of the defaultConstructor for
     *         eventTypeKey (if one is set), or NULL.
     */
    virtual UtlBoolean getPublished(const char* resourceId,
                                    const char* eventTypeKey,
                                    int maxContentTypes,
                                    int& numContentTypes,
                                    HttpBody* eventContent[],
                                    SipPublishContentMgrDefaultConstructor**
                                    defaultConstructor);

    /** Get the content for the given resourceId, eventTypeKey and contentTypes
     *  Retrieves the content type identified by the resourceID and eventTypeKey.
     *  The given contentTypes indicates what content types are accepted (i.e.
     *  the mime types from the SUBSCRIBE Accept header).
     *  \param resourceId - a unique id for the resource, typically the
     *         identity or AOR for the event type content.  There is no
     *         semantics enforced.  This is an opaque string used as part
     *         of the key.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain 
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no semantics enforced.  This is an opaque string used 
     *         as part of the key.
     *  \param eventType - SIP event type token
     *  \param acceptHeaderValue - the mime types allowed to be returned in 
     *         the content argument.  The first match is the one returned.
     *         This string has the same syntax/format as the SIP Accept header.
     *  \param content - the content body if a match was found, otherwise NULL.
     *         The content body is a copy that must be freed.
     *  \param isDefaultContent - if there was no content specific to the resourceId
     *         and default content was provided for the given eventTypeKey,
     *         then isDefaultContent is set to TRUE and 'content' contains
     *         values from the eventTypeKey content default.
     */
    virtual UtlBoolean getContent(const char* resourceId,
                                  const char* eventTypeKey,
                                  const char* eventType,
                                  const char* acceptHeaderValue,
                                  HttpBody*& content,
                                  UtlBoolean& isDefaultContent);

    /** Set the callback which gets invoked whenever the content changes
     *  Currently only one observer is allowed per eventTypeKey.  If
     *  a subsequent observer is set for the same eventTypeKey, it replaces
     *  the existing one.  The arguments of the callback function have
     *  the same meaning as getContent.
     *  Note: the callback is invoked when the default content changes as well.
     *  When the default content for an eventTypeKey changes, the
     *  resourceId is NULL.  The application is responsible for knowing
     *  which resources do not have specific content (i.e. are observing
     *  the default content and may need to be notified).
     *  \param eventTypeKey - SIP event type key
     *  \param applicationData - application specific data that is to be
     *         passed back to the application in the callback function.
     *  Returns TRUE if the callback is set for the eventTypeKey.  Will
     *  not set the callback if it is already set for the given eventTypeKey
     */
    virtual UtlBoolean setContentChangeObserver(const char* eventType,
                                                void* applicationData,
                             SipPublisherContentChangeCallback callbackFunction);

    /** Remove the current observer for the eventTypeKey
     *  If the given callbackFunction does not match the existing one,
     *  this method returns FALSE and the existing observer(s) remain.
     */
    virtual UtlBoolean removeContentChangeObserver(const char* eventType,
                                                   void*& applicationData,
                            SipPublisherContentChangeCallback& callbackFunction);


/* ============================ ACCESSORS ================================= */

    /// Get some debug information
    void getStats(int& numDefaultContent,
                  int& numDefaultConstructor,
                    int& numResourceSpecificContent,
                    int& numCallbacksRegistered);

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    /// parse the accept header field and create a HashMap with a UtlString for each MIME type
    UtlBoolean buildContentTypesContainer(const char* acceptHeaderValue, 
                                          UtlHashMap& contentTypes);

    /// Copy constructor NOT ALLOWED
    SipPublishContentMgr(const SipPublishContentMgr& rSipPublishContentMgr);

    /// Assignment operator NOT ALLOWED
    SipPublishContentMgr& operator=(const SipPublishContentMgr& rhs);

    /// lock for single thread use
    void lock();

    /// unlock for use
    void unlock();

    OsMutex mPublishMgrMutex;
    // Indexed by strings "resourceId\001eventTypeKey".
    UtlHashMap mContentEntries; 
    // Indexed by strings "\001eventTypeKey".
    UtlHashMap mDefaultContentEntries;
    UtlHashMap mDefaultContentConstructors;
    // Indexed by strings "eventType".
    UtlHashMap mEventContentCallbacks;
};

/**
 * Helper class for SipPublishContentMgr.
 *
 * Each instance is a device for producing default content for a
 * resource/event-type when generateDefaultContent is set but there is no
 * content for the resource/event-type.
 *
 * SipPublicContentMgrDefaultConstructor is pure virtual.  Instances
 * can only be created of subclasses that provide a getContent()
 * method.
 */
class SipPublishContentMgrDefaultConstructor : public UtlContainableAtomic
{
  public:

   /** Generate the content for a resource and event.
    *  Called when getContent is called for a resourceId/eventTypeKey
    *  that has no published content.  generateDefaultContent may set
    *  content for that combination, or it can do nothing, which
    *  forces getContent to use the default content (if any) for that
    *  eventTypeKey.  If generateDefaultContent calls
    *  contentMgr->publish(), it must provide noNotify = TRUE, because
    *  the caller will generate NOTIFYs for this content.
    */
   void virtual generateDefaultContent(SipPublishContentMgr* contentMgr,
                                       const char* resourceId,
                                       const char* eventTypeKey,
                                       const char* eventType) = 0;

   /// Make a copy of this object according to its real type.
   virtual SipPublishContentMgrDefaultConstructor* copy() = 0;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipPublishContentMgr_h_
