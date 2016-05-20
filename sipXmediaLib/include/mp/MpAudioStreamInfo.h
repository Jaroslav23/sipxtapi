//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpAudioStreamInfo_h__
#define MpAudioStreamInfo_h__

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
* Information about audio stream.
*/
class MpAudioStreamInfo
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor.
   MpAudioStreamInfo();

   /// Constructor.
   MpAudioStreamInfo(double inputLatency,
                     double outputLatency,
                     double sampleRate);

   /// Destructor.
   virtual ~MpAudioStreamInfo(void);
   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{
   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   double getInputLatency() const { return m_inputLatency; }
   void setInputLatency(double val) { m_inputLatency = val; }

   double getOutputLatency() const { return m_outputLatency; }
   void setOutputLatency(double val) { m_outputLatency = val; }

   double getSampleRate() const { return m_sampleRate; }
   void setSampleRate(double val) { m_sampleRate = val; }

   //@}
   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{
   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   double m_inputLatency; ///< input stream latency in seconds
   double m_outputLatency; ///< output stream latency in seconds
   double m_sampleRate; ///< the real sample rate of stream, may differ slightly from requested
};

#endif // MpAudioStreamInfo_h__
