//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpVolumeMeter_h__
#define MpVolumeMeter_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpVolumeMeterBase.h"

// DEFINES
#ifndef ABS
#define ABS(x) ((x) < 0 ? (-(x)) : (x))
#endif

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
* Class for calculating volume level from samples of different sample size.
* It uses either VU or PPM algorithm.
*/
template <class SampleType, int maxSampleValue>
class MpVolumeMeter : public MpVolumeMeterBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor.
   MpVolumeMeter(int channelCount = 1, double sampleRate = 8000)
   : m_channelCount(channelCount)
   , m_sampleRate(sampleRate)
   , m_samplesBufferPos(0)
   , m_samplesBufferSize(0)
   , m_pSamplesBuffer(NULL)
   {
      // use buffer for 40ms of data
      int samplesPerChannel = (int)(((double)40 / 1000) * sampleRate);
      m_samplesBufferSize = channelCount * samplesPerChannel;
      if (m_samplesBufferSize > 0)
      {
         m_pSamplesBuffer = new SampleType[m_samplesBufferSize];
         memset(m_pSamplesBuffer, 0, m_samplesBufferSize * sizeof(SampleType));
      }
   }

   /// Destructor.
   virtual ~MpVolumeMeter(void)
   {
      delete [] m_pSamplesBuffer;
   }
   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{
   
   /**
    * Pushes a buffer with frames to volume meter. Frame is composed
    * of sample for each channel. For mono, sample=frame.
    */
   virtual void pushBuffer(const void* pBuffer, unsigned int frameCount)
   {
      if (m_pSamplesBuffer)
      {
         SampleType* ptr = (SampleType*)pBuffer;
         unsigned int topIndex = m_channelCount * frameCount;
         for (unsigned int i = 0; i < topIndex; i++)
         {
            m_pSamplesBuffer[m_samplesBufferPos] = ptr[i];
            m_samplesBufferPos = (m_samplesBufferPos + 1) % m_samplesBufferSize;
         }
      }
   }

   /**
    * Resets volume meter
    */
   virtual void resetMeter()
   {
      if (m_pSamplesBuffer)
      {
         m_samplesBufferPos = 0;
         memset(m_pSamplesBuffer, 0, m_samplesBufferSize * sizeof(SampleType));
      }
   }

   /**
    * Calculates volume by VU algorithm.
    */
   virtual double getVUVolume() const
   {
      if (m_pSamplesBuffer)
      {
         // count sum of all sample values divided by buffer size
         double volume = 0.0;
         for (int i = 0; i < m_samplesBufferSize; i++)
         {
            volume+= ABS(m_pSamplesBuffer[i]);
         }
         volume = volume / m_samplesBufferSize;

         return ((volume / maxSampleValue) * 100);
      }

      return 0;
   }

   /**
    * Calculates volume by PPM reading
    */
   virtual double getPPMVolume() const
   {
      if (m_pSamplesBuffer)
      {
         // find maximum from all samples
         SampleType max = 0;
         for (int i = 0; i < m_samplesBufferSize; i++)
         {
            if (max < ABS(m_pSamplesBuffer[i]))
            {
               max = ABS(m_pSamplesBuffer[i]);
            }
         }

         return (((double)max / maxSampleValue) * 100);
      }

      return 0;
   }

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{
   //@}
   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{
   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int m_channelCount;
   double m_sampleRate;
   SampleType* m_pSamplesBuffer;
   int m_samplesBufferSize;
   int m_samplesBufferPos;
};

#endif // MpVolumeMeter_h__
