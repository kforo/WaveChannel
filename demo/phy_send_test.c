#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "proto_utils/wt_proto_physical_layer.h"

int GetNextFreqMarks(WaveTransMixinfFreqInfo *freq_info)
{
  int temp = 0;
  int i;
  for (i = 0; i < 4; i++) {
    temp += (freq_info->marks_[i] - ((unsigned char)i*4)) * ((int)pow(4, i));
  }
  if (temp >= ((int)pow(4, 4) - 1)) {
    return 1;
  }
  temp++;
  freq_info->marks_[0] = temp % 4;
  freq_info->marks_[1] = ((temp % (4 * 4) - freq_info->marks_[0]) / 4) + 0x4;
  freq_info->marks_[2] = ((temp % (4 * 4 * 4) - (freq_info->marks_[0] + (freq_info->marks_[1]-0x4) * 4)) / (4 * 4)) + 0x8;
  int add = freq_info->marks_[0] + (freq_info->marks_[1] - 0x4) * 4 + (freq_info->marks_[2] - 0x8) * (4 * 4);
  freq_info->marks_[3] = ((temp % (4 * 4 * 4 * 4) - add) / (4 * 4 * 4)) + 0xc;
  return 0;
}


int main()
{
  WaveTransMixinfFreqInfo freq_info;
  freq_info.freq_mark_num_ = 4;
  if (freq_info.marks_ == NULL) {
    printf("malloc freq_info.marks memory failed\n");
    return 1;
  }
  freq_info.marks_[0] = 0;
  freq_info.marks_[1] = 0x4;
  freq_info.marks_[2] = 0x8;
  freq_info.marks_[3] = 0xc;
  FILE *fp = NULL;
  fp = fopen("mix-test.pcm", "wb");
  if (fp == NULL) {
    printf("open file mix-test.pcm failed\n");
    return 1;
  }
  short pcm_buf[384] = { 0 };
  while (1) {
    if (WTPhysicalFreqMarksToPcm(&freq_info, pcm_buf, sizeof(short) * 384, 16, 16000) != 0) {
      printf("wave trans freqmarks to pcm failed\n");
      break;
    }   
    int ret = fwrite(pcm_buf, 1, sizeof(short) * 384, fp);
    if (ret != sizeof(short) * 384) {
      printf("write file failed\n");
      break;
    }
    if (GetNextFreqMarks(&freq_info) != 0) {
      break;
    }
  }
  fclose(fp);
  return 0;
}