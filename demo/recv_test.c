#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proto_utils/wt_proto_common.h"
#include "interface/wave_trans_recv.h"





int main()
{
  FILE *fp = NULL;
  int ret;
  RecvAudioType pcm_buf[1024];
  char data_temp[10] = { 0 };
  if (WaveTransRecvInit() != 0) {
    printf("wave trans recv init failed\n");
    return 1;
  }
  fp = fopen("test.pcm", "rb");
  if (fp == NULL) {
    printf("open file test.pcm failed\n");
    WaveTransRecvExit();
    return 1;
  }
  while (1) {
    ret = fread(pcm_buf, 1, sizeof(RecvAudioType) * 1024, fp);
    WaveTransRecvSetPcm(pcm_buf, ret / sizeof(RecvAudioType));
    ret = WaveTransRecvGetContext(data_temp, sizeof(char) * 10);
    if (ret != 0) {
      data_temp[ret] = '\0';
      printf("%s", data_temp);
    }
  }
  WaveTransRecvExit();
  return 0;
}