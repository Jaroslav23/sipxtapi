//
// Copyright (C) 2007 Robert J. Andreasen, Jr.
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

/**
 * @mainpage SDK Overview
 * 
 * @htmlinclude sipXtapi-overview.html
 */ 

/** 
 * @file sipXtapi.h
 *
 * sipXtapi main API declarations
 **/

#ifndef _sipXtapi_h_
#define _sipXtapi_h_

// SYSTEM INCLUDES
#include <memory.h>
#include <string.h>
#include <stddef.h>       // size_t

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
#include <windows.h>
#include <iphlpapi.h>
#endif

#ifdef DIRECT_SHOW_RENDER
#include <windows.h>
#include <Unknwn.h>
#    if !defined __strmif_h__
#        include <strmif.h>
#    endif 
#endif

// APPLICATION INCLUDES
#include <os/OsDefs.h>

// DEFINES

/**
 * The SIPX_CALLING_CONVENTION define controls the default calling convention.
 * define "SIPX_USE_STDCALL" to use the __stdcall calling convention under
 * MSVC.
 */
#ifdef SIPX_USE_STDCALL
#define SIPX_CALLING_CONVENTION __stdcall
#else
#define SIPX_CALLING_CONVENTION
#endif 

#define DEFAULT_UDP_PORT        5060    /**< Default UDP port */
#define DEFAULT_TCP_PORT        5060    /**< Default TCP port */
#define DEFAULT_TLS_PORT        5061    /**< Default TLS port */
#define DEFAULT_RTP_START_PORT  9000    /**< Starting RTP port for RTP port range.
                                             The user agent will use ports ranging 
                                             from the start port to the start port 
                                             + (default connections * 2). */
#define DEFAULT_STUN_PORT       3478    /**< Default stun server port */

#define DEFAULT_CONNECTIONS     100      /**< Default number of max sim. conns. */
#define DEFAULT_IDENTITY        "sipx"  /**< sipx@<IP>:UDP_PORT used as identify if lines
                                             are not defined.  This define only controls
                                             the userid portion of the SIP url. */
#define DEFAULT_BIND_ADDRESS    "0.0.0.0" /**< Bind to the first physical interface discovered */

#define INPUT_VOLUME_MIN                0       /**< Min acceptable gain value. This gain will mute mic. */
#define INPUT_VOLUME_MAX                100     /**< Max acceptable gain value */

#define BALANCE_MIN -100
#define BALANCE_MAX 100

#define OUTPUT_VOLUME_MIN              0       /**< Min acceptable volume value */
#define OUTPUT_VOLUME_MAX              100     /**< Max acceptable volume value */

#define MAX_AUDIO_DEVICES       16      /**< Max number of input/output audio devices */
#define MAX_VIDEO_DEVICES       8       /**< Max number of video capture devices. */
#define MAX_VIDEO_DEVICE_LENGTH 256     /**< Max length of video capture device string. */

#define SIPX_MAX_IP_ADDRESSES   32      /**< Maximum number of IP addresses on the host */


#define SIPX_PORT_DISABLE       -1      /**< Special value that disables the transport 
                                             type (e.g. UDP, TCP, or TLS) when passed 
                                             to sipXinitialize */
#define SIPX_PORT_AUTO          -2      /**< Special value that instructs sipXtapi to
                                             automatically select an open port for 
                                             signaling or audio when passed to 
                                             sipXinitialize */

#define SIPXTAPI_VERSION_STRING "sipXtapi SDK %s.%s (built %s)" /**< Version string format string */
#define SIPXTAPI_VERSION        "3.3.0"      /**< sipXtapi API version -- automatically filled in 
                                                  during release process */   
#define SIPXTAPI_BUILDNUMBER "0"             /**< Default build number -- automatically filled in 
                                                  during release process*/
#define SIPXTAPI_STRING_MEDIUM_LENGTH 30          /**< Maximum length for generic string */

#ifdef _WIN32
#  ifdef SIPXTAPI_EXPORTS
#    define SIPXTAPI_API extern "C" __declspec(dllexport)  /**< Used for Win32 imp lib creation */
#  elif SIPXTAPI_STATIC
#    define SIPXTAPI_API extern "C"  /**< Used for Win32 imp lib creation */
#  else
#    define SIPXTAPI_API extern "C" __declspec(dllimport)  /**< Used for Win32 imp lib creation */
#  endif
#else
#define SIPXTAPI_API extern "C"   /**< Assume extern "C" for non-win32 platforms */
#endif

#ifndef MAX_ADAPTER_NAME_LENGTH
#define MAX_ADAPTER_NAME_LENGTH 256
#endif

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class securityHelper;

// STRUCTS
// TYPEDEFS

/**
 * Strategy for verifying SSL certificates. By default, certificate
 * must have valid date, must be signed by approved certificate authority (CA),
 * and its common name (CN) must match server/client hostname.
 *
 * Testing certificates can be used by setting policy to always accept.
 * SipXtapi is not supplied with default store of approved certificate authorities.
 */
typedef enum
{
   SIPX_SSL_VERIFICATION_DEFAULT = 0, ///< A valid CRT parent CA, hostname & CN match, and valid date are required
   SIPX_SSL_ALWAYS_ACCEPT ///< Always accept all certificates, even invalid ones
} SIPX_SSL_CRT_VERIFICATION;

/**
 * Codec bandwidth ids are used to select a group of codecs with equal or lower
 * bandwidth requirements.
 *
 * High bandwidth - bitrate lower than 20 kbit/s
 * Normal bandwidth - bitrate 20 kbit/s - 40 kbit/s
 * High bandwidth - bitrate higher than 40 kbit/s
 *
 * Bitrates of supported codecs:
 *
 * <pre>
 *           PCMU      64 kbit/s, 8Khz sampling rate
 *           PCMA      64 kbit/s, 8Khz sampling rate
 *           iLBC      13.33 kbit/s, 30 ms frame size, 8Khz sampling rate
 *                     15.2 kbit/s, 20 ms frame size
 *           GSM       13 kbit/s, 8Khz sampling rate
 *           AMR       4.75 kbit/s, bandwidth efficient, 8Khz sampling rate
 *           AMR       10.2 kbit/s, octet aligned, 8Khz sampling rate
 *           AMR wb    12.65 kbit/s, bandwidth efficient, 16Khz sampling rate
 *           AMR wb    23.85 kbit/s, octet aligned, 16Khz sampling rate
 *           G722      64 kbit/s, 16Khz sampling rate
 *           G722.1_16 16 kbit/s, 16Khz sampling rate
 *           G722.1_24 24 kbit/s
 *           G722.1_32 32 kbit/s
 *           G723.1    6.3 kbit/s, 30 ms frame size (using 24 byte frames), 8Khz sampling rate
 *                     5.3 kbit/s, 30 ms frame size (using 20 byte frames)
 *           G726_16   16 kbit/s, 8Khz sampling rate
 *           G726_24   24 kbit/s
 *           G726_32   32 kbit/s
 *           G726_40   40 kbit/s
 *           G728      16 kbit/s, 12.8 kbit/s, 8Khz sampling rate
 *           G729      annex A/B depending on VAD mode, 8 kbit/s, 8Khz sampling rate
 *           G729D     6.4 kbit/s, 8 kbit/s, 8Khz sampling rate
 *           G729E     12.4 kbit/s, 8Khz sampling rate
 *           SPEEX_5   5.95 kbit/s, 8Khz sampling rate
 *           SPEEX_8   8 kbit/s
 *           SPEEX_11  11 kbit/s
 *           SPEEX_15  15 kbit/s
 *           SPEEX_18  18.2 kbit/s
 *           SPEEX_24  24.6 kbit/s
 *           SPEEX_WB_9 9.8 kbit/s, 16Khz sampling rate
 *           SPEEX_WB_12 12.8 kbit/s
 *           SPEEX_WB_16 16.8 kbit/s
 *           SPEEX_WB_20 20.6 kbit/s
 *           SPEEX_WB_23 23.8 kbit/s
 *           SPEEX_WB_27 27.8 kbit/s
 *           SPEEX_WB_34 34.4 kbit/s
 *           SPEEX_WB_42 42.4 kbit/s
 *           SPEEX_UWB_11 11.6 kbit/s, 32Khz sampling rate
 *           SPEEX_UWB_14 14.6 kbit/s
 *           SPEEX_UWB_18 18.6 kbit/s
 *           SPEEX_UWB_22 22.4 kbit/s
 *           SPEEX_UWB_25 25.6 kbit/s
 *           SPEEX_UWB_29 29.6 kbit/s
 *           SPEEX_UWB_36 36.0 kbit/s
 *           SPEEX_UWB_44 44.0 kbit/s
 *           L16 8Khz mono 128 kbit/s
 *           L16 11Khz mono 176 kbit/s
 *           L16 16Khz mono 258 kbit/s
 *           L16 22Khz mono 352 kbit/s
 *           L16 24Khz mono 384 kbit/s
 *           L16 32Khz mono 512 kbit/s
 *           L16 44Khz mono 704 kbit/s
 *           L16 48Khz mono 768 kbit/s
 * </pre>
 * Note that L16 is uncompressed 16bit audio at various sampling rates.
 *
 * Encoders/Decoders which support VAD (voice activity detection) with DTX and built in PLC
 * have these features enabled.
 */
typedef enum
{
    AUDIO_CODEC_BW_VARIABLE = 0,   /**< ID for codecs with variable bandwidth requirements */
    AUDIO_CODEC_BW_LOW,          /**< ID for codecs with low bandwidth requirements */
    AUDIO_CODEC_BW_NORMAL,       /**< ID for codecs with normal bandwidth requirements */
    AUDIO_CODEC_BW_HIGH,         /**< ID for codecs with high bandwidth requirements */
} SIPX_AUDIO_BANDWIDTH_ID;

/**
 * Indicates relative CPU cost of an audio or video codec.
 */
typedef enum
{
   SIPX_CODEC_CPU_LOW = 0, ///< complexity of algorithm is lower than 8
   SIPX_CODEC_CPU_NORMAL, ///< complexity of algorithm is lower than 12
   SIPX_CODEC_CPU_HIGH
} SIPX_CODEC_CPU_COST;

/**
 * Video Codec bandwidth ids are used to select a group of codecs with equal 
 * or lower bandwidth requirements. The codec name is a combination of the
 * actual codec name and the video resolution.
 *
 * Supported codecs are:
 * 
 *    VP71, IYUV, I420, and RGB24
 *
 * Supported resolutions are
 * 
 *    CIF (352x288), QCIF (176x144), SQCIF (128x96), and QVGA (320x240)
 *
 * A VP71 codec in QCIF resolution would be named VP71-QCIF.
 */
typedef enum
{
    VIDEO_CODEC_BW_VARIABLE = 0,   /**< ID for codecs with variable bandwidth requirements */
    VIDEO_CODEC_BW_LOW,          /**< ID for codecs with low bandwidth requirements */
    VIDEO_CODEC_BW_NORMAL,       /**< ID for codecs with normal bandwidth requirements */
    VIDEO_CODEC_BW_HIGH,         /**< ID for codecs with high bandwidth requirements */
} SIPX_VIDEO_BANDWIDTH_ID;

/**
 * Video Codec quality definitions.  Quality is used as a trade off between between 
 * CPU usage and the amount of bandwidth used.
 */
typedef enum
{
    VIDEO_QUALITY_LOW = 1,         /**< Low quality video */
    VIDEO_QUALITY_NORMAL = 2,      /**< Normal quality video */
    VIDEO_QUALITY_HIGH = 3         /**< High quality video */
} SIPX_VIDEO_QUALITY_ID;

/**
 *  Enumeration of possible video sizes.
 */
typedef enum
{
    VIDEO_FORMAT_ANY = -1,
    VIDEO_FORMAT_CIF = 0,          /**< 352x288   */ 
    VIDEO_FORMAT_QCIF,             /**< 176x144   */
    VIDEO_FORMAT_SQCIF,            /**< 128x96    */
    VIDEO_FORMAT_QVGA              /**< 320x240   */
} SIPX_VIDEO_FORMAT;


/**
 * Format definitions for memory resident audio data
 */
typedef enum
{
    RAW_PCM_16 = 0                 /**< Signed 16 bit PCM data, mono, 8KHz, no header */
} SIPX_AUDIO_DATA_FORMAT;


/**
 * Types of volume meters that can be used to query current input/output "energy"
 * VU (Volume Unit) is average over certain amount of frames, and more corresponds
 * to sensing of human ear.
 * PPM (Peak Program Meters) is maximum from certain amount of frames. It reacts
 * quicker to change, is louder than VU.
 *
 */
typedef enum
{
   SIPX_VOLUME_METER_VU = 0,
   SIPX_VOLUME_METER_PPM
} SIPX_VOLUME_METER_TYPE;
// keep in sync with MEDIA_VOLUME_METER_TYPE


/**
 * Signature for a log callback function that gets passed three strings,
 * first string is the priority level, second string is the source id of 
 * the subsystem that generated the message, and the third string is the 
 * message itself.
 *
 * @NOTE: Keep in sync OsSysLogCallback with sipxLogCallback
 */
typedef void (*sipxLogCallback)(const char* szPriority,
                                const char* szSource,
                                const char* szMsg);

/**
 * SIPX_RESULT is an enumeration with all the possible result/return codes.
 */ 
typedef enum SIPX_RESULT 
{
    SIPX_RESULT_SUCCESS = 0,         /**< Success */
    SIPX_RESULT_FAILURE,             /**< Generic Failure*/
    SIPX_RESULT_NOT_IMPLEMENTED,     /**< Method/API not implemented */
    SIPX_RESULT_OUT_OF_MEMORY,       /**< Unable to allocate enough memory to perform operation*/
    SIPX_RESULT_INVALID_ARGS,        /**< Invalid arguments; bad handle, argument out of range, 
                                          etc.*/
    SIPX_RESULT_BAD_ADDRESS,         /**< Invalid SIP address */
    SIPX_RESULT_OUT_OF_RESOURCES,    /**< Out of resources (hit some max limit) */
    SIPX_RESULT_INSUFFICIENT_BUFFER, /**< Buffer too short for this operation */
    SIPX_RESULT_EVAL_TIMEOUT,        /**< The evaluation version of this product has expired */
    SIPX_RESULT_BUSY,                /**< The operation failed because the system was busy */
    SIPX_RESULT_INVALID_STATE,       /**< The operation failed because the object was in
                                          the wrong state.  For example, attempting to split
                                          a call from a conference before that call is 
                                          connected. */
    SIPX_RESULT_MISSING_RUNTIME_FILES,/**< The operation failed because required runtime dependencies are missing. */
    SIPX_RESULT_TLS_DATABASE_FAILURE, /**< The operation failed because the certificate database did not initialize. */
    SIPX_RESULT_TLS_BAD_PASSWORD,     /**< The operation failed because the certificate database did not accept the password.*/
    SIPX_RESULT_TLS_TCP_IMPORT_FAILURE, /**< The operation failed because a TCP socket could not be imported by the SSL/TLS module. */
    SIPX_RESULT_NSS_FAILURE,          /**< The operation failed due to an NSS failure. */
    SIPX_RESULT_NOT_SUPPORTED,        /**< The operation is not supported in this build/configuration */
    SIPX_RESULT_NETWORK_FAILURE       /**< The network is down or failing */
    
} SIPX_RESULT;

/**
* Various modes for sending DTMF.
*
* @NOTE: Keep in sync with MEDIA_OUTBOUND_DTMF_MODE 
*/
typedef enum
{
	SIPX_OUTBOUND_DTMF_DISABLED,
	SIPX_OUTBOUND_DTMF_INBAND,
	SIPX_OUTBOUND_DTMF_RFC2833
} SIPX_OUTBOUND_DTMF_MODE;

/**
* Various modes for receving DTMF.
*
* @NOTE: Keep in sync with MEDIA_INBOUND_DTMF_MODE 
*/
typedef enum
{
	SIPX_INBOUND_DTMF_INBAND,
	SIPX_INBOUND_DTMF_RFC2833
} SIPX_INBOUND_DTMF_MODE;

/**
 * DTMF/other tone ids used with sipxCallStartTone/sipxCallStopTone 
 */
typedef enum
{
    ID_DTMF_0              = 0,   /**< DMTF 0 */
    ID_DTMF_1              = 1,   /**< DMTF 1 */
    ID_DTMF_2              = 2,   /**< DMTF 2 */
    ID_DTMF_3              = 3,   /**< DMTF 3 */
    ID_DTMF_4              = 4,   /**< DMTF 4 */
    ID_DTMF_5              = 5,   /**< DMTF 5 */
    ID_DTMF_6              = 6,   /**< DMTF 6 */
    ID_DTMF_7              = 7,   /**< DMTF 7 */
    ID_DTMF_8              = 8,   /**< DMTF 8 */
    ID_DTMF_9              = 9,   /**< DMTF 9 */
    ID_DTMF_STAR           = 10,   /**< DMTF * */
    ID_DTMF_POUND          = 11,   /**< DMTF # */
    ID_DTMF_A              = 12,   /**< DMTF A */
    ID_DTMF_B              = 13,   /**< DMTF B */
    ID_DTMF_C              = 14,   /**< DMTF C */
    ID_DTMF_D              = 15,   /**< DMTF D */
    ID_DTMF_FLASH          = 16,   /**< DTMF Flash */
    ID_TONE_DIALTONE      = 512,   /**< Dialtone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_BUSY,                   /**< Call-busy tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_RINGBACK,               /**< Remote party is ringing feedback tone 
                                         (Not supported with GIPS VoiceEngine)*/
    ID_TONE_RINGTONE,               /**< Default ring/alert tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_CALLFAILED,             /**< Fasy Busy / call failed tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_SILENCE,                /**< Silence 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_BACKSPACE,              /**< Backspace tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_CALLWAITING,            /**< Call waiting alert tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_CALLHELD,               /**< Call held feedback tone 
                                         (Not supported with GIPS VoiceEngine) */
    ID_TONE_LOUD_FAST_BUSY          /**< Off hook / fast busy tone 
                                         (Not supported with GIPS VoiceEngine)*/
} SIPX_TONE_ID;                 

/**
 * Configuration of session timer refresher. Refresher is side which is responsible
 * for periodically refreshing the session with re-INVITE or UPDATE within session
 * expiration time. If no session refresh occurs until that period, session may be
 * torn down. Refresher is negotiated by default.
 */
typedef enum
{
   SIPX_SESSION_REFRESH_AUTO = 0, /**< Refresher negotiation is automatic  */
   SIPX_SESSION_REFRESH_LOCAL,  /**< Our side will carry out session refresh */
   SIPX_SESSION_REFRESH_REMOTE   /**< Remote side will carry out session refresh */
} SIPX_SESSION_TIMER_REFRESH;

/**
 * Configuration of SIP UPDATE method. By default, processing of inbound UPDATE
 * is enabled, but sipXtapi will never send UPDATE itself. It is possible to enable
 * sending UPDATE instead of re-INVITE for hold/unhold/session refresh/codec renegotiation.
 * UPDATE is faster than re-INVITE, but requires immediate response without user interaction,
 * and could therefore be rejected.
 *
 * UPDATE will only be sent, if remote party supports it. Otherwise re-INVITE will be used, even
 * if UPDATE is enabled.
 */
typedef enum
{
   SIPX_SIP_UPDATE_DISABLED = 0, /**< UPDATE is completely disabled, UPDATE requests will be rejected,
                                  *   but sipXtapi will continue advertising support for UPDATE
                                  *   method. RFC4916 (Connected Identity) will use re-INVITE.
                                  */
   SIPX_SIP_UPDATE_ONLY_INBOUND, /**< UPDATE is enabled only for inbound requests - default.
                                  *   RFC4916 (Connected Identity) will use re-INVITE.
                                  */
   SIPX_SIP_UPDATE_BOTH          /**< We may send UPDATE if remote side supports it,
                                  *   and accept inbound requests. RFC4916 support will use
                                  *   UPDATE if possible.
                                  */
} SIPX_SIP_UPDATE_CONFIG;

/**
 * Configuration of reliable provisional responses (100rel) support in sipXtapi. Reliable
 * provisional responses are defined in rfc3262. Reliable 18x responses are important for
 * interoperability with PTST world, and also bring advantages for early session negotiation.
 *
 * SipXtapi supports sending SDP offer in unreliable 18x responses, to enable unreliable early
 * audio for inbound calls. It is also capable of processing SDP answers in unreliable 18x responses.
 * This method is unreliable, because the 18x response could be lost, and cannot be used if late SDP
 * negotiation is employed, when SDP offer of caller not presented in INVITE.
 *
 * With reliable 18x responses, it is possible to send SDP offer in the reliable response itself.
 * Remote side then must send SDP answer in PRACK, and thus early session can be established. Or
 * if SDP offer was in PRACK, then SDP answer will be in PRACK response.
 *
 * 100rel support also enables usage of UPDATE method for negotiation of early session parameters.
 * So called "early-session" disposition type (rfc3959) is not supported. This however doesn't prevent
 * negotiation of early session parameters.
 */
