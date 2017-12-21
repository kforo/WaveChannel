#ifndef __WT_PROTO_PHYSICAL_LAYER_H__
#define __WT_PROTO_PHYSICAL_LAYER_H__
#include "wt_proto_common.h"



typedef unsigned char WTPhyFreqMarkType;


int WTPhysicalPcmToFreqMark(const RecvAudioType *pcm_buf, int pcm_len, WTPhyFreqMarkType *freq_mark);

int WTPhysicalFreqMarkToPcm(WTPhyFreqMarkType freq_mark, SendAudioType *pcm_buf, int pcm_len);

int WTPhyAnalysisNumToRealNum(int ana_num);








#endif
