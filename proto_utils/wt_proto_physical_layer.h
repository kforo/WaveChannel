#ifndef __WT_PROTO_PHYSICAL_LAYER_H__
#define __WT_PROTO_PHYSICAL_LAYER_H__
#include "wt_proto_common.h"





typedef struct {
  void          *buff_;
  int           buff_len_;
  int           sample_rate_;
  int           sample_bit_;
}WTSendPcmBuffType;


typedef struct {
  unsigned char                   freq_mark_num_;
  WTPhyFreqMarkType               marks_[MIXING_FREQ_NUM];
}WaveTransMixMarksType;



int WTPhysicalPcmToFreqMark(const RecvAudioType *pcm_buf, int pcm_len, WTPhyFreqMarkType *freq_mark);

int WTPhysicalFreqMarkToPcm(WTPhyFreqMarkType freq_mark, void  *pcm_buf, int pcm_len,int sample_bit,int sample_rate);

int WTPhysicalPcmToFreqMarks(const RecvAudioType *pcm_buf, int pcm_len, WaveTransMixMarksType *freq_marks);

int WTPhysicalFreqMarksToPcm(const WaveTransMixMarksType *freq_marks, void *pcm_buf, int pcm_len, int sample_bit, int sample_rate);

int WTPhyAnalysisNumToRealNum(int ana_num);







#endif