typedef enum
{
   SIPX_100REL_PREFER_UNRELIABLE = 0, /**< Prefer sending unreliable 18x responses */
   SIPX_100REL_PREFER_RELIABLE,       /**< Prefer sending reliable 18x responses, if remote
                                       *   side supports it - default.
                                       */
   SIPX_100REL_REQUIRE_RELIABLE       /**< We will require support for 100rel when connecting
                                       *   new outbound calls, and prefer sending reliable 18x
                                       *   responses when possible for inbound calls.
                                       */
} SIPX_100REL_CONFIG;

/**
* We support 2 SDP offering modes - immediate and delayed. Immediate sends
* offer as soon as possible, to be able to receive early audio.
* Delayed offering sends SDP offer as late as possible. This saves media
* resources, in case lots of calls are made which might be rejected.
*/
typedef enum
{
   SIPX_SDP_OFFERING_IMMEDIATE = 0, /**
                                     * Offer SDP in the initial INVITE request.
                                     */
   SIPX_SDP_OFFERING_DELAYED = 1,   /**
                                     * Do not offer SDP in INVITE. SDP will be sent in ACK or PRACK for
                                     * outbound calls.
                                     */
} SIPX_SDP_OFFERING_MODE;

/**
 * Configures how sipXtapi should manage focus for calls. If call is focused then speaker and microphone
 * are available for it.
 */
typedef enum
{
   SIPX_FOCUS_MANUAL = 0,   /**< Setting focus is completely manual, call will not be put in focus */
   SIPX_FOCUS_IF_AVAILABLE, /**< Focus call only if there is no other focused call */
   SIPX_FOCUS_ALWAYS        /**< Always focus new call, and defocus previously active call */
} SIPX_FOCUS_CONFIG;

/**
 * Various log levels available for the sipxConfigEnableLog method.  
 * Developers can choose the amount of detail available in the log.
 * Each level includes messages generated at lower levels.  For 
 * example, LOG_LEVEL_EMERG will limit the log to emergency messages,
 * while LOG_LEVEL_ERR includes emergency messages, alert messages, 
 * critical messages, and errors.  LOG_LEVEL_ERR is probably best for
 * general runtime situations.  LOG_LEVEL_INFO or LOG_LEVEL_DEBUG is 
 * best for diagnosing problems.
 *
 * @NOTE: Keep in sync OsSysLogPriority with SIPX_LOG_LEVEL 
 */
typedef enum
{
    LOG_LEVEL_DEBUG,     /**< debug-level messages */
    LOG_LEVEL_INFO,      /**< informational messages */
    LOG_LEVEL_NOTICE,    /**< normal, but significant, conditions */
    LOG_LEVEL_WARNING,   /**< warning conditions */
    LOG_LEVEL_ERR,       /**< error conditions */
    LOG_LEVEL_CRIT,      /**< critical conditions */
    LOG_LEVEL_ALERT,     /**< action must be taken immediately */
    LOG_LEVEL_EMERG,     /**< system is unusable */
    LOG_LEVEL_NONE,      /**< disable logging */
} SIPX_LOG_LEVEL;

#define MAX_SRTP_KEY_LENGTH   31        /**< srtp key length */
#define MAX_SMIME_KEY_LENGTH  2048      /**< s/mime key length */
#define MAX_PKCS12_KEY_LENGTH 4096      /**< pkcs12 key length */
#define MAX_PASSWORD_LENGTH   32        /**< maximum password length PKI operations */

/**
 * Enumeration of the possible levels of SRTP. 
 */
enum SIPX_SRTP_LEVEL
{
    SRTP_LEVEL_NONE = 0,
    SRTP_LEVEL_ENCRYPTION,
    SRTP_LEVEL_AUTHENTICATION,
    SRTP_LEVEL_ENCRYPTION_AND_AUTHENTICATION
};

/**
 * Container class for security attributes.
 *
 * @NOTE: Keep in sync SIPX_SECURITY_ATTRIBUTES with SIPXTACK_SECURITY_ATTRIBUTES
 */

class SIPX_SECURITY_ATTRIBUTES
{
public:
    friend class SecurityHelper;
    
    SIPX_SECURITY_ATTRIBUTES() 
    {
        nSrtpKeyLength = 0 ;
        nSmimeKeyLength = 0 ;
        nSrtpLevel = SRTP_LEVEL_NONE ;
        memset(szSrtpKey, 0, sizeof(szSrtpKey));
        memset(szSmimeKeyDer, 0, sizeof(szSmimeKeyDer));
        memset(dbLocation, 0, sizeof(dbLocation));
        memset(szMyCertNickname, 0, sizeof(szMyCertNickname));
        memset(szCertDbPassword, 0, sizeof(szCertDbPassword));
    }    
    
    SIPX_SECURITY_ATTRIBUTES(const SIPX_SECURITY_ATTRIBUTES& ref)
    {
        copyData(ref);
    }    
    
    virtual ~SIPX_SECURITY_ATTRIBUTES() { }    
    
    SIPX_SECURITY_ATTRIBUTES& operator=(const SIPX_SECURITY_ATTRIBUTES& ref)
    {
        if (this == &ref) return *this;
        copyData(ref);
        return *this;
    }    
    
     
    void setSrtpKey(const char* szKey, const int length)
    {
        int safeLen = (length < (int) sizeof(szSrtpKey)) ? length : (int) sizeof(szSrtpKey);
        memcpy(szSrtpKey, szKey, safeLen);
        nSrtpKeyLength = safeLen;
    }    
    
     
    void setSmimeKey(const char* szKey, const int length)
    {
        int safeLen = (length < (int) sizeof(szSmimeKeyDer)) ? length : (int) sizeof(szSmimeKeyDer);
        memcpy(szSmimeKeyDer, szKey, safeLen);
        nSmimeKeyLength = safeLen;
    }
         
    void setSecurityLevel(SIPX_SRTP_LEVEL security) { nSrtpLevel = security; }
    
     
    const char* getSrtpKey() const  { return szSrtpKey; }    
         
    const char* getSmimeKey() const { return szSmimeKeyDer; }
    
     
    const int getSrtpKeyLength() const  { return nSrtpKeyLength; }
         
    const int getSmimeKeyLength() const { return nSmimeKeyLength; }
   
     
    const int getSecurityLevel() const {return nSrtpLevel;}
    
     
    const char* getCertDbLocation() const { return dbLocation; }
private:
    SIPX_SRTP_LEVEL nSrtpLevel;
    char szSrtpKey[MAX_SRTP_KEY_LENGTH];
    int  nSrtpKeyLength;    
    char szSmimeKeyDer[MAX_SMIME_KEY_LENGTH];
    int  nSmimeKeyLength; 
    // internally set private member, use sipxConfigSetSecurityParameters
    char dbLocation[256];                         
    // internally set private member, use sipxConfigSetSecurityParameters
    char szMyCertNickname[32];   
    // internally set private member, use sipxConfigSetSecurityParameters
    char szCertDbPassword[MAX_PASSWORD_LENGTH];   
    void copyData(const SIPX_SECURITY_ATTRIBUTES& ref)
    {
        nSrtpLevel = ref.nSrtpLevel;
        nSrtpKeyLength = ref.nSrtpKeyLength;
        nSmimeKeyLength = ref.nSmimeKeyLength;
        memcpy(szSrtpKey, ref.szSrtpKey, ref.nSrtpKeyLength);
        memcpy(szSmimeKeyDer, ref.szSmimeKeyDer, ref.nSmimeKeyLength);
        SAFE_STRNCPY(dbLocation, ref.dbLocation, sizeof(dbLocation) - 1);
        SAFE_STRNCPY(szMyCertNickname, ref.szMyCertNickname, sizeof(szMyCertNickname) - 1);
        SAFE_STRNCPY(szCertDbPassword, ref.szCertDbPassword, sizeof(szCertDbPassword) - 1);
    }
};

/**
 * SIPX_CONTACT_TYPE is an enumeration of possible address types for use with
 * SIP contacts and SDP connection information.  Application developers and 
 * choose to setup calls with specific contact types (e.g. use my local IP 
 * address, a stun-derived IP address, turn-derived IP address, etc).  Unless
 * you have complete knowledge and control of your network environment, you
 * should likely use CONTACT_AUTO.
 */
typedef enum
{
    CONTACT_AUTO = 0,   /**< Automatic contact selection; used for API 
                             parameters */
    CONTACT_LOCAL,      /**< Local address for a particular interface */
    CONTACT_NAT_MAPPED, /**< NAT mapped address (e.g. STUN)           */
    CONTACT_RELAY      /**< Relay address (e.g. TURN)                */
} SIPX_CONTACT_TYPE;

/**
 * SIPX_TRANSPORT_TYPE defines various protocols use for signaling 
 * transport.  The SIPX_TRANSPORT_TYPE is return in contact 
 * addresses.
 * Keep in sync with SIP_TRANSPORT_TYPE.
 */
typedef enum
{
    TRANSPORT_AUTO = -1,  /**< Automatic transport (we don't care) */
    TRANSPORT_UDP = 0,  /**< Indicator for a UDP socket type. */
    TRANSPORT_TCP,  /**< Indicator for a TCP socket type. */ 
    TRANSPORT_TLS,  /**< Indicator for a TLS socket type. */
} SIPX_TRANSPORT_TYPE;

/**
 * Type for storing a "window object handle" - in Windows,
 * the application should cast their HWND to a SIPX_WINDOW_HANDLE.
 */
typedef void* SIPX_WINDOW_HANDLE;

/**
 * Enum for specifying the type of display object
 * to be used for displaying video
 */
typedef enum
{
    SIPX_WINDOW_HANDLE_TYPE,     /**< A handle to the window for
                                      the remote video display */
    DIRECT_SHOW_FILTER           /**< A DirectShow render filter object for
                                      handling the remote video display */
} SIPX_VIDEO_DISPLAY_TYPE;

/**
 * Structure used to pass window handle/filter interface for video calls.
 */
struct SIPX_VIDEO_DISPLAY
{
	/** Default constructor */
    SIPX_VIDEO_DISPLAY()
    {
        cbSize = sizeof(SIPX_VIDEO_DISPLAY);
        type = SIPX_WINDOW_HANDLE_TYPE;
        handle = NULL;
    }
    /** Destructor. */
    ~SIPX_VIDEO_DISPLAY()
    {
        if (type == DIRECT_SHOW_FILTER)
        {
#ifdef DIRECT_SHOW_RENDER
            if (handle) ((IUnknown*)handle)->Release();
#endif              
        }
    }
	/** Copy constructor */
    SIPX_VIDEO_DISPLAY(const SIPX_VIDEO_DISPLAY& ref)
    {
        copy(ref);
    }
    /** Assignment operator. */
    SIPX_VIDEO_DISPLAY& operator=(const SIPX_VIDEO_DISPLAY& ref)
    {
        // check for assignment to self
        if (this == &ref) return *this;
        copy(ref);
        return *this;
    }    
    int cbSize;						/**< Size of structure */
    SIPX_VIDEO_DISPLAY_TYPE type;	/**< Type of video display */
    union
    {
		SIPX_WINDOW_HANDLE handle;	/**< Window handle if type SIPX_WINDOW_HANDLE_TYPE */
#ifdef DIRECT_SHOW_RENDER
		IBaseFilter* filter;		/**< Direct Show filter if type is DIRECT_SHOW_FILTER */
#endif
    };
private:
    void copy(const SIPX_VIDEO_DISPLAY& ref)
    {
        cbSize = ref.cbSize;
        type = ref.type;
        handle = ref.handle;
        if (type == DIRECT_SHOW_FILTER)
        {
#ifdef DIRECT_SHOW_RENDER
            // we should addRef here.
            if (handle) ((IBaseFilter*)handle)->AddRef();
#endif            
        }
    }
    
};

/**
 * Use when contact should be selected automatically.
 */
#define SIPX_AUTOMATIC_CONTACT_ID -1

/** 
 * Type for storing Contact Record identifiers 
 * @see sipxConfigGetLocalContacts
 */
typedef int SIPX_CONTACT_ID; 

/**
 * The CONTACT_ADDRESS structure includes contact information (IP-address and
 * port), address source type, and interface.
 *
 * @see sipxConfigGetLocalContacts
 */
struct SIPX_CONTACT_ADDRESS
{
    /** Constructor. */
    SIPX_CONTACT_ADDRESS()
    {
        cbSize = sizeof(SIPX_CONTACT_ADDRESS);
        memset((void*)cInterface, 0, sizeof(cInterface));
        memset((void*)cIpAddress, 0, sizeof(cIpAddress));
        memset((void*)cInterfaceIp, 0, sizeof(cInterfaceIp));
        eContactType = CONTACT_AUTO;
        eTransportType = TRANSPORT_UDP;
        id = SIPX_AUTOMATIC_CONTACT_ID;
        iPort = -1;
    }
    int                 cbSize;		 /**< Size of structure         */
    SIPX_CONTACT_ID     id;              /**< Contact record Id      */
    SIPX_CONTACT_TYPE   eContactType ;   /**< Address type/source    */
    SIPX_TRANSPORT_TYPE eTransportType ; /**< Contact transport type */
    char                cInterface[MAX_ADAPTER_NAME_LENGTH + 4] ; /**< Source interface if available */
    char                cInterfaceIp[28]; ///< interface ip address if available
    char                cIpAddress[28] ; /**< contact IP Address    */
    int                 iPort ;          /**< contact Port         */
};

/**
 * RTCP statistics computed according to RFC 3550
 */
typedef struct 
{
   int cbSize;						/**< Size of structure */

	unsigned short fraction_lost;   /**< Fraction of lost packets. */
	unsigned long cum_lost;         /**< Cumulative lost packets. */
	unsigned long ext_max;          /**< Max size of RTCP extension header. */
	unsigned long jitter;           /**< Jitter measurement. */
	int RTT;                        /**< Round trip time. */
	int bytesSent;                  /**< Number of bytes sent. */
	int packetsSent;                /**< Number of packets sent. */
	int bytesReceived;              /**< Number of bytes received. */
	int packetsReceived;            /**< Number of packets received. */
} SIPX_RTCP_STATS;

/**
* The SIPX_AUDIO_DEVICE structure holds information about audio device.
*/
typedef struct  
{
#define SIPXTAPI_AUDIO_DEVICE_STRLEN 80
   char deviceName[SIPXTAPI_AUDIO_DEVICE_STRLEN]; ///< device name
   char driverName[SIPXTAPI_AUDIO_DEVICE_STRLEN]; ///< name of driver for this device
   int maxChannels; ///< maximum channels supported
   double defaultSampleRate; ///< default sample rate
   int bIsInput; ///< whether it is input device
} SIPX_AUDIO_DEVICE;

/**
* The SIPX_AUDIO_CODEC structure includes codec name and bandwidth info.
*/
typedef struct 
{
   int payloadType; /**< Codec payload id used in SDP. */
   char cCodecName[SIPXTAPI_STRING_MEDIUM_LENGTH];  /**< Codec short name. Used for codec selection. Uniquely identifies codec. */
   char cDisplayName[SIPXTAPI_STRING_MEDIUM_LENGTH]; /**< Name of codec that should be displayed in dialogs */
   char cSubMimeType[SIPXTAPI_STRING_MEDIUM_LENGTH]; /**< Submime type used in SDP. */
   SIPX_AUDIO_BANDWIDTH_ID bandWidth; /**< Bandwidth requirement */
   int sampleRate; /**< Codec sample rate in Hz */
   int frameLength; /**< Frame length in milliseconds */
   int numChannels; /**< Number of channels, 1 - mono */
   char cFormatSpecificData[SIPXTAPI_STRING_MEDIUM_LENGTH]; /**< Codec specific parameters, like mode=6 */
   SIPX_CODEC_CPU_COST cpuCost; /**< CPU consumption of codec */
} SIPX_AUDIO_CODEC;

/**
 * The SIPX_VIDEO_CODEC structure includes codec name and bandwidth info.
 */
typedef struct 
{
   int payloadType; /**< Codec payload id used in SDP. */
   char cCodecName[SIPXTAPI_STRING_MEDIUM_LENGTH];  /**< Codec name    */
   char cDisplayName[SIPXTAPI_STRING_MEDIUM_LENGTH]; /**< Name of codec that should be displayed in dialogs */
   char cSubMimeType[SIPXTAPI_STRING_MEDIUM_LENGTH]; /**< Submime type used in SDP. */
   SIPX_VIDEO_BANDWIDTH_ID bandWidth;  /**< Bandwidth requirement */
   char cFormatSpecificData[SIPXTAPI_STRING_MEDIUM_LENGTH]; /**< Codec specific parameters, like mode=6 */
   int cpuCost; /**< CPU consumption of codec */
} SIPX_VIDEO_CODEC;


/**
 * In the MEDIA_LOCAL_START and MEDIA_REMOTE_START events the SIPX_CODEC_INFO 
 * structure is being passed up to the event handler and contains information 
 * about the negotiated audio and video codec.
 */
typedef struct
{
   char cAudioCodecName[SIPXTAPI_STRING_MEDIUM_LENGTH];  /**< Short audio codec name. Example: PCMA */
   char cVideoCodecName[SIPXTAPI_STRING_MEDIUM_LENGTH];  /**< Short video codec name */
   int bIsEncrypted;                /**< 1 if SRTP is enabled */
} SIPX_CODEC_INFO;

/**
 * This structure gets passed into sipxCallConnect, sipxCallAccept, and
 * sipxConferenceAdd calls and sets options on a per call basis.
 */
typedef struct
{
    int cbSize;                          /**< Size of structure          */
    int sendLocation;                    /**< True sends HTTP location header */
    SIPX_CONTACT_ID contactId;           /**< desired contactId (only used for 
                                              sipxCallAccept at this moment) 
                                              pass 0 for automatic contact. Real contact
                                              IDs start with 1 */
    SIPX_TRANSPORT_TYPE transportType;   /**< Transport to use for call. When contactId identifies
                                              an automatic contact, then specified transport will be
                                              used. If contactId identifies a specific contact then
                                              this transport must match that of contact. */
    /*
     * NOTE: When adding new data to this structure, please always add it to
     *       the end.  This will allow us to maintain some drop-in 
     *       backwards compatibility between releases.
     */
} SIPX_CALL_OPTIONS;


/** 
 * The SIPX_INST handle represents an instance of a user agent.  A user agent 
 * includes a SIP stack and media processing framework.  sipXtapi does support 
 * multiple instances of user agents in the same process space, however, 
 * certain media processing features become limited or ambiguous.  For 
 * example, only one user agent should control the local system's input and 
 * output audio devices. */
typedef void* SIPX_INST;         
const SIPX_INST SIPX_INST_NULL = 0; /**< Represents a null instance handle */

/** 
 * The SIPX_LINE handle represents an inbound or outbound identity.  When 
 * placing outbound the application programmer must define the outbound 
 * line.  When receiving inbound calls, the application can query the 
 * line.
 */
typedef unsigned int SIPX_LINE;
const SIPX_LINE SIPX_LINE_NULL = 0; /**< Represents a null line handle */

/** 
 * The SIPX_CALL handle represents a call or connection between the user 
 * agent and another party.  All call operations require the call handle
 * as a parameter.
 */
typedef unsigned int SIPX_CALL;
const SIPX_CALL SIPX_CALL_NULL = 0; /**< Represents a null call handle */

/** 
 * The SIPX_CONF handle represents a collection of CALLs that have bridge
 * (mixed) audio.  Application developers can manipulate each leg of the 
 * conference through various conference functions.
 */
typedef unsigned int SIPX_CONF;
const SIPX_CONF SIPX_CONF_NULL = 0; /**< Represents a null conference handle */

/**
 * The SIPX_PUB handle represent a publisher context.  Publisher are used
 * to publish application-data to interested parties (Subscribers).  This
 * maps directly to the SIP SUBSCRIBE, and NOTIFY methods.  The handle is
 * used to mange the life cycle of the publisher.
 *
 * SIPX_PUB handles are created by using sipxCreatePublisher.
 * SIPX_PUB handles should be torn down using sipxDestroyPublisher.
 */
typedef unsigned int SIPX_PUB;

const SIPX_PUB SIPX_PUB_NULL = 0; /**< Represents a null publisher handle */

/**
 * A SIPX_SUB handle represent a subscription to a remote publisher.  This
 * maps directly to the SIP SUBSCRIBE, and NOTIFY methods.  The handle is 
 * used to mange the life cycle of the subscription.
 *
 * SIPX_SUB handles are created by using the sipxCallSubscribe function.
 * SIPX_SUB handles should be destroyed using the sipxCallUnsubscribe function.
 */
