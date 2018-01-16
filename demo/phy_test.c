#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proto_utils/wt_proto_physical_layer.h"


//int main()
//{
//  short pcm_buf[384];
//  memset(pcm_buf, 0, sizeof(short) * 384);
//  int buf_len = 384;
//  FILE *fp = NULL;
//  fp = fopen("phy.pcm", "wb");
//  if (fp == NULL) {
//    printf("open file phy.pcm failed\n");
//    return 1;
//  }
//  WaveTransMixMarksType marks;
//  marks.freq_mark_num_ = 3;
//  marks.marks_[0] = 0;
//  marks.marks_[1] = 1;
//  marks.marks_[2] = 2;
//  marks.marks_[3] = 3;
//  if (WTPhysicalFreqMarksToPcm(&marks, pcm_buf, buf_len * sizeof(short), 16, 16000) != 0) {
//    printf("marks to pcm failed\n");
//    fclose(fp);
//    return 1;
//  }
//  int ret = fwrite(pcm_buf, 1, sizeof(short)*buf_len, fp);
//  if (ret != sizeof(short)*buf_len) {
//    printf("write file failed\n");
//    fclose(fp);
//    return 1;
//  }
//  fclose(fp);
//  return 0;
//}

int main()
{
  short pcm_buf[384];
  memset(pcm_buf, 0, sizeof(short) * 384);
  int buf_len = 384;
  FILE *fp = NULL;
  fp = fopen("phy.pcm", "wb");
  if (fp == NULL) {
    printf("open file phy.pcm failed\n");
    return 1;
  }
  WTPhyFreqMarkType mark;
  int i;
  for (i = 0; i < 18; i++) {
    mark = i;
   // mark = 2;
    if (WTPhysicalFreqMarkToPcm(mark, pcm_buf, sizeof(short)*buf_len, 16, 16000) != 0) {
      fclose(fp);
      return 1;
    }
    int ret = fwrite(pcm_buf, 1, sizeof(short)*buf_len, fp);
    if (ret != sizeof(short)*buf_len) {
      printf("write file failed\n");
      fclose(fp);
      return 1;
    }
  }
  fclose(fp);
  return 0;
}