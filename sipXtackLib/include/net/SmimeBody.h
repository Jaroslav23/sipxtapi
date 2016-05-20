//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Dan Petrie (dpetrie AT SIPez DOT com)


#ifndef _SmimeBody_h_
#define _SmimeBody_h_

// SYSTEM INCLUDES
//#include <...>
#ifdef HAVE_NSS

// hack to define int32 for nss-3.9.1 as they have a bug
#ifdef _WIN32
#include <winsock2.h>
   #ifndef int32
   #include <os/msinttypes/stdint.h>
   typedef int32_t int32;
   #endif
#endif

#include <nspr.h>
#include <nss.h>
#include <secutil.h>
#include <secport.h>
#include <certdb.h>
#include <ssl.h>
#include <cms.h>
#include <cert.h>
#include <pk11func.h>
#include <pkcs12.h>
#include <p12plcy.h>
#include <p12.h>
#include <ciferfam.h>
#include <base64.h>
#include "net/pk12wrapper.h"
#endif

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <net/HttpBody.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// ENUMS

/**
 * Container class for security attributes.  
 */

#define SIPXTACK_MAX_SRTP_KEY_LENGTH   31
#define SIPXTACK_MAX_SMIME_KEY_LENGTH  2048
#define SIPXTACK_MAX_PKCS12_KEY_LENGTH 4096
#define SIPXTACK_MAX_CERT_LENGTH       4096
#define SIPXTACK_MAX_PASSWORD_LENGTH   32

/*
typedef enum SMIME_ERRORS
{
    SMIME_SUCCESS,
    SMIME_ENCRYPT_FAILURE_LIB_INIT,
    SMIME_ENCRYPT_FAILURE_BAD_PUBLIC_KEY, 
    SMIME_ENCRYPT_FAILURE_INVALID_PARAMETER,
    SMIME_DECRYPT_FAILURE_DB_INIT,
    SMIME_DECRYPT_FAILURE_BAD_DB_PASSWORD,
    SMIME_DECRYPT_FAILURE_INVALID_PARAMETER,
    SMIME_DECRYPT_BAD_SIGNATURE,
    SMIME_DECRYPT_MISSING_SIGNATURE,
    SMIME_UNKNOWN
} SMIME_ERRORS;
*/

enum SIPXTACK_SRTP_LEVEL
{
    SIPXTACK_SRTP_LEVEL_NONE,
    SIPXTACK_SRTP_LEVEL_ENCRYPTION,
    SIPXTACK_SRTP_LEVEL_AUTHENTICATION,
    SIPXTACK_SRTP_LEVEL_ENCRYPTION_AND_AUTHENTICATION
};

/**
 * Enumeration of possible security events
 */
typedef enum SIP_SECURITY_EVENT
{
    SIP_SECURITY_UNKNOWN       = 0,/**< An UNKNOWN event is generated when the state for a call 
                                 is no longer known.  This is generally an error 
                                 condition; see the minor event for specific causes. */
    SIP_SECURITY_ENCRYPT   = 1000, /**< The ENCRYPT event indicates that an SMIME encryption has been
                                  attempted.  See the cause code for the encryption outcome,
                                  and the info structure for more information. */
    SIP_SECURITY_DECRYPT   = 2000, /**< The DECRYPT event indicates that an SMIME decryption has been
                                  attempted.  See the cause code for the encryption outcome,
                                  and the info structure for more information. */
    SIP_SECURITY_TLS       = 4000, /**< TLS related security event. */
} SIP_SECURITY_EVENT;

/**
 * Enumeration of possible security causes
 */