typedef unsigned int SIPX_SUB;

/**
 * SIPX_KEEPALIVE_TYPEs define different methods of keeping NAT/firewall
 * port open.   These approaches are used for the signaling path of a call
 * and are generally only needed under specific network configurations.
 * 
 * Examples: - When not using a proxy
 *           - When the registration period is longer then NAT bindings 
 *             timeout
 *
 * The STUN, and SIP_OPTIONS events may also give you more 
 * information about your network NAT mappings.  When you add a keepalive,
 * you may get KEEPALIVE_FEEDBACK events with the IP/port that your
 * peer thinks is you.  For STUN, this comes from the STUN response, for 
 * the SIP keepalives, this comes from the "via" response if the remote
 * supports rport/symmetric signaling.
 */
typedef enum
{
    SIPX_KEEPALIVE_CRLF = 0,    /**<Send a Carriage Return/Line Feed to other side */
    SIPX_KEEPALIVE_STUN,        /**<Send a Stun request to the other side */
    SIPX_KEEPALIVE_SIP_OPTIONS, /**<Send a SIP OPTIONS method request to the other side */
} SIPX_KEEPALIVE_TYPE;

/** 
 * SIPX_AEC_MODE defines different AEC modes.  Options included DISABLED,
 * SUPPRESS, CANCEL, and CANCEL_AUTO.
 *
 * NOTE: This functionally is only supported when sipXtapi is bundled with
 * VoiceEngine from Global IP Sound or Speex library.
 * 
 * NOTE: SIPX_AEC_SUPPRESS is not available if Speex library used.
 *
 * NOTE: Keep in sync MEDIA_AEC_MODE with SIPX_AEC_MODE
 */
typedef enum SIPX_AEC_MODE
{
    SIPX_AEC_DISABLED,   /**<Disabled AEC; do not attempt to cancel or 
                             suppress echo */
    SIPX_AEC_SUPPRESS,   /**<Echo suppression; attempt to suppress echo by
                             effectively forcing a half-duplex audio channel.
                             If you are speaking, the speaker will be silenced
                             to avoid echo.  Echo cancellation is consider a 
                             better approach/experience, however, requires more
                             CPU consumption.  Not supported with Speex.*/
    SIPX_AEC_CANCEL,     /**<Full echo cancellation; attempt to cancel echo 
                             between the the speaker and microphone.  Depending
                             on the quality of your speaker/microphone, this 
                             may result in some suppression.  For example, if 
                             either the speaker or microphone distorts the 
                             signal (making it non-linear), it is becomes 
                             increasingly difficult to cancel.  This is 
                             consider a full-duplex solution. */
    SIPX_AEC_CANCEL_AUTO,/**<Full echo cancellation; attempt to cancel echo 
                             between the the speaker and microphone; however,
                             automatically disable echo cancellation if it
                             appears not needed. With Speex has the same effect
                             as SIPX_AEC_CANCEL */

} SIPX_AEC_MODE;


/**
 * SIPX_NOISE_REDUCTION_MODE defines the various noise reduction options.  
 * Options include, DISABLED, LOW, MEDIUM, and HIGH.  When selecting a 
 * noise reduction level, you are trading off reducing back ground noise
 * with the possibility of suppressing speech.  We recommend selecting the
 * LOW level.
 *
 * NOTE: This functionally is only supported when sipXtapi is bundled with
 * VoiceEngine from Global IP Sound or Speex library.
 * 
 * NOTE: When Speex library is used there is no difference between
 *       SIPX_NOISE_REDUCTION_LOW, SIPX_NOISE_REDUCTION_MEDIUM and
 *       SIPX_NOISE_REDUCTION_HIGH.
 *
 * NOTE: Keep in sync MEDIA_NOISE_REDUCTION_MODE with SIPX_NOISE_REDUCTION_MODE
 */
typedef enum SIPX_NOISE_REDUCTION_MODE
{
    SIPX_NOISE_REDUCTION_DISABLED,  /**< Disable NR; Do not attempt to reduce 
                                         background noise */
    SIPX_NOISE_REDUCTION_LOW,       /**< Enable NR with least amount of 
                                         aggressiveness. */
    SIPX_NOISE_REDUCTION_MEDIUM,    /**< Enable NR with modest amount of 
                                         aggressiveness. */
    SIPX_NOISE_REDUCTION_HIGH,      /**< Enable NR with highest amount of 
                                         aggressiveness. */
} SIPX_NOISE_REDUCTION_MODE;

/* ============================ FUNCTIONS ================================= */

/** @name Initialization */
//@{


/** 
 * Initialize the sipX TAPI-like API layer.  This method initialized the
 * basic SIP stack and media process resources and must be called before 
 * any other sipxXXX methods.  Additionally, this method fills in a 
 * SIPX_INST parameter which must be passed to a number of sipX methods.
 *
 * @param phInst A pointer to a hInst that must be various other
 *        sipXtapi routines. 
 * @param udpPort The default UDP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.  Right now, 
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of 
 *        SIPX_PORT_DISABLE (-1) to disable disable UDP or a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open UDP port.
 * @param tcpPort The default TCP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.    Right now, 
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of 
 *        SIPX_PORT_DISABLE (-1) to disable disable TCP or a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open TCP port.
 * @param tlsPort **NOT YET SUPPORTED**
 * @param rtpPortStart The starting port for inbound RTP traffic.  The
 *        sipX layer will use ports starting at rtpPortStart and ending
 *        at (rtpPortStart + 2 * maxConnections) - 1.  Pass a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open port.
 * @param maxConnections The maximum number of simultaneous connections
 *        that the sipX layer will support.
 * @param szIdentity The default outbound identity used by the SIP stack
 *        if no lines are defined. Generally, the szIdentity is only used
 *        for inbound calls since all of sipXtapi APIs required a line ID
 *        for outbound calls.  The identity will be used to form the 
 *        "From" field (caller-id) and the username/URL parameters are 
 *        may be used as part of the "Contact" header.  In other words,
 *        this field does not impact any routing aspects of the call
 *        session.
 * @param szBindToAddr Defines which IP/address the user agent / rtp 
 *        stack will listen on.  The default "0.0.0.0" listens on all
 *        interfaces.  The address must be in dotted decimal form -- 
 *        hostnames will not work.
 * @param bUseSequentialPorts If unable to bind to the udpPort, tcpPort, 
 *        or tlsPort, try sequential ports until a successful port is 
 *        found.  If enabled, sipXtapi will try 10 sequential port 
 *        numbers after the initial port.
 */
SIPXTAPI_API SIPX_RESULT sipxInitialize(SIPX_INST* phInst,
                                        const int udpPort = DEFAULT_UDP_PORT,
                                        const int tcpPort = DEFAULT_TCP_PORT,
                                        const int tlsPort = DEFAULT_TLS_PORT,
                                        const int rtpPortStart = DEFAULT_RTP_START_PORT,
                                        const int maxConnections = DEFAULT_CONNECTIONS,
                                        const char* szIdentity = DEFAULT_IDENTITY,
                                        const char* szBindToAddr = DEFAULT_BIND_ADDRESS,
                                        int         bUseSequentialPorts = 0);


/** 
 * Re-initialize the sipX TAPI-like API layer.  This method will remove all lines,
 * conferences, calls, publishers, and subscribers, and listeners.
 * Before calling this function, the application should unregister all registered 
 * lines.
 *
 * You should also reset any of your configuration settings (rport, outbound proxy, etc).
 *
 * @param phInst A pointer to a SIPX_INST variable.  Your old SIPX_INST
 *        handle will be invalid after this call.  Calling any routines
 *        with the old SIPX_INST variable is undefined and may result in
 *        an exception.
 * @param udpPort The default UDP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.  Right now, 
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of 
 *        SIPX_PORT_DISABLE (-1) to disable disable UDP or a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open UDP port.
 * @param tcpPort The default TCP port for the SIP protocol stack.  The
 *        port cannot be changed after initialization.    Right now, 
 *        the UDP port and TCP port numbers MUST be equal.  Pass a value of 
 *        SIPX_PORT_DISABLE (-1) to disable disable TCP or a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open TCP port.
 * @param tlsPort **NOT YET SUPPORTED**
 * @param rtpPortStart The starting port for inbound RTP traffic.  The
 *        sipX layer will use ports starting at rtpPortStart and ending
 *        at (rtpPortStart + 2 * maxConnections) - 1.  Pass a value of 
 *        SIPX_PORT_AUTO (-2) to automatically select an open port.
 * @param maxConnections The maximum number of simultaneous connections
 *        that the sipX layer will support.
 * @param szIdentity The default outbound identity used by the SIP stack
 *        if no lines are defined. Generally, the szIdentity is only used
 *        for inbound calls since all of sipXtapi APIs required a line ID
 *        for outbound calls.  The identity will be used to form the 
 *        "From" field (caller-id) and the username/URL parameters are 
 *        may be used as part of the "Contact" header.  In other words,
 *        this field does not impact any routing aspects of the call
 *        session.
 * @param szBindToAddr Defines which IP/address the user agent / rtp 
 *        stack will listen on.  The default "0.0.0.0" listens on all
 *        interfaces.  The address must be in dotted decimal form -- 
 *        hostnames will not work.
 * @param bUseSequentialPorts If unable to bind to the udpPort, tcpPort, 
 *        or tlsPort, try sequential ports until a successful port is 
 *        found.  If enabled, sipXtapi will try 10 sequential port 
 *        numbers after the initial port.
 */
SIPXTAPI_API SIPX_RESULT sipxReInitialize(SIPX_INST* phInst,
                                          const int udpPort = DEFAULT_UDP_PORT,
                                          const int tcpPort = DEFAULT_TCP_PORT,
                                          const int tlsPort = DEFAULT_TLS_PORT,
                                          const int rtpPortStart = DEFAULT_RTP_START_PORT,
                                          const int maxConnections = DEFAULT_CONNECTIONS,
                                          const char* szIdentity = DEFAULT_IDENTITY,
                                          const char* szBindToAddr = DEFAULT_BIND_ADDRESS,
                                          int         bUseSequentialPorts = 0);

/** 
 * Uninitialize the sipX TAPI-like API layer.  This method tears down the
 * basic SIP stack and media process resources and should be called before 
 * exiting the process.  Users are responsible for ending all calls and 
 * unregistering line appearances before calling sipxUnInitialize.  Failing
 * to end calls/conferences or remove lines will result in a 
 * SIPX_RESULT_BUSY return code. All event listeners for this sipxtapi
 * instance are automatically unregistered.
 *
 * @param hInst An instance handle obtained from sipxInitialize. 
 * @param bForceShutdown forces sipXtapi to shutdown regardless of live 
 *        calls/unregistered lines.  Enabling this in NOT RECOMMENDED,
 *        please tear down all calls and lines prior to calling 
 *        sipxUnitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxUnInitialize(SIPX_INST hInst, int bForceShutdown = 0);

//@}
/** @name Call Methods */
//@{

/**
 * Accepts an inbound call and proceed immediately to alerting.  This method
 * is invoked in response to a NEWCALL event.  Whenever a new call is received,
 * the application developer should ACCEPT (proceed to ringing), REJECT (send
 * back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param options Pointer to a SIPX_CALL_OPTIONS structure.
 * @param bSendSdp Flag to send SDP in 180 Ringing response, resulting in
 *        early media being sent/received. Either SDP offer or answer will be sent
 *        depending on SDP negotiation state. This flag will be ignored if INVITE
 *        didn't contain SDP and reliable provisional response (100rel) is required.
 *        In that case SDP offer will be sent in 18x response.
 *
 * @see sipxConfigSetLocationHeader
 * @see sipxConfigSetAudioCodecPreferences
 */
SIPXTAPI_API SIPX_RESULT sipxCallAccept(const SIPX_CALL hCall, 
                                        SIPX_CALL_OPTIONS* options = NULL,
                                        int bSendSdp = 0);


/**
 * Reject an inbound call (prior to alerting the user).  This method must
 * be invoked before the end user is alerted (before sipxCallAccept).
 * Whenever a new call is received, the application developer should ACCEPT 
 * (proceed to ringing), REJECT (send back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallReject(const SIPX_CALL hCall);


/**
 * Redirect an inbound call (prior to alerting the user).  This method must
 * be invoked before the end user is alerted (before sipxCallAccept).
 * Whenever a new call is received, the application developer should ACCEPT 
 * (proceed to ringing), REJECT (send back busy), or REDIRECT the call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szForwardURL SIP url to forward/redirect the call to.
 */
SIPXTAPI_API SIPX_RESULT sipxCallRedirect(const SIPX_CALL hCall,
                                          const char* szForwardURL);

/**
 * Answer an alerting call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param bTakeFocus Should SIPxua place the newly answered call in focus
 *        (engage local microphone and speaker).  In some cases, application
 *        developer may want to answer the call in the background and play
 *        audio while the user finishes up with their active (in focus) call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAnswer(const SIPX_CALL hCall, 
                                        int bTakeFocus = 1);

/**
 * Accepts inbound call transfer request. Call which progresses to
 * CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER state will contain
 * szReferredBy and szReferTo fields in SIPX_CALLSTATE_INFO structure.
 * Based on this information, a decision should be made whether to accept
 * or reject the transfer request. This decision must be made immediately.
 *
 * @param hCall Handle to a call. Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAcceptTransfer(const SIPX_CALL hCall);

/**
 * Rejects inbound call transfer request.
 *
 * @param hCall Handle to a call. Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallRejectTransfer(const SIPX_CALL hCall);

/**
 * Create a new call for the purpose of creating an outbound connection/call.
 * As a side effect, a DIALTONE event is fired to simulate the PSTN world.
 * Generally an application would simulate dialtone in reaction to that
 * event.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param hLine Line Identity for the outbound call.  The line identity 
 *        helps defines the "From" caller-id.
 * @param phCall Pointer to a call handle.  Upon success, this value is
 *        replaced with a valid call handle.  Success is determined by
 *        the SIPX_RESULT result code.
 */
SIPXTAPI_API SIPX_RESULT sipxCallCreate(const SIPX_INST hInst, 
                                        const SIPX_LINE hLine,
                                        SIPX_CALL* phCall);

/**
 * Create a new call for the purpose of creating an outbound connection/call.
 * As a side effect, a DIALTONE event is fired to simulate the PSTN world.
 * Generally an application would simulate dialtone in reaction to that
 * event. Use this function only if you need to dial from a virtual line
 * that you don't want to register with sipxtapi.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szLine Arbitrary Line Identity for the outbound call.  The line identity 
 *        defines the "From" caller-id. If this line is not registered with sipxtapi
 *        SIPX_LINE_NULL will be returned in all call events associated with this call.
 *        "transport" parameter is stripped from szLine, since there is different transport
 *        selection logic.
 * @param phCall Pointer to a call handle.  Upon success, this value is
 *        replaced with a valid call handle.  Success is determined by
 *        the SIPX_RESULT result code.
 */
SIPXTAPI_API SIPX_RESULT sipxCallCreateOnVirtualLine(const SIPX_INST hInst,
                                                     const char* szLine,
                                                     SIPX_CALL* phCall);

/**
 * Connects an idle call to the designated target address
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAddress SIP url of the target party
 * @param takeFocus Should SIPxua place the this call in focus (engage 
 *        local microphone and speaker).  In some cases, application developer
 *        may want to place the call in the background and play audio while 
 *        the user finishes up with their active (in focus) call. 
 * @param options Pointer to a SIPX_CALL_OPTIONS structure. It contains
 *        contactId - Id of the desired contact record to use for this call.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a 
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or 
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 *        The Contact Record's eTransportType field indicates the 
 *        type of network transport to be used for call signalling.
 *        this can be UPD, TCP, or TLS.  If the eTransportType field value
 *        is greater than 3, this indicates that a custom EXTERNAL TRANSPORT
 *        mechanism is to be used, and the value of the eTransportType field
 *        indicates the SIPX_TRANSPORT handle associated with the EXTERNAL
 *        TRANSPORT.
 * @param szCallId A call-id for the session, if NULL, one is generated.
 *
 * @see sipxConfigSetLocationHeader
 * @see sipxConfigSetAudioCodecPreferences
 */
SIPXTAPI_API SIPX_RESULT sipxCallConnect(const SIPX_CALL hCall,
                                         const char* szAddress,
                                         SIPX_FOCUS_CONFIG takeFocus = SIPX_FOCUS_ALWAYS,
                                         SIPX_CALL_OPTIONS* options = NULL,
                                         const char* szSessionCallId = NULL);

/**
 * Place the specified call on hold.  When placing calls on hold or
 * having a remote party place you on hold the event sequences will
 * differ.  In this documentation, we refer to "local" hold and/or 
 * "focus" and "remote" and/or "full" hold.  A call is on local hold 
 * when that call is taken out of focus and is no longer connected to
 * the local microphone and speaker.  Remote hold is used to indicate 
 * that RTP is no longer flowing between parties.  The "bRemoteStopAudio" 
 * flags to this method controls whether the party is placed on local 
 * hold or full hold.  See the table below for expected events:
 *
 *<pre>
 *                   RTP Flowing  RTP Stopped
 *                   ---------    -----------
 *       In Focus    CONNECTED    REMOTE_HELD
 *   Out of Focus     BRIDGED        HELD
 *</pre>
 *
 * CONNECTED indicates that both RTP is flowing and the call is attached
 *    to the local.  This is the normal state for a connected call.
 *
 * BRIDGED indicates that RTP is flowing, but the call is out of focus.
 *    This event is generally caused by holding a conference (conference 
 *    will bridge by default) or if you accept/place a new call without
 *    explicitly holding the active call.
 *
 * REMOTE_HELD indicates that RTP has stopped flowing.  This is generally
 *    caused when the remote side places you on hold.  The call is still
 *    locally in focus and audio will automatically resume once your are 
 *    take off remote hold.
 *
 * HELD indicates that both RTP has stopped flowing and the call is out
 *    of focus.
 *
 * Developers can also expect media events (e.g. MEDIA_LOCAL_STOP) 
 * whenever RTP is stopped (REMOTE_HELD and HELD).  Since media is still 
 * flowing for CONNECTED and BRIDGED, no media stop events are sent.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param bStopRemoteAudio Flag which controls whether sipXtapi takes
 *        the call out of focus (stops engaging local audio microphone
 *        and speaker) or stops sending/receiving audio.  Specify true
 *        to stop audio (default) or false to take the call out of 
 *        focus.  To play audio or generate tones to a remote connection
 *        while on hold, please specify false.  This parameter is 
 *        ignored (and assumed true) if the call is part of a conference.
 */ 
SIPXTAPI_API SIPX_RESULT sipxCallHold(const SIPX_CALL hCall, 
                                      int bStopRemoteAudio = 1);


/**
 * Take the specified call off hold. If bTakeFocus is true, call will
 * be taken into focus engaging mic/speaker.
 * 
 * @see sipxCallHold for a description of expected events 
 *      associated with hold events.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param bTakeFocus Flag which controls whether sipXtapi takes the call
 *        into focus engaging mic/speaker.
 */ 
SIPXTAPI_API SIPX_RESULT sipxCallUnhold(const SIPX_CALL hCall,
                                        int bTakeFocus = 1);


/**
 * Drop/Destroy the specified call.
 *
 * If a sipxCallDestroy is invoked while an audio buffer is playing,
 * playback will stop automatically. In that case MEDIA_PLAYBUFFER_STOP
 * is not fired.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */ 
SIPXTAPI_API SIPX_RESULT sipxCallDestroy(SIPX_CALL* hCall);

/**
 * Initiates RTP redirect between two calls. SipXtapi will no longer send or receive RTP, and all audio
 * resources will be released. Source call will use SDP of remote party of destination call and vice versa.
 * This will result in source and destination calls to send RTP directly to each other, bypassing sipXtapi.
 *
 * Because both parties will talk directly to each other, they both must have public IP addresses.
 * SipXtapi will keep control over SIP signalling and will be able to drop both calls.
 *
 * If operation is successful, the following event sequence can be expected:
 * - RTP_REDIRECT_REQUESTED
 * - RTP_REDIRECT_ACTIVE
 *
 * If operation fails before RTP redirect is activated, RTP_REDIRECT_ERROR event will be reported.
 * If RTP redirect succeeds but later cannot continue due to attached call being dropped or re-INVITE codec
 * renegotiation failure, RTP_REDIRECT_STOP will be fired with proper event cause.
 *
 * RTP_REDIRECT_STOP will be fired only after RTP_REDIRECT_ACTIVE.
 *
 * If one of calls participating in RTP redirect is destroyed, the other call automatically stops redirecting
 * RTP. Operation will fail if one of calls has RTP redirect request pending, RTP redirect is active or
 * call is in conference.
 *
 * NOT FULLY IMPLEMENTED
 *
 * @param hSrcCall Call handle of source call. Must be in established state.
 * @param hDstCall Call handle of destination call. Must be in established state and belong to the same
 *        sipXtapi instance like hSrcCall.
 */
