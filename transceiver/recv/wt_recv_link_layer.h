/*
#
# Copyright 2018, sohu ipc
# author: ChenFawang
# main.cc
# 2018-01-26
# mail: fawangchen@sohu-inc.com
*/
#ifndef __WT_RECV_LINK_LAYER_H__
#define __WT_RECV_LINK_LAYER_H__
#include "proto_utils/wt_proto_physical_layer.h"


int WTRecvLinkLayerInit();

void WTRecvLinkLayerExit();

int WTRecvLinkLayerGetData(void *buf, int buf_len);

#endif
