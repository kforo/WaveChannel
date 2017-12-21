#ifndef __WT_PROTO_LINK_LAYER_H__
#define __WT_PROTO_LINK_LAYER_H__
#include "wt_proto_common.h"
#include "wt_proto_physical_layer.h"

typedef struct {
  WTPhyFreqMarkType   st_mark_[START_FREQ_NUM];
  WTPhyFreqMarkType   half_byte_data_[HBYTE_DATA_NUM];
  WTPhyFreqMarkType   check_half_byte_data_[HBYTE_CHECKSUM_NUM];
}WaveTransPackageHalf;

typedef struct {
  unsigned char       byte_data_[HBYTE_DATA_NUM / 2];
  unsigned short      check_sum_;
  int                 real_data_num_;
}WaveTransPackage;

typedef struct {
  int                     package_num_;
  WaveTransPackageHalf    *package_;
}WaveTransPackageS;

void WTLinkHalfPackageToByte(WaveTransPackageHalf * half_package, WaveTransPackage * package);

int WTLinkCheckStMark(WTPhyFreqMarkType *st_mark);

int WTLinkChecksumOk(WaveTransPackage * package);

WaveTransPackageS * WTLinkGetPackageS(const void *data, int len);

void WTLinkReleasePackageS(WaveTransPackageS *packages);



#endif