//SIPXTAPI_API SIPX_RESULT sipxCallStartRtpRedirect(const SIPX_CALL hSrcCall, const SIPX_CALL hDstCall);

/**
 * Stops redirecting RTP on given call, and its attached call. Causes initialization
 * of local audio resources and re-INVITE with local codecs. It is not necessary to call
 * this function if sipxCallStartRtpRedirect failed, or the RTP redirection fails at some point.
 *
 * This operation is executed asynchronously. Immediate SIPX_RESULT_SUCCESS merely means
 * that call was found and command dispatched. Success will be confirmed by RTP_REDIRECT_STOP event.
 *
 * Must be called only for a single call participating in RTP redirect. Other call will be notified
 * automatically and stop RTP redirect. Operation will fail if RTP redirect has been requeted (but not
 * activated) or is already inactive.
 *
 * NOT FULLY IMPLEMENTED
 *
 * @param hCall Handle to a call participating in RTP redirect. May be source or destination
 *        call from sipxCallStartRtpRedirect.
 */
//SIPXTAPI_API SIPX_RESULT sipxCallStopRtpRedirect(const SIPX_CALL hCall);

/**
 * Get the SIP Call-Id of the call represented by the specified call handle.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szSipCallId Buffer to store the call-id.  A zero-terminated string will be 
 *        copied into this buffer on success. Empty string will be passed
 *        if call is not connected.
 * @param iMaxLength Max length of the szSipCallId buffer
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetSipCallId(const SIPX_CALL hCall,
                                              char* szSipCallId, 
                                              const size_t iMaxLength);

/**
 * Get the SIP identity of the local connection. It may contain a tag.
 * This value is taken from SIP message from field for outbound calls,
 * and to field for inbound calls.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szLocalField Buffer to store the ID.  A zero-terminated string will be 
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the ID buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetLocalField(const SIPX_CALL hCall, 
                                               char* szLocalField, 
                                               const size_t iMaxLength);


/**
 * Get the SIP identity of the remote connection. It may contain a tag.
 * This value is taken from SIP message to field for outbound calls,
 * and from field for inbound calls.
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szRemoteField Buffer to store the ID.  A zero-terminated string will be 
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the ID buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteField(const SIPX_CALL hCall, 
                                                char* szRemoteField, 
                                                const size_t iMaxLength);

/**
 * Get the SIP address of local contact.  The identity represents
 * the originator of the message, and is sent in contact SIP message field.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szContactAddress Buffer to store the ID.  A zero-terminated string will be 
 *        copied into this buffer on success.
 * @param iMaxLength Max length of the ID buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetLocalContact(const SIPX_CALL hCall, 
                                                 char* szContactAddress, 
                                                 const size_t iMaxLength);

/**
 * Gets the media connection ID. For testing only.
 *
 * Media connection ID is an internal identifier used to manage media
 * connections of sipXmediaAdapterLib.
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param connectionId Reference to the returned connection identifier.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetMediaConnectionId(const SIPX_CALL hCall,
                                                      int* connectionId);


/**
 * Get the conference handle for the specified call
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * 
 * @param hConf Conference handle for this call (if the call is part of a
 *        conference)
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetConference(const SIPX_CALL hCall,
                                               SIPX_CONF* hConf);
                                                 

/**
 * Get the SIP request URI that initiated the call. It is retrieved either
 * from sent or received SIP INVITE depending on whether it is an outbound
 * or inbound call.
 *
 * If outbound call was redirected, SIP request URI will contain SIP URI
 * of the correct party. For redirected calls, sipxCallGetRemoteField
 * and sipxCallGetRequestURI will yield different results, with sipxCallGetRequestURI
 * being correct.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szRequestUri Buffer to store the request URI.  A zero-terminated string will
 *        be copied into this buffer on success.
 * @param iMaxLength Max length of the request URI buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRequestURI(const SIPX_CALL hCall, 
                                               char* szRequestUri, 
                                               const size_t iMaxLength);



/**
 * Get the SIP remote contact. It may contain a display name.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szContactAddress Buffer to store the remote contact.  A zero-terminated
 *        string will be copied into this buffer on success.
 * @param iMaxLength Max length of the remote contact buffer.
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteContact(const SIPX_CALL hCall, 
                                                  char* szContactAddress, 
                                                  const size_t iMaxLength);

                                                           
/**
 * Get the remote user agent of the call represented by the specified call handle.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAgent Buffer to store the user agent name.  A zero-terminated string
 *        will be copied into this buffer on success.
 * @param iMaxLength Max length of the buffer
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteUserAgent(const SIPX_CALL hCall,
                                                    char* szAgent, 
                                                    const size_t iMaxLength);


/**
 * Play a tone (DTMF, dialtone, ring back, etc) to the local and/or
 * remote party.  See the DTMF_ constants for built-in tones.
 *
 * DTMF is sent via RFC 2833 method or in-band. DTMF method is configured
 * via sipxConfigSetOutboundDTMFMode function. sipxCallDestroy stops tones automatically.
 * Minimum DTMF length must be 60msec.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param toneId ID of the tone to play
 * @param bLocal Should the tone be played locally? 
 * @param bRemote Should the tone be played to the remote party?
 * @param duration Tone duration in milliseconds. Minimum value is 60. Use -1 for manual stop.
 */
SIPXTAPI_API SIPX_RESULT sipxCallStartTone(const SIPX_CALL hCall, 
                                           const SIPX_TONE_ID toneId,
                                           const int bLocal,
                                           const int bRemote,
                                           const int duration = 120);


/**
 * Stop playing a tone (DTMF, dialtone, ring back, etc). to local
 * and remote parties. sipxCallDestroy stops tones automatically.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallStopTone(const SIPX_CALL hCall);

/**
 * Play the designated file to remote call participant.
 * The file must be a WAV file 8bit unsigned or 16bit signed, mono or stereo.
 * Stereo files will be merged to mono.
 *
 * Sampling rate of supplied file will be automatically resampled to internal
 * sampling rate of sipXmediaLib. If wideband audio is enabled, then internal sampling
 * rate is 48Khz, otherwise it is 8Khz. If sipXtapi is not bundled with Speex library,
 * then only downsampling is supported.
 *
 * If a sipxCallDestroy is attempted while an audio file is playing,
 * sipxCallDestroy will fail with a SIPX_RESULT_BUSY return code.
 * Call sipxCallAudioPlayFileStop before making the call to
 * sipxCallDestroy.
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played in the
 *        context of a call.
 * @param szFile Filename for the audio file to be played.
 * @param bRepeat True if the file is supposed to be played repeatedly
 * @param bLocal True if the audio file is to be rendered locally.
 * @param bRemote True if the audio file is to be rendered by the remote
 *                endpoint.
 * @param bMixWithMicrophone True to mix the audio with the microphone
 *        data or false to replace it.  This option is only supported 
 *        when sipXtapi is bundled with GIPS VoiceEngine.
 * @param fVolumeScaling Volume down scaling for the audio file.  Valid 
 *        values are between 0 and 1.0, where 1.0 is the no down-scaling.
 *        This option is only supported when sipXtapi is bundled with GIPS
 *        VoiceEngine.
 * @param pCookie Custom parameter passed in playback events to help resolving
 *               problem with identifying correct file when
 *               sipxCallAudioPlayFileStart is called multiple times very fast.
*/
SIPXTAPI_API SIPX_RESULT sipxCallAudioPlayFileStart(const SIPX_CALL hCall, 
                                                    const char* szFile,
                                                    const int bRepeat,
                                                    const int bLocal,
                                                    const int bRemote,
                                                    const int bMixWithMicrophone = 0,
                                                    const float fVolumeScaling = 1.0,
                                                    void* pCookie = NULL);

/**
 * Stop playing a file started with sipxCallPlayFileStart
 * If a sipxCallDestroy is attempted while an audio file is playing,
 * sipxCallDestroy will fail with a SIPX_RESULT_BUSY return code.
 * Call sipxCallAudioPlayFileStop before making the call to
 * sipxCallDestroy.
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played and stopped
 *        in the context of a call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAudioPlayFileStop(const SIPX_CALL hCall); 

/**
 * Pause playing file or buffer on given call. Prior to attempting pause,
 * playback on call must be started by sipxCallAudioPlayFileStart or
 * sipxCallPlayBufferStart. SipXtapi can play only 1 buffer/file at time
 * on a call. MEDIA_PLAYBACK_PAUSED event will be received if pause was
 * successful.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAudioPlaybackPause(const SIPX_CALL hCall);

/**
 * Resume playing file or buffer on given call. Prior to attempting resume,
 * playback on call must be started by sipxCallAudioPlayFileStart or
 * sipxCallPlayBufferStart and paused. SipXtapi can play only 1 buffer/file
 * at time on a call. MEDIA_PLAYBACK_RESUMED event will be received if resume
 * was successful.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAudioPlaybackResume(const SIPX_CALL hCall);

/**
 * Record a call session (including other parties if this is a multi-party 
 * call / conference) to a file.  The resulting file will be a PCM WAV file.
 * Conference join operation on this call will cause the recording to stop.
 * In case of conference recording, this function should be called for single
 * conference call only. Conference recording will continue even after the
 * original call had been dropped/split, as long as there is at least one call
 * left in the conference. If the original call has been dropped, use handle
 * of other call in the conference to stop recording.
 *
 * @note Use sipxConferenceAudioRecordFileStart for conference recording
 *       instead of this method.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played and stopped
 *        in the context of a call.
 * @param szFile Filename for the resulting audio file.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordFileStart(const SIPX_CALL hCall,
                                                      const char* szFile);

/**
 * Stop recording a call to file.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio files can only be played and stopped
 *        in the context of a call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordFileStop(const SIPX_CALL hCall);


/**
 * Play the specified audio data. Currently the only data format that
 * is supported is raw 16 bit signed PCM, mono, little endian. Sampling rate
 * must match internal sampling rate of sipXmediaLib. If wideband audio is
 * enabled, then internal sampling rate is 48Khz, otherwise it is 8Khz.
 *
 * If a sipxCallDestroy is invoked while an audio buffer is playing,
 * playback will stop automatically.
 * If call is destroyed but sipxCallPlayBufferStop is not called, MEDIA_PLAYBUFFER_STOP
 * will be missing.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio can only be played in the context
 *        of a call.
 * @param pBuffer Pointer to the audio data to be played.
 * @param bufSize Length, in bytes, of the audio data.
 * @param bufType The audio encoding format for the data as specified
 *                by the SIPX_AUDIO_DATA_FORMAT enumerations.  Currently
 *                only RAW_PCM_16 is supported.
 * @param bRepeat True if the audio is supposed to be played repeatedly
 * @param bLocal True if the audio is to be rendered locally.
 * @param bRemote True if the audio is to be rendered by the remote endpoint.
 * @param pCookie Custom parameter passed in playback events to help resolving
 *               problem with identifying correct buffer when
 *               sipxCallPlayBufferStart is called multiple times very fast.
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStart(const SIPX_CALL hCall,
                                                 const void* pBuffer,
                                                 const int bufSize,
                                                 const int bufType,
                                                 const int bRepeat,
                                                 const int bLocal,
                                                 const int bRemote,
                                                 void* pCookie = NULL);


/**
 * Stop playing the audio started with sipxCallPlayBufferStart
 * If a sipxCallDestroy is invoked while an audio buffer is playing,
 * playback will stop automatically. In that case MEDIA_PLAYBUFFER_STOP
 * is not fired.
 * 
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.  Audio can only be played and stopped
 *        in the context of a call.
 */
SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStop(const SIPX_CALL hCall); 


/**
 * Subscribe for NOTIFY events which may be published by the other end-point 
 * of the call.  This API differs from sipxConfigSubscribe in that it allows
 * you to use the contact address from the remote party as the subscription
 * target (see the bRemoteContactIsGruu parameter).
 *
 * Successful invocation will result in sending SIP SUBSCRIBE message in a new
 * sip dialog separate from the sip dialog of the call.
 *
 * sipXtapi will automatically refresh subscriptions until sipxCallUnsubscribe
 * is called.  Please make sure you call sipxCallUnsubscribe before tearing 
 * down your call, although this is only a recommendation and not a requirement.
 *
 * @param hCall The call handle of the call associated with the subscription.
 *        Used for selection of from and to fields, unless bRemoteContactIsGruu
 *        is true. It doesn't mean subscription is in any way associated with
 *        the call.
 * @param szEventType A string representing the type of event that can be 
 *        published.  This string is used to populate the "Event" header in
 *        the SIP SUBSCRIBE request.  For example, if checking voicemail 
 *        status, your would use "message-summary". For presence, it would be
 *        "presence". Acceptable values can be found in event package RFCs.
 * @param szAcceptType A string representing the types of NOTIFY events that 
 *        this client will accept.  This string is used to populate the 
 *        "Accept" header in the SIP SUBSCRIBE request.  For example, if
 *        checking voicemail status, you would use "application/simple-message-summary".
 *        This value is optional. Check event package RFC for its meaning.
 * @param phSub Pointer to a subscription handle whose value is set by this 
 *        function.  This handle allows you to cancel the subscription and
 *        differentiate between NOTIFY events.
 * @param bRemoteContactIsGruu indicates whether the Contact for the remote 
 *        side of the call can be assumed to be a Globally Routable Unique URI
 *        (GRUU).  Normally one cannot assume that a contact is a GRUU and the
 *        To or From address for the remote side is assumed to be an Address Of
 *        Record (AOR) that is globally routable. 0 value is recommended.
 * @param subscriptionPeriod Subscription expiration period. After this
 *        period, new SUBSCRIBE message will be sent.
 * @param transport Transport to use for sending SUBSCRIBE message
 */                                          
SIPXTAPI_API SIPX_RESULT sipxCallSubscribe(const SIPX_CALL hCall,
                                           const char* szEventType,
                                           const char* szAcceptType,
                                           SIPX_SUB* phSub,
                                           int bRemoteContactIsGruu = 0,
                                           int subscriptionPeriod = 3600,
                                           SIPX_TRANSPORT_TYPE transport = TRANSPORT_AUTO);

/**
 * Unsubscribe from previously subscribed NOTIFY events.  This method will
 * send another subscription request with an expires time of 0 (zero) to end
 * your subscription.  
 *
 * @param hSub The subscription handle obtained from the call to 
 *             sipxCallSubscribe.
 */                                          
SIPXTAPI_API SIPX_RESULT sipxCallUnsubscribe(const SIPX_SUB hSub);


/**
 * Sends an INFO event to the specified call.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szContentType String representation of the INFO content type
 * @param pContent Pointer to the INFO message's content. Can be a NULL terminated
 *        string or binary data.
 * @param nContentLength Length of data in pContent
 * @param pCookie Optional argument that will be passed into info status
 *        event to match send requests with responses.
 */
SIPXTAPI_API SIPX_RESULT sipxCallSendInfo(const SIPX_CALL hCall,
                                          const char* szContentType,
                                          const char* pContent,
                                          const size_t nContentLength,
                                          void* pCookie = NULL);

/**
 * Blind transfer the specified call to another party using REFER (rfc3515).
 * Monitor the TRANSFER state events for details on the transfer attempt.
 *
 * There are three parties involved in blind transfer:
 * 1.) Transfer controller (this user agent) - sends REFER request
 * 2.) Transferee - receives REFER request, accepts or rejects it, creates
 *     new call and drops original call automatically.
 * 3.) Transfer target - receives INVITE request from transferee
 *
 * Received event sequence depends on whether transferee supports sending NOTIFY
 * requests to transfer controller for the duration of call transfer. If NOTIFY
 * is not supported by transferee, then some transfer events will be missing.
 *
 * SIP message flow of unattended transfer can be seen
 * on http://www.tech-invite.com/Ti-sip-service-4.html .
 *
 * Supression of implicit REFER subscription (norefersub - rfc4488) is supported.
 * SipXtapi will never ask for supression of subscription, but approves it if asked
 * in inbound REFER request.
 *
 * <h3>Transferee (party being transfered):</h3>
 *
 * The transfer is implemented as a new call. New call will have different SIPX_CALL
 * handle, and will not be part of any conference even if original call was.
 *
 * <pre>
 * Original Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER
 * Original Call: MEDIA_LOCAL_STOP
 * Original Call: MEDIA_REMOTE_STOP
 * Original Call: CALLSTATE_DISCONNECTED
 * Original Call: CALLSTATE_DESTROYED
 *
 * New Call: CALLSTATE_DIALTONE::CALLSTATE_CAUSE_TRANSFER
 * New Call: CALLSTATE_REMOTE_OFFERING
 * New Call: CALLSTATE_REMOTE_ALERTING
 * New Call: CALLSTATE_CONNECTED
 * New Call: MEDIA_LOCAL_START
 * New Call: MEDIA_REMOTE_START
 * </pre>
 *
 * sipxCallAcceptTransfer or sipxCallRejectTransfer must be called
 * when original call enters CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER
 * state.
 *
 * After the transfer completes, the original call will be destroyed automatically
 * by sipXtapi.
 *
 * <h3>Transfer Controller (this user agent):</h3>
 *
 * The transfer controller will see the following call state sequence:
 *
 * <pre>
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_INITIATED
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_ACCEPTED
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_TRYING
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_RINGING
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_SUCCESS
 * Source Call: CALLSTATE_DISCONNECTED
 * Source Call: MEDIA_LOCAL_STOP
 * Source Call: MEDIA_REMOTE_STOP
 * Source Call: CALLSTATE_DESTROYED
 * </pre>
 *
 * Upon success, the call will be automatically destroyed. Reception of
 * CALLSTATE_CAUSE_TRANSFER_TRYING, CALLSTATE_CAUSE_TRANSFER_RINGING and
 * CALLSTATE_CAUSE_TRANSFER_SUCCESS events will depend on reception of corresponding
 * SIP NOTIFY messages.
 *
 * <h3>Transfer Target (identified by szAddress):</h3>
 *
 * The transfer target will go through the normal event progression for an incoming
 * call:
 *
 * <pre>
 * New Call: CALLSTATE_NEWCALL::CALLSTATE_CAUSE_NORMAL
 * New Call: CALLSTATE_OFFERING::CALLSTATE_CAUSE_NORMAL
 * New Call: CALLSTATE_ALERTING::CALLSTATE_CAUSE_NORMAL
 * New Call: MEDIA_LOCAL_START
 * New Call: MEDIA_REMOTE_START
 * New Call: CALLSTATE_CONNECTED::CALLSTATE_CAUSE_NORMAL
 * </pre>
 *
 * If the transfer target rejects the call or fails to answer, the transfer 
 * will fail. If transfer fails, original calls will return back into
 * CALLSTATE_CONNECTED state.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAddress SIP url identifing the transfer target (who the call
 *        identified by hCall will be transfered to).
 *
 * @see SIPX_CALLSTATE_EVENT
 * @see SIPX_CALLSTATE_CAUSE
 */
SIPXTAPI_API SIPX_RESULT sipxCallBlindTransfer(const SIPX_CALL hCall, 
                                               const char* szAddress);