typedef enum SIP_SECURITY_CAUSE
{
    SIP_SECURITY_CAUSE_UNKNOWN = 0,                      /**< An UNKNOWN cause code is generated when the state
                                                          for the security operation 
                                                          is no longer known.  This is generally an error 
                                                          condition; see the info structure for details. */
    SIP_SECURITY_CAUSE_NORMAL,                           /**< Event was fired as part of the normal encryption / decryption process. */
    SIP_SECURITY_CAUSE_ENCRYPT_SUCCESS,                  /**< An S/MIME encryption succeeded. */
    SIP_SECURITY_CAUSE_ENCRYPT_FAILURE_LIB_INIT,         /**< An S/MIME encryption failed because the
                                                          security library could not start. */
    SIP_SECURITY_CAUSE_ENCRYPT_FAILURE_BAD_PUBLIC_KEY,   /**< An S/MIME encryption failed because of a bad certificate / public key. */
    SIP_SECURITY_CAUSE_ENCRYPT_FAILURE_INVALID_PARAMETER,/**< An S/MIME encryption failed because of an invalid parameter. */
    SIP_SECURITY_CAUSE_DECRYPT_SUCCESS,                  /**< An S/MIME decryption succeeded. */ 
    SIP_SECURITY_CAUSE_DECRYPT_FAILURE_DB_INIT,          /**< An S/MIME decryption failed due to a failure to initialize the certificate database. */
    SIP_SECURITY_CAUSE_DECRYPT_FAILURE_BAD_DB_PASSWORD,  /**< An S/MIME decryption failed due to an invalid certificate database password. */
    SIP_SECURITY_CAUSE_DECRYPT_FAILURE_INVALID_PARAMETER,/**< An S/MIME decryption failed due to an invalid parameter. */
    SIP_SECURITY_CAUSE_DECRYPT_BAD_SIGNATURE,            /**< An S/MIME decryption operation aborted due to a bad signature. */
    SIP_SECURITY_CAUSE_DECRYPT_MISSING_SIGNATURE,        /**< An S/MIME decryption operation aborted due to a missing signature. */
    SIP_SECURITY_CAUSE_DECRYPT_SIGNATURE_REJECTED,       /**< An S/MIME decryption operation aborted because the signature was rejected. */
    SIP_SECURITY_CAUSE_TLS_SERVER_CERTIFICATE,           /**< A TLS server certificate is being presented to the application for possible rejection. 
                                                          The application must respond to this message.
                                                          If the application returns false, the certificate is rejected and the call will not
                                                          complete.  If the application returns true, the certificate is accepted. */
    SIP_SECURITY_CAUSE_TLS_BAD_PASSWORD,                /**< A TLS operation failed due to a bad password. */
    SIP_SECURITY_CAUSE_TLS_LIBRARY_FAILURE,             /**< A TLS operation failed. */
    SIP_SECURITY_CAUSE_REMOTE_HOST_UNREACHABLE,         /**< The remote host is not reachable. */
    SIP_SECURITY_CAUSE_TLS_CONNECTION_FAILURE,          /**< A TLS connection to the remote party failed. */
    SIP_SECURITY_CAUSE_TLS_HANDSHAKE_FAILURE,           /**< A failure occured during the TLS handshake. */
    SIP_SECURITY_CAUSE_SIGNATURE_NOTIFY,                /**< The SIGNATURE_NOTIFY event is fired when the user-agent
                                                         receives a SIP message with signed SMIME as its content.
                                                         The signer's certificate will be located in the info structure
                                                         associated with this event.  The application can choose to accept
                                                         the signature, by returning 'true' in response to this message
                                                         or can choose to reject the signature
                                                         by returning 'false' in response to this message. */
    SIP_SECURITY_CAUSE_TLS_CERTIFICATE_REJECTED         /** < The application has rejected the server's TLS certificate. */
} SIP_SECURITY_CAUSE;

/**
 * Container class for security attributes.  
 */
