//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpVolumeMeterBase_h__
#define MpVolumeMeterBase_h__

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
* Base class for calculating volume level from samples of different sample size.
* It uses either VU or PPM algorithm.
*/
class MpVolumeMeterBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor.
   MpVolumeMeterBase()
   {
   }

   /// Destructor.
   virtual ~MpVolumeMeterBase(void)
   {
   }

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /**
   * Pushes a buffer with frames to volume meter. Frame is composed
   * of sample for each channel. For mono, sample=frame.
   */
   virtual void pushBuffer(const void* pBuffer, unsigned int frameCount) = 0;

   /**
   * Resets volume meter
   */
   virtual void resetMeter() = 0;

   /**
   * Calculates volume by VU algorithm.
   */
   virtual double getVUVolume() const = 0;

   /**
   * Calculates volume by PPM reading
   */
   virtual double getPPMVolume() const = 0;

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
};

#endif // MpVolumeMeterBase_h__
