#ifndef __PCM_TO_WAV_H__
#define __PCM_TO_WAV_H__
#include "proto_utils/wt_proto_physical_layer.h"





int PcmToWavGetWavSize(const WTSendPcmBuffType *pcm_buff);

int PcmToWavGetWavData(const WTSendPcmBuffType *pcm_buff, void *wav_buf, int wav_buf_len);



#endif