/**
 * Transfer the source call to the target call.  This method can be used
 * to implement consultative transfer (transfer initiator can speak with 
 * the transfer target prior to transferring).  If you wish to consult 
 * privately, create a new call to the transfer target.  If you wish 
 * consult and allow the source (transferee) to participant in the 
 * conversation, create a conference and then transfer one leg to 
 * another.
 *
 * Consultative transfer is implemented using in dialog REFER (rfc3515) request.
 * Out of dialog transfers are not supported and are always rejected.
 *
 * There are three parties involved in consultative transfer:
 * 1.) Transfer controller (this user agent) - sends REFER request
 * 2.) Transferee - receives REFER request, accepts or rejects it, creates
 *     new call and drops original call automatically.
 * 3.) Transfer target - receives INVITE request from transferee, with Replaces header
 *     set. If INVITE succeeds, referenced call is dropped.
 *
 * Received event sequence depends on whether transferee supports sending NOTIFY
 * requests to transfer controller for the duration of call transfer. If NOTIFY
 * is not supported by transferee, then some transfer events will be missing.
 *
 * SIP message flow of attended transfer can be seen
 * on http://www.tech-invite.com/Ti-sip-service-5.html .
 *
 * Supression of implicit REFER subscription (norefersub - rfc4488) is supported.
 * SipXtapi will never ask for supression of subscription, but approves it if asked
 * in inbound REFER request.
 *
 * SipXtapi does not put calls on hold to speed up the call transfer. If user wishes
 * to place calls on hold, they must be placed on hold using sipxCallHold. User must
 * then wait for CALLSTATE_HELD event before proceeding with attended transfer.
 *
 * <h3>Transfer Controller (this user agent):</h3>
 *
 * <pre>
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_INITIATED
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_ACCEPTED
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_TRYING
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_RINGING
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER_SUCCESS
 * Source Call: MEDIA_LOCAL_STOP
 * Source Call: MEDIA_REMOTE_STOP
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_DISCONNECTED
 * Source Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_DESTROYED
 * </pre>
 *
 * The source call will automatically be destroyed if the transfer is 
 * successful. CALLSTATE_CAUSE_TRANSFER_ACCEPTED will be fired when REFER
 * request is accepted with 202 response. Reception of
 * CALLSTATE_CAUSE_TRANSFER_TRYING, CALLSTATE_CAUSE_TRANSFER_RINGING and
 * CALLSTATE_CAUSE_TRANSFER_SUCCESS events will depend on reception of corresponding
 * SIP NOTIFY messages.
 *
 * <pre>
 * Target Call: MEDIA_LOCAL_STOP
 * Target Call: MEDIA_REMOTE_STOP
 * Target Call: CALLSTATE_DISCONNECTED
 * Target Call: CALLSTATE_DESTROYED
 * </pre>
 * 
 * Target call will be automatically destroyed when remote party hangs up
 * after receiving INVITE with Replaces.
 *
 * <h3>Transferee (user agent on other side of hSourceCall):</h3>
 *
 * The transferee will create a new call to the transfer target and 
 * automatically disconnect the original call upon success.  The new call
 * will be created with a cause of CALLSTATE_CAUSE_TRANSFER in the
 * SIPX_CALLSTATE_INFO event data.
 *
 * <pre>
 * Original Call: CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER
 * Original Call: MEDIA_LOCAL_STOP
 * Original Call: MEDIA_REMOTE_STOP
 * Original Call: CALLSTATE_DISCONNECTED
 * Original Call: CALLSTATE_DESTROYED
 *
 * New Call: CALLSTATE_DIALTONE::CALLSTATE_CAUSE_TRANSFER
 * New Call: CALLSTATE_REMOTE_OFFERING
 * New Call: CALLSTATE_REMOTE_ALERTING
 * New Call: CALLSTATE_CONNECTED
 * New Call: MEDIA_LOCAL_START
 * New Call: MEDIA_REMOTE_START
 * </pre>
 *
 * sipxCallAcceptTransfer or sipxCallRejectTransfer must be called
 * when original call enters CALLSTATE_TRANSFER_EVENT::CALLSTATE_CAUSE_TRANSFER
 * state.
 *
 * Original call will be automatically destroyed if transfer succeeds.
 * If transfer fails, new call will be disconnected and original call
 * will return into CALLSTATE_CONNECTED state.
 *
 * <h3>Transfer Target (user agent on other side of hTargetCall):</h3>
 *
 * The transfer target will receive INVITE request with Replaces header
 * from the transferee.  After this completes, the referenced call is 
 * disconnected. hAssociatedCall member of SIPX_CALLSTATE_INFO will be
 * set to the value of call referenced by Replaces header.
 *
 * <pre>
 * New Call: CALLSTATE_NEWCALL::CALLSTATE_CAUSE_TRANSFERRED
 * New Call: CALLSTATE_OFFERING
 * New Call: CALLSTATE_ALERTING
 * New Call: CALLSTATE_CONNECTED
 * New Call: MEDIA_LOCAL_START
 * New Call: MEDIA_REMOTE_START
 *
 * Referenced Call: MEDIA_LOCAL_STOP
 * Referenced Call: MEDIA_REMOTE_STOP
 * Referenced Call: CALLSTATE_DISCONNECTED
 * Referenced Call: CALLSTATE_DESTROYED
 * </pre>
 *
 * Transfer target may reject the new call. In that case call transfer will fail,
 * and original calls will be reclaimed automatically. If call is accepted and
 * proceeds into CALLSTATE_CONNECTED, referenced call is destroyed automatically.
 *
 * New call will not be part of a local conference.
 *
 * @param hSourceCall Handle to the source call (transferee).
 * @param hTargetCall Handle to the target call (transfer target).
 *
 * @see SIPX_CALLSTATE_EVENT
 * @see SIPX_CALLSTATE_CAUSE
 */
SIPXTAPI_API SIPX_RESULT sipxCallTransfer(const SIPX_CALL hSourceCall,
                                          const SIPX_CALL hTargetCall);



/**
 * Updates the Video window with a new frame buffer.  Should be called
 * when the window receives a PAINT message.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param hWnd Window handle of the video preview window.
 */
SIPXTAPI_API SIPX_RESULT sipxCallUpdateVideoWindow(const SIPX_CALL hCall,
                                                   const SIPX_WINDOW_HANDLE hWnd);


/**
 * Resizes the video window.  Should be called when the window receives a SIZE message.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param hWnd Window handle of the video window.
 */
SIPXTAPI_API SIPX_RESULT sipxCallResizeWindow(const SIPX_CALL hCall,
                                              const SIPX_WINDOW_HANDLE hWnd);


/** 
 * Gets the sending and receiving Audio RTP SSRC IDs.  The SSRC ID is used to 
 * identify the RTP/audio stream.  The call must be in the connected state
 * for this request to succeed.
 *
 * This API is only supported when sipXtapi is bundled with VoiceEngine from 
 * GIPS.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param iSendSSRC The RTP SSRC used when sending audio
 * @param iReceiveSSRC The RTP SSRC used by the remote party to sending audio
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetAudioRtpSourceIds(const SIPX_CALL hCall,
                                                      unsigned int* iSendSSRC,
                                                      unsigned int* iReceiveSSRC);

/**
 * Obtain RTCP stats for the specified call.
 *
 * This API is only supported when sipXtapi is bundled with VoiceEngine from 
 * GIPS.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 *
 * @param pStats Structure to place call stats, the structure's cbSize 
 *        member must be filled out prior to calling this API.  For example:
 *        myStats.cbSize = sizeof(SIPX_RTCP_STATS);
 */
SIPXTAPI_API SIPX_RESULT sipxCallGetAudioRtcpStats(const SIPX_CALL hCall,
                                                   SIPX_RTCP_STATS* pStats);

/**
 * Limits the codec preferences on given call. Can be used on a connected
 * call to limit preferences for that call.
 * Preferences will take effect after next unhold.
 *
 * Care should be taken when using this API, since using it may easily violate
 * rfc4566 - dynamic payload formats may not change during session.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param szAudioCodecs Codec names that limit the supported audio codecs.
 * @param szVideoCodecs Codec names that limit the supported video codecs
 *        to this one video codec.
 *        
 * @see sipxConfigSetVideoBandwidth
 */
SIPXTAPI_API SIPX_RESULT sipxCallLimitCodecPreferences(const SIPX_CALL hCall,
                                                       const char* szAudioCodecs,
                                                       const char* szVideoCodecs);

/**
* Limits the codec preferences on given call. Can only be used on a connected
* call. Codec renegotiation will be triggered. If call is held, then after
* renegotiation it will be held again.
*
* Care should be taken when using this API, since using it may easily violate
* rfc4566 - dynamic payload formats may not change during session.
*
* @param hCall Handle to a call.  Call handles are obtained either by 
*        invoking sipxCallCreate or passed to your application through
*        a listener interface.
* @param szAudioCodecs Codec names that limit the supported audio codecs.
* @param szVideoCodecs Codec names that limit the supported video codecs
*        to this one video codec.
*        
* @see sipxConfigSetVideoBandwidth
*/
SIPXTAPI_API SIPX_RESULT sipxCallRenegotiateCodecPreferences(const SIPX_CALL hCall,
                                                             const char* szAudioCodecs,
                                                             const char* szVideoCodecs);

/**
 * Enables/disables discarding of inbound RTP for given call. Should be used
 * in server applications where local audio is disabled.
 *
 * @param hCall Handle to a call.  Call handles are obtained either by 
 *        invoking sipxCallCreate or passed to your application through
 *        a listener interface.
 * @param bMute Whether to mute or unmute inbound audio
 */
SIPXTAPI_API SIPX_RESULT sipxCallMuteInput(const SIPX_CALL hCall, const int bMute);

//@}

/** @name Publishing Methods */
//@{


/**
 * Creates a publishing context, which performs the processing necessary
 * to accept SUBSCRIBE requests, and to publish NOTIFY messages to subscribers. 
 * The resource may be specific to a single call, conference or global
 * to this user agent.  The naming of the resource ID determines the scope.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param phPub Pointer to a publisher handle - this method modifies the value
 *              to refer to the newly created publishing context.
 * @param szResourceId The resourceId to the state information being 
 *        published.  This must match the request URI of the incoming 
 *        SUBSCRIBE request (only the user ID, host and port are significant
 *        in matching the request URI).  Examples: fred\@10.0.0.1:5555, 
 *               sip:conference1\@192.160.0.1, sip:kate\@example.com  
 * @param szEventType A string representing the type of event that can be 
 *               published.
 * @param szContentType String representation of the content type being 
 *        published.
 * @param pContent Pointer to the NOTIFY message's body content.
 * @param nContentLength Size of the content to be published. Set to -1
 *                       if it should be determined automatically.
 *
 * @return If the resource already has a a publisher created for the given
 *               event type, SIPX_RESULT_INVALID_ARGS is returned.
 */
SIPXTAPI_API SIPX_RESULT sipxPublisherCreate(const SIPX_INST hInst, 
                                             SIPX_PUB* phPub,
                                             const char* szResourceId,
                                             const char* szEventType,
                                             const char* szContentType,
                                             const char* pContent,
                                             const int nContentLength = -1);

/**
 * Tears down the publishing context.  Any existing subscriptions
 * are sent a final NOTIFY request.  If pFinalContent is not NULL and 
 * nContentLength > 0 the given publish state is given otherwise
 * the final NOTIFY requests are sent with no body or state.
 * 
 * @param hPub Handle of the publishing context to destroy 
 *              (returned from a call to sipxCreatePublisher)
 * @param szContentType String representation of the content type being 
 *        published
 * @param pFinalContent Pointer to the NOTIFY message's body content
 * @param nContentLength Size of the content to be published. Set to -1
 *                       if it should be determined automatically.
 */
SIPXTAPI_API SIPX_RESULT sipxPublisherDestroy(const SIPX_PUB hPub,
                                              const char* szContentType,
                                              const char* pFinalContent,
                                              const int nContentLength = -1);

/**
 * Publishes an updated state to specific event via NOTIFY to its subscribers.
 * 
 * @param hPub Handle of the publishing context 
 *              (returned from a call to sipxCreatePublisher)
 * @param szContentType String representation of the content type being 
 *        published
 * @param pContent Pointer to the NOTIFY message's body content
 * @param nContentLength Size of the content to be published. Set to -1
 *                       if it should be determined automatically.
 */
SIPXTAPI_API SIPX_RESULT sipxPublisherUpdate(const SIPX_PUB hPub,
                                             const char* szContentType,
                                             const char* pContent,
                                             const int nContentLength = -1);

//@}

/** @name Conference Methods */
//@{


/**
 * Create a conference handle.  Conferences are an association of calls 
 * where the audio media is mixed.  sipXtapi supports conferences up to
 * 4 (CONF_MAX_CONNECTIONS) parties in its default configuration. An
 * empty shell call is automatically created which is invisible to the
 * client. Conference by default cannot receive inbound calls.
 *
 * Public conference may be created by specifying szConferenceUri.
 * This enables conference to handle inbound calls.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param phConference Pointer to a conference handle.  Upon success, 
 *        this value is replaced with a valid conference handle.  
 *        Success is determined by the SIPX_RESULT result code.
 * @param szConferenceUri optional conference sip uri. If present conference
 *        can handle inbound calls.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceCreate(const SIPX_INST hInst,
                                              SIPX_CONF* phConference,
                                              const char* szConferenceUri = NULL);

/**
 * Join (add) an existing connected call into a conference.
 * 
 * Call must be connected for this operation to succeed. This operation
 * executes asynchronously and will result in CONFERENCE_CALL_ADDED or
 * CONFERENCE_CALL_ADD_FAILURE event. Until one of these events is received
 * no other operation on the call must be performed.
 *
 * If an INVITE or UPDATE is in progress operation will fail with
 * CONFERENCE_CAUSE_INVALID_STATE and may be retried later.
 *
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 * @param hCall Call handle of the call to join into the conference.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceJoin(const SIPX_CONF hConf,
                                            const SIPX_CALL hCall);

/**
 * Split (remove) a call from a conference. This method will remove
 * the specified call from the conference. Specified call will continue
 * to be active.
 *
 * The call must be connected for this operation to succeed. This operation
 * executes asynchronously and will result in CONFERENCE_CALL_REMOVED or
 * CONFERENCE_CALL_REMOVE_FAILURE event. Until one of these events is received
 * no other operation on the call must be performed.
 *
 * If an INVITE or UPDATE is in progress operation will fail with
 * CONFERENCE_CAUSE_INVALID_STATE and may be retried later.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param hCall Call handle of the call that should be removed from the
 *        the conference.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceSplit(const SIPX_CONF hConf,
                                             const SIPX_CALL hCall);

/**
 * Add a new party to an existing conference.  A connection is automatically
 * initiated for the specified address. If operation fails and returned
 * call handle is non zero, call needs to be destroyed manually. If remote
 * party hangs up a call in conference, the call is destroyed automatically.
 * 
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param hLine Line Identity for the outbound call.  The line identity 
 *        helps defines the "From" caller-id.
 * @param szAddress SIP URL of the conference participant to add
 * @param phNewCall Pointer to a call handle to store new call.
 * @param takeFocus Should SIPxua place the newly answered call in focus
 *        (engage local microphone and speaker).  In some cases, application
 *        developer may want to answer the call in the background and play
 *        audio while the user finishes up with their active (in focus) call.
 * @param options Pointer to a SIPX_CALL_OPTIONS structure.
 *
 * @see sipxConferenceCreate
 * @see sipxConfigSetLocationHeader
 * @see sipxConfigGetLocalContacts
 * @see sipxConfigSetAudioCodecPreferences
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceAdd(const SIPX_CONF hConf,
                                           const SIPX_LINE hLine,
                                           const char* szAddress,
                                           SIPX_CALL* phNewCall,
                                           SIPX_FOCUS_CONFIG takeFocus = SIPX_FOCUS_IF_AVAILABLE,
                                           SIPX_CALL_OPTIONS* options = NULL);

/**
 * Removes a participant from conference by hanging up on them. Call
 * is automatically destroyed. Upon success, call handle is zeroed.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param hCall Call handle identifying which call to remove from the
 *        conference by hanging up.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceRemove(const SIPX_CONF hConf,
                                              SIPX_CALL* hCall);


/**
 * Gets all of the calls participating in a conference.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param calls An array of call handles filled in by the API.
 * @param iMax The maximum number of call handles to return.
 * @param nActual The actual number of call handles returned.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceGetCalls(const SIPX_CONF hConf,
                                                SIPX_CALL calls[],
                                                const size_t iMax, 
                                                size_t* nActual);

/**
 * Places a conference on hold.  This API can be used to place a 
 * conference on local hold (continue to bridge participants) or full hold
 * (remaining participants cannot talk to each other).   The default is
 * local hold/bridged.  The bBridged flag can be used to change this
 * behavior (false for full hold).
 *
 * Developers may also hold/unhold individual conference participants by
 * calling sipxCallHold and sipxCallUnhold on individual call handles.  The
 * sipxConferenceGetCalls API can be used to enumerate conference 
 * participants.
 *
 * @see sipxCallHold for a description of the expected
 *      events.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param bBridging true for a bridging conference hold,
 *        false for a non-bridging conference hold.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceHold(const SIPX_CONF hConf,
                                            int bBridging = 1);
                                            
/**
 * Removes conference members from a held state.  This method will take a call
 * off either local or remote/full hold.
 *
 * @see sipxConferenceHold for details on holding 
 *      conferences.
 * @see sipxCallHold for a description of the expected
 *      events.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceUnhold(const SIPX_CONF hConf);


/**
 * Play the designated audio file to all conference participants and/or the 
 * local speaker. 
 *
 * The file must be a WAV file 8bit unsigned or 16bit signed, mono or stereo.
 * Stereo files will be merged to mono.
 *
 * Sampling rate of supplied file will be automatically resampled to internal
 * sampling rate of sipXmediaLib. If wideband audio is enabled, then internal sampling
 * rate is 48Khz, otherwise it is 8Khz. If sipXtapi is not bundled with Speex library,
 * then only downsampling is supported.
*
 * If a sipxConferenceDestroy is attempted while an audio file is playing,
 * sipxConferenceDestroy will fail with a SIPX_RESULT_BUSY return code.
 * Call sipxConferencePlayAudioFileStop before making the call to
 * sipxConferenceDestroy.
 * 
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 * @param szFile Filename for the audio file to be played.
 * @param bRepeat True if the file is supposed to be played repeatedly
 * @param bLocal True if the audio file is to be rendered locally.
 * @param bRemote True if the audio file is to be rendered by the remote
 *                endpoint.
 * @param bMixWithMicrophone True to mix the audio with the microphone
 *        data or false to replace it.  This option is only supported 
 *        when sipXtapi is bundled with GIPS VoiceEngine.
 * @param fVolumeScaling Volume down scaling for the audio file.  Valid 
 *        values are between 0 and 1.0, where 1.0 is the no down-scaling.
 *        This option is only supported when sipXtapi is bundled with GIPS
 *        VoiceEngine.
 */

SIPXTAPI_API SIPX_RESULT sipxConferencePlayAudioFileStart(const SIPX_CONF hConf, 
                                                          const char* szFile,
                                                          const int bRepeat,
                                                          const int bLocal,
                                                          const int bRemote,
                                                          const int bMixWithMicrophone = 0,
                                                          const float fVolumeScaling = 1.0);

/*
 * Stop playing a file started with sipxConferencePlayAudioFileStart
 * If a sipxConferenceDestroy is attempted while an audio file is playing,
 * sipxConferenceDestroy will fail with a SIPX_RESULT_BUSY return code.
 * Call sipxConferencePlayAudioFileStop before making the call to
 * sipxConferenceDestroy.
 * 
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 */
SIPXTAPI_API SIPX_RESULT sipxConferencePlayAudioFileStop(const SIPX_CONF hConf);

/**
 * Start recording conference into given file. The resulting file will
 * be a PCM WAV file. Recording can be started only if there is at least
 * one call in the conference, and stops when the last call disconnects.
 * If an abandoned conference is populated by calls again, recording needs
 * to be started again too.
 *
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 * @param szFile Name of file for recording.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceAudioRecordFileStart(const SIPX_CONF hConf, const char* szFile);

/**
 * Stop recording conference started by sipxConferenceAudioRecordFileStart.
 *
 * @param hConf Conference handle obtained by calling sipxConferenceCreate.
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceAudioRecordFileStop(const SIPX_CONF hConf);

/**
 * Destroys a conference.  All participants within a conference are
 * dropped. 
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 */ 
SIPXTAPI_API SIPX_RESULT sipxConferenceDestroy(SIPX_CONF hConf);

/**
 * Limits the codec preferences on a conference. Supplied settings will be applied
 * for new conference calls, or calls that are unheld. First supplied audio codec
 * matching remote party audio codec will be used.
 *
 * Care should be taken when using this API, since using it may easily violate
 * rfc4566 - dynamic payload formats may not change during session.
 *
 * @param hConf Handle to a conference.  Conference handles are obtained 
 *        by invoking sipxConferenceCreate.
 * @param szVideoCodecNames Codec names that limit the supported audio codecs.
 * @param szVideoCodecNames Codec names that limit the supported video codecs
 *        to this one video codec.
 *        
 * @see sipxConfigSetVideoBandwidth
 */
SIPXTAPI_API SIPX_RESULT sipxConferenceLimitCodecPreferences(const SIPX_CONF hConf,
                                                             const char* szAudioCodecNames,
                                                             const char* szVideoCodecNames);

/**
* Limits the codec preferences on a conference. Supplied settings will be applied
* immediately. First supplied audio codec matching remote party audio codec will be used.
*
* Care should be taken when using this API, since using it may easily violate
* rfc4566 - dynamic payload formats may not change during session.
*
* @param hConf Handle to a conference.  Conference handles are obtained 
*        by invoking sipxConferenceCreate.
* @param szVideoCodecNames Codec names that limit the supported audio codecs.
* @param szVideoCodecNames Codec names that limit the supported video codecs
*        to this one video codec.
*        
* @see sipxConfigSetVideoBandwidth
*/
SIPXTAPI_API SIPX_RESULT sipxConferenceRenegotiateCodecPreferences(const SIPX_CONF hConf,
                                                                   const char* szAudioCodecNames,
                                                                   const char* szVideoCodecNames);

