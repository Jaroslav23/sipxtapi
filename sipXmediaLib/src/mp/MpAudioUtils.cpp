//  
// Copyright (C) 2006-2007 SIPez LLC. 
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


// SYSTEM INCLUDES
#include <stdlib.h>

#ifdef WIN32 /* [ */
#   ifndef WINCE
#       include <io.h>
#       include <fcntl.h>
#   endif   // ifndef WINCE
#endif /* WIN32 ] */

#ifdef __pingtel_on_posix__
#include <unistd.h>
#include <fcntl.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsFS.h"
#include "mp/MpTypes.h"
#include "mp/MpAudioUtils.h"
#include "mp/MpAudioWaveFileRead.h"

#include <os/fstream>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

#define MAX_WAVBUF_SIZE 65535

/* ============================ FUNCTIONS ================================= */

int gcd(int a, int b)
{
   if(b > a)
      return gcd(b, a);
   /* a >= b */
   while(b)
   {
      int c = a % b;
      a = b;
      b = c;
   }
   return a;
}

int mergeChannels(char * charBuffer, int Size, int nTotalChannels)
{
   MpAudioSample * buffer = (MpAudioSample *) charBuffer;
   
   if(nTotalChannels == 2)
   {
      int targetSamples = Size / (sizeof(MpAudioSample) * 2);
      int targetSample = 0, sourceSample = 0;
      
      for(; targetSample < targetSamples; targetSample++)
      {
         int mergedSample = buffer[sourceSample++];
         mergedSample += buffer[sourceSample++];
         buffer[targetSample] = mergedSample / 2;
      }
      
      return targetSample * sizeof(MpAudioSample);
   }
   /* Test for this afterwards, to optimize 2-channel mixing */
   else if(nTotalChannels == 1)
      return Size;
   
   int targetSamples = Size / (sizeof(MpAudioSample) * nTotalChannels);
   int targetSample = 0, sourceSample = 0;
   
   for(; targetSample < targetSamples; targetSample++)
   {
      int mergedSample = 0;
      for(int i = 0; i < nTotalChannels; i++)
         mergedSample += buffer[sourceSample++];
      buffer[targetSample] = mergedSample / nTotalChannels;
   }
   
   return targetSample * sizeof(MpAudioSample);
}


OsStatus WriteWaveHdr(OsFile &file)
{
    OsStatus retCode = OS_FAILED;
    char tmpbuf[80];
    short bitsPerSample = 16;

    short sampleSize = sizeof(MpAudioSample); 
    short compressionCode = 1; //PCM
    short numChannels = 1; 
    unsigned long samplesPerSecond = 8000;
    unsigned long averageSamplePerSec = samplesPerSecond*sampleSize;
    short blockAlign = sampleSize*numChannels; 
    unsigned long bytesWritten = 0;
    long TotalBytesWritten = 0;

    //write RIFF & length
    //8 bytes written
    strcpy(tmpbuf,"RIFF");
    unsigned long length = 0;
    
    file.write(tmpbuf, strlen(tmpbuf),bytesWritten);
    TotalBytesWritten += bytesWritten;
    
    file.write((char*)&length, sizeof(length),bytesWritten); //filled in on close
    TotalBytesWritten += bytesWritten;
    
    //write WAVE & length
    //8 bytes written
    strcpy(tmpbuf,"WAVE");

    file.write(tmpbuf, strlen(tmpbuf),bytesWritten);
    TotalBytesWritten += bytesWritten;

    //write fmt & length
    //8 bytes written
    strcpy(tmpbuf,"fmt ");
    length = 16;
    
    file.write(tmpbuf,strlen(tmpbuf),bytesWritten);
    TotalBytesWritten += bytesWritten;

    file.write((char*)&length,sizeof(length),bytesWritten); //filled in on close
    TotalBytesWritten += bytesWritten;
    
    //now write each piece of the format
    //16 bytes written
    file.write((char*)&compressionCode, sizeof(compressionCode),bytesWritten);
    TotalBytesWritten += bytesWritten;

    file.write((char*)&numChannels, sizeof(numChannels),bytesWritten);
    TotalBytesWritten += bytesWritten;
    
    file.write((char*)&samplesPerSecond, sizeof(samplesPerSecond),bytesWritten);
    TotalBytesWritten += bytesWritten;
    
    file.write((char*)&averageSamplePerSec, sizeof(averageSamplePerSec),bytesWritten);
    TotalBytesWritten += bytesWritten;
    
    file.write((char*)&blockAlign, sizeof(blockAlign),bytesWritten);
    TotalBytesWritten += bytesWritten;
    
    file.write((char*)&bitsPerSample, sizeof(bitsPerSample),bytesWritten);
    TotalBytesWritten += bytesWritten;


    //write data and length
    strcpy(tmpbuf,"data");
    length = 0;
    
    file.write(tmpbuf,strlen(tmpbuf),bytesWritten);
    TotalBytesWritten += bytesWritten;
    
    file.write((char*)&length,sizeof(length),bytesWritten); //filled in on close
    TotalBytesWritten += bytesWritten;
    
    //total length at this point should be 44 bytes
    if (TotalBytesWritten == 44)
        retCode = OS_SUCCESS;

    return retCode;

}


