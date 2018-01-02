#include "proto_utils/wt_proto_physical_layer.h"
#include "proto_utils/wt_proto_common.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>




int main()
{
  FILE *fp = NULL;
  WaveTransMixinfFreqInfo freq_info;
  short pcm_buf[128];
  int buf_len = 128;
  int ret;
  printf("test demo start\n");
  fp = fopen("r-test.pcm", "rb");
  if (fp == NULL) {
    printf("open file r-test.pcm failed\n");
    return 1;
  }
  while ((ret = fread(pcm_buf, 1, sizeof(short)*buf_len, fp)) == (sizeof(short)*buf_len)) {
    if (WTPhysicalPcmToFreqMarks(pcm_buf, buf_len, &freq_info) != 0) {
      printf("WTPhysicalPcmToFreqMarks failed\n");
      continue;
    }
    int i;
    if (freq_info.freq_mark_num_ != 0) {
      for (i = 0; i < freq_info.freq_mark_num_; i++) {
        printf(" %d ", freq_info.marks_[i]);
      }
      printf("\n");
    }
  }
  fclose(fp);
  getchar();
  return 0;
}