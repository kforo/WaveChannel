#ifndef __WT_SEND_LINK_LAYER_H__
#define __WT_SEND_LINK_LAYER_H__
#include "proto_utils/wt_proto_link_layer.h"
#define ONE_PACKAGE_NUM             (START_FREQ_NUM + HBYTE_DATA_NUM + HBYTE_CHECKSUM_NUM)

int WTSendLinkLayerInit();

void WTSendLinkLayerExit();

void WTSendLinkLayerSetData(const void *buf, int buf_len);

int WTSendLinkGetPackage(WaveTransPackageHalf *package, int package_len);


#endif
