#ifndef __WT_SEND_LINK_LAYER_H__
#define __WT_SEND_LINK_LAYER_H__
#include "proto_utils/wt_proto_link_layer.h"
#define ONE_PACKAGE_NUM             (START_FREQ_NUM + HBYTE_DATA_NUM + HBYTE_CHECKSUM_NUM)

typedef struct {
  void          *data_;
}WTSendLinkHander;

typedef struct {
  int                     package_num_;
  WaveTransPackageHalf    *package_;
}WTSendLinkPackageS;

WTSendLinkHander * WTSendLinkLayerCreateHander(void);

void WTSendLinkLayerDestroyHander(WTSendLinkHander *hander);

WTSendLinkPackageS *WTSendLinkLayerGetPackage(WTSendLinkHander *hander, void *context, int context_len);

void WTSendLinkLayerReleasePackage(WTSendLinkHander *hander);


#endif
