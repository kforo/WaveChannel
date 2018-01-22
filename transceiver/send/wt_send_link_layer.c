#include "wt_send_link_layer.h"
#include "buff_utils/ring_buff.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  WTSendLinkPackageS       packages_;
}WTLinkSendHanderData;

typedef struct {
  WTSendLinkMixPackageS     packages_;
}WTLinkSendHanderDataForMix;

typedef struct {
  WTSendLinkComparePackageS      packages_;
}WTLinkSendHanderDataCompare;

WTSendLinkHander * WTSendLinkLayerCreateHander(void)
{
  WTLinkSendHanderData *hander_data = (WTLinkSendHanderData *)malloc(sizeof(WTLinkSendHanderData));
  if (hander_data == NULL) {
    return NULL;
  }
  hander_data->packages_.package_num_ = 0;
  hander_data->packages_.package_ = NULL;
  WTSendLinkHander *hander = (WTSendLinkHander *)malloc(sizeof(WTSendLinkHander));
  if (hander == NULL) {
    free(hander_data);
    return NULL;
  }
  hander->data_ = hander_data;
  return hander;
}

void WTSendLinkLayerDestroyHander(WTSendLinkHander * hander)
{
  WTLinkSendHanderData *hander_data = (WTLinkSendHanderData *)hander->data_;
  if (hander_data->packages_.package_ != NULL) {
    free(hander_data->packages_.package_);
    hander_data->packages_.package_ = NULL;
  }
  free(hander->data_);
  free(hander);
}

WTSendLinkPackageS * WTSendLinkLayerGetPackage(WTSendLinkHander * hander,const void * context, int context_len)
{
  int package_num;
  int data_r_addr = 0;
  WaveTransLinkPackage one_package;
  int i;
  if (context_len % (HBYTE_DATA_NUM / 2) != 0) {
    package_num = context_len / (HBYTE_DATA_NUM / 2) + 1;
  }
  else {
    package_num = context_len / (HBYTE_DATA_NUM / 2);
  }
  WTLinkSendHanderData *hander_data = (WTLinkSendHanderData *)hander->data_;
  hander_data->packages_.package_ = (WaveTransPhyPackage *)malloc(sizeof(WaveTransPhyPackage)*package_num);
  if (hander_data->packages_.package_ == NULL) {
    return NULL;
  }
  hander_data->packages_.package_num_ = package_num;
  for (i = 0; i < context_len / 4; i++) {
    memcpy(&one_package.byte_data_, ((unsigned char *)context + data_r_addr), HBYTE_DATA_NUM / 2);
    one_package.real_data_num_ = HBYTE_DATA_NUM / 2;
    WTLinkGetDataChecksum(&one_package);
    WTLinkPackageToHalf(&one_package, &hander_data->packages_.package_[i]);
    data_r_addr += HBYTE_DATA_NUM / 2;
  }
  if (context_len % (HBYTE_DATA_NUM / 2) != 0) {
    memcpy(&one_package.byte_data_, ((unsigned char *)context + data_r_addr), context_len % (HBYTE_DATA_NUM / 2));
    one_package.real_data_num_ = context_len % (HBYTE_DATA_NUM / 2);
    WTLinkGetDataChecksum(&one_package);
    WTLinkPackageToHalf(&one_package, &hander_data->packages_.package_[hander_data->packages_.package_num_ - 1]);
  }
  return &hander_data->packages_;
}

void WTSendLinkLayerReleasePackage(WTSendLinkHander * hander)
{
  WTLinkSendHanderData *hander_data = (WTLinkSendHanderData *)hander->data_;
  free(hander_data->packages_.package_);
  hander_data->packages_.package_ = NULL;
}

WTSendLinkForMixHander * WTSendLinkLayerCreateHanderForMix(void)
{
  WTLinkSendHanderDataForMix *hander_data = (WTLinkSendHanderDataForMix *)malloc(sizeof(WTLinkSendHanderDataForMix));
  if (hander_data == NULL) {
    return NULL;
  }
  hander_data->packages_.package_num_ = 0;
  hander_data->packages_.package_ = NULL;
  WTSendLinkForMixHander *hander = (WTSendLinkForMixHander *)malloc(sizeof(WTSendLinkForMixHander));
  if (hander == NULL) {
    free(hander_data);
    return NULL;
  }
  hander->data_ = hander_data;
  return hander;
}

void WTSendLinkLayerDestroyHanderForMix(WTSendLinkForMixHander * hander)
{
  WTLinkSendHanderDataForMix *hander_data = (WTLinkSendHanderDataForMix *)hander->data_;
  if (hander_data->packages_.package_ != NULL) {
    free(hander_data->packages_.package_);
    hander_data->packages_.package_ = NULL;
  }
  free(hander->data_);
  free(hander);
}

WTSendLinkMixPackageS * WTSendLinkLayerGetPackageForMix(WTSendLinkForMixHander * hander, const void * context, int context_len)
{
  int package_num;
  int data_r_addr = 0;
  WaveTransMixLinkPackage one_package;
  int i;
  if (context_len % MIXING_BYTE_DATA_NUM != 0) {
    package_num = context_len / MIXING_BYTE_DATA_NUM + 1;
  }
  else {
    package_num = context_len / MIXING_BYTE_DATA_NUM ;
  }
  WTLinkSendHanderDataForMix *hander_data = (WTLinkSendHanderDataForMix *)hander->data_;
  hander_data->packages_.package_ = (WaveTransMixPhyPackage *)malloc(sizeof(WaveTransMixPhyPackage)*package_num);
  if (hander_data->packages_.package_ == NULL) {
    return NULL;
  }
  hander_data->packages_.package_num_ = package_num;
  for (i = 0; i < context_len / MIXING_BYTE_DATA_NUM; i++) {
    memcpy(&one_package.byte_data_, ((unsigned char *)context + data_r_addr), MIXING_BYTE_DATA_NUM);
    one_package.real_data_num_ = MIXING_BYTE_DATA_NUM;
    WTLinkGetDataChecksumMix(&one_package);
    WTLinkPackageToMixPackage(&one_package, &hander_data->packages_.package_[i]);
    data_r_addr += MIXING_BYTE_DATA_NUM;
  }
  if (context_len % MIXING_BYTE_DATA_NUM != 0) {
    memcpy(&one_package.byte_data_, ((unsigned char *)context + data_r_addr), context_len % MIXING_BYTE_DATA_NUM);
    one_package.real_data_num_ = context_len % MIXING_BYTE_DATA_NUM;
    WTLinkGetDataChecksumMix(&one_package);
    WTLinkPackageToMixPackage(&one_package, &hander_data->packages_.package_[hander_data->packages_.package_num_ - 1]);
  }
  return &hander_data->packages_;
}

void WTSendLinkLayerReleasePackageForMix(WTSendLinkForMixHander * hander)
{
  WTLinkSendHanderDataForMix *hander_data = (WTLinkSendHanderDataForMix *)hander->data_;
  free(hander_data->packages_.package_);
  hander_data->packages_.package_ = NULL;
}

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
  free(hander_data->packages_.package_);
  hander_data->packages_.package_ = NULL;
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
