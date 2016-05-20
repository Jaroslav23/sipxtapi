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

#ifndef XCpMessageTypes_h__
#define XCpMessageTypes_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

namespace CpMessageTypes
{
   typedef enum
   {
      AC_COMMAND = OsMsg::USER_START,///< abstract call command
      AC_NOTIFICATION,///< abstract call notification
      CM_COMMAND,///< call manager command
      CM_NOFITICATION, ///< call manager notification
      SC_COMMAND,///< sip connection command
      SC_NOFITICATION ///< sip connection notification
   } XCpMessageTypesEnum;
};

#endif // XCpMessageTypes_h__
