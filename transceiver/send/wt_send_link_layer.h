#ifndef __WT_SEND_LINK_LAYER_H__
#define __WT_SEND_LINK_LAYER_H__
#include "proto_utils/wt_proto_link_layer.h"


typedef struct {
  void          *data_;
}WTSendLinkForCompareHander;

typedef struct {
  int                           package_num_;
  WaveTransComparePhyPackage    *package_;
}WTSendLinkComparePackageS;



WTSendLinkForCompareHander * WTSendLinkLayerCreateHanderForCompare(void);

void WTSendLinkLayerDestroyHanderForCompare(WTSendLinkForCompareHander *hander);

WTSendLinkComparePackageS *WTSendLinkLayerGetPackageForCompare(WTSendLinkForCompareHander *hander, const void *context, int context_len);

void WTSendLinkLayerReleasePackageForCompare(WTSendLinkForCompareHander *hander);

#endif
