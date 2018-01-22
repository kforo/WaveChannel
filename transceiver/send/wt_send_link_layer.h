#ifndef __WT_SEND_LINK_LAYER_H__
#define __WT_SEND_LINK_LAYER_H__
#include "proto_utils/wt_proto_link_layer.h"
#define ONE_PACKAGE_NUM             (START_FREQ_NUM + HBYTE_DATA_NUM + HBYTE_CHECKSUM_NUM)

typedef struct {
  void          *data_;
}WTSendLinkHander;

typedef struct {
  void          *data_;
}WTSendLinkForMixHander;

typedef struct {
  void          *data_;
}WTSendLinkForCompareHander;

typedef struct {
  int                     package_num_;
  WaveTransPhyPackage    *package_;
}WTSendLinkPackageS;


typedef struct {
  int                     package_num_;
  WaveTransMixPhyPackage     *package_;
}WTSendLinkMixPackageS;

typedef struct {
  int                           package_num_;
  WaveTransComparePhyPackage    *package_;
}WTSendLinkComparePackageS;

WTSendLinkHander * WTSendLinkLayerCreateHander(void);

void WTSendLinkLayerDestroyHander(WTSendLinkHander *hander);

WTSendLinkPackageS *WTSendLinkLayerGetPackage(WTSendLinkHander *hander,const void *context, int context_len);

void WTSendLinkLayerReleasePackage(WTSendLinkHander *hander);


WTSendLinkForMixHander * WTSendLinkLayerCreateHanderForMix(void);

void WTSendLinkLayerDestroyHanderForMix(WTSendLinkForMixHander *hander);

WTSendLinkMixPackageS *WTSendLinkLayerGetPackageForMix(WTSendLinkForMixHander *hander, const void *context, int context_len);

void WTSendLinkLayerReleasePackageForMix(WTSendLinkForMixHander *hander);


WTSendLinkForCompareHander * WTSendLinkLayerCreateHanderForCompare(void);

void WTSendLinkLayerDestroyHanderForCompare(WTSendLinkForCompareHander *hander);

WTSendLinkComparePackageS *WTSendLinkLayerGetPackageForCompare(WTSendLinkForCompareHander *hander, const void *context, int context_len);

void WTSendLinkLayerReleasePackageForCompare(WTSendLinkForCompareHander *hander);

#endif