class SIPXTACK_SECURITY_ATTRIBUTES
{
  public:
    friend class SipMessage;
    SIPXTACK_SECURITY_ATTRIBUTES()
    {
        nSrtpKeyLength = 0 ;
        nSmimeKeyLength = 0 ;
        nSrtpLevel = SIPXTACK_SRTP_LEVEL_NONE ;
        memset(szSrtpKey, 0, sizeof(szSrtpKey));
        memset(szSmimeKeyDer, 0, sizeof(szSmimeKeyDer));
        memset(dbLocation, 0, sizeof(dbLocation));
        memset(szMyCertNickname, 0, sizeof(szMyCertNickname));
        memset(szCertDbPassword, 0, sizeof(szCertDbPassword));
        nSmimeKeyLength = 0 ;
        nSrtpLevel = SIPXTACK_SRTP_LEVEL_NONE ;
    }    
    SIPXTACK_SECURITY_ATTRIBUTES(const SIPXTACK_SECURITY_ATTRIBUTES& ref)
    {
        copyData(ref);
    }    
    virtual ~SIPXTACK_SECURITY_ATTRIBUTES() { }    
    SIPXTACK_SECURITY_ATTRIBUTES& operator=(const SIPXTACK_SECURITY_ATTRIBUTES& ref)
    {
        if (this == &ref) return *this;
        copyData(ref);
        return *this;
    }    
    /**
     * Sets the symmetric srtp key.  If this is not supplied by the user,
     * sipXtapi will generate a random key.
     */
    void setSrtpKey(const char* szKey, const int length)
    {
        SAFE_STRNCPY(szSrtpKey, szKey, length);
        nSrtpKeyLength = length;
    }    
    /**
     * Sets the public key of the remote party, which is used to
     * encrypt the S/MIME container for the SDP.
     */
    void setSmimeKey(const char* szKey, const int length)
    {
        memcpy((void*)szSmimeKeyDer, (void*)szKey, length);
        nSmimeKeyLength = length;
    }
    /**
     * Sets the S/MIME & SRTP security level
     */
    void setSecurityLevel(SIPXTACK_SRTP_LEVEL security) { nSrtpLevel = security; }
    /**
     * Gets the symmetric srtp key.
     */
    const char* getSrtpKey() const  { return szSrtpKey; }    
    /**
     * Gets the public key of the remote party, which is used to
     * encrypt the S/MIME container for the SDP.
     */
    const char* getSmimeKey() const { return szSmimeKeyDer; }
    /**
     * Gets the symmetric srtp key length.
     */
    const int getSrtpKeyLength() const  { return nSrtpKeyLength; }
    /**
     * Gets the public key of the remote party, which is used to
     * encrypt the S/MIME container for the SDP.
     */
    const int getSmimeKeyLength() const { return nSmimeKeyLength; }
    /**
     * Sets the S/MIME & SRTP security level
     */
    const int getSecurityLevel() const {return nSrtpLevel;}
    /**
     * Gets the Certificate Database location (set internally to
     * the location specified in the call to 
     * sipxConfigSetSecurityParameters() )
     */
    const char* getCertDbLocation() const { return dbLocation; }

  private:
    SIPXTACK_SRTP_LEVEL nSrtpLevel;
    char szSrtpKey[SIPXTACK_MAX_SRTP_KEY_LENGTH];
    int  nSrtpKeyLength;    
    char szSmimeKeyDer[SIPXTACK_MAX_SMIME_KEY_LENGTH];
    int  nSmimeKeyLength; 
    // internally set private member, use sipxConfigSetSecurityParameters
    char dbLocation[256];         
    // internally set private member, use sipxConfigSetSecurityParameters
    char szMyCertNickname[32];                
    // internally set private member, use sipxConfigSetSecurityParameters
    char szCertDbPassword[SIPXTACK_MAX_PASSWORD_LENGTH];   
    void copyData(const SIPXTACK_SECURITY_ATTRIBUTES& ref)
    {
        nSrtpLevel = ref.nSrtpLevel;
        nSrtpKeyLength = ref.nSrtpKeyLength;
        nSmimeKeyLength = ref.nSmimeKeyLength;
        SAFE_STRNCPY(szSrtpKey, ref.szSrtpKey, sizeof(szSrtpKey));
        memcpy((void*)szSmimeKeyDer, (void*)ref.szSmimeKeyDer, ref.nSmimeKeyLength);
        SAFE_STRNCPY(dbLocation, ref.dbLocation, sizeof(dbLocation) - 1);
        SAFE_STRNCPY(szMyCertNickname, ref.szMyCertNickname, sizeof(szMyCertNickname) - 1);
        SAFE_STRNCPY(szCertDbPassword, ref.szCertDbPassword, sizeof(szCertDbPassword) - 1);
        return;
    }
};


//: Interface defintion for receiving events: SMIME errors or SMIME signature notifications.
class ISmimeNotifySink
{
public:
        virtual void OnError(SIP_SECURITY_EVENT event, SIP_SECURITY_CAUSE cause) = 0;
        virtual bool OnSignature(void* pCert, char* szSubjAltName) = 0 ;

        virtual ~ISmimeNotifySink() { } ;
};
//! class to contain an PKCS7 (S/MIME) body
/*! This class can be used to create an encrypted S/MIME
    body as well as to decrypt an encrypted body.
 */
class SmimeBody : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum ContentEncoding
    {
        SMIME_ENODING_UNKNOWN = 0,
        SMIME_ENODING_BINARY,
        SMIME_ENODING_BASE64
    };

/* ============================ CREATORS ================================== */

    //! default constructor
    SmimeBody();

    //! Construct an SmimeBody from a bunch of bytes
    SmimeBody(const char* bytes, 
              int length,
              const char* contentEncodingValueString);

    //! Copy constructor
    SmimeBody(const SmimeBody& rSmimeBody);

    //! Destructor
    virtual
    ~SmimeBody();

