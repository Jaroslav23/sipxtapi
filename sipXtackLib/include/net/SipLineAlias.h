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

#ifndef SipLineAlias_h__
#define SipLineAlias_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <net/Url.h>
#include <utl/UtlCopyableContainable.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class SipLine;

/**
* Line alias enables correct routing of sip messages to lines with alternate
* identities. 
*
* We do not store the aliased SipLine internally, because it would bring too many
* problems with pointers. Instead we store only original line uri, which then can
* be used to look up the line.
*
* aliasUri is used for generating hash, and is needed for hashmap lookup.
*/
class SipLineAlias : public UtlCopyableContainable
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   /* ============================ CREATORS ================================== */

   /**
   * Constructor. Creates new alias with given aliasUri for given sipLine. AliasUri
   * is converted into proper lineUri used internally in the sipxtacklib, therefore
   * caller should use getAliasUri to retrieve the real alias uri used.
   */
   SipLineAlias(const Url& aliasUri, const SipLine& sipLine);

   /** Constructor. */
   SipLineAlias(const Url& aliasUri, const Url& sipLineUri);

   /** Destructor */
   virtual ~SipLineAlias();

   /** Copy constructor */
   SipLineAlias(const SipLineAlias& rSipLineAlias);

   /** Assignment operator */
   SipLineAlias& operator=(const SipLineAlias& rSipLineAlias);

   /* ============================ MANIPULATORS ============================== */

   /**
   * Get the ContainableType for a UtlContainable-derived class.
   */
   virtual UtlContainableType getContainableType() const;

   /** Calculate a hash code for this object. */
   virtual unsigned hash() const;

   /** Compare this object to another object. */
   virtual int compareTo(UtlContainable const *compareContainable) const;

   /** Creates a copy of object */
   virtual UtlCopyableContainable* clone() const;

   /* ============================ ACCESSORS ================================= */

   /** Gets sip uri of the alias */
   Url getAliasUri() const;

   /** Gets sip uri of the original line */
   Url getOriginalLineUri() const;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   Url m_aliasUri; ///< alias of line, used for generating hash
   Url m_originalLineUri; ///< uri of original line, can be used to lookup SipLine
};

#endif // SipLineAlias_h__
