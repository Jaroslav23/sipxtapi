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

#ifndef SipLineList_h__
#define SipLineList_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlHashMap.h>
#include <net/SipLine.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * List of SipLine instances. Only one SipLine with given lineURI may be present
 * at time. Some of methods allow to retrieve pointer to internal SipLine object.
 * It must be ensured externally that it doesn't get deleted during manipulation
 * by usage of a mutex.
 * Additionally to SipLine instances, we also keep SipLineAlias instances here
 * in a separate hashmap. We do it because we want line lookups to look transparent
 * to caller, in regard to line aliases.
 *
 * Not thread safe.
 */
class SipLineList
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   SipLineList();

   /**
    * Destructor.
    */
   virtual ~SipLineList();

   /* ============================ MANIPULATORS ============================== */

   /** Adds a new line to the list. Returns FALSE if it was not added. */
   UtlBoolean add(SipLine* pLine);

   /** Adds a copy of line to the list. Returns FALSE if it was not added. */
   UtlBoolean add(const SipLine& line);

   /** Adds an alias for given line uri. */
   UtlBoolean addAlias(const Url& aliasUri, const Url& lineUri);

   /** Removes line and its aliases with the same lineIdentityUri from line list and deletes it */
   UtlBoolean remove(const SipLine& line);

   /** Removes line and its aliases with the same lineIdentityUri from line list and deletes it */
   UtlBoolean remove(const Url& aliasUri);

   /** Removes line alias */
   UtlBoolean removeAlias(const Url& aliasUri);

   /** Removes all lines and aliases from list and deletes them */
   void removeAll();

   /** Prints line list into log file */
   void dumpLines();

   /** Prints line alias list into log file */
   void dumpLineAliases();

   /**
    * Tries to find line according to given parameters. First hash lookup by lineUri.
    * If not found by identityUri, try slow scan by userId. This method is slow if userId
    * is provided and lineUri doesn't match.
    */
   SipLine* findLine(const Url& lineUri,
                     const UtlString& userId,
                     UtlBoolean bConsiderAliases = TRUE) const;

   /* ============================ ACCESSORS ================================= */

   /** Copies line clones into supplied list */
   void getLineCopies(UtlSList& lineList) const;

   /** Gets LineURIs of all SipLines */
   void getLineUris(UtlSList& lineList) const;

   /** Gets number of lines in list */
   size_t getLinesCount() const;

   /** Gets number of line aliases in list */
   size_t getLineAliasesCount() const;

   /**
    * Gets line from the list by lineUri. Also line aliases may be scanned.
    * This is fast lookup by hashcode, unlike findLine which is slower but more likely
    * to match.
    */
   SipLine* getLine(const Url& lineUri, UtlBoolean bConsiderAliases = TRUE) const;

   /**
    * Gets line from the list by userID. Multiple lines might match, but
    * only the first match is returned. UserId comparison is case sensitive.
    */
   SipLine* getLineByUserId(const UtlString& userId) const;

   /* ============================ INQUIRY =================================== */

   /**
   * Checks whether this line is present in line list. LineURI of passed object is used
   * for lookup. Also line aliases may be scanned.
   */
   UtlBoolean lineExists(const SipLine& line, UtlBoolean bConsiderAliases = TRUE) const;

   /** Checks whether this line is present in line list. Also line aliases may be scanned. */
   UtlBoolean lineExists(const Url& lineUri, UtlBoolean bConsiderAliases = TRUE) const;

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   void removeAliasesForLine(const Url& lineURI);

   mutable UtlHashMap m_lineMap; ///< map of SipLine instances
   mutable UtlHashMap m_lineAliasMap; ///< map of SipLineAlias instances. Key is alias URI, value is SipLineAlias
};

#endif // SipLineList_h__
