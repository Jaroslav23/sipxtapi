//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _Url_h_
#define _Url_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "utl/UtlRegex.h"
class UtlDList;

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class NameValuePair ;

/// URL parser and constructor
/**
 * This object is used to parse and construct URL strings.  This object
 * contains all of the parsed components of a URL.  It has the ability
 * to construct a serialized string of the object using the toString()
 * method.  It can also be used as a parser using the constructor 
 * which accepts a string as input.  This is intended to be a
 * generic URL parser for all schema/protocol types.  It is currently
 * tested and known to work for sip, sips, http, https, ftp, and file type
 * URLs. There are accessors for the various parts of the URL
 * These parts appear in URLs such as the following:
 * @code
 * "display name"<protocol:[//]user:password@host:port;urlparm=value?headerParam=value>;fieldParam=value
 * @endcode
 * The routines for the various parts are:
 * - display name
 *   - getDisplayName()
 *   - setDisplayName()
 * - protocol
 *   - getUrlType()
 *   - setUrlType()
 * - user
 *   - getUserId()
 *   - setUserId()
 * - password
 *   - getPassword()
 *   - setPassword()
 * - host
 *   - getHostAddress()
 *   - setHostAddress()
 * - port
 *   - getHostPort()
 *   - setHostPort()
 * - URL parameter names and values
 *   - getUrlParameter()
 *   - getUrlParameters()
 *   - setUrlParameter()
 *   - removeUrlParameter()
 *   - removeUrlParameters()
 * - header (or CGI) parameter names and values
 *   - getHeaderParameter()
 *   - getHeaderParameters()
 *   - setHeaderParameter()
 *   - removeHeaderParameter()
 *   - removeHeaderParameters()
 * - field parameter names and values
 *   - getFieldParameter()
 *   - getFieldParameters()
 *   - setFieldParameter()
 *   - removeFieldParameter()
 *   - removeFieldParameters()
 * 
 */

class Url
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   
   /// Identifiers for all supported URI schemes
   typedef enum
      {
         UnknownUrlScheme,  ///< invalid or unset scheme
         SipUrlScheme,      ///< sip:    (RFC 3261)
         SipsUrlScheme,     ///< sips:   (RFC 3261)
         HttpUrlScheme,     ///< http:   (RFC 2616)
         HttpsUrlScheme,    ///< https:  (RFC 2818)
         FtpUrlScheme,      ///< ftp:    (RFC 1738)
         FileUrlScheme,     ///< file:   (RFC 1738)
         MailtoUrlScheme,   ///< mailto: (RFC 2368)
         NUM_SUPPORTED_URL_SCHEMES
      }  Scheme;
   /**<
    * URI types are distinguished by the scheme names.
    * The scheme method translates names into these enumerations;
    * the schemeName method translates these into the canonical string for a URI.
    * The authoritative record of what schemes are defined is at:
    *   http://www.iana.org/assignments/uri-schemes
    * The general rules for URIs are in RFC 3986
    */

/* ============================ CREATORS ================================== */

   /// Default constructor from string
   Url(const char* urlString = NULL, ///< string to parse URL from
       UtlBoolean isAddrSpec = FALSE /**< - TRUE if this is an addrspec (a Request URI)
                                      *   - FALSE if this is a URL from a header field */
       );

    //! Copy constructor
    Url(const Url& rUrl);

    //! Destructor
    virtual
    ~Url();

/* ============================ MANIPULATORS ============================== */

     //! Assignment operator
    Url& operator=(const Url& rhs);

    //! Assignment from string
    /*! Parse the given null terminated string and set the
     * URL parts as found in the string.
     */
    Url& operator=(const char* urlString);

    /// set the value of this url by parsing the given string.
    void fromString(const UtlString& urlString,
                    UtlBoolean isAddrSpec = FALSE
                    );
    ///< all arguments are the same as for the constructor.

    //! Serialize this URL to a string in name-addr format, suitable for use
    //  as a field in a header.
    void toString(UtlString& urlString) const;

    //! Serialize this URL to a string in name-addr format, suitable for use
    //  as a field in a header.
    UtlString toString() const;

    //! Debug dump to STDOUT
    void dump();

    //! Clear the contents of this URL
    void reset();

    //! Remove all of the URL, header and field parameters and values
    void removeParameters();