OsStatus updateWaveHeaderLengths(OsFile &file)
{
    OsStatus retCode = OS_FAILED;
    unsigned long bytesWritten = 0;
    //find out how many bytes were written so far
    unsigned long length;
    file.getLength(length);
    
    //no go back to beg
    file.setPosition(4);
    
    //and update the RIFF length
    unsigned long rifflength = length-8;
    file.write((char*)&rifflength,sizeof(length),bytesWritten);
    if (bytesWritten == sizeof(length))
    {

        //now seek to the data length
        file.setPosition(40);
    
        //this should be the length of just the data
        unsigned long datalength = length-44;
        file.write((char*)&datalength,sizeof(datalength),bytesWritten);

        if (bytesWritten == sizeof(datalength))
            retCode = OS_SUCCESS;
    }

    return retCode;
}

/* The number of bits required by each value */
static unsigned char numBits[] = {
   0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
   6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
   8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
};

/* Mu-Law conversions */
static bool muLawDecodeTableInitialized = false;
static MpAudioSample muLawDecodeTable[256];
static bool aLawDecodeTableInitialized = false;
static MpAudioSample aLawDecodeTable[256];

// Constructor initializes the decoding table
void InitG711Tables()
{
   if (!muLawDecodeTableInitialized) {
      muLawDecodeTableInitialized = true;
      for(int i=0;i<256;i++)
         muLawDecodeTable[i] = MuLawDecode2(i);
   }

   if (!aLawDecodeTableInitialized) {
      aLawDecodeTableInitialized = true;
      for(int i=0;i<256;i++)
         aLawDecodeTable[i] = ALawDecode2(i);
   }
}

size_t DecompressG711MuLaw(MpAudioSample *buffer,size_t length)
{
   unsigned char *byteBuff =
      reinterpret_cast<unsigned char *>(buffer);

   for(long i=length-1; i>=0; i--)
   {
      buffer[i] = muLawDecodeTable[byteBuff[i]];
   }
   return length;
}

unsigned char MuLawEncode2(MpAudioSample s)
{
   unsigned char sign = (s<0)?0:0x80; // Save the sign
   if (s<0) s=-s; // make sample positive
   signed long adjusted = static_cast<long>(s) << (16-sizeof(MpAudioSample)*8);
   adjusted += 128L+4L;
   if (adjusted > 32767) adjusted = 32767;
   unsigned char exponent = numBits[(adjusted>>7)&0xFF] - 1;
   unsigned char mantissa = (unsigned char)((adjusted >> (exponent + 3)) & 0xF);
   return ~(sign | (exponent << 4) | mantissa);
}

MpAudioSample MuLawDecode2(unsigned char ulaw)
{
   ulaw = ~ulaw;
   unsigned char exponent = (ulaw >> 4) & 0x7;
   unsigned char mantissa = (ulaw & 0xF) + 16;
   unsigned long adjusted = (mantissa << (exponent + 3)) - 128 - 4;
   MpAudioSample sRet = (MpAudioSample) adjusted;
   return (ulaw & 0x80)? sRet : -sRet;
}


size_t DecompressG711ALaw(MpAudioSample *buffer, size_t length)
{
   unsigned char *byteBuff =
      reinterpret_cast<unsigned char *>(buffer);

   for(long i=length-1; i>=0; i--)
      buffer[i] = aLawDecodeTable[ byteBuff[i] ];
   return length;
}

unsigned char ALawEncode2(MpAudioSample s)
{
   unsigned char sign = (s<0)?0:0x80; // Save the sign
   if (s<0) s=-s; // make sample positive
   signed long adjusted = static_cast<long>(s)+8L; // Round it
   if (adjusted > 32767) adjusted = 32767; // Clip it
   unsigned char exponent = numBits[(adjusted>>8)&0x7F];
   unsigned char mantissa = (unsigned char)((adjusted >> (exponent + 4)) & 0xF);
   return sign | (((exponent << 4) | mantissa) ^ 0x55);
}

MpAudioSample ALawDecode2(unsigned char alaw)
{
   alaw ^= 0x55;
   unsigned char exponent = (alaw >> 4) & 0x7;
   unsigned char mantissa = (alaw & 0xF) + (exponent?16:0);
   unsigned long adjusted = (mantissa << (exponent + 4));
   MpAudioSample sRet = (MpAudioSample) adjusted;
   return (alaw & 0x80)? -sRet : sRet;
}
