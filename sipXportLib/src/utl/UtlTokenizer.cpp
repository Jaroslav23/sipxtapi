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

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <utl/UtlString.h>
#include <utl/UtlTokenizer.h>


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
#if defined(_VXWORKS)
extern "C" char* strdup(const char*);
#endif

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
UtlTokenizer::UtlTokenizer(const UtlString& tokens)
{
    m_tokens = (char *)strdup(tokens.data());
    m_tokenPosition = 0;
}

// Destructor
UtlTokenizer::~UtlTokenizer()
{
    free(m_tokens);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/**
 * Tokenize a string into a series of smaller strings based on a set
 * of single character delimiters. Delimitters are not returned and
 * can vary with each call to this method.
 *
 * Sample token strings and token counts where delimitters = "X"
 * 
 * |    string   | count |
 * +-------------+-------+
 *   ---X---X---    3
 *   X--X--         2
 *   ---            1
 *   ---X           1
 *   ---X           1
 *   ---XXX---      2
 *   XXXX           0
 */
UtlBoolean UtlTokenizer::next(UtlString &token, const char *delim)
{
    size_t len = strlen(m_tokens);
    UtlBoolean done = FALSE;

    token.remove(0) ;
    for (size_t i = m_tokenPosition; i < len && !done; i++)
    {
        // token starts at first non-delimiter
        if (!isDelim(m_tokens[i], delim))
        {
            int end = nextDelim(m_tokens, i, len, delim);
            token.append(m_tokens + i, end - i);
            m_tokenPosition = end;
            done = TRUE;
        }
    }

    return !token.isNull() ;
}

int UtlTokenizer::nextDelim(const char *tokens, const int start, const int len, const char *delim)
{
    int end = start;
    for (; end < len && !isDelim(tokens[end], delim); end++);
    
    return end;
}

UtlBoolean UtlTokenizer::isDelim(const char c, const char *delim)
{
    UtlBoolean match = FALSE;
    size_t delimLen = strlen(delim);
    for (size_t i = 0; i < delimLen && !match; i++)
    {
        match = (delim[i] == c);
    }

    return match;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Copy constructor
UtlTokenizer::UtlTokenizer(const UtlTokenizer& rUtlTokenizer)
{
}

// Assignment operator
UtlTokenizer&
UtlTokenizer::operator=(const UtlTokenizer& rhs)
{
   if (this != &rhs)
   {
      // ... handle assignment from another...
   }
   return *this;
}

/* ============================ FUNCTIONS ================================= */


