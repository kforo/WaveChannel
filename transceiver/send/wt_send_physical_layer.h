#ifndef __WT_SEND_PHYSICAL_LAYER_H__
#define __WT_SEND_PHYSICAL_LAYER_H__
#include "proto_utils/wt_proto_physical_layer.h"
#include "proto_utils/wt_proto_link_layer.h"
#include "transceiver/send/wt_send_link_layer.h"

typedef struct {
  void      *data_;
}WTSendPhyForCompareHander;

typedef struct {
  int           sample_rate_;
  int           sample_bit_;
}WTSendPhyHanderAttr;

WTSendPhyForCompareHander *WTSendPhyLayerCreateHanderForCompare(WTSendPhyHanderAttr *attr);

void WTSendPhyLayerDestroyHanderForCompare(WTSendPhyForCompareHander * hander);

WTSendPcmBuffType * WTSendPhyLayerGetPcmCompare(WTSendPhyForCompareHander *hander, WTSendLinkComparePackageS *packages);

void WTSendPhyLayerReleasePcmCompare(WTSendPhyForCompareHander *hander);


#endif

