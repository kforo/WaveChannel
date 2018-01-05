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
  WaveTransMixMarksType       st_mark_[MIXING_BYTE_ST_NUM];
  WaveTransMixMarksType       byte_data_[MIXING_BYTE_DATA_NUM];
  WaveTransMixMarksType       check_byte_data_[MIXING_CHECKSUM_NUM];
}WaveTransMixPhyPackage;

typedef struct {
  int                 real_data_num_;
  unsigned short      check_sum_;
  unsigned char       byte_data_[HBYTE_DATA_NUM / 2];
}WaveTransLinkPackage;

typedef struct {
  int               real_data_num_;
  unsigned short    check_sum_;
  unsigned char     byte_data_[MIXING_BYTE_DATA_NUM];
}WaveTransMixLinkPackage;



void WTLinkHalfPackageToByte(WaveTransPhyPackage * half_package, WaveTransLinkPackage * package);

void WTLinkMuxPackageToByte(WaveTransMixPhyPackage *mux_package, WaveTransMixLinkPackage *package);

int WTLinkCheckStMark(WTPhyFreqMarkType st_mark,int mark_num);

int WTLinkCheckStMarkMux(WaveTransMixMarksType *marks, int mark_num);

int WTLinkChecksumOk(WaveTransLinkPackage * package);

int WTLinkChecksumOkMux(WaveTransMixLinkPackage *package);

void WTLinkGetDataChecksum(WaveTransLinkPackage *package);

void WTLinkGetDataChecksumMix(WaveTransMixLinkPackage *package);

void WTLinkPackageToHalf(const WaveTransLinkPackage * package, WaveTransPhyPackage * half_package);

void WTLinkPackageToMixPackage(const WaveTransMixLinkPackage *package, WaveTransMixPhyPackage *mix_package);


#endif
