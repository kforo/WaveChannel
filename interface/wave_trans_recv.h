#ifndef __WAVE_TRANS_RECV_H__
#define __WAVE_TRANS_RECV_H__
#include "proto_utils/wt_proto_common.h"


int WaveTransRecvInit(void);

void WaveTransRecvExit(void);

void WaveTransRecvSetPcm(const RecvAudioType *pcm, int pcm_len);

int WaveTransRecvGetContext(void *context, int context_len);


#endif
