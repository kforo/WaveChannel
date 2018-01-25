#ifndef __WT_PROTO_LINK_LAYER_H__
#define __WT_PROTO_LINK_LAYER_H__
#include "wt_proto_common.h"
#include "wt_proto_physical_layer.h"

typedef struct {
  WTFreqCodeType              st_mark_[COMPARE_FREQ_ST_NUM];
  WTFreqCodeType              byte_data_[COMPARE_FREQ_DATA_NUM];
  WTFreqCodeType              check_byte_data_[COMPARE_FREQ_CHECKSUM_NUM];
}WaveTransPhyPackage;;

typedef struct {
  int               real_data_num_;
  unsigned char    check_sum_[COMPARE_FREQ_CHECKSUM_NUM];
  unsigned char     byte_data_[COMPARE_FREQ_DATA_NUM];
}WaveTransLinkPackage;


int WTLinkCheckStCode(WTFreqCodeType code, int addr);

int WTLinkChecksumDecode(WaveTransLinkPackage *package);

void WTLinkChecksumEncode(WaveTransLinkPackage *package);

void WTLinkPackageEncode(const WaveTransLinkPackage *package, WaveTransPhyPackage *phy_pack);

void WTLinkPackageDecode(const WaveTransPhyPackage *package, WaveTransLinkPackage *link_pack);


#endif