/* ============================ ACCESSORS ================================= */

    /// Construct the cannonical identity.
    void getIdentity(UtlString& identity) const;
    /**<
     * In some applications this is used to compare if this
     * URL refers to the same destination.  The identity is:
     * @code
     * "user@host:port"
     * @endcode
     */

    /// Get the URL application layer protocol scheme string.
    void getUrlType(UtlString& urlProtocol) const;
    /**<
     * If you are going to make decisions based on the type,
     * it is more efficient to use getScheme to get the Scheme enumerated form
     * rather than comparing strings.
     */

    /// Set the URL application layer protocol using the scheme name string
    void setUrlType(const char* urlProtocol);
    ///< also see setScheme.

    /// Get the URL display name if present
    void getDisplayName(UtlString& displayName) const;

    /// Set the URL display name
    void setDisplayName(const char* displayName);

    /// Get the URL user identity if present
    void getUserId(UtlString& userId) const;
    UtlString getUserId() const;

    /// Set the URL user identity
    void setUserId(const char* userId);

    /// Get the users password if present in the URL
    UtlBoolean getPassword(UtlString& userId) const;

    /// Set the users password in the URL
    void setPassword(const char* userId);
    /**<
     * Putting a password in a URL is a
     * <strong>really really bad idea</strong>.
     * RFC 3261 says:
     *
     * While the SIP and SIPS URI syntax allows this field to be
     * present, its use is NOT RECOMMENDED, because the passing
     * of authentication information in clear text (such as URIs)
     * has proven to be a security risk in almost every case where
     * it has been used.
     */

    /// Get the URL host name or IP address
    void getHostAddress(UtlString& address) const;
    UtlString getHostAddress() const;

    /// Get the host and port together as a string "host:port"
    void getHostWithPort(UtlString& domain) const;
    
    /// Set the URL host name or IP address
    void setHostAddress(const char* address);

    /// Get the URL host port
    int getHostPort() const;

    /// Set the URL host port
    void setHostPort(int port);

    /// Get the file path from the URL
    UtlBoolean getPath(UtlString& path,
                       UtlBoolean getStyle = FALSE /**< TRUE will put header (or CGI) parameters
                                                    *   in path in the format needed for an HTTP
                                                    *   GET.  FALSE will form the path without
                                                    *   the header parameters as formated for a
                                                    *   HTTP POST. */

                       );


    /// Set the file path
    void setPath(const char* path);
    /**< @note the path should \a not contain header (or CGI) parameters @endnote */

    /// Get the named URL parameter value
    UtlBoolean getUrlParameter(const char* name,  ///< the parameter name to get
                               UtlString& value,  ///< the value of the named parameter
                               int index = 0 
                               );
    /**<
     * Gets the index occurrence of the named parameter (the same parameter name may
     * occur multiple times in a URL).
     * @return TRUE if the indicated parameter exists
     */

    /// Get the name and value of the URL parameter at the indicated index.
    UtlBoolean getUrlParameter(int urlIndex,    /**< the index indicting which URL parameter to 
                                                 *   get (starting at 0 for the first one). */
                               UtlString& name,  ///< the parameter name at urlIndex
                               UtlString& value  ///< the value of the parameter at urlIndex
                               );
    /**< @return TRUE if the indicated parameter exists. */

    /// Set the named URL parameter to the given value
    /*! Adds the parameter if it does not exist, sets the value if
     * it does exist.
     * \param name - the parameter name
     * \param value - the value of the parameter
     */
    void setUrlParameter(const char* name, const char* value);

    /// Removes all of the URL parameters
    void removeUrlParameters();

    /// Removes all of the URL parameters with the given name
    void removeUrlParameter(const char* name);

    /// Gets all of the URL parameters and values
    /*! \param iMaxReturn (in) - the maximum number of items to return
     * \param pNames (out) - Pointer to a preallocated array of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param pValues (out) - Pointer to a preallocated array of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the \a iActualReturn will contain the actual
     *        number of parameters.
     * \param iActualReturn (out) - The actual number of items returned
     * \returns TRUE if values are returned otherwise FALSE   
     */
    UtlBoolean getUrlParameters(int iMaxReturn, 
                               UtlString *pNames, 
                               UtlString *pValues, 
                               int& iActualReturn);

    /// Get the named header parameter value
    /*! \param name - the parameter name to get
     * \param value - the value of the named parameter
     * \param index - gets the \a index occurance of the named parameter 
     *        (the same parameter name may occur multiple times in the URL).
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getHeaderParameter(const char* name, 
                                 UtlString& value, 
                                 int index = 0);

    /// Get the name and value of the header parameter at the indicated index
    /*! \param headerIndex - the index indicting which header parameter to 
     *          get (starting at 0 for the first one).
     * \param name - the parameter name at headerIndex
     * \param value - the value of the parameter at headerIndex
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getHeaderParameter(int headerIndex, 
                                 UtlString& headerName, 
                                 UtlString& headerValue);

    /// Set the named header parameter to the given value
    void setHeaderParameter(const char* name,  ///< the parameter name
                            const char* value  ///< the value of the parameter
                            );
    /**<
     * For sip and sips URLs, this sets the header parameter to the value if the header name
     * is unique according to SipMessage::isUrlHeaderUnique, overwriting any previous value.
     * If the header is not unique, then this appends the header parameter.
     *
     * For all other URL schemes, the parameter is always appended.
     *
     * To ensure that any previous value is replaced, call removeHeaderParameter(name) first.
     */

    /// Removes all of the header parameters
    void removeHeaderParameters();

    /// Removes all of the header parameters with the given name
    void removeHeaderParameter(const char* name);

    ///Gets all of the Header parameters
    /*! \param iMaxReturn (in) - the maximum number of items to return
     * \param pNames (out) - Pointer to a preallocated collection of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param pValues (out) - Pointer to a preallocated collection of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param iActualReturn (out) - The actual number of items returned
     * \return TRUE if values are returned otherwise FALSE
     */
    UtlBoolean getHeaderParameters(int iMaxReturn, UtlString *pNames, UtlString *pValues, int& iActualReturn);


    /// Get the named field parameter value
    /*! \param name - the parameter name to get
     * \param value - the value of the named parameter
     * \param index - gets the \a index occurance of the named parameter 
     *        (the same parameter name may occur multiple times in the URL).
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getFieldParameter(const char* name, 
                                UtlString& value, 
                                int index = 0) const;

    /// Get the name and value of the field parameter at the indicated
    //! index
    /*! \param fieldIndex - the index indicting which field parameter to 
     *          get (starting at 0 for the first one).
     * \param name - the parameter name at fieldIndex
     * \param value - the value of the parameter at fieldIndex
     * \return TRUE if the indicated parameter exists
     */
    UtlBoolean getFieldParameter(int fieldIndex, 
                                UtlString& name, 
                                UtlString& value);

    /// Set the named field parameter to the given value
    /*! Adds the parameter if it does not exist, sets the value if
     * it does exist.
     * \param name - the parameter name
     * \param value - the value of the parameter
     */
    void setFieldParameter(const char* name, 
                           const char* value);

    /// Removes all of the field parameters
    void removeFieldParameters();

    /// Removes all of the field parameters with the given name
    void removeFieldParameter(const char* name);

    /// Gets all of the Header parameters
    /*! \param iMaxReturn (in) - the maximum number of items to return
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param pValues (out) - Pointer to a preallocated collection of 
     *        UtlStrings.  If a null is specified, the function will
     *        return false and the iActualReturn will contain the actual
     *        number of parameters.
     * \param iActualReturn (out) - The actual number of items returned
     * \return TRUE if values are returned otherwise FALSE
     */
    UtlBoolean getFieldParameters(int iMaxReturn, 
                                 UtlString *pNames, 
                                 UtlString *pValues, 
                                 int& iActualReturn);

    /// Forces the presence of angle brackets (i.e. <>) in the URL 
    //! when serialized
    void includeAngleBrackets();

    /// Remove the angle brackets (i.e. <>) from the URL
    void removeAngleBrackets();
    /**<
     * This does not really do anything - the toString function always puts
     * out a canonical form that does not include angle brackets if it is
     * possible to omit them.
     */

    /// Escape a string as a gen_value, which is what field-parameters use for values.
    static void gen_value_escape(UtlString& escapedText);

    /// Un-escape a string as a gen_value, which is what field-parameters use for values.
    static void gen_value_unescape(UtlString& escapedText);

    /**
     * Converts URL into URI (URIs cannot have display name, <>, field parameters)
     * URI is defined in RFC 2396
     *
     * Examples:
     * sip:user:password@host:port;urlparm=value?headerParam=value
     */
    void getUri(UtlString& Uri);

    /**
    * Converts URL into URI (URIs cannot have display name, <>, field parameters)
    * URI is defined in RFC 2396
    *
    * Examples:
    * sip:user:password@host:port;urlparm=value?headerParam=value
    */
    void getUri(Url& uri) const;

    /**
    * Converts URL into URI (URIs cannot have display name, <>, field parameters)
    * URI is defined in RFC 2396
    *
    * Examples:
    * sip:user:password@host:port;urlparm=value?headerParam=value
    */
    Url getUri() const;

