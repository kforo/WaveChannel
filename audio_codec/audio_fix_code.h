#ifndef __AUDIO_FIX_CODE_H__
#define __AUDIO_FIX_CODE_H__
#include "proto_utils/wt_proto_physical_layer.h"



void WTAudioPrefixEncode(WTSendPcmBuffType *pcm);

void WTAudioSuffixEncode(WTSendPcmBuffType *pcm);



#endif