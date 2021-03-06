//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _WNT_NTERRNO_H_
#define _WNT_NTERRNO_H_

#ifndef WINCE
#	define EMSGSIZE        97      /* The message is too long. */
#	define ECONNRESET      131     /* The connection was reset by the peer. */
#	define ETIMEDOUT       145     /* The connection has timed out. */
#	define ECONNREFUSED    146     /* The connection was refused. */
#endif

#endif		// _WNT_NTERRNO_H_
