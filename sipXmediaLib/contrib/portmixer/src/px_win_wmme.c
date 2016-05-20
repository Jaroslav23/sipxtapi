/*
 * PortMixer
 * Windows WMME Implementation
 *
 * Copyright (c) 2002, 2006
 *
 * Written by Dominic Mazzoni and Augustus Saunders
 *        and Leland Lucius
 *
 * PortMixer is intended to work side-by-side with PortAudio,
 * the Portable Real-Time Audio Library by Ross Bencina and
 * Phil Burk.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <windows.h>

// For some reason this must manually be included for MSVC7.1
#if defined(_MSC_VER) && _MSC_VER == 1310
  #include <limits.h>
#endif

#include "portaudio.h"
#include "pa_win_wmme.h"

#include "portmixer.h"
#include "px_mixer.h"
#include "px_win_common.h"

static int open_mixers(px_mixer *Px, UINT deviceIn, UINT deviceOut)
{
   PxInfo*info;
   MMRESULT res;

   if (deviceIn == UINT_MAX && deviceOut == UINT_MAX) {
      return FALSE;
   }

   if (!initialize(Px)) {
      return FALSE;
   }

   info = (PxInfo *) Px->info;
   info->hInputMixer = NULL;
   info->hOutputMixer = NULL;
   info->numInputs = 0;
   info->muxID = 0;
   info->speakerID = 0;
   info->waveID = 0;

   if (deviceIn != UINT_MAX) {
      res = mixerOpen((LPHMIXER) &info->hInputMixer,
                      deviceIn,
                      0,
                      0,
                      MIXER_OBJECTF_MIXER);
      if (res != MMSYSERR_NOERROR) {
         return cleanup(Px);
      }

      info->muxID = find_ctrl(info->hInputMixer,
                              MIXERLINE_COMPONENTTYPE_DST_WAVEIN,
                              MIXERCONTROL_CONTROLTYPE_MUX);
      if (info->muxID == -1) {
         info->muxID = find_ctrl(info->hInputMixer,
                                 MIXERLINE_COMPONENTTYPE_DST_WAVEIN,
                                 MIXERCONTROL_CONTROLTYPE_MIXER);
      }

      info->numInputs = get_ctrls(info->hInputMixer,
                                  MIXERLINE_COMPONENTTYPE_DST_WAVEIN,
                                  &info->src);

      if (info->numInputs == 0) {
         return cleanup(Px);
      }
   }

   if (deviceOut != UINT_MAX) {
      res = mixerOpen((LPHMIXER) &info->hOutputMixer,
                      deviceOut,
                      0,
                      0,
                      MIXER_OBJECTF_MIXER);
      if (res != MMSYSERR_NOERROR) {
         return cleanup(Px);
      }

      info->speakerID = find_ctrl(info->hOutputMixer,
                                  MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,
                                  MIXERCONTROL_CONTROLTYPE_VOLUME);

      info->waveID = find_ctrl(info->hOutputMixer,
                               MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT,
                               MIXERCONTROL_CONTROLTYPE_VOLUME);

      info->numOutputs = get_ctrls(info->hOutputMixer,
                                   MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,
                                   &info->dst);

      if (info->numOutputs == 0) {
         return cleanup(Px);
      }
   }

   return TRUE;
}

int OpenMixer_Win_MME(px_mixer *Px, int index)
{
   HWAVEIN hWaveIn;
   HWAVEOUT hWaveOut;
   MMRESULT res;
   UINT deviceIn = UINT_MAX;
   UINT deviceOut = UINT_MAX;

   hWaveIn = PaWinMME_GetStreamInputHandle(Px->pa_stream, 0);
   hWaveOut = PaWinMME_GetStreamOutputHandle(Px->pa_stream, 0);

   if (hWaveIn) {
      res = mixerGetID((HMIXEROBJ) hWaveIn,
                       &deviceIn,
                       MIXER_OBJECTF_HWAVEIN);
      if (res != MMSYSERR_NOERROR) {
         return FALSE;
      }
   }

   if (hWaveOut) {
      res = mixerGetID((HMIXEROBJ) hWaveOut,
                       &deviceOut,
                       MIXER_OBJECTF_HWAVEOUT);
      if (res != MMSYSERR_NOERROR) {
         return FALSE;
      }
   }

   if (open_mixers(Px, deviceIn, deviceOut)) {
      return TRUE;
   }

   return FALSE;
}
