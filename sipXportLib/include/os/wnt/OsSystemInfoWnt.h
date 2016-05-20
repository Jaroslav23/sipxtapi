//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef OsSystemInfoWnt_h__
#define OsSystemInfoWnt_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
 * Helper class for getting various operating system information.
 */
class OsSystemInfoWnt
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum 
   {
      VERSION_ERROR = -1, ///< error occurred during check
      UNKNOWN_VERSION = 0, ///< we were unable to determine version, user should assume it has no more capability than win95
      WINDOWS_95,
      WINDOWS_NT4,
      WINDOWS_98,
      WINDOWS_ME,
      WINDOWS_2000,
      WINDOWS_XP,
      WINDOWS_SERVER_2003,
      WINDOWS_VISTA,
      FUTURE_VERSION ///< windows is newer than windows vista
   } OsWindowsVersion;

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   ~OsSystemInfoWnt(void);

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{
   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{
   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

      static OsWindowsVersion getOsVersion();

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsSystemInfoWnt(void);

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Copy constructor (not implemented for this task)
   OsSystemInfoWnt(const OsSystemInfoWnt& rOsSystemInfoWnt);

   /// Assignment operator (not implemented for this task)
   OsSystemInfoWnt& operator=(const OsSystemInfoWnt& rhs);
};


#endif // OsSystemInfoWnt_h__