/* ============================ MANIPULATORS ============================== */

    //! Assignment operator
    SmimeBody& operator=(const SmimeBody& rhs);

    //! Decrypt this body using the given private key and cert. contained in the pkcs12 package
    /*! Decrypts this body using the derPkcs12PrivateKey.
     *  \param derPkcs12 - DER format pkcs12 container for the 
     *         private key and public key/Certificate for a recipent who is 
     *         allowed to decrypt this pkcs7 (S/MIME) encapsulated body.
     *  \param derPkcs12Length - length in bytes of derPkcs12PrivateKey
     *  \param pkcs12Password - symetric key (password) used to protect 
     *         (encrypt) the derPkcs12PrivateKey (the private key is 
     *         contained in a pkcs12 in an encrypted format to protect 
     *         it from theft).  Must be NULL terminated string.
     */
    UtlBoolean decrypt(const char* derPkcs12,
                      int derPkcs12Length,
                      const char* pkcs12password,
                      const char* certDbPassword,
                      const char* signerCertDER,
                      int signerCertDERLength,                      
                      ISmimeNotifySink* pSink = NULL);

    //! Encrypt the given body for the given list of recipients
    /*! \param bodyToEncrypt - Body to encrypt, note bodyToEncrypt
    *         will be attached to and deleted with this SmimeBody.
    *         bodyToEncrypt can be retrieved after invoking decrypt
    *         using the getDecyptedBody method.
    *  \param numRecipients - number of recipients for which 
    *         bodyToEncrypt will be encrypted.  For each recipient
    *         an element in derPublicKeyCerts and derPubliceKeyCertLengths 
    *         must be given.
    *  \param derPublicKeyCerts - array containing a DER format
    *         certificate (containing the public key) for each recipient.
    *  \param derPubliceKeyCertLengths - length in bytes of the 
    *         corresponding element in derPublicKeyCerts.
    */
    UtlBoolean encrypt(HttpBody* bodyToEncrypt,
                      int numRecipients,
                      const char* derPublicKeyCerts[],
                      int derPubliceKeyCertLengths[],
                      const char* szMyCertNickname,
                      const char* szCertDbPassword,
                      ISmimeNotifySink* pSink = NULL);


#ifdef HAVE_NSS
    static void getSubjAltName(char* szSubjAltName,
                        const CERTCertificate* pCert,
                        const size_t length);
