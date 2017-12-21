#ifndef __WAVE_TRANS_SEND_H__
#define __WAVE_TRANS_SEND_H__
#include "proto_utils/wt_proto_common.h"

int WaveTransSendInit();

void WaveTransSendExit();

void WaveTransSendSetContext(const void *context,int context_len);

int WaveTransSendGetPcm(SendAudioType *pcm_buf,int buf_len);



#endif
