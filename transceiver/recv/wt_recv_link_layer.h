#ifndef __WT_RECV_LINK_LAYER_H__
#define __WT_RECV_LINK_LAYER_H__
#include "proto_utils/wt_proto_physical_layer.h"

int WTRecvLinkLayerInit(void);

void WTRecvLinkLayerExit(void);

int WTRecvLinkLayerGetData(void *buf, int buf_len);


int WTRecvLinkLayerInitForMix(void);

void WTRecvLinkLayerExitForMix(void);

int WTRecvLinkLayerGetDataForMix(void *buf, int buf_len);


int WTRecvLinkLayerInitForCompare();

void WTRecvLinkLayerExitForCompare();

int WTRecvLinkLayerGetDataForCompare(void *buf, int buf_len);

#endif
