#include "wt_send_link_layer.h"
#include "buff_utils/ring_buff.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  WTSendLinkComparePackageS      packages_;
}WTLinkSendHanderDataCompare;

WTSendLinkForCompareHander * WTSendLinkLayerCreateHanderForCompare(void)
{
  WTLinkSendHanderDataCompare *hander_data = (WTLinkSendHanderDataCompare *)malloc(sizeof(WTLinkSendHanderDataCompare));
  if (hander_data == NULL) {
    return NULL;
  }
  hander_data->packages_.package_num_ = 0;
  hander_data->packages_.package_ = NULL;
  WTSendLinkForCompareHander *hander = (WTSendLinkForCompareHander *)malloc(sizeof(WTSendLinkForCompareHander));
  if (hander == NULL) {
    free(hander_data);
    return NULL;
  }
  hander->data_ = hander_data;
  return hander;
}

void WTSendLinkLayerDestroyHanderForCompare(WTSendLinkForCompareHander * hander)
{
  WTLinkSendHanderDataCompare *hander_data = (WTLinkSendHanderDataCompare *)hander->data_;
  if (hander_data->packages_.package_ != NULL) {
    free(hander_data->packages_.package_);
    hander_data->packages_.package_ = NULL;
  }
  free(hander_data);
  free(hander);
}

WTSendLinkComparePackageS * WTSendLinkLayerGetPackageForCompare(WTSendLinkForCompareHander * hander, const void * context, int context_len)
{
  int package_num;
  int data_r_addr = 0;
  WaveTransCompareLinkPackage one_package;
  int i;
  if (context_len % COMPARE_FREQ_DATA_NUM != 0) {
    package_num = context_len / COMPARE_FREQ_DATA_NUM + 1;
  }
  else {
    package_num = context_len / COMPARE_FREQ_DATA_NUM;
  }
  WTLinkSendHanderDataCompare *hander_data = (WTLinkSendHanderDataCompare *)hander->data_;
  hander_data->packages_.package_ = (WaveTransComparePhyPackage *)malloc(sizeof(WaveTransComparePhyPackage)*package_num);
  if (hander_data->packages_.package_ == NULL) {
    return NULL;
  }
  hander_data->packages_.package_num_ = package_num;
  for (i = 0; i < context_len / COMPARE_FREQ_DATA_NUM; i++) {
    memcpy(&one_package.byte_data_, ((unsigned char *)context + data_r_addr), COMPARE_FREQ_DATA_NUM);
    one_package.real_data_num_ = COMPARE_FREQ_DATA_NUM;
    WTLinkChecksumEncode(&one_package);
    WTLinkPcakgeToPhyPack(&one_package, &hander_data->packages_.package_[i]);
    data_r_addr += COMPARE_FREQ_DATA_NUM;
  }
  if (context_len % COMPARE_FREQ_DATA_NUM != 0) {
    memcpy(&one_package.byte_data_, ((unsigned char *)context + data_r_addr), context_len % COMPARE_FREQ_DATA_NUM);
    one_package.real_data_num_ = context_len % COMPARE_FREQ_DATA_NUM;
    WTLinkChecksumEncode(&one_package);
    WTLinkPcakgeToPhyPack(&one_package, &hander_data->packages_.package_[hander_data->packages_.package_num_ - 1]);
  }
  return &hander_data->packages_;
}

void WTSendLinkLayerReleasePackageForCompare(WTSendLinkForCompareHander * hander)
{
  WTLinkSendHanderDataCompare *hander_data = (WTLinkSendHanderDataCompare *)hander->data_;
  free(hander_data->packages_.package_);
  hander_data->packages_.package_ = NULL;
}
