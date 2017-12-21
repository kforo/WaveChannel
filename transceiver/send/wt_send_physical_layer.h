#ifndef __WT_SEND_PHYSICAL_LAYER_H__
#define __WT_SEND_PHYSICAL_LAYER_H__
#include "proto_utils/wt_proto_physical_layer.h"


int WTSendPhyLayerInit();

void WTSendPhyLayerExit();

int WTSendPhyLayerGetPcm(SendAudioType *pcm, int pcm_len);


#endif

