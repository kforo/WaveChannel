#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "interface/wave_trans_send.h"


int main(void)
{
  int ret;
  FILE *in_fp = NULL;
  FILE *out_fp = NULL;
  char data_buff[1024] = { 0 };
  int buff_len = 1024;
  char input_file[] = "send-data.txt";
  char output_file[] = "test.pcm";
  WaveTransSendHander *wt_send_hander = NULL;
  WaveTransSendAttr attr;
  attr.sample_bit_ = 16;
  attr.sample_rate_ = 16000;
  wt_send_hander = WaveTransSendCreateHander(&attr);
  if (wt_send_hander == NULL) {
    printf("wave trans send create hander failed\n");
    return 1;
  }
  in_fp = fopen(input_file, "r");
  if (in_fp == NULL) {
    printf("without %s file\n", input_file);
    WaveTransSendDestroyHander(wt_send_hander);
    return 1;
  }
  out_fp = fopen(output_file, "wb");
  if (out_fp == NULL) {
    printf("open file :%s failed\n", output_file);
    WaveTransSendDestroyHander(wt_send_hander);
    return 1;
  }
  while ((ret = fread(data_buff, 1, sizeof(char)*buff_len, in_fp)) != 0) {
    WaveTransPcmInfo *pcm_info = WaveTransSendGetPcm(wt_send_hander, data_buff, ret);
    if (pcm_info == NULL) {
      printf("get pcm data failed\n");
      continue;
    }
    int temp = fwrite(pcm_info->pcm_buff_, 1, pcm_info->buff_len_, out_fp);
    if (temp != pcm_info->buff_len_) {
      printf("write pcm data failed\n");
      continue;
    }
  }
  if (in_fp != NULL) {
    fclose(in_fp);
  }
  if (out_fp != NULL) {
    fclose(out_fp);
  }
  if (wt_send_hander != NULL) {
    WaveTransSendDestroyHander(wt_send_hander);
  }
  return 0;
}