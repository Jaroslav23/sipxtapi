//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SdpCodecList_h_
#define _SdpCodecList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlSList.h>
#include <os/OsBSem.h>
#include <os/OsMutex.h>
#include <sdp/SdpCodec.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
* Container for SdpCodec instances.
*/
class SdpCodecList
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */

   /** Default constructor */
   SdpCodecList(int numCodecs = 0, SdpCodec* codecArray[] = NULL);

   /** Constructor from list of SdpCodec instances */
   SdpCodecList(const UtlSList& sdpCodecList);

   /** Copy constructor */
   SdpCodecList(const SdpCodecList& rSdpCodecList);

   /** Assignment operator */
   SdpCodecList& operator=(const SdpCodecList& rhs);

   /** Destructor */
   ~SdpCodecList();

   /* ============================ MANIPULATORS ============================== */

   /** Append a new codec type to the list of known codecs */
   void addCodec(const SdpCodec& newCodec);

   /**
   * Append a new codec types to the list of known codecs. Copies of codec items
   * are made.
   *
   * @param sdpCodecList List of SdpCodec instances.
   */
   void addCodecs(const UtlSList& sdpCodecList);

   /** Append new codec types from supplied SdpCodecList into current list */
   void addCodecs(const SdpCodecList& sdpCodecList);

   /** Add copies of the array of codecs */
   void addCodecs(int numCodecs, SdpCodec* newCodecs[]);

   /**
   * Adds codecs from given list into codec factory. Codecs are identified by short string name separated
   * by space or comma.
   */
   void addCodecs(const UtlString &codecList);

   /** Assign any unset payload type ids */
   void bindPayloadIds();

   /** Discard all codecs */
   void clearCodecs(void);

   /* ============================ ACCESSORS ================================= */

   /** Get a codec given an internal codec id. Returns internal pointer! */
   const SdpCodec* getCodec(SdpCodec::SdpCodecTypes internalCodecId) const;

   /** Get a codec given an internal codec id. Returns TRUE if found. */
   UtlBoolean getCodec(SdpCodec::SdpCodecTypes internalCodecId, SdpCodec& sdpCodec) const;

   /** Get a codec with given mimeType and index. Returns TRUE if found. */
   UtlBoolean getCodecByIndex(const UtlString& mimeType, int index, SdpCodec& sdpCodec) const;

   /** Get a codec given the payload type id. Returns internal pointer!*/
   const SdpCodec* getCodecByPayloadId(int payloadTypeId) const;

   /** Get a codec given the mime type and subtype. Returns internal pointer! */
   const SdpCodec* getCodec(const char* mimeType, 
                            const char* mimeSubType,
                            int sampleRate,
                            int numChannels,
                            const UtlString& fmtp) const;

   /** Get the number of codecs */
   int getCodecCount() const;

   /** Get the number of codecs by mime type MIME_TYPE_AUDIO or MIME_TYPE_VIDEO */
   int getCodecCount(const UtlString& mimeType) const;

   /**
    * Returns TRUE if there is at least 1 non signalling codec of given mime type.
    */
   UtlBoolean hasNonSignallingCodec(const UtlString& mimeType) const;

   /** Gets list of SdpCodec instances */
   void getCodecs(UtlSList& sdpCodecList) const;

   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray) const;

   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray,
                  const char* mimeType) const;

   void getCodecs(int& numCodecs, 
                  SdpCodec**& codecArray,
                  const char* mimeType,
                  const char* subMimeType) const;

   /** String representation of factory and codecs */
   void toString(UtlString& str) const;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlSList m_codecsList;
   mutable OsMutex m_memberMutex;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCodecList_h_
