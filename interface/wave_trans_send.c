#include "wave_trans_send.h"
#include "transceiver/send/wt_send_link_layer.h"
#include "transceiver/send/wt_send_physical_layer.h"
#include "proto_utils/wt_proto_common.h"
#include "audio_codec/pcm_to_wav.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  WTSendLinkForCompareHander      *link_hander_;
  WTSendPhyForCompareHander       *phy_hander_;
  WaveTransPcmInfo                pcm_info_;
  WaveTransWavInfo                wav_info_;
}WaveTransSendHanderDataForCompare;


static int WaveTransSendAttrCheck(WaveTransSendAttr *attr)
{
  if (attr == NULL) {
    return -1;
  }
  if (attr->sample_bit_ != 16 && attr->sample_bit_ != 8) {
    return -1;
  }
  if (attr->sample_rate_ / 2 < MAX_FREQ) {
    return -1;
  }
  return 0;
}

static WaveTransSendHanderDataForCompare * WaveTransSendHanderDataInitCompare(WaveTransSendAttr *attr)
{
  WaveTransSendHanderDataForCompare *hander_data = (WaveTransSendHanderDataForCompare *)malloc(sizeof(WaveTransSendHanderDataForCompare));
  if (hander_data == NULL) {
    return NULL;
  }
  hander_data->link_hander_ = WTSendLinkLayerCreateHanderForCompare();
  if (hander_data->link_hander_ == NULL) {
    free(hander_data);
    return NULL;
  }
  WTSendPhyHanderAttr phy_attr;
  phy_attr.sample_bit_ = attr->sample_bit_;
  phy_attr.sample_rate_ = attr->sample_rate_;
  hander_data->phy_hander_ = WTSendPhyLayerCreateHanderForCompare(&phy_attr);
  if (hander_data->phy_hander_ == NULL) {
    WTSendLinkLayerDestroyHanderForCompare(hander_data->link_hander_);
    free(hander_data);
    return NULL;
  }
  hander_data->pcm_info_.pcm_buff_ = NULL;
  hander_data->pcm_info_.buff_len_ = 0;
  hander_data->wav_info_.wav_buff_ = NULL;
  hander_data->wav_info_.buff_len_ = 0;
  return hander_data;
}

static void WaveTransSendHanderDataExitCompare(WaveTransSendHanderDataForCompare *hander_data)
{
  WTSendLinkLayerDestroyHanderForCompare(hander_data->link_hander_);
  hander_data->link_hander_ = NULL;
  WTSendPhyLayerDestroyHanderForCompare(hander_data->phy_hander_);
  hander_data->link_hander_ = NULL;
  if (hander_data->pcm_info_.pcm_buff_ != NULL) {
    free(hander_data->pcm_info_.pcm_buff_);
    hander_data->pcm_info_.pcm_buff_ = NULL;
  }
  if (hander_data->wav_info_.wav_buff_ != NULL) {
    free(hander_data->wav_info_.wav_buff_);
    hander_data->wav_info_.wav_buff_ = NULL;
  }
  free(hander_data);
}


static WaveTransPcmInfo * GetPcmCompare(WaveTransSendHander *hander, const void * context, int context_len)
{
  WaveTransSendHanderDataForCompare *hander_data = (WaveTransSendHanderDataForCompare *)hander->data_;
  WTSendLinkComparePackageS *packages = WTSendLinkLayerGetPackageForCompare(hander_data->link_hander_, context, context_len);
  if (packages == NULL) {
    return NULL;
  }
  WTSendPcmBuffType *pcm_type = WTSendPhyLayerGetPcmCompare(hander_data->phy_hander_, packages);
  if (pcm_type == NULL) {
    WTSendLinkLayerReleasePackageForCompare(hander_data->link_hander_);
    return NULL;
  }
  if (hander_data->pcm_info_.pcm_buff_ != NULL) {
    free(hander_data->pcm_info_.pcm_buff_);
    hander_data->pcm_info_.pcm_buff_ = NULL;
  }
  hander_data->pcm_info_.pcm_buff_ = malloc(pcm_type->buff_len_);
  if (hander_data->pcm_info_.pcm_buff_ == NULL) {
    WTSendPhyLayerReleasePcmCompare(hander_data->phy_hander_);
    WTSendLinkLayerReleasePackageForCompare(hander_data->link_hander_);
    return NULL;
  }
  memcpy(hander_data->pcm_info_.pcm_buff_, pcm_type->buff_, pcm_type->buff_len_);
  hander_data->pcm_info_.buff_len_ = pcm_type->buff_len_;
  hander_data->pcm_info_.sample_bit_ = pcm_type->sample_bit_;
  hander_data->pcm_info_.sample_rate_ = pcm_type->sample_rate_;
  WTSendPhyLayerReleasePcmCompare(hander_data->phy_hander_);
  WTSendLinkLayerReleasePackageForCompare(hander_data->link_hander_);
  return &hander_data->pcm_info_;
}