//@}

/** @name Audio Methods */
//@{

/**
 * Returns name of input mixer for the active input device. If no input
 * device is selected, it will be empty. Function guarantees to terminate
 * string with 0.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param name Buffer where name of mixer should be copied.
 * @param buffSize Size of buffer in bytes.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetInputMixerName(const SIPX_INST hInst,
                                                    char* name,
                                                    int buffSize);

/**
 * Returns name of output mixer for the active input device. If no output
 * device is selected, it will be empty. Function guarantees to terminate
 * string with 0.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param name Buffer where name of mixer should be copied.
 * @param buffSize Size of buffer in bytes.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputMixerName(const SIPX_INST hInst,
                                                     char* name,
                                                     int buffSize);

/**
 * Gets master audio volume. This volume is common for all audio output
 * devices.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the master volume
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetMasterVolume(const SIPX_INST hInst,
                                                  int* iLevel);

/**
 * Sets master audio volume. This volume is common for all audio output
 * devices.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the master volume
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetMasterVolume(const SIPX_INST hInst,
                                                  int iLevel);

/**
 * Gets balance for output device. Balance is in range -100..100
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iBalance The value of balance
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputBalance(const SIPX_INST hInst,
                                                   int* iBalance);

/**
 * Sets balance for output device. Balance is in range -100..100
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iBalance The value of balance
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetOutputBalance(const SIPX_INST hInst,
                                                   int iBalance);

/**
 * Set the local microphone volume.  If the microphone is muted, 
 * resetting the volume will not enable audio -- you must unmute
 * the microphone.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the local microphone volume 0-100
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetInputVolume(const SIPX_INST hInst,
                                                 const int iLevel);


/**
 * Get the current microphone volume.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the volume of the microphone 0-100
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetInputVolume(const SIPX_INST hInst,
                                                 int* iLevel);


/**
 * Mute or unmute the microphone.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bMute True if the microphone is to be muted and false if it 
 *        is not to be muted
 */
SIPXTAPI_API SIPX_RESULT sipxAudioMuteInput(const SIPX_INST hInst,
                                            const int bMute);


/**
* Mute or unmute the speaker.
*
* @param hInst Instance pointer obtained by sipxInitialize.
* @param bMute True if the speaker is to be muted and false if it 
*        is not to be muted
*/
SIPXTAPI_API SIPX_RESULT sipxAudioMuteOutput(const SIPX_INST hInst,
                                             const int bMute);

/**
 * Gets the mute state of the microphone.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bMuted True if the microphone has been muted and false if it 
 *        is not mute
 */
SIPXTAPI_API SIPX_RESULT sipxAudioIsInputMuted(const SIPX_INST hInst,
                                               int* bMuted);

/**
* Gets the mute state of the speaker.
*
* @param hInst Instance pointer obtained by sipxInitialize.
* @param bMuted True if the speaker has been muted and false if it 
*        is not mute
*/
SIPXTAPI_API SIPX_RESULT sipxAudioIsOutputMuted(const SIPX_INST hInst,
                                                int* bMuted);


/**
 * Sets the audio level for speaker.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the volume of the speaker 0-100
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetOutputVolume(const SIPX_INST hInst,
                                                  const int iLevel);


/**
 * Gets the audio level for speaker.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param iLevel The level of the volume of the speaker 0-100
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputVolume(const SIPX_INST hInst,
                                                  int* iLevel);


/**
 * Gets reading of volume from input volume meter. It calculates volume
 * from a few last audio frames recorded.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param type Type of volume meter to use. Can either be by VU or PPM
 *        algorithm. @see SIPX_VOLUME_METER_TYPE
 * @param level The level of the volume 0-100
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetInputEnergy(const SIPX_INST hInst,
                                                 SIPX_VOLUME_METER_TYPE type,
                                                 double* level);

/**
* Gets reading of volume from output volume meter. It calculates volume
* from a few last audio frames played.
*
* @param hInst Instance pointer obtained by sipxInitialize.
* @param type Type of volume meter to use. Can either be by VU or PPM
*        algorithm. @see SIPX_VOLUME_METER_TYPE
* @param level The level of the volume 0-100
*/
SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputEnergy(const SIPX_INST hInst,
                                                  SIPX_VOLUME_METER_TYPE type,
                                                  double* level);


/**
 * Enables or disables Acoustic Echo Cancellation (AEC).  By default, sipXtapi
 * assumes SIPX_AEC_CANCEL_AUTO.  Change this parameter will modify the policy 
 * for both existing and new calls.
 *
 * Note: This API is only supported when bundled with VoiceEngine from 
 * Global IP Sound or Speex library.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param mode AEC mode.
 *
 * @see SIPX_AEC_MODE
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetAECMode(const SIPX_INST hInst,
                                             const SIPX_AEC_MODE mode);


/**
 * Get the mode of Acoustic Echo Cancellation (AEC).
 * This setting comes into effect for new calls.
*
 * Note: This API is only supported when bundled with Speex library.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param mode AEC mode.
 *
 * @see SIPX_AEC_MODE
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetAECMode(const SIPX_INST hInst,
                                             SIPX_AEC_MODE* mode);


/**
 * Enable/Disable Automatic Gain Control (AGC).  By default, AGC is disabled.
 * This setting comes into effect for new calls.
 *
 * Note: This API is only supported when bundled with Speex library.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bEnable true to enable AGC or false to disable
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetAGCMode(const SIPX_INST hInst,
                                             const int bEnable);

/**
 * Get the enable/disable state of Automatic Gain Control (AGC).
 * This setting comes into effect for new calls.
 *
 * Note: This API is only supported when bundled with Speex library.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bEnabled true if AGC is enabled; otherwise false.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetAGCMode(const SIPX_INST hInst,
                                             int* bEnabled);


/**
 * Set the noise reduction mode/policy for suppressing background noise.  By
 * default sipXtapi assumes SIPX_NOISE_REDUCTION_LOW.  Change this parameter
 * will modify the policy for both existing and new calls.
 *
 * Note: This API is only supported when bundled with Speex library.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param mode noise reduction mode.
 *
 * @see SIPX_NOISE_REDUCTION_MODE
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetNoiseReductionMode(const SIPX_INST hInst,
                                                        const SIPX_NOISE_REDUCTION_MODE mode);


/**
 * Get the mode/policy for Noise Reduction (NR).
 *
 * Note: This API is only supported when bundled with Speex library.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param mode noise reduction mode.
 *
 * @see SIPX_NOISE_REDUCTION_MODE
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetNoiseReductionMode(const SIPX_INST hInst,
                                                        SIPX_NOISE_REDUCTION_MODE* mode);

/**
 * Enables/Disables voice activity detection. When enabled, then also
 * DTX (discontinuous transmission) is enabled.
 *
 * Enables internal VAD for codecs which have it built in (Intel IPP),
 * and enables generic speex VAD for all other codecs. For codecs with VAD
 * and DTX support, a so called SID frame will be sent during silence,
 * for other codecs no frames will be sent at all.
 *
 * Note: This API is only supported when bundled with Speex library
 * for codecs without built in VAD support. Codecs with built in VAD support
 * do not require speex library.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bEnabled TRUE when VAD should be enabled.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetVADMode(const SIPX_INST hInst,
                                             int bEnabled);

/**
 * Gets status of voice activity detection.
 *
 * Note: This API is only supported when bundled with Speex library.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param bEnabled TRUE when VAD is enabled.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetVADMode(const SIPX_INST hInst,
                                             int* bEnabled);

/**
 * Get the number of input devices available on this system.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param numDevices The number of input devices available
 *        on this system. 
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetNumInputDevices(const SIPX_INST hInst,
                                                     int* numDevices);

/**
 * Get the name/identifier for input device at position index.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param index Zero based index of the input device to be queried.
 * @param deviceInfo SIPX_AUDIO_DEVICE structure that will receive
 *        information about audio device. There are always 2 special
 *        devices - "None" and "Default".
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetInputDeviceInfo(const SIPX_INST hInst,
                                                     const int index,
                                                     SIPX_AUDIO_DEVICE* deviceInfo);

/**
 * Get the number of output devices available on this system
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param numDevices The number of output devices available
 *        on this system. 
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetNumOutputDevices(const SIPX_INST hInst,
                                                      int* numDevices);

/**
 * Get the name/identifier for output device at position index
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param index Zero based index of the output device to be queried.
 * @param deviceInfo SIPX_AUDIO_DEVICE structure that will receive
 *        information about audio device. There are always 2 special
 *        devices - "None" and "Default".
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputDeviceInfo(const SIPX_INST hInst,
                                                      const int index,
                                                      SIPX_AUDIO_DEVICE* deviceInfo);

/**
 * Set the call input device (in-call microphone).  
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szDevice Character string pointer to be set to a string
 *        name of the output device. Pass "None" to completely disable
 *        audio input device. Pass "Default" to select platform
 *        independent audio input device.
 * @param Name of driver of use for device. Should be NULL for "None"
 *        or "Default" device. Otherwise it should be non NULL.
 *        Application can pass NULL if it doesn't care which driver
 *        should be selected. Then the first driver which has given
 *        device available will be selected.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioSetInputDevice(const SIPX_INST hInst,
                                                 const char* szDevice,
                                                 const char* szDriver = NULL);


/**
 * Returns current active input audio device and its driver.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param deviceInfo Pointer to structure which will receive information
 *        about active input audio device. "None" will be returned as name
 *        if device is disabled. "Default" will never be returned, instead
 *        a real default device name will be.
 */
SIPXTAPI_API SIPX_RESULT sipxAudioGetInputDevice(const SIPX_INST hInst,
                                                 SIPX_AUDIO_DEVICE* deviceInfo);

/**
 * Set the call output device (in-call speaker).
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szDevice The call output device. Pass "None" to completely disable
 *        audio output device. Pass "Default" to select platform
 *        independent audio output device.
 * @param Name of driver of use for device. Should be NULL for "None"
 *        or "Default" device. Otherwise it should be non NULL.
 *        Application can pass NULL if it doesn't care which driver
 *        should be selected. Then the first driver which has given
 *        device available will be selected.
*/
SIPXTAPI_API SIPX_RESULT sipxAudioSetOutputDevice(const SIPX_INST hInst,
                                                  const char* szDevice,
                                                  const char* szDriver = NULL);


/**
* Returns current active output audio device and its driver.
*
* @param hInst Instance pointer obtained by sipxInitialize.
* @param deviceInfo Pointer to structure which will receive information
*        about active output audio device. "None" will be returned as name
*        if device is disabled. "Default" will never be returned, instead
*        a real default device name will be.
*/
SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputDevice(const SIPX_INST hInst,
                                                  SIPX_AUDIO_DEVICE* deviceInfo);


/**
* Configures audio driver latency. Settings will be applied next time audio driver
* is reset. To trigger reset of audio driver, set audio device to "None" and then
* back to previous audio device.
*
* @param hInst Instance pointer obtained by sipxInitialize.
* @param inputLatency Suggested latency in seconds. 0.04 is the default value.
* @param outputLatency Suggested latency in seconds. 0.04 is the default value.
*/
SIPXTAPI_API SIPX_RESULT sipxAudioSetDriverLatency(const SIPX_INST hInst,
                                                   double inputLatency,
                                                   double outputLatency);

/**
* Gets audio driver latency. If "None" audio device is selected, then initial
* latency will be returned. If some real audio device is selected and audio stream
* is active, then real latency of audio stream will be returned.
*
* Latency returned by this function doesn't reflect the whole internal latency
* of sipXtapi media processing, it merely contains information about latency of
* audio driver. Other sources of latency are jitter buffer, encoder/decoder
* and operating system.
*
* By default synchronous Portaudio streams are used. It is possible to enable
* asynchronous Portaudio streams in sipXmediaLib, but that leads to increase
* of latency at least 80ms for both input and output.
*
* @param hInst Instance pointer obtained by sipxInitialize.
* @param inputLatency Latency in seconds.
* @param outputLatency Latency in seconds.
*/
SIPXTAPI_API SIPX_RESULT sipxAudioGetDriverLatency(const SIPX_INST hInst,
                                                   double* inputLatency,
                                                   double* outputLatency);

//@}
/** @name Line / Identity Methods*/
//@{


/**
 * Adds a line appearance.  A line appearance defines your address of record
 * and is used both as your "From" caller-id and as the public identity to 
 * which you will receive calls for.  Directing calls to a particular user 
 * agent is achieved using registrations.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szLineURL The address of record for the line identity.  Can be 
 *        prepended with a Display Name.
 *        e.g. -    "Zaphod Beeblebrox" <sip:zaphb\@fourty-two.net>
 * @param phLine Pointer to a line handle.  Upon success, a handle to the 
 *        newly added line is returned.
 * @param contactId Id of the desired contact record to use for this line.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a 
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or 
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 * @param transport to use for sending REGISTER messages if automatic
 *        contact is selected. If some specific contact is selected then
 *        this transport must match that contact transport.
 *
 * @see sipxConfigGetLocalContacts
 */
SIPXTAPI_API SIPX_RESULT sipxLineAdd(const SIPX_INST hInst,
                                     const char* szLineURL,
                                     SIPX_LINE* phLine,
                                     SIPX_CONTACT_ID contactId = SIPX_AUTOMATIC_CONTACT_ID,
                                     SIPX_TRANSPORT_TYPE transport = TRANSPORT_AUTO);

/**
 * Adds an alias for a line definition.  Line aliases are used to map an 
 * inbound call request to an existing line definition.  You should only 
 * need to an a alias if your network infrastructure directs calls to this
 * user agent using multiple identities.  For example, if user agent 
 * registers as "sip:bandreasen\@example.com"; however, calls can also be
 * directed to you via an exention (e.g. sip:122\@example.com).
 *
 * If sipXtapi receives a call with an unknown line, you can still answer
 * and interact wtih the call; however, the line handle will be SIPX_LINE_NULL
 * in all event callbacks.  Adding an aliases allows you to correlate another 
 * line url with your line definition and receive real line handles with event
 * callbacks.
 *
 * @see sipxConfigGetLocalContacts
 */
SIPXTAPI_API SIPX_RESULT sipxLineAddAlias(const SIPX_LINE hLine,
                                          const char* szLineURL);

/**
 * Registers a line with the proxy server.  Registrations will be re-registered
 * automatically, before they expire.
 *
 * Unless your user agent is designated a static IP address or DNS name and 
 * that routing information is provisioned into a SIP server, you should 
 * register the line by calling this function.
 *
 * Please unregister your line before calling sipxLineRemove.
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param bRegister true if Registration is desired, otherwise, an Unregister
 *        is performed.
 */
SIPXTAPI_API SIPX_RESULT sipxLineRegister(const SIPX_LINE hLine,
                                          const int bRegister);

/**
 * Remove the designated line appearance.  If the line was previous registered 
 * using the sipxLineRegister API, please unregister the line and wait for the
 * unregistered event before calling sipxLineRemove.  Otherwise, the line will
 * be removed without unregistering.  
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 */ 
SIPXTAPI_API SIPX_RESULT sipxLineRemove(SIPX_LINE hLine);

/**
 * Adds authentication credentials to the designated line appearance.  
 * Credentials are often required by registration services to verify that the
 * line is being used by the line appearance/address of record owner. 
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param szUserID user id used for the line appearance.
 * @param szPasswd passwd used for the line appearance.
 * @param szRealm realm for which the user and passwd are valid. Pass
 *        empty realm for automatic realm.
 */ 
SIPXTAPI_API SIPX_RESULT sipxLineAddCredential(const SIPX_LINE hLine,                                                 
                                               const char* szUserID,
                                               const char* szPasswd,
                                               const char* szRealm);

/**
* Sets line outbound proxy servers. This setting overrides the default
* outbound proxy servers set by sipxConfigSetOutboundProxy. Works only on
* lines created by sipxLineAdd and their aliases.
* 
* @param hLine Handle to a line appearance.  Line handles are obtained by
*        creating a line using the sipxLineAdd function.
* @param szProxyServers The addresses and ports of proxy servers to use - i.e
*        hostname.domain:port. Use IP address if possible to avoid unnecessary
*        DNS lookups. Can be multiple values separated by ,
*
* @see sipxConfigSetOutboundProxy
*/
SIPXTAPI_API SIPX_RESULT sipxLineSetOutboundProxy(const SIPX_LINE hLine,
                                                  const char* szProxyServers);

/**
 * Gets the active list of line identities.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param lines Pre-allocated array of line handles.
 * @param max Maximum number of lines to return.
 * @param actual Actual number of valid lines returned.
 */ 

SIPXTAPI_API SIPX_RESULT sipxLineGet(const SIPX_INST hInst,
                                     SIPX_LINE lines[],
                                     const size_t max,
                                     size_t* actual);

/**
 * Get the Line URI for the designated line handle
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param szBuffer Buffer to place line URL.  A NULL value will return
 *        the amount of storage needed in nActual.
 * @param nBuffer Size of szBuffer in bytes (not to exceed)
 * @param nActual Actual number of bytes written
 */
SIPXTAPI_API SIPX_RESULT sipxLineGetURI(const SIPX_LINE hLine,
                                        char* szBuffer,
                                        const size_t nBuffer,
                                        size_t* nActual);

/**
 * Get the contact info for the designated line handle
 *
 * @param hLine Handle to a line appearance.  Line handles are obtained by
 *        creating a line using the sipxLineAdd function or by receiving
 *        a line event notification.
 * @param szContactAddress Buffer to hold actual contact address.
 * @param nContactAddressSize Size of szContactAddress buffer.
 * @param contactPort Actual line contact port
 * @param contactType Type of contact being used by line.
 * @param transport Type of transport being used in contact.
 */
SIPXTAPI_API SIPX_RESULT sipxLineGetContactInfo(const SIPX_LINE  hLine,
                                                char* szContactAddress,
                                                const size_t nContactAddressSize,
                                                int* contactPort,
                                                SIPX_CONTACT_TYPE* contactType,
                                                SIPX_TRANSPORT_TYPE* transport);

/**
 * Find a line definition given a URI.  
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szURI URI used to search for a line definition
 * @param phLine line handle if successful
 */
SIPXTAPI_API SIPX_RESULT sipxLineFindByURI(const SIPX_INST hInst,
                                           const char* szURI,
                                           SIPX_LINE* phLine);

//@}
/** @name Configuration Methods*/
//@{

