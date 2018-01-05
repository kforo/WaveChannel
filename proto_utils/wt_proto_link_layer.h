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
  WaveTransMixFreqMark       st_mark_[MIXING_BYTE_ST_NUM];
  WaveTransMixFreqMark       byte_data_[MIXING_BYTE_DATA_NUM];
  WaveTransMixFreqMark       check_byte_data_[MIXING_CHECKSUM_NUM];
}WaveTransPackageMix;

typedef struct {
  int                 real_data_num_;
  unsigned short      check_sum_;
  unsigned char       byte_data_[HBYTE_DATA_NUM / 2];
}WaveTransPackage;

typedef struct {
  int               real_data_num_;
  unsigned short    check_sum_;
  unsigned char     byte_data_[MIXING_BYTE_DATA_NUM];
}WaveTransPackageMux;



void WTLinkHalfPackageToByte(WaveTransPackageHalf * half_package, WaveTransPackage * package);

void WTLinkMuxPackageToByte(WaveTransPackageMix *mux_package, WaveTransPackageMux *package);

int WTLinkCheckStMark(WTPhyFreqMarkType st_mark,int mark_num);

int WTLinkCheckStMarkMux(WaveTransMixFreqMark *marks, int mark_num);

int WTLinkChecksumOk(WaveTransPackage * package);

int WTLinkChecksumOkMux(WaveTransPackageMux *package);

void WTLinkGetDataChecksum(WaveTransPackage *package);

void WTLinkGetDataChecksumMix(WaveTransPackageMux *package);

void WTLinkPackageToHalf(const WaveTransPackage * package, WaveTransPackageHalf * half_package);

void WTLinkPackageToMixPackage(const WaveTransPackageMux *package, WaveTransPackageMix *mix_package);


#endif
