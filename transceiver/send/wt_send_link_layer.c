#include "wt_send_link_layer.h"
#include "buff_utils/ring_buff.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
  WTSendLinkPackageS       packages_;
}WTLinkSendHanderData;

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
  WaveTransPackage one_package;
  int i;
  if (context_len % (HBYTE_DATA_NUM / 2) != 0) {
    package_num = context_len / (HBYTE_DATA_NUM / 2) + 1;
  }
  else {
    package_num = context_len / (HBYTE_DATA_NUM / 2);
  }
  WTLinkSendHanderData *hander_data = (WTLinkSendHanderData *)hander->data_;
  hander_data->packages_.package_ = (WaveTransPackageHalf *)malloc(sizeof(WaveTransPackageHalf)*package_num);
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