#endif
    // Lower level utility to do S/MIME encryption using the NSS library.
    /*! Encrypts the given data for the recipients represented by the
     *  array of certificates containing the public keys.
     *  \param numRecipientCerts the number of recipient certificates in
     *         the derPublicKeyCerts array.
     *  \param derPublicKeyCerts - array containing DER format certificates.
     *  \param derPublicKeyCertLengths - array containing the length of the
     *         corresponding certificate DER data.
     *  \param dataToEncrypt - raw data to encrypt using PKCS7 S/MIME format
     *  \param dataToEncryptLength length in bytes of dataToEncrypt
     *  \param encryptedDataInBase64Format - TRUE: output encrypted data in
     *         base64 format, FALSE: output data in raw binary format.  Typically
     *         for SIP one should send in binary format.
     *  \param encryptedData - string containing the encrypted result.
     */
    static UtlBoolean nssSmimeEncrypt(int numRecipientCerts,
                                       const char* derPublicKeyCerts[], 
                                       int derPublicKeyCertLengths[],
                                       const char* szMyCertNickname,
                                       const char* szCertDbPassword,
                                       const char* dataToEncrypt,
                                       int dataToEncryptLength,
                                       UtlBoolean encryptedDataInBase64Format,
                                       UtlString& encryptedData,
                                       ISmimeNotifySink* pSmimeSink);

    // Lower level utility to do S/MIME decryption using the NSS library
    /*! Decrypts this body using the derPkcs12PrivateKey.
     *  \param derPkcs12 - DER format pkcs12 container for the 
     *         private key and public key/Certificate for a recipent who is 
     *         allowed to decrypt this pkcs7 (S/MIME) encapsulated body.
     *  \param derPkcs12Length - length in bytes of derPkcs12PrivateKey
     *  \param pkcs12Password - symetric key (password) used to protect 
     *         (encrypt) the derPkcs12PrivateKey (the private key is 
     *         contained in a pkcs12 in an encrypted format to protect 
     *         it from theft).  Must be NULL terminated string.
     *  \param dataIsInBase64Format - TRUE: encrypted data is in base64
     *         format, FALSE: encrypted data is in binary format.
     *  \param dataToDecrypt - raw data to be decrypted. Must be in binary
     *         or base64 format.  Does NOT need to be NULL terminated.
     *  \param dataToDecryptLength - length of the data in dataToDecrypt
     *         to be decrypted.
     *  \param decryptedData - string to contain the resulting decrypted
     *         data.
     */
    static UtlBoolean nssSmimeDecrypt(const char* derPkcs12,
                                      int derPkcs12Length,
                                      const char* pkcs12Password,
                                      const char* certDbPassword,
                                      const char* signerCertDER,
                                      int signerCertDERLength,                      
                                      UtlBoolean dataIsInBase64Format,
                                      const char* dataToDecrypt,
                                      int dataToDecryptLength,
                                      UtlString& decryptedData,
                                      ISmimeNotifySink* pSmimeSink);

    // Lower level utility to do S/MIME encryption using the NSS library.
    /*! Encrypts the given data for the recipients represented by the
     *  array of certificates containing the public keys.
     *  \param numRecipientCerts the number of recipient certificates in
     *         the derPublicKeyCerts array.
     *  \param derPublicKeyCerts - array containing DER format certificates.
     *  \param derPublicKeyCertLengths - array containing the length of the
     *         corresponding certificate DER data.
     *  \param dataToEncrypt - raw data to encrypt using PKCS7 S/MIME format
     *  \param dataToEncryptLength length in bytes of dataToEncrypt
     *  \param encryptedDataInBase64Format - TRUE: output encrypted data in
     *         base64 format, FALSE: output data in raw binary format.  Typically
     *         for SIP one should send in binary format.
     *  \param encryptedData - string containing the encrypted result.
     */
    static UtlBoolean opensslSmimeEncrypt(int numRecipientCerts,
                                          const char* derPublicKeyCerts[], 
                                          int derPublicKeyCertLengths[],
                                          const char* dataToEncrypt,
                                          int dataToEncryptLength,
                                          UtlBoolean encryptedDataInBase64Format,
                                          UtlString& encryptedData);

    // Lower level utility to do S/MIME decryption using the OpenSSL library
    /*! Decrypts this body using the derPkcs12PrivateKey.
     *  \param derPkcs12 - DER format pkcs12 container for the 
     *         private key and public key/Certificate for a recipent who is 
     *         allowed to decrypt this pkcs7 (S/MIME) encapsulated body.
     *  \param derPkcs12Length - length in bytes of derPkcs12PrivateKey
     *  \param pkcs12Password - symetric key (password) used to protect 
     *         (encrypt) the derPkcs12PrivateKey (the private key is 
     *         contained in a pkcs12 in an encrypted format to protect 
     *         it from theft).  Must be NULL terminated string.
     *  \param dataIsInBase64Format - TRUE: encrypted data is in base64
     *         format, FALSE: encrypted data is in binary format.
     *  \param dataToDecrypt - raw data to be decrypted. Must be in binary
     *         or base64 format.  Does NOT need to be NULL terminated.
     *  \param dataToDecryptLength - length of the data in dataToDecrypt
     *         to be decrypted.
     *  \param decryptedData - string to contain the resulting decrypted
     *         data.
     */
    static UtlBoolean opensslSmimeDecrypt(const char* derPkcs12,
                                   int derPkcs12Length,
                                   const char* pkcs12Password,
                                   UtlBoolean dataIsInBase64Format,
                                   const char* dataToDecrypt,
                                   int dataToDecryptLength,
                                   UtlString& decryptedData);

    //! Utility to convert PEM format data to DER format
    static UtlBoolean convertPemToDer(UtlString& pemData,
                                      UtlString& derData);
                                      
    static bool importPKCS12Object(const char* derPkcs12,
                                       int derPkcs12Length,
                                       const char* pkcs12Password,
                                       const char* certDbLocation,
                                       const char* certDbPassword);


/* ============================ ACCESSORS ================================= */

    //! Gets the decrypted form of this body if available
    const HttpBody* getDecryptedBody() const;

/* ============================ INQUIRY =================================== */

   //! Query if this body has been decrypted
   UtlBoolean isDecrypted() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   HttpBody* mpDecryptedBody;
   enum ContentEncoding mContentEncoding;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
#ifdef HAVE_NSS
    static UtlString createSignedData(CERTCertificate *cert,
                          const char* dataToSign,
                          const int dataToSignLength,
                          NSSCMSSignedData*& sigd, 
                          char* szCertDbPassword);


#endif
    ISmimeNotifySink* mpSmimeSink;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SmimeBody_h_
