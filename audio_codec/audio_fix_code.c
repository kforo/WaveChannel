#include "audio_fix_code.h"
#include "proto_utils/wt_proto_common.h"
#include "proto_utils/wt_proto_physical_layer.h"
#include <stdio.h>
#include <string.h>

const int fix_data_len_ = 8;
const double prefix_audio_attenuate_scale_ = 0.2;
const double suffix_audio_attenuate_scale_ = 0.2;
const WTFreqCodeType fix_data_[] = { 0x100,0xff,0x100,0x91,0x1e3,0x44,0x10c,0x6a };

static void GetFixAudio(WTSendPcmBuffType *pcm, int is_right_align)
{
  int buff_time_ms = sample_len_to_time_ms((pcm->buff_len_ / (pcm->sample_bit_ / 8)), pcm->sample_rate_);
  if (buff_time_ms% ONE_FREQ_TIME_MS != 0) {
    buff_time_ms = (buff_time_ms / ONE_FREQ_TIME_MS + 1)*ONE_FREQ_TIME_MS;
  }
  int create_buff_len = time_ms_to_length(buff_time_ms, pcm->sample_rate_)*(pcm->sample_bit_ / 8);
  char *create_buff = (char *)malloc(sizeof(char)*create_buff_len);
  if (create_buff == NULL) {
    memset(pcm->buff_, 0, pcm->buff_len_);
    return;
  }
  int buff_w_addr = 0;
  int fix_r_addr = 0;
  RefPhaseInfo ref_phase;
  ref_phase.bit_num_ = COMPARE_FREQ_BIT + COMPARE_FREQ_PARITY_BIT;
  memset(&ref_phase.left_phase_, 0, sizeof(double)*(COMPARE_FREQ_BIT + COMPARE_FREQ_PARITY_BIT));
  memset(&ref_phase.right_phase_, 0, sizeof(double)*(COMPARE_FREQ_BIT + COMPARE_FREQ_PARITY_BIT));
  WTSendPcmBuffType pcm_type;
  pcm_type.sample_bit_ = pcm->sample_bit_;
  pcm_type.sample_rate_ = pcm->sample_rate_;
  do {
    pcm_type.buff_ = create_buff + buff_w_addr;
    pcm_type.buff_len_ = time_ms_to_length(ONE_FREQ_TIME_MS, pcm->sample_rate_)*(pcm->sample_bit_ / 8);
    WTPhysicalPcmEncode(fix_data_[fix_r_addr], &pcm_type, &ref_phase);
    fix_r_addr++;
    if (fix_r_addr >= fix_data_len_) {
      fix_r_addr = 0;
    }
    buff_w_addr += pcm_type.buff_len_;
  } while (buff_w_addr <  create_buff_len);
  if (is_right_align) {
    memcpy(pcm->buff_, create_buff + (create_buff_len - pcm->buff_len_), sizeof(char)*pcm->buff_len_);
  }
  else {
    memcpy(pcm->buff_, create_buff, sizeof(char)*pcm->buff_len_);
  }
  free(create_buff);
}

static void AudioToAttenuate(WTSendPcmBuffType *pcm, int is_start_right, double attenuate_scale)
{
  double attenuate_step = (1 - attenuate_scale) / (pcm->buff_len_ / (pcm->sample_bit_ / 8));
  double attenuate;
  if (is_start_right) {
    attenuate = attenuate_scale;
  }
  else {
    attenuate = 1;
  }
  int buff_w_addr = 0;
  int data_w_addr = 0;
  do {
    if (pcm->sample_bit_ == 8) {
      ((char *)pcm->buff_)[data_w_addr] = (char)(((char *)pcm->buff_)[data_w_addr] * attenuate);
      buff_w_addr += 1;
    }
    else if (pcm->sample_bit_ == 16) {
      ((short *)pcm->buff_)[data_w_addr] = (short)(((short *)pcm->buff_)[data_w_addr] * attenuate);
      buff_w_addr += 2;
    }
    data_w_addr++;
    if (is_start_right) {
      attenuate += attenuate_step;
    }
    else {
      attenuate -= attenuate_step;
    }
  } while (buff_w_addr < pcm->buff_len_);
}


void WTAudioPrefixEncode(WTSendPcmBuffType *pcm)
{
  GetFixAudio(pcm, 1);
  AudioToAttenuate(pcm, 1, prefix_audio_attenuate_scale_);
}

void WTAudioSuffixEncode(WTSendPcmBuffType *pcm)
{
  GetFixAudio(pcm, 0);
  AudioToAttenuate(pcm, 0, suffix_audio_attenuate_scale_);
}
