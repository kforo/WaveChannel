#include "proto_utils/wt_proto_common.h"
#include "interface/wave_trans_send.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



int main()
{
  int ret;
  FILE *fp = NULL;
  SendAudioType pcm_buf[2048] = { 0 };
  char test_context[] = "abcdefghigklmnopqrstuvwxyz123456789!@#$%^&*()";
  if (WaveTransSendInit() != 0) {
    printf("wave trans send init failed\n");
    return 1;
  }
  WaveTransSendSetContext(test_context, strlen(test_context));
  fp = fopen("test.pcm", "wb");
  if (fp == NULL) {
    printf("open file test.pcm failed\n");
    WaveTransSendExit();
    return 1;
  }
  while (1) {
    ret = WaveTransSendGetPcm(pcm_buf, 2048);
    if (ret <= 0) {
      break;
    }
    int temp = ret;
    ret = fwrite(pcm_buf, 1, sizeof(SendAudioType)*temp, fp);
    if (ret != (int)(temp*sizeof(SendAudioType))) {
      printf("write to file failed,ret:%d\n", ret);
      break;
    }
  }
  WaveTransSendExit();
  return 0;
}