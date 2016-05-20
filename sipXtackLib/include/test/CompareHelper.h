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

#ifndef CompareHelper_h__
#define CompareHelper_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <utl/UtlString.h>
#include <net/Url.h>
#include <net/SipLine.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

static UtlBoolean areTheSame(UtlString& string1, UtlString& string2)
{
   UtlBoolean result = FALSE;

   if (string1.compareTo(string2, UtlString::matchCase) == 0)
   {
      // string match
      result = TRUE;
   }

   return result;
}

static UtlBoolean areTheSame(Url& url1, Url& url2)
{
   return areTheSame(url1.toString(), url2.toString());
}

static UtlBoolean areTheSame(int num1, int num2)
{
   return num1 == num2;
}

static UtlBoolean areTheSame(size_t num1, size_t num2)
{
   return num1 == num2;
}

static UtlBoolean areTheSame(const char* str1, const char* str2)
{
   if (str1 && str2)
   {
      return strcmp(str1, str2) == 0;
   }

   return (!str1 && !str2);
}

static UtlBoolean areTheSame(SipLine& line1, SipLine& line2)
{
   UtlBoolean result = FALSE;

   if (areTheSame(line1.getLineUri(), line2.getLineUri()) &&
      areTheSame(line1.getNumOfCredentials(), line2.getNumOfCredentials()) &&
      areTheSame(line1.getPreferredContactUri(), line2.getPreferredContactUri()) &&
      areTheSame(line1.getState(), line2.getState()) &&
      areTheSame(line1.getFullLineUrl(), line2.getFullLineUrl()) &&
      areTheSame(line1.getUserId(), line2.getUserId())
      )
   {
      result = TRUE;
   }

   return result;
}

#endif // CompareHelper_h__
