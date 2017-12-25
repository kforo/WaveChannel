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
  int                 real_data_num_;
  unsigned short      check_sum_;
  unsigned char       byte_data_[HBYTE_DATA_NUM / 2];
}WaveTransPackage;



void WTLinkHalfPackageToByte(WaveTransPackageHalf * half_package, WaveTransPackage * package);

int WTLinkCheckStMark(WTPhyFreqMarkType *st_mark);

int WTLinkChecksumOk(WaveTransPackage * package);

void WTLinkGetDataChecksum(WaveTransPackage *package);

void WTLinkPackageToHalf(const WaveTransPackage * package, WaveTransPackageHalf * half_package);


#endif