/**
 * Initializes sipxtapi logging. Must be called before initializing sipxtapi
 * if it is desired to log to a custom file. If it is not called, sipxtapi
 * logging is initialized by sipxInitialize. Logging priority and filename
 * can later be changed by sipxConfigSetLogLevel and sipxConfigSetLogFile.
 *
 * NOTE: At this time no validation is performed on the specified filename.  
 * Please make sure the directories exist and the appropriate permissions
 * are available.
 *
 * @param logLevel Designates the amount of detail includes in the log.  See
 *        SIPX_LOG_LEVEL for more details.
 * @param szFilename The filename for the log file.  Designated a NULL 
 *        filename will disable logging, however, threads/resources will not
 *        be deallocated.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigInitLogging(const char* szFilename, SIPX_LOG_LEVEL logLevel);

/**
 * The sipxConfigEnableLog method enables logging for the sipXtapi API,
 * media processing, call processing, SIP stack, and OS abstraction layer.
 * Logging is disabled by default.  The underlying framework makes no attempts 
 * to bound the log file to a fixed size.
 *
 * Log Format:
 *    time:event id:facility:priority:host name:task name:task id:process id:log message
 *
 * @param logLevel Designates the amount of detail includes in the log.  See
 *        SIPX_LOG_LEVEL for more details.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLogLevel(SIPX_LOG_LEVEL logLevel);


/** 
 * The sipxConfigSetlogFile method sets the filename of the log file and 
 * directs output to that file
 *
 * NOTE: At this time no validation is performed on the specified filename.  
 * Please make sure the directories exist and the appropriate permissions
 * are available.
 *
 * @param szFilename The filename for the log file.  Designated a NULL 
 *        filename will disable logging, however, threads/resources will not
 *        be deallocated.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLogFile(const char *szFilename);


/**
 * Set a callback function to collect logging information. This function
 * directs logging output to the specified function.
 *
 * @param pCallback is a pointer to a callback function. This callback function
 *        gets passed three strings, first string is the priority level,
 *        second string is the source id of the subsystem that generated
 *        the message, and the third string is the message itself.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLogCallback(sipxLogCallback pCallback);


/**
 * Sets the User-Agent name to be used with outgoing SIP messages.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szName The user-agent name.
 * @param bIncludePlatformName Indicates whether or not to append the
 *        platform description onto the user agent name.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetUserAgentName(const SIPX_INST hInst, 
                                                    const char* szName, 
                                                    const int bIncludePlatformName = 1);
                                                    
/**
 * Defines the SIP proxies used for outbound requests. 
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szProxy the new outbound proxy. The Format is
 *        hostname1.domain:port,hostname2.domain:port etc.
 *        Use IP address if possible, to avoid DNS lookup.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetOutboundProxy(const SIPX_INST hInst,
                                                    const char* szProxy);

/**
 * Modifies the timeout values used for DNS SRV lookups.  In generally,
 * you shouldn't need to modified these, however, if you find yourself
 * in a situation where a router/network fails to send responses to
 * DNS SRV requests these values can be tweaked.  Note, failing to send
 * responses is different then a receiving an no-such-animal response.
 * <p>
 * The default values are initialTimeout = 5 seconds, and 4 retries.  The
 * time waited is doubled after each timeout, so with the default settings,
 * a single DNS SRV can block for 75 seconds (5 + 10 + 20 + 40).  In general,
 * 4 DNS SRV requests are made for each hostname (e.g. domain.com):
 * <ul>
 *   <li> _sip._udp.domain.com</li>
 *   <li> _sip._tcp.domain.com</li>
 *   <li> _sip._udp.domain.com.domain.com</li>
 *   <li> _sip._tcp.domain.com.domain.com</li>
 * </ul>
 *
 * If DNS response are dropped in the network (or your DNS server is down), 
 * the API will block for 3 minutes.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetDnsSrvTimeouts(const int initialTimeoutInSecs, 
                                                     const int retries);


/**
 * Specifies the time to wait for a REGISTER response before sending a 
 * LINESTATE_REGISTER_FAILED (or LINESTATE_UNREGISTER_FAILED) message.
 * If not set, the user-agent will use a 4 second timeout.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param seconds The number of seconds to wait for a REGISTER response,
                  before it is considered a failure.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetRegisterResponseWaitSeconds(const SIPX_INST hInst,
                                                                  const int seconds);   
/**
 * Specifies the time to wait before trying the next DNS SRV record.  The user
 * agent will attempt to obtain DNS SRV resolutions for the child DNS SRV 
 * records.  This setting is the time allowed for attempting a lookup - if the 
 * time expires without a lookup, then next child is attempted.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param failoverTimeoutInSecs Number of seconds until the next DNS SRV 
 *        record is tried.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetDnsSrvFailoverTimeout(const SIPX_INST hInst,
                                                            const int failoverTimeoutInSecs); 


/**
 * Enable or disable the use of "rport".  If rport is included on a via,
 * responses should be sent back to the originating port -- not what is
 * advertised as part of via.
 *
 * @param hInst Instance pointer obtained by sipxInitialize. 
 * @param bEnable Enable or disable the use of rport.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableRport(const SIPX_INST hInst,
                                               const int bEnable);
                                               
/**
 * Specifies the expiration period for registration.  After setting this 
 * configuration, all subsequent REGISTER messages will be sent with the new 
 * registration period. 
 *
 * @param hInst Instance pointer obtained by sipxInitialize. 
 * @param nRegisterExpirationSecs Number of seconds until the expiration of a 
 *        REGISTER message
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetRegisterExpiration(const SIPX_INST hInst,
                                                         const int nRegisterExpirationSecs);
  
/**
 * Enables STUN (Simple Traversal of UDP through NAT) support for both 
 * UDP SIP signaling and UDP audio/video (RTP).  STUN helps user agents
 * determine thier external IP address from the inside of NAT/Firewall. 
 * This method should be invoked immediately after calling sipxInitialize 
 * and before creating any lines or calls.  Enabling STUN while calls are 
 * setup should not effect the media path of existing calls.  The "contact"
 * address uses for UDP signaling may change on the next request.
 *
 * TODO :: STUN conforms to IETF RFC/DRAFT XXXX with the following exceptions:
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szServer The stun server that should be used for discovery.
 * @param iServerPort The port of the stun server that should be used for 
 *        discovery.
 * @param iKeepAliveInSecs This setting controls how often to refresh the stun
 *        binding.  The most aggressive NAT/Firewall solutions free port 
 *        mappings after 30 seconds of non-use.  We recommend a value of 28 
 *        seconds to be safe.
 *         
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableStun(const SIPX_INST hInst,
                                              const char* szServer, 
                                              int iServerPort,
                                              int iKeepAliveInSecs);

/**
 * Disable the use of STUN.  See sipxConfigEnableStun for details on STUN.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigDisableStun(const SIPX_INST hInst);


/**
 * Enable TURN for support for UDP audio/video (RTP).  TURN allows VoIP
 * communications while operating behind a symmetric NAT or firewall (cannot 
 * only receive data from the same IP/port that you have sent to).
 *
 * This implementation is based on draft-rosenberg-midcom-turn-04.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 * @param szServer The TURN server that should be used for relaying.
 * @param iServerPort The TURN sever port that should be used for relaying
 * @param szUsername TURN username for authentication
 * @param szPassword TURN password for authentication
 * @param iKeepAliveInSecs This setting controls how often to refresh the TURN
 *        binding.  The recommended value is 28 seconds.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableTurn(const SIPX_INST hInst,
                                              const char* szServer,
                                              const int iServerPort,
                                              const char* szUsername,
                                              const char* szPassword,
                                              const int iKeepAliveInSecs);

/**
 * Disable the use of TURN.  See sipxConfigEnableTurn for details TURN.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigDisableTurn(const SIPX_INST hInst);


/**
 * Enables an ICE-like mechanism for determining connectivity of remote 
 * parties dynamically.  By default, ICE is disabled.
 * 
 * The current sipXtapi implementation is a bastardization of 
 * draft-ietf-mmusic-ice-04.  In subsequent release, this will
 * conform to draft-ietf-mmusic-ice-05 or the latest draft.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableIce(const SIPX_INST hInst);


/**
 * Disable the use of ICE.  See sipxConfigEnableICE for details.
 *
 * @param hInst Instance pointer obtained by sipxInitialize.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigDisableIce(const SIPX_INST hInst);


/**
 * Add a signaling keep alive to a remote ip address.  
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param contactId Contact ID used for the keep alive.  sipXtapi will
 *        send keep alives from the interface identified by the 
 *        contactId.  Specify a contactId of -1 to send keep alives from
 *        all interfaces.
 * @param type Designates the method of keep alives.
 * @param remoteIp Remote IP address used to send keep alives.  The caller is
 *        responsible for converting hostnames to IP address.
 * @param remotePort Remote port used to send keep alives.
 * @param keepAliveSecs The number of seconds to wait before resending.  If 
 *        the value is <= 0, only one keep alive will be sent (calling 
 *        sipxConfigKeepAliveRemove will fail).
 *
 * @see sipxConfigGetLocalContacts
 */
SIPXTAPI_API SIPX_RESULT sipxConfigKeepAliveAdd(const SIPX_INST hInst,
                                                SIPX_CONTACT_ID contactId,
                                                SIPX_KEEPALIVE_TYPE type,
                                                const char* remoteIp,
                                                int remotePort,
                                                int keepAliveSecs);


/**
 * Remove a signaling keepalive.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param contactId Contact ID used for the keep alive.  sipXtapi will
 *        remove keep alives from the interface identified by the 
 *        contactId.
 * @param type Designates the method of keep alives.
 * @param remoteIp Remote IP address used to send keep alives.  The caller is
 *        responsible for converting hostnames to IP address.  This value must
 *        match what was specified in sipxConfigKeepAliveAdd.
 * @param remotePort Remote port used to send keep alives.
 *
 * @see sipxConfigGetLocalContacts
 */
SIPXTAPI_API SIPX_RESULT sipxConfigKeepAliveRemove(const SIPX_INST hInst,
                                                   SIPX_CONTACT_ID contactId,
                                                   SIPX_KEEPALIVE_TYPE type,
                                                   const char* remoteIp,
                                                   int remotePort);


/**
 * Set mode of sending DTMF tones. Tones can be sent in-band, via RFC2833
 * and INFO. Currently only RFC2833 is supported.
 *
 * Generally, RFC2833 DTMF should always be enabled.  In-band DTMF
 * can be distorted and unrecognized by gateways/IVRs/ACDs when using
 * compressed codecs such as G729.  By nature, many compressed codecs
 * are lossy and cannot regenerate DTMF signals.  If you find that you 
 * need to disable out-of-band DTMF (due to duplicate DTMF signals) on 
 * another device, please consider reconfiguring that other device.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnable Enable or disable out-of-band DTMF tones.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetOutboundDTMFMode(const SIPX_INST hInst,
                                                       const SIPX_OUTBOUND_DTMF_MODE mode);

/**
* Determines mode of sending DTMF tones.
*
* @param hInst Instance pointer obtained by sipxInitialize
* @param bEnable in-band DTMF tones enabled or disabled.
*/
SIPXTAPI_API SIPX_RESULT sipxConfigGetOutboundDTMFMode(const SIPX_INST hInst,
                                                       SIPX_OUTBOUND_DTMF_MODE* mode);


/**
 * Enables reception of certain types of DTMF. Setting will be in effect
 * for all new calls for all sipxtapi instances. Old calls will not be affected. 
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param mode Mode of DTMF to enable/disable
 * @param bEnable Set to true to enable, false to disable
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableInboundDTMF(const SIPX_INST hInst,
                                                     SIPX_INBOUND_DTMF_MODE mode,
                                                     int bEnable);

/**
* Determines the state of reception of certain types of DTMF. This setting
* is global for all sipxtapi instances.
*
* @param hInst Instance pointer obtained by sipxInitialize
* @param mode Mode of DTMF to inquire
* @param bEnabled Returns state of reception
*/
SIPXTAPI_API SIPX_RESULT sipxConfigIsInboundDTMFEnabled(const SIPX_INST hInst,
                                                        SIPX_INBOUND_DTMF_MODE mode,
                                                        int* bEnabled);

/**
 * Enable or disable sending RTCP reports.  By default, RTCP is enabled and
 * sends reports every ~5 seconds.  RTCP is enabled by default.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnable Enable or disable sending of RTCP reports.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableRTCP(const SIPX_INST hInst,
                                              const int bEnable);
  	 

/**
 * Enables/disables sending of DNS SRV request for all sipXtapi instances. 
 * DNS SRV is enabled by default.
 *
 * @param bEnable Enable or disable DNS SRV resolution.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableDnsSrv(const int bEnable);


/**
 * Get the sipXtapi API version string.
 *
 * @param szVersion Buffer to store the version string. A zero-terminated 
 *        string will be copied into this buffer on success.
 * @param nBuffer Size of szBuffer in bytes (not to exceed). A size of 48 bytes
 *        should be sufficient in most cases.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVersion(char* szVersion,
											             const size_t nBuffer);

/**
 * Get the local UDP port for SIP signaling.  The port is supplied in the 
 * call to sipXinitialize; however, the port may be allocated dynamically.
 * This method will return SIPX_RESULT_SUCCESS if able to return the port
 * value.  SIPX_RESULT_FAILURE is returned if the protocol is not enabled.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pPort Pointer to a port number.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipUdpPort(SIPX_INST hInst,
                                                      int* pPort);


/**
 * Get the local TCP port for SIP signaling.  The port is supplied in the 
 * call to sipXinitialize; however, the port may be allocated dynamically.
 * This method will return SIPX_RESULT_SUCCESS if able to return the port
 * value.  SIPX_RESULT_FAILURE is returned if the protocol is not enabled.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pPort Pointer to a port number.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipTcpPort(SIPX_INST hInst,
                                                      int* pPort);


/**
 * Get the local TLS port for SIP signaling.  The port is supplied in the 
 * call to sipXinitialize; however, the port may be allocated dynamically.
 * This method will return SIPX_RESULT_SUCCESS if able to return the port
 * value.  SIPX_RESULT_FAILURE is returned if the protocol is not enabled.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pPort Pointer to a port number.  This value must not be NULL.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalSipTlsPort(SIPX_INST hInst,
                                                      int* pPort);

/**
 * Set the codecs by short names. The name must match one of the supported codecs
 * otherwise this function will fail. Codecs must be separated by " ".
 *
 * Only one ILBC version should be enabled at time - either 20ms or 30ms. Enabling
 * both versions might confuse other clients, since rfc3952 didn't expect to handle
 * two ILBC codec offers in SDP. 20ms ILBC version might be overridden, so it is
 * possible that 30ms version will be used even if 20ms was selected.
 * sipXtapi is capable of decoding both 20ms and 30ms ILBC regardless of mode negotiated
 * in SDP.
 *
 * This method will return SIPX_RESULT_SUCCESS if able to set audio codecs.
 * SIPX_RESULT_FAILURE is returned if the codec is not set.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCodecNames multiple codec names separated by space.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSelectAudioCodecByName(const SIPX_INST hInst, 
                                                          const char* szCodecNames);

/**
 * Get the number of selected audio codecs. 
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the number of codecs can
 * no be retrieved.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pNumCodecs Pointer to the number of codecs.  This value must not be NULL. 
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetNumSelectedAudioCodecs(const SIPX_INST hInst, 
                                                             int* pNumCodecs);

/**
* Get the number of all available audio codecs. 
* This method will return SIPX_RESULT_SUCCESS if able to get the number of
* codecs. SIPX_RESULT_FAILURE is returned if the number of codecs can
* no be retrieved.
* 
* @param hInst Instance pointer obtained by sipxInitialize
* @param pNumCodecs Pointer to the number of codecs.  This value must not be NULL. 
*
*/
SIPXTAPI_API SIPX_RESULT sipxConfigGetNumAvailableAudioCodecs(const SIPX_INST hInst, 
                                                              int* pNumCodecs);

/**
 * Get the audio codec at a certain index in the list of codecs. Use this 
 * function in conjunction with sipxConfigGetNumSelectedAudioCodecs to enumerate
 * the list of selected audio codecs. This method in conjunction with 
 * sipxConfigGetNumSelectedAudioCodecs will enumerate only currently selected codecs,
 * and not all available codecs.
 *
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the audio codec can not
 * be retrieved.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param index Index in the list of codecs
 * @param pCodec SIPX_AUDIO_CODEC structure that holds information
 *        (name, bandwidth requirement) about the codec.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetSelectedAudioCodec(const SIPX_INST hInst, 
                                                         const int index, 
                                                         SIPX_AUDIO_CODEC* pCodec);

/**
* Get the audio codec at a certain index in the list of codecs. Use this 
* function in conjunction with sipxConfigGetNumAvailableAudioCodecs to enumerate
* the list of available audio codecs. This method in conjunction with 
* sipxConfigGetNumAvailableAudioCodecs will enumerate all supported codecs.
*
* This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
* preferences.  SIPX_RESULT_FAILURE is returned if the audio codec can not
* be retrieved.
* 
* @param hInst Instance pointer obtained by sipxInitialize
* @param index Index in the list of codecs
* @param pCodec SIPX_AUDIO_CODEC structure that holds information
*        (name, bandwidth requirement) about the codec.
*
*/
SIPXTAPI_API SIPX_RESULT sipxConfigGetAvailableAudioCodec(const SIPX_INST hInst, 
                                                          const int index, 
                                                          SIPX_AUDIO_CODEC* pCodec);

/**
 * Gets the list of video capture devices.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param arrSzCaptureDevices Array of character arrays to be populated
 *        by this function call.
 * @param nDeviceStringLength Length of buffer in arrSzCaptureDevice array.
 * @param nArrayLength Number of strings (of length nDeviceStringLength) in
 *        the arrSzCaptureDevice array.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCaptureDevices(const SIPX_INST hInst,
                                                          char **arrSzCaptureDevices,
                                                          int nDeviceStringLength,
                                                          int nArrayLength);
                                                          

/**
 * Gets the current video capture device.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCaptureDevice Character array to be populated by this function 
 *        call. 
 * @param nLength Max length of szCaptureDevice buffer.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetVideoCaptureDevice(const SIPX_INST hInst,
                                                         char* szCaptureDevice,
                                                         int nLength);
                                                          
/**
 * Sets the video capture device.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCaptureDevice Pointer to a character array containing the
 *        name of the desired video capture device.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCaptureDevice(const SIPX_INST hInst,
                                                         const char* szCaptureDevice);
                                                          
                            
/**
 * Set the codec by name. The name must match one of the supported codecs
 * otherwise this function will fail.
 * This method will return SIPX_RESULT_SUCCESS if able to set the video codec.
 * SIPX_RESULT_FAILURE is returned if the codec is not set.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szCodecName codec name
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSelectVideoCodecByName(const SIPX_INST hInst, 
                                                          const char* szCodecName);

/**
 * Reset the codec list if it was modified by sipxConfigSelectVideoCodecByName. This
 * resets the selection to a full codec list.
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec.
 * SIPX_RESULT_FAILURE is returned if the codec is not set.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigResetSelectedVideoCodecs(const SIPX_INST hInst);

/**
 * Get the number of selected video codecs. 
 * This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the number of codecs can
 * no be retrieved.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pNumCodecs Pointer to the number of codecs.  This value must not be NULL. 
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetNumSelectedVideoCodecs(const SIPX_INST hInst, 
                                                             int* pNumCodecs);

/**
* Get the number of available video codecs. 
* This method will return SIPX_RESULT_SUCCESS if able to set the audio codec
* preferences.  SIPX_RESULT_FAILURE is returned if the number of codecs can
* no be retrieved.
* 
* @param hInst Instance pointer obtained by sipxInitialize
* @param pNumCodecs Pointer to the number of codecs.  This value must not be NULL. 
*
*/
SIPXTAPI_API SIPX_RESULT sipxConfigGetNumAvailableVideoCodecs(const SIPX_INST hInst, 
                                                              int* pNumCodecs);

/**
 * Set the supported video format
 * This method will limit the supported video format to either
 * VIDEO_FORMAT_CIF (352x288), VIDEO_FORMAT_QCIF (176x144),
 * VIDEO_FORMAT_SQCIF (128x92), or VIDEO_FORMAT_QVGA (320x240).
 * The method will return SIPX_RESULT_SUCCESS if it is able to set the video
 * format, SIPX_RESULT_FAILURE is returned if the video format can not be set.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoFormat(const SIPX_INST hInst,
                                                  SIPX_VIDEO_FORMAT videoFormat);


/**
 * Get selected video codec at a certain index in the list of codecs. Use this 
 * function in conjunction with sipxConfigGetNumSelectedVideoCodecs to enumerate
 * the list of currently selected video codecs.
 * This method will return SIPX_RESULT_SUCCESS if able to set the video codec
 * preferences.  SIPX_RESULT_FAILURE is returned if the video codec can not
 * be retrieved.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param index Index in the list of codecs
 * @param pCodec SIPX_VIDEO_CODEC structure that holds information
 *        (name, bandwidth requirement) about the codec.
 *
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetSelectedVideoCodec(const SIPX_INST hInst, 
                                                         const int index, 
                                                         SIPX_VIDEO_CODEC* pCodec);

/**
* Get available video codec at a certain index in the list of codecs. Use this 
* function in conjunction with sipxConfigGetNumAvailableVideoCodecs to enumerate
* the list of all available video codecs.
* This method will return SIPX_RESULT_SUCCESS if able to set the video codec
* preferences.  SIPX_RESULT_FAILURE is returned if the video codec can not
* be retrieved.
* 
* @param hInst Instance pointer obtained by sipxInitialize
* @param index Index in the list of codecs
* @param pCodec SIPX_VIDEO_CODEC structure that holds information
*        (name, bandwidth requirement) about the codec.
*
*/
SIPXTAPI_API SIPX_RESULT sipxConfigGetAvailableVideoCodec(const SIPX_INST hInst, 
                                                          const int index, 
                                                          SIPX_VIDEO_CODEC* pCodec);

/**
 * Get the local contact address available for outbound/inbound signaling and
 * audio.  The local contact addresses will always include the local IP 
 * addresses.  The local contact addresses may also include external NAT-
 * derived addresses (e.g. STUN).  See the definition of SIPX_CONTACT_ADDRESS
 * for more details on the structure.
 *
 * Determining which contact address to use depends on your network topology.  
 * If you have a proxy/edge proxy within the same firewall/NAT space, you can 
 * use the LOCAL UDP, TCP, or TLS contact type for your calls.  If your 
 * proxy resides outside of the firewall/NAT space, you should use the 
 * NAT_MAPPED or RELAY contact type (UDP only).  Both NAT_MAPPED and RELAY 
 * use your STUN-derived IP address, however RELAY requests TURN for media 
 * paths. 
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param addresses A pre-allocated list of SIPX_CONTACT_ADDRESS 
 *        structures.  This data will be filled in by the API call.
 * @param nMaxAddresses The maximum number of addresses supplied by the 
 *        addresses parameter.
 * @param nActualAddresses The actual number of addresses populated in
 *        the addresses parameter.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalContacts(const SIPX_INST hInst,
                                                    SIPX_CONTACT_ADDRESS addresses[],
                                                    size_t nMaxAddresses,
                                                    size_t* nActualAddresses);

/**
 * Get our local ip/port combination for the designated remote ip/port.  This
 * API will look at all of the stun and/or SIP message results to see if a
 * NAT binding exists for this particular host. If using a proxy server, 
 * this is generally never needed, however, in peer-to-peer modes this can 
 * sometimes help you get through NATs when using out-of-band registrars /
 * signaling helpers (not recommended -- use a proxy instead).
 *
 * For this API to be useful, you need to add a keepalive to the remote host
 * prior to calling this API.  This may optionally block if a keep-alive request 
 * has been started, but we are waiting for a response.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szRemoteIp IP of remote party
 * @param iRemotePort port or remote party
 * @param szContactIp Buffer to place local contact IP if successful
 * @param nContactIpLength Length of szContactIp buffer
 * @param iContactPort Int to place contact port
 * @param iTimeoutMs Timeout in MS.  Values of 0 (or less) signal not to 
 *        block.  Any other value is rounded up to multiple of 50ms.  For
 *        VoIP, a value of 500ms seems plenty (latency longer than 300ms 
 *        will result in a fairly bad audio experience).
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetLocalFeedbackAddress(const SIPX_INST hInst,
                                                           const char* szRemoteIp,
                                                           const int iRemotePort,
                                                           char* szContactIp,
                                                           size_t nContactIpLength,
                                                           int* iContactPort,
                                                           int iTimeoutMs);

/**
 * Populates an array of IP Addresses in char* form.  Caller must supply array of char*.
 * Function will allocate memory for the IP addresses and adapter names, which must
 * be freed by caller.
 *
 * @param arrAddresses Array to be popluated with ip addresses.
 * @param arrAddressAdapter For each record in arrAddresses, there is a corresponding record,
 *        with the same index, in arrAddressAdpater which represents the 
 *        "sipx adapter name" for that address
 * @param numAddresses Input: Size of the preallocated arrays.
 *                     Output: Number of IPs found by the system.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetAllLocalNetworkIps(char* arrAddresses[], 
                                                         char* arrAddressAdapter[],
                                                         int* numAddresses);
                                                         
/**
 * Set security parameters for TLS. Needs to be called before sipxInitialize and
 * all other functions. Can be executed only once, subsequent executions have no effect.
 * Certificates used must be in PEM format.
 *
 * To generate testing private key without encryption:
 * openssl genrsa -out mykey.pem 1024
 * 
 * Generate a certificate using the new key:
 * openssl req -new -x509 -key mykey.pem -out mycert.pem -days 3650
 *
 * without -x509 option a CSR (certificate request) is created. This can be sent to a 
 * certificate authority to be signed. Signed certificate can then be also used by sipXtapi.
 *
 * WARNING: when using TLS, it is recommended to compile OpenSSL from source code in Windows.
 * Debug and Release builds of sipXtapi with TLS require separate libeay32.lib, ssleay32.lib files
 * for Debug and Release mode. Mixing these will result in a crash.
 * 
 * @param verificationMode Determines how SSL/TLS certificates are verified. By default
 *        they are verified internally using callback. Certificates with invalid date,
 *        CN and hostname mismatch or not signed by approved CA are rejected. Use
 *        SIPX_SSL_ALWAYS_ACCEPT for testing certificates.
 * @param szCApath Path to directory containing several CA certificates. Links to certificates
 *        must be prepared with c_rehash according to http://www.openssl.org/docs/ssl/SSL_CTX_load_verify_locations.html
 *        If not supplied, then ./ssl/authorities is used.
 * @param szCAfile Alternative to szCApath. Path to PEM file (base64) containing CA certificates.
 *        If set to NULL, no CAfile is used.
 * @param szCertificateFile Path to PEM file with OpenSSL certificate. Certificate should be
 *        unencrypted. By default ./ssl/ssl.crt is used.
 * @param szPrivateKeyFile Path to PEM file with OpenSSL private key. Can be encrypted.
 *        By default ./ssl/ssl.key is used.
 * @param szPassword Password to use for decrypting any private keys or certificates.
 *        If private key is not encrypted, it will not be used.
 * 
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetTLSSecurityParameters(SIPX_SSL_CRT_VERIFICATION verificationMode = SIPX_SSL_VERIFICATION_DEFAULT,
                                                            const char* szCApath = NULL,
                                                            const char* szCAfile = NULL,
                                                            const char* szCertificateFile = NULL,
                                                            const char* szPrivateKeyFile = NULL,
                                                            const char* szPassword = NULL);

/**
 * Enables/Disables use of short field names in sip messages.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnabled True if short names, false if long names
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipShortNames(const SIPX_INST hInst, 
                                                       const int bEnabled);

/**
 * Enables/Disables use of date header in sip messages.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnabled True if date header, false if no date header
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipDateHeader(const SIPX_INST hInst, 
                                                       const int bEnabled);
/**
 * Enables/Disables use of Allow header in sip messages. If disabled, then
 * Allow will never be sent in requests or responses, even in OPTIONS responses.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnabled True if Allow header, false if no Allow header
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipAllowHeader(const SIPX_INST hInst, 
                                                        const int bEnabled);
/**
* Enables/Disables use of Supported header in sip messages. If disabled, then
* Supported will never be sent in requests or responses, even in OPTIONS responses.
*
* @param hInst Instance pointer obtained by sipxInitialize
* @param bEnabled True if Supported header, false if no Supported header
*/
SIPXTAPI_API SIPX_RESULT sipxConfigEnableSipSupportedHeader(const SIPX_INST hInst, 
                                                            const int bEnabled);

/**
 * Sets the Accept Language used in sip messages. e.g. - "EN"
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szLanguage - Accept Language string
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetSipAcceptLanguage(const SIPX_INST hInst, 
                                                        const char* szLanguage);
/**
 * Sets the location header for SIP messages.  The location header will be
 * included in SIP requests and responses.  You can disable the location 
 * header on a call-by-call basis in the by changing the bEnableLocationHeader
 * flag on sipxCallAccept, sipxCallConnect, and sipxConferenceAdd methods.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param szHeader - Location header
 *
 * @see sipxCallAccept
 * @see sipxCallConnect
 * @see sipxConferenceAdd
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetLocationHeader(const SIPX_INST hInst, 
                                                     const char* szHeader);

/**
 * Set the connection idle timeout.  If a media connection is idle for this
 * threshold, a SILENCE event will be fired to the application layer.  
 * 
 * Applications may decide to tear down the call after receiving this event 
 * under the assumption that the remote party is gone away.  Be careful when 
 * using codecs that support silence suppression -- Some implementations 
 * continue to send RTP heartbeats, however, others will not send any data 
 * and may appear to be dead.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param idleTimeout The time in seconds that a socket is idle before a
 *        MEDIA_REMOTE_SILENT event is fired.        
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetConnectionIdleTimeout(const SIPX_INST hInst, 
                                                            const int idleTimeout);

/**
 * Call this function to prepare a sipXtapi instance for a 
 * system hibernation.  This function is not thread-safe.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 */
SIPXTAPI_API SIPX_RESULT sipxConfigPrepareToHibernate(const SIPX_INST hInst);


/**
 * Call this function upon returning from a system hibernation.
 * This function is not thread-safe.
 * @param hInst Instance pointer obtained by sipxInitialize
 */
SIPXTAPI_API SIPX_RESULT sipxConfigUnHibernate(const SIPX_INST hInst);


/**
 * Enables RTP streaming over TCP.  Enabling this feature
 * allows the application to use RTP streaming over TCP or UDP.
 * RTP over TCP is currently not implemented.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bEnable True allows RTP-over-TCP, false disallows it.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigEnableRtpOverTcp(const SIPX_INST hInst,
                                                    int bEnable);

/**
 * Sets the display object for the "video preview".
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param pDisplay Pointer to a video preview display object.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoPreviewDisplay(const SIPX_INST hInst, 
                                                          SIPX_VIDEO_DISPLAY* const pDisplay);


/**
 * Updates the Preview window with a new frame buffer.  Should be called
 * when the window receives a PAINT message.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param hWnd Window handle of the video preview window.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigUpdatePreviewWindow(const SIPX_INST hInst,
                                                       const SIPX_WINDOW_HANDLE hWnd);


/**
 * Sets the video quality.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param quality Id setting the video quality.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoQuality(const SIPX_INST hInst,
                                                   const SIPX_VIDEO_QUALITY_ID quality);

/**
 * Sets the bit rate and frame rate parameters for video.
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bitRate Bit rate parameter in kbps
 * @param frameRate Frame rate parameter frames per second
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoParameters(const SIPX_INST hInst, 
                                                      const int bitRate,
                                                      const int frameRate);

/**
 * Sets the video bitrate
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param bitRate Bit rate parameter
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoBitrate(const SIPX_INST hInst, 
                                                   const int bitRate);

/**
 * Sets the video framerate
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param frameRate Frame rate parameter 
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoFramerate(const SIPX_INST hInst, 
                                                     const int frameRate);

/**
 * Set the cpu usage
 *
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param cpuUsage CPU usage in percent
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetVideoCpuUsage(const SIPX_INST hInst, 
                                                    const int cpuUsage);


/** 
 * Subscribe for NOTIFY events which may be published by another end-point or
 * server.
 *
 * Successful invocation will result in sending SIP SUBSCRIBE message in a new
 * sip dialog.
 *
 * sipXtapi will automatically refresh subscriptions until 
 * sipxConfigUnsubscribe is called.  Please make sure you call 
 * sipxCallUnsubscribe before tearing down your instance of sipXtapi.
 * 
 * @param hInst Instance pointer obtained by sipxInitialize
 * @param hLine Line Identity for the outbound call. Line identity 
 *        defines the "From" field.
 * @param szTargetUrl The Url of the publishing end-point. 
 * @param szEventType A string representing the type of event that can be 
 *        published.  This string is used to populate the "Event" header in
 *        the SIP SUBSCRIBE request.  For example, if checking voicemail 
 *        status, your would use "message-summary". For presence, it would be
 *        "presence". Acceptable values can be found in event package RFCs.
 * @param szAcceptType A string representing the types of NOTIFY events that 
 *        this client will accept.  This string is used to populate the 
 *        "Accept" header in the SIP SUBSCRIBE request.  For example, if
 *        checking voicemail status, you would use "application/simple-message-summary".
 *        This value is optional. Check event package RFC for its meaning.
 * @param contactId Id of the desired contact record to use for this call.
 *        The id refers to a Contact Record obtained by a call to
 *        sipxConfigGetLocalContacts.  The application can choose a 
 *        contact record of type LOCAL, NAT_MAPPED, CONFIG, or RELAY.
 *        The Contact Type allows you to control whether the
 *        user agent and  media processing advertises the local address
 *         (e.g. LOCAL contact of 10.1.1.x or 
 *        192.168.x.x), the NAT-derived address to the target party,
 *        or, local contact addresses of other types.
 * @param phSub Pointer to a subscription handle whose value is set by this 
 *        funtion.  This handle allows you to cancel the subscription and
 *        differeniate between NOTIFY events.
 * @param subscriptionPeriod Subscription expiration period. After this
 *        period, new SUBSCRIBE message will be sent.
 * @param transport Transport which should be used for sending SUBSCRIBE request.
 *        If contactId identifies a specific contact (not automatic), then
 *        transport must match contact transport.
 */ 
SIPXTAPI_API SIPX_RESULT sipxConfigSubscribe(const SIPX_INST hInst, 
                                             const SIPX_LINE hLine, 
                                             const char* szTargetUrl, 
                                             const char* szEventType, 
                                             const char* szAcceptType, 
                                             const SIPX_CONTACT_ID contactId, 
                                             SIPX_SUB* phSub,
                                             int subscriptionPeriod = 3600,
                                             SIPX_TRANSPORT_TYPE transport = TRANSPORT_AUTO); 
/**
 * Unsubscribe from previously subscribed NOTIFY events.  This method will
 * send another subscription request with an expires time of 0 (zero) to end
 * your subscription.
 *
 * @param hSub The subscription handle obtained from the call to 
 *             sipxConfigSubscribe.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigUnsubscribe(const SIPX_SUB hSub); 


/**
 * Gets current session interval defined in RFC4028 (session timers) - time between
 * session updates (INVITE or UPDATE) in seconds. If refresh fails, call is dropped.
 *
 * @param hInst An instance handle obtained from sipxInitialize.
 * @param iSessionInterval Current value of session timer expiration.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetSessionTimer(const SIPX_INST hInst,
                                                   int* iSessionInterval,
                                                   SIPX_SESSION_TIMER_REFRESH* refresh);

/**
 * Sets current session interval defined in RFC4028 (session timers) - time between
 * session updates (re-INVITE or UPDATE) in seconds. If refresh fails, call is dropped.
 * By default re-INVITE is used unless UPDATE is enabled.
 *
 * @param hInst An instance handle obtained from sipxInitialize.
 * @param iSessionInterval New value of session timer expiration. Values lower than 90
 *        are ignored. Minimum value is 90.
 * @param refresh Configures side which is responsible for session refresh. Can be automatic,
 *        local, or remote.
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetSessionTimer(const SIPX_INST hInst,
                                                   int iSessionInterval,
                                                   SIPX_SESSION_TIMER_REFRESH refresh);

/**
 * Gets configuration of SIP UPDATE method. By default we accept UPDATE, but never send it.
 * SIP UPDATE is faster, but could potentially be rejected, as it requires an immediate response.
 * It is possible to enable UPDATE for hold/unhold/session refresh/codec renegotiation. If remote
 * side doesn't support UPDATE, then re-INVITE will be used.
 *
 * @param hInst An instance handle obtained from sipxInitialize.
 * @param updateConfig Configuration of SIP UPDATE method.
 *
 * @see SIPX_SIP_UPDATE_CONFIG
 */
SIPXTAPI_API SIPX_RESULT sipxConfigGetUpdateSetting(const SIPX_INST hInst,
                                                    SIPX_SIP_UPDATE_CONFIG* updateConfig);

/**
 * Sets configuration of SIP UPDATE method.
 * It is recommended to leave this setting at default value.
 *
 * @param hInst An instance handle obtained from sipxInitialize.
 * @param updateConfig Configuration of SIP UPDATE method.
 *
 * @see sipxConfigGetUpdateSetting
 * @see SIPX_SIP_UPDATE_CONFIG
 */
SIPXTAPI_API SIPX_RESULT sipxConfigSetUpdateSetting(const SIPX_INST hInst,
                                                    SIPX_SIP_UPDATE_CONFIG updateConfig);

/**
* Gets configuration of 100rel (PRACK) support. 100rel enables sending of reliable provisional
* responses, which are by default unreliable. SDP may also be negotiated in reliable 18x
* responses and PRACKs.
*
* @param hInst An instance handle obtained from sipxInitialize.
* @param relConfig Configuration 100rel support.
*
* @see SIPX_100REL_CONFIG
*/
SIPXTAPI_API SIPX_RESULT sipxConfigGet100relSetting(const SIPX_INST hInst,
                                                    SIPX_100REL_CONFIG* relConfig);

/**
* Sets configuration of 100rel (PRACK) support. See rfc3262.
* It is recommended to leave this setting at default value.
*
* @param hInst An instance handle obtained from sipxInitialize.
* @param relConfig Configuration 100rel support.
*
* @see sipxConfigGet100relSetting
* @see SIPX_100REL_CONFIG
*/
SIPXTAPI_API SIPX_RESULT sipxConfigSet100relSetting(const SIPX_INST hInst,
                                                    SIPX_100REL_CONFIG relConfig);

/**
* Gets configuration of SDP offering mode.
*
* @param hInst An instance handle obtained from sipxInitialize.
* @param sdpOfferingMode Configuration of SDP offering. Can be early or immediate.
*        Immediate SDP offering is the default.
*
* @see SIPX_SDP_OFFERING_MODE
*/
SIPXTAPI_API SIPX_RESULT sipxConfigGetSdpOfferingMode(const SIPX_INST hInst,
                                                      SIPX_SDP_OFFERING_MODE* sdpOfferingMode);

/**
* Sets configuration of SDP offering mode. This setting only affects the initial
* INVITE, and not subsequent re-INVITE requests used to hold/unhold call. It is not
* possible to use late SDP negotiation for unhold, since remote party might disallow
* audio stream activation.
* For outbound calls, we can send SDP offer either in initial INVITE, or SDP answer in ACK.
* If 100rel extension is supported by remote client, and there was no SDP offer in INVITE,
* one will be received in reliable 18x response. SDP answer will then be sent in PRACK request.
*
* Late SDP negotiation saves media resources, if call is likely to be rejected.
*
* @param hInst An instance handle obtained from sipxInitialize.
* @param sdpOfferingMode Configuration of SDP offering. Can be early or immediate.
*        Immediate SDP offering is the default.
*
* @see sipxConfigGetSdpOfferingMode
* @see SIPX_SDP_OFFERING_MODE
*/
SIPXTAPI_API SIPX_RESULT sipxConfigSetSdpOfferingMode(const SIPX_INST hInst,
                                                      SIPX_SDP_OFFERING_MODE sdpOfferingMode = SIPX_SDP_OFFERING_IMMEDIATE);

//@}
/** @name Utility Functions */
//@{

/**
 * Simple utility function to parse the username, host, and port from
 * a URL.  All url, field, and header parameters are ignored.  You may also 
 * specify NULL for any parameters (except szUrl) which are not needed.  
 * Lastly, the caller must allocate enough storage space for each url
 * component -- if in doubt use the length of the supplied szUrl.
 */
SIPXTAPI_API SIPX_RESULT sipxUtilUrlParse(const char* szUrl,
                                          char* szUsername,
                                          char* szHostname,
                                          int* iPort);

/**
 * Simple utility function to parse the display name from a SIP URL.
 */
SIPXTAPI_API SIPX_RESULT sipxUtilUrlGetDisplayName(const char* szUrl,
                                                   char* szDisplayName,
                                                   size_t nDisplayName);

/**
 * Simple utility function to update a URL.  If the szUrl isn't large enough,
 * or is NULL, this function will fail, however, the nUrl will contained the 
 * required size in bytes.
 *
 * To leave an existing component unchanged, use NULL for strings and -1 for 
 * ports.
 */
SIPXTAPI_API SIPX_RESULT sipxUtilUrlUpdate(char* szUrl,
                                           size_t* nUrl,
                                           const char* szNewUsername,
                                           const char* szNewHostname,
                                           const int iNewPort);


/**
 * Get the Nth named url parameter from the designated url. 
 *
 * @param szUrl The url to parse
 * @param szParamName Name of the url parameter
 * @param nParamIndex Index of the url parameter (zero-based).  This is used 
 *        if you have multiple url parameters with the same name -- otherwise,
 *        you should use 0.
 * @param szParamValue Buffer to place parameter value
 * @param nParamValue size of parameter value buffer (szParamValue)
 */
SIPXTAPI_API SIPX_RESULT sipxUtilUrlGetUrlParam(const char* szUrl,
                                                const char* szParamName,
                                                size_t nParamIndex,
                                                char* szParamValue,
                                                size_t nParamValue);
//@}

/**
* Adds a log entry to the system log - made necessary to add logging
* capability on the API level.
* 
* @param logLevel priority of the log entry
* @param format a format string for the following variable argument list
*/
SIPXTAPI_API void sipxLogEntryAdd(SIPX_LOG_LEVEL logLevel, 
                                  const char *format,
                                  ...);

/**
* Send pager message to the destinationAor. This function is experimental
* and is not meant for public use. It might be removed at any time.
*
* @param hInst SipXtapi instance handle
* @param destinationAor Destination URI
* @param messageText Text of the message to send
* @param subject Subject of the message
* @param responseCode Integer response code. If message was delivered
*        should be 200
* @param responseCodeText Text of the response code
* @param buffLength Length of responseCodeText buffer
* @param transport Transport which should be used for sending MESSAGE request
*/
SIPXTAPI_API SIPX_RESULT sipxPIMSendPagerMessage(SIPX_INST hInst,
                                                 const char* destinationAor, 
                                                 const char* messageText,
                                                 const char* subject,
                                                 int* responseCode,
                                                 char* responseCodeText,
                                                 size_t buffLength,
                                                 SIPX_TRANSPORT_TYPE transport = TRANSPORT_AUTO);

/**
 * Saves full memory dump to given file. It is supported only in Windows.
 * A .dmp file will be produced, which can be opened in visual studio,
 * where threads and memory content can be investigated.
 *
 * @param filePath Path and name of file to which memory content should
 *                 be saved
 */
SIPXTAPI_API SIPX_RESULT sipxSaveMemoryDump(const char* filePath);

#endif // _sipXtapi_h_
