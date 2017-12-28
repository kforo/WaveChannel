#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "interface/wave_trans_send.h"


int main(void)
{
  int ret;
  FILE *fp = NULL;
  char test_context[] = "1234567891239";
  WaveTransSendHander *wt_send_hander = NULL;
  WaveTransSendAttr attr;
  attr.sample_bit_ = 16;
  attr.sample_rate_ = 16000;
  wt_send_hander = WaveTransSendCreateHander(&attr);
  if (wt_send_hander == NULL) {
    printf("wave trans send create hander failed\n");
    return 1;
  }
  WaveTransPcmInfo *pcm_info = WaveTransSendGetPcm(wt_send_hander, test_context, (int)strlen(test_context));
  if (pcm_info == NULL) {
    printf("wave trans send get pcm failed\n");
    return 1;
  }
  fp = fopen("test.pcm", "wb");
  if (fp == NULL) {
    printf("open file test.pcm failed\n");
    WaveTransSendDestroyHander(wt_send_hander);
    return 1;
  }
  ret = (int)fwrite(pcm_info->pcm_buff_, (size_t)1, (size_t)pcm_info->buff_len_, fp);
  if (ret != pcm_info->buff_len_) {
    fclose(fp);
    WaveTransSendDestroyHander(wt_send_hander);
    return 1;
  }
  fclose(fp);
  WaveTransSendDestroyHander(wt_send_hander);
  return 0;
}