WaveTransWavInfo * GetWavCompare(WaveTransSendHander * hander, const void * context, int context_len)
{
  WaveTransSendHanderDataForCompare *hander_data = (WaveTransSendHanderDataForCompare *)hander->data_;
  WTSendLinkComparePackageS *packages = WTSendLinkLayerGetPackageForCompare(hander_data->link_hander_, context, context_len);
  if (packages == NULL) {
    return NULL;
  }
  WTSendPcmBuffType *pcm_type = WTSendPhyLayerGetPcmCompare(hander_data->phy_hander_, packages);
  if (pcm_type == NULL) {
    WTSendLinkLayerReleasePackageForCompare(hander_data->link_hander_);
    return NULL;
  }
  int none_len = AUDIO_NONE_LEN(pcm_type->sample_rate_)*(pcm_type->sample_bit_ / 8);
  int out_pcm_len = 2 * none_len + pcm_type->buff_len_;
  WTSendPcmBuffType temp_pcm_type;
  temp_pcm_type.buff_ = malloc(out_pcm_len);
  if (temp_pcm_type.buff_ == NULL) {
    WTSendPhyLayerReleasePcmCompare(hander_data->phy_hander_);
    WTSendLinkLayerReleasePackageForCompare(hander_data->link_hander_);
    return NULL;
  }
  temp_pcm_type.buff_len_ = out_pcm_len;
  temp_pcm_type.sample_bit_ = pcm_type->sample_bit_;
  temp_pcm_type.sample_rate_ = pcm_type->sample_rate_;
  memset(temp_pcm_type.buff_, 0, none_len);
  memcpy(((unsigned char *)temp_pcm_type.buff_ + none_len), pcm_type->buff_, pcm_type->buff_len_);
  memset(((unsigned char *)temp_pcm_type.buff_ + none_len + pcm_type->buff_len_), 0, none_len);
  WTSendPhyLayerReleasePcmCompare(hander_data->phy_hander_);
  WTSendLinkLayerReleasePackageForCompare(hander_data->link_hander_);


  if (hander_data->wav_info_.wav_buff_ != NULL) {
    free(hander_data->wav_info_.wav_buff_);
    hander_data->wav_info_.wav_buff_ = NULL;
  }
  int wav_len = PcmToWavGetWavSize(&temp_pcm_type);
  hander_data->wav_info_.wav_buff_ = malloc(wav_len);
  if (hander_data->wav_info_.wav_buff_ == NULL) {
    free(temp_pcm_type.buff_);
    return NULL;
  }
  hander_data->wav_info_.buff_len_ = wav_len;
  if (PcmToWavGetWavData(&temp_pcm_type, hander_data->wav_info_.wav_buff_, hander_data->wav_info_.buff_len_) != 0) {
    free(hander_data->wav_info_.wav_buff_);
    hander_data->wav_info_.wav_buff_ = NULL;
    free(temp_pcm_type.buff_);
    return NULL;
  }
  free(temp_pcm_type.buff_);
  return &hander_data->wav_info_;
}


WaveTransSendHander * WaveTransSendCreateHander(WaveTransSendAttr * attr)
{
  if (WaveTransSendAttrCheck(attr) != 0) {
    return NULL;
  }
  WaveTransSendHanderDataForCompare *hander_data = WaveTransSendHanderDataInitCompare(attr);
  if (hander_data == NULL) {
    return NULL;
  }
  WaveTransSendHander *hander = (WaveTransSendHander *)malloc(sizeof(WaveTransSendHander));
  if (hander == NULL) {
    WaveTransSendHanderDataExitCompare(hander_data);
    return NULL;
  }
  hander->data_ = hander_data;
  return hander;
}

void WaveTransSendDestroyHander(WaveTransSendHander * hander)
{
  WaveTransSendHanderDataForCompare *hander_data = (WaveTransSendHanderDataForCompare *)hander->data_;
  WaveTransSendHanderDataExitCompare(hander_data);
  free(hander);
}

WaveTransPcmInfo * WaveTransSendGetPcm(WaveTransSendHander *hander, const void * context, int context_len)
{
  return GetPcmCompare(hander, context, context_len);
}
WaveTransWavInfo * WaveTransSendGetWav(WaveTransSendHander * hander, const void * context, int context_len)
{
  return GetWavCompare(hander, context, context_len);
}




#ifdef __cplusplus
}
#endif