/* ============================ INQUIRY =================================== */

    /// Is string all digits
    /*! \param dialedCharacters - null terminated string containing 
     *       ascii test
     * \ return TRUE if the dialedCharacters are all digits
     */
   static UtlBoolean isDigitString(const char* dialedCharacters);

   /// Compare two URLs to see if the have the same scheme, user, host and port
   UtlBoolean isSchemeUserHostPortEqual(const Url& uri,
                                        int impliedPort = PORT_NONE) const;

   /// Compare two URLs to see if the have the same user, host and port
   UtlBoolean isUserHostPortEqual(const Url& uri,
                                  int impliedPort = PORT_NONE 
                                  ) const ;
   /**<
    * Follows the rules of RFC 3261 section 19.1.4, especially that
    * that no port specifies is NOT the same as the default port for
    * the URL type/protocol (but see the description of impliedPort).
    *
    * Assumes that host is not case sensitive (because DNS names are not by definition),
    * but that user id is case sensitive.
    *
    * If the impliedPort is some value other than PORT_NONE, then that port number
    * is considered to be equal to an unspecified port number.  For example:
    * @code
    *   Url implicitPortUrl("sip:user@example.com");
    *   UtlBoolean result;
    *   
    *   Url explicitPortUrl("<sip:user@Example.COM:5060>;param=x");
    *   result = implicitPortUrl.isUserHostPortEqual(explicitPortUrl);
    *   // result is FALSE because an implicit port != 5060
    *
    *   Url unspecifiedPortUrl("<sip:user@Example.COM>;param=x");
    *   result = implicitPortUrl.isUserHostPortEqual(unspecifiedPortUrl);
    *   // result is TRUE, despite case difference in domain name
    *
    *   Url otherPortUrl("<sip:user@Example.COM:5999>;param=x");
    *   result = implicitPortUrl.isUserHostPortEqual(otherPortUrl, 5999);
    *   // result is TRUE because the specified implicit port is
    *   // considered to be that implied by the unspecified port in implicitPortUrl
    *
    *   result = implicitPortUrl.isUserHostPortEqual(otherPortUrl, 5060);
    *   // result is FALSE because the specified implicit port does not match
    * @endcode
    *
    * @return TRUE if the user Id, host and port are the same
    */

   /// Compare two URLs to see if the have the same scheme, user, host
   UtlBoolean isSchemeUserHostEqual(const Url& uri) const;

   /// Compare two URLs to see if the have the same user and host
   UtlBoolean isUserHostEqual(const Url& uri) const ;
   /**<
    * Assumes that host is not case sensitive, but that user id is case sensitive.
    * @return TRUE if the user Id and host are the same
    */   

   /// Compare two URLs to see if the have the same user
   UtlBoolean isUserEqual(const Url& url) const ;
   /**<
    * Assumes that user id is case sensitive.
    * @return TRUE if the user Id are the same
    */   

   /// Are angle brackets explicitly included
   UtlBoolean isIncludeAngleBracketsSet() const ;
   /**<
    * @note does not test if angle brackets are required or will be added
    * implicitly.
    * @returns TRUE if angle brackets were found when parsing or if they are
    *          explicitly set to be inserted during serialization.
    */

   /// Translate a scheme string (not including the terminating colon) to a Scheme enum.
   Scheme scheme( const UtlString& schemeName );

   /// Get the canonical (lowercase) name constant for a supported Scheme.
   const char* schemeName( Scheme scheme );

   /// Get the enumerator for the URL scheme type (more convenient than getUrlType).
   Scheme getScheme() const;

   /// Set the scheme to be used (also see setUrlType).
   void setScheme(Scheme scheme);
   
   /** Returns TRUE if this URL is empty */
   UtlBoolean isNull() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /// parse a URL in string form into its component parts
   void parseString(const char* urlString, ///< the raw URL string
                    UtlBoolean isAddrSpec = FALSE  /**< TRUE if this came from a Request URI or
                                                    *   other place where only the addr-spec production
                                                    *   is valid. */
                    );
   void initAngleBrackets(UtlBoolean isAddrSpec = FALSE);

   Scheme    mScheme;

   UtlString mDisplayName;
   UtlString mUserId;
   UtlString mPassword;
   UtlBoolean mPasswordSet;
   UtlString mHostAddress;
   int mHostPort;
   UtlString mPath;

   UtlString mRawUrlParameters;
   bool      parseUrlParameters();//< lazy parser for url parameters
   UtlDList* mpUrlParameters;

   UtlString mRawHeaderOrQueryParameters;
   bool      parseHeaderOrQueryParameters();//< lazy parser for header or query parameters
   UtlDList* mpHeaderOrQueryParameters;

   UtlString mRawFieldParameters;
   bool      parseFieldParameters(); //< lazy parser for field parameters
   UtlDList* mpFieldParameters;

   UtlBoolean mAngleBracketsIncluded;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _Url_h_
