#ifndef __WT_PROTO_PHYSICAL_LAYER_H__
#define __WT_PROTO_PHYSICAL_LAYER_H__
#include "wt_proto_common.h"



typedef unsigned char WTPhyFreqMarkType;


typedef struct {
  unsigned char                   freq_mark_num_;
  WTPhyFreqMarkType               marks_[MIXING_FREQ_NUM];
}WaveTransMixinfFreqInfo;



int WTPhysicalPcmToFreqMark(const RecvAudioType *pcm_buf, int pcm_len, WTPhyFreqMarkType *freq_mark);

int WTPhysicalFreqMarkToPcm(WTPhyFreqMarkType freq_mark, void  *pcm_buf, int pcm_len,int sample_bit,int sample_rate);

int WTPhysicalPcmToFreqMarks(const RecvAudioType *pcm_buf, int pcm_len, WaveTransMixinfFreqInfo *freq_marks);

int WTPhysicalFreqMarksToPcm(const WaveTransMixinfFreqInfo *freq_marks, void *pcm_buf, int pcm_len, int sample_bit, int sample_rate);

int WTPhyAnalysisNumToRealNum(int ana_num);








#endif
