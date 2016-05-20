//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprRtpStartReceiveMsg_h_
#define _MprRtpStartReceiveMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
#include <utl/UtlSListIterator.h>
#include <sdp/SdpCodecList.h>
#include "mp/MpResourceMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Message object used to communicate with the media processing task
class MprRtpStartReceiveMsg : public MpResourceMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MprRtpStartReceiveMsg(const UtlString& targetResourceName,
                         const SdpCodecList& sdpCodecList,
                         OsSocket& rRtpSocket,
                         OsSocket& rRtcpSocket)
      : MpResourceMsg(MPRM_START_RECEIVE_RTP, targetResourceName)
      , mpRtpSocket(&rRtpSocket)
      , mpRtcpSocket(&rRtcpSocket)
      , mSdpCodecList(sdpCodecList)
   {
   };

   /// Copy constructor
   MprRtpStartReceiveMsg(const MprRtpStartReceiveMsg& resourceMsg)
      : MpResourceMsg(resourceMsg)
      , mpRtpSocket(resourceMsg.mpRtpSocket)
      , mpRtcpSocket(resourceMsg.mpRtcpSocket)
      , mSdpCodecList(resourceMsg.mSdpCodecList)
   {
   };

   /// Create a copy of this msg object (which may be of a derived type)
   OsMsg* createCopy(void) const 
   {
      return new MprRtpStartReceiveMsg(*this); 
   }

   /// Destructor
   ~MprRtpStartReceiveMsg() 
   {
   };

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /// Assignment operator
   MprRtpStartReceiveMsg& operator=(const MprRtpStartReceiveMsg& rhs)
   {
      if (this == &rhs) 
         return *this;  // handle the assignment to self case

      MpResourceMsg::operator=(rhs);  // assign fields for parent class
      mSdpCodecList = rhs.mSdpCodecList;
      mpRtpSocket = rhs.mpRtpSocket;
      mpRtcpSocket = rhs.mpRtcpSocket;

      return *this;
   }

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   void getCodecList(UtlSList& codecList)
   {
      mSdpCodecList.getCodecs(codecList);
   }

   OsSocket* getRtpSocket(){return(mpRtpSocket);};

   OsSocket* getRtcpSocket(){return(mpRtcpSocket);};

   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    OsSocket* mpRtpSocket;
    OsSocket* mpRtcpSocket;
    SdpCodecList mSdpCodecList;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprRtpStartReceiveMsg_h_
