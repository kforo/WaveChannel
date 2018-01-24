#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interface/wave_trans_send.h"
#define DATA_FILE             "data.txt"



int main()
{
  FILE *fp = NULL;
  WaveTransSendHander *hander = NULL;
  unsigned char *data_buf = NULL;
  WaveTransSendAttr attr;
  attr.sample_bit_ = 16;
  attr.sample_rate_ = 16000;
  hander = WaveTransSendCreateHander(&attr);
  if (hander == NULL) {
    printf("create hander failed\n");
    goto err_exit;
  }
  fp = fopen(DATA_FILE, "r");
  if (fp == NULL) {
    printf("open file: %d failed", DATA_FILE);
    goto err_exit;
  }
  int ret = fseek(fp, NULL, SEEK_END);
  if (ret != 0) {
    printf("fseed failed\n");
    goto err_exit;
  }
  int file_len = ftell(fp)-1;
  if (fseek(fp, NULL, SEEK_SET) != 0) {
    printf("fseek failed\n");
    goto err_exit;
  }
  data_buf = (unsigned char *)malloc(sizeof(unsigned char)*file_len);
  if (data_buf == NULL) {
    printf("malloc data_buf memory failed\n");
    goto err_exit;
  }
  ret = fread(data_buf, 1, sizeof(unsigned char)*file_len, fp);
  if (ret != file_len) {
    printf("read file failed\n");
    goto err_exit;
  }
  WaveTransWavInfo *wav_info = WaveTransSendGetWav(hander, data_buf, file_len);
  if (wav_info == NULL) {
    printf("get wav buf failed\n");
    goto err_exit;
  }
  FILE *wav_fp = fopen("test.wav", "wb");
  if (wav_fp == NULL) {
    printf("open test.wav failed\n");
    goto err_exit;
  }
  ret = fwrite(wav_info->wav_buff_, 1, wav_info->buff_len_, wav_fp);
  if (ret != wav_info->buff_len_) {
    printf("write wav buff failed\n");
    goto err_exit;
  }
  fclose(wav_fp);
  free(data_buf);
  fclose(fp);
  WaveTransSendDestroyHander(hander);
  return 0;
  
err_exit:
  if (data_buf != NULL) {
    free(data_buf);
  }
  if (fp != NULL) {
    fclose(fp);
  }
  if (hander != NULL) {
    WaveTransSendDestroyHander(hander);
  }
  return 1;
}