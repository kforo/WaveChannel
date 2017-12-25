#ifndef __WAVE_TRANS_SEND_H__
#define __WAVE_TRANS_SEND_H__

typedef struct {
  void            *data_;
}WaveTransSendHander;

typedef struct {
  int           sample_bit_;
  int           sample_rate_;
}WaveTransSendAttr;

typedef struct {
  void            *pcm_buff_;
  int             buff_len_;
  int             sample_rate_;
  int             sample_bit_;
}WaveTransPcmInfo;

WaveTransSendHander *WaveTransSendCreateHander(WaveTransSendAttr *attr);

void WaveTransSendDestroyHander(WaveTransSendHander *hander);

WaveTransPcmInfo *WaveTransSendGetPcm(WaveTransSendHander *hander,void *context, int context_len);


#endif
