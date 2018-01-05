#include "pcm_to_wav.h"
#include <string.h>

typedef struct {
  char            chunk_id_[4];
  unsigned int    chunk_size_;
  unsigned short  format_tag_;
  unsigned short  channels_;
  unsigned int    sample_rate_;
  unsigned int    avg_bytes_per_sec_;
  unsigned short  block_align_;
  unsigned short  sample_bit_;
}FormatChunk;

typedef struct {
  char            chunk_id_[4];
  unsigned int    chunk_size_;
}DataChunk;

typedef struct {
  char            chunk_id_[4];
  unsigned int    chunk_size_;
  unsigned char   wave_form_data_[4];
}HeadChunk;


int PcmToWavGetWavSize(const WTSendPcmBuffType *pcm_buff)
{
  int wav_head_size;
  wav_head_size = sizeof(HeadChunk) + sizeof(FormatChunk) + sizeof(DataChunk);
  return wav_head_size + pcm_buff->buff_len_;
}

int PcmToWavGetWavData(const WTSendPcmBuffType * pcm_buff, void * wav_buf, int wav_buf_len)
{
  int wav_size = PcmToWavGetWavSize(pcm_buff);
  if (wav_buf_len < wav_size) {
    return -1;
  }
  HeadChunk head_chunk;
  
  strcpy(head_chunk.chunk_id_, "RIFF");
  head_chunk.chunk_size_ = wav_size - 8;
  strcpy(head_chunk.wave_form_data_, "WAVE");
  
  FormatChunk format_chunk;
  strcpy(format_chunk.chunk_id_, "fmt ");
  format_chunk.chunk_size_ = sizeof(FormatChunk) - 4 - sizeof(unsigned int);
  format_chunk.format_tag_ = 1;
  format_chunk.channels_ = 1;
  format_chunk.sample_rate_ = pcm_buff->sample_rate_;
  format_chunk.sample_bit_ = pcm_buff->sample_bit_;
  format_chunk.block_align_ = format_chunk.channels_ * (format_chunk.sample_bit_ >> 3);
  format_chunk.avg_bytes_per_sec_ = format_chunk.block_align_*format_chunk.sample_rate_;

  DataChunk data_chunk;
  strcpy(data_chunk.chunk_id_, "data");
  data_chunk.chunk_size_ = pcm_buff->buff_len_;
  int buff_w_addr = 0;
  memcpy(((unsigned char*)wav_buf + buff_w_addr), &head_chunk, sizeof(HeadChunk));
  buff_w_addr += sizeof(HeadChunk);
  memcpy(((unsigned char *)wav_buf + buff_w_addr), &format_chunk, sizeof(FormatChunk));
  buff_w_addr += sizeof(FormatChunk);
  memcpy(((unsigned char *)wav_buf + buff_w_addr), &data_chunk, sizeof(DataChunk));
  buff_w_addr += sizeof(DataChunk);
  memcpy(((unsigned char *)wav_buf + buff_w_addr), pcm_buff->buff_, pcm_buff->buff_len_);
  return 0;
}
