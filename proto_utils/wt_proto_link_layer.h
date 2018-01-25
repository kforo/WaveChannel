#ifndef __WT_PROTO_LINK_LAYER_H__
#define __WT_PROTO_LINK_LAYER_H__
#include "wt_proto_common.h"
#include "wt_proto_physical_layer.h"

typedef struct {
  WTPhyFreqMarkType   st_mark_[START_FREQ_NUM];
  WTPhyFreqMarkType   half_byte_data_[HBYTE_DATA_NUM];
  WTPhyFreqMarkType   check_half_byte_data_[HBYTE_CHECKSUM_NUM];
}WaveTransPhyPackage;

typedef struct {
  WTFreqCodeType              st_mark_[COMPARE_FREQ_ST_NUM];
  WTFreqCodeType              byte_data_[COMPARE_FREQ_DATA_NUM];
  WTFreqCodeType              check_byte_data_[COMPARE_FREQ_CHECKSUM_NUM];
}WaveTransComparePhyPackage;;

typedef struct {
  int                 real_data_num_;
  unsigned short      check_sum_;
  unsigned char       byte_data_[HBYTE_DATA_NUM / 2];
}WaveTransLinkPackage;

typedef struct {
  int               real_data_num_;
  unsigned char    check_sum_[COMPARE_FREQ_CHECKSUM_NUM];
  unsigned char     byte_data_[COMPARE_FREQ_DATA_NUM];
}WaveTransCompareLinkPackage;

void WTLinkHalfPackageToByte(WaveTransPhyPackage * half_package, WaveTransLinkPackage * package);

int WTLinkCheckStMark(WTPhyFreqMarkType st_mark,int mark_num);

int WTLinkChecksumOk(WaveTransLinkPackage * package);

void WTLinkGetDataChecksum(WaveTransLinkPackage *package);

void WTLinkPackageToHalf(const WaveTransLinkPackage * package, WaveTransPhyPackage * half_package);

int WTLinkCheckStCode(WTFreqCodeType code, int addr);

int WTLinkChecksumDecode(WaveTransCompareLinkPackage *package);

void WTLinkChecksumEncode(WaveTransCompareLinkPackage *package);

void WTLinkPcakgeToPhyPack(const WaveTransCompareLinkPackage *package, WaveTransComparePhyPackage *phy_pack);

void WTLinkPhyPcakgeToLinkPack(const WaveTransComparePhyPackage *package, WaveTransCompareLinkPackage *link_pack);


#endif
