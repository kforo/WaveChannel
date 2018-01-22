#include "wt_proto_link_layer.h"
#include "checksum_utils/crc_codec.h"
#include "checksum_utils/rs_code.h"
#include <stdlib.h>
#include <string.h>

#define RS_SYMSIZE			8
#define RS_GFPOLY			0x11d
#define RS_FCR				1
#define RS_PRIM				1

static WTPhyFreqMarkType proto_st_mark_[START_FREQ_NUM] = START_FREQ_MARK;

static WTFreqCodeType freq_code_st_mark_[COMPARE_FREQ_ST_NUM] = COMPARE_ST_CODE;

static WaveTransMixMarksType proto_st_mark_mux_ = {
  4,
  {0,3,16,17},
};

static WaveTransMixMarksType proto_none_mark_mux_ = {
  4,
  { 0,2,16,17 },
};

static WaveTransPhyPackage package_temp;
static int package_temp_addr_ = 0;

static void SetPackageStMark(WTPhyFreqMarkType *mark_data)
{
  int i;
  for (i = 0; i < START_FREQ_NUM; i++) {
    mark_data[i] = proto_st_mark_[i];
  }
}

static void SetPackageStMarkForMix(WaveTransMixMarksType *marks_data)
{
  int i;
  for (i = 0; i < MIXING_BYTE_ST_NUM; i++) {
    memcpy(&marks_data[i], &proto_st_mark_mux_, sizeof(WaveTransMixMarksType));
  }
}

static int MixCheckIsNoneMarks(WaveTransMixMarksType *marks)
{
  if (marks->freq_mark_num_ != proto_none_mark_mux_.freq_mark_num_) {
    return 0;
  }
  int i;
  for (i = 0; i < marks->freq_mark_num_; i++) {
    if (marks->marks_[i] != proto_none_mark_mux_.marks_[i]) {
      return 0;
    }
  }
  return 1;
}

static void MixSetNoneMarks(WaveTransMixMarksType *marks)
{
  memcpy(marks, &proto_none_mark_mux_, sizeof(WaveTransMixMarksType));
}

static unsigned char GetByteDataFromPackage(const WaveTransMixMarksType *marks)
{
  int i;
  unsigned char data = 0;
  for (i = 0; i < MIXING_FREQ_NUM; i++) {
    data += (marks->marks_[i] & 0x03)<<(i*2);
  }
  return data;
}

static void SetMixDataForPackage(unsigned char in_data, WaveTransMixMarksType *out_marks)
{
  out_marks->freq_mark_num_ = MIXING_FREQ_NUM;
  memset(out_marks->marks_, 0, sizeof(WTPhyFreqMarkType)*out_marks->freq_mark_num_);
  int i;
  for (i = 0; i < out_marks->freq_mark_num_; i++) {
    out_marks->marks_[i] = (in_data >> (i * 2)) & 0x03;
    out_marks->marks_[i] += ((unsigned char)i) << 2;
  }
}

void WTLinkGetDataChecksum(WaveTransLinkPackage *package)
{
  void *rs_hander = NULL;
  rs_hander = init_rs(RS_SYMSIZE, RS_GFPOLY, RS_FCR, RS_PRIM, HBYTE_CHECKSUM_NUM/2, ((1 << RS_SYMSIZE) - 1 - (package->real_data_num_ + HBYTE_CHECKSUM_NUM/2)));
  if (rs_hander == NULL) {
    return;
  }
  encode_rs_char(rs_hander, (data_t *)package->byte_data_, (data_t *)(&package->check_sum_));
  free_rs_cache();
}

void WTLinkGetDataChecksumMix(WaveTransMixLinkPackage * package)
{
  void *rs_hander = NULL;
  rs_hander = init_rs(RS_SYMSIZE, RS_GFPOLY, RS_FCR, RS_PRIM, MIXING_CHECKSUM_NUM, ((1 << RS_SYMSIZE) - 1 - (package->real_data_num_ + MIXING_CHECKSUM_NUM)));
  if (rs_hander == NULL) {
    return;
  }
  encode_rs_char(rs_hander, (data_t *)package->byte_data_, (data_t *)(&package->check_sum_));
  free_rs_cache();
}


void WTLinkPackageToHalf(const WaveTransLinkPackage * package, WaveTransPhyPackage * half_package)
{
  SetPackageStMark(half_package->st_mark_);
  int i, j = 0;
  for (i = 0; i < package->real_data_num_; i++) {
    half_package->half_byte_data_[j] = package->byte_data_[i] & 0x0f;
    half_package->half_byte_data_[j + 1] = (package->byte_data_[i] & 0xf0) >> 4;
    j += 2;
  }
  for (; i < HBYTE_DATA_NUM / 2; i++) {
    half_package->half_byte_data_[j] = NONE_MAEK;
    half_package->half_byte_data_[j + 1] = NONE_MAEK;
    j += 2;
  }
  j = 0;
  unsigned char *temp = (unsigned char *)(&package->check_sum_);
  for (i = 0; i < HBYTE_CHECKSUM_NUM / 2; i++) {
    half_package->check_half_byte_data_[j] = temp[i] & 0x0f;
    half_package->check_half_byte_data_[j + 1] = (temp[i] & 0xf0) >> 4;
    j += 2;
  }
}

void WTLinkPackageToMixPackage(const WaveTransMixLinkPackage * package, WaveTransMixPhyPackage * mix_package)
{
  SetPackageStMarkForMix(mix_package->st_mark_);
  int i, j = 0;
  for (i = 0; i < package->real_data_num_; i++) {
    SetMixDataForPackage(package->byte_data_[i], &mix_package->byte_data_[i]);
  }
  for (; i < MIXING_BYTE_DATA_NUM; i++) {
    MixSetNoneMarks(&mix_package->byte_data_[i]);
  }
  j = 0;
  unsigned char *temp = (unsigned char *)(&package->check_sum_);
  for (i = 0; i < MIXING_CHECKSUM_NUM; i++) {
    SetMixDataForPackage(temp[i], &mix_package->check_byte_data_[i]);
  }
}

int WTLinkCheckStCode(WTFreqCodeType code, int addr)
{
  if (code != freq_code_st_mark_[addr]) {
    return 0;
  }
  return 1;
}

int WTLinkChecksumDecode(WaveTransCompareLinkPackage * package)
{
  void *rs_hander = NULL;
  int eras_pos[COMPARE_FREQ_DATA_NUM + COMPARE_FREQ_CHECKSUM_NUM];
  unsigned char buff[COMPARE_FREQ_DATA_NUM + COMPARE_FREQ_CHECKSUM_NUM] = { 0 };
  memset(eras_pos, 0, sizeof(int)*(COMPARE_FREQ_DATA_NUM + COMPARE_FREQ_CHECKSUM_NUM));
  rs_hander = init_rs(RS_SYMSIZE, RS_GFPOLY, RS_FCR, RS_PRIM, COMPARE_FREQ_CHECKSUM_NUM, ((1 << RS_SYMSIZE) - 1 - (package->real_data_num_ + COMPARE_FREQ_CHECKSUM_NUM)));
  if (rs_hander == NULL) {
    return 1;
  }
  memcpy(buff, package->byte_data_, package->real_data_num_);
  memcpy(buff + package->real_data_num_, &package->check_sum_, COMPARE_FREQ_CHECKSUM_NUM);
  decode_rs_char(rs_hander, buff, eras_pos, 0);
  memcpy(package->byte_data_, buff, package->real_data_num_);
  free_rs_cache();
  return 1;
}

void WTLinkChecksumEncode(WaveTransCompareLinkPackage * package)
{
  void *rs_hander = NULL;
  rs_hander = init_rs(RS_SYMSIZE, RS_GFPOLY, RS_FCR, RS_PRIM, COMPARE_FREQ_CHECKSUM_NUM / 2, ((1 << RS_SYMSIZE) - 1 - (package->real_data_num_ + COMPARE_FREQ_CHECKSUM_NUM / 2)));
  if (rs_hander == NULL) {
    return;
  }
  encode_rs_char(rs_hander, (data_t *)package->byte_data_, (data_t *)(&package->check_sum_));
  free_rs_cache();
}

void WTLinkPcakgeToPhyPack(const WaveTransCompareLinkPackage * package, WaveTransComparePhyPackage * phy_pack)
{
  int i;
  for (i = 0; i < COMPARE_FREQ_ST_NUM; i++) {
    phy_pack->st_mark_[i] = freq_code_st_mark_[i];
  }
  for (i = 0; i < package->real_data_num_; i++) {
    phy_pack->byte_data_[i] = package->byte_data_[i];
  }
  for (; i < COMPARE_FREQ_DATA_NUM; i++) {
    phy_pack->byte_data_[i] = COMPARE_FREQ_NONE;
  }
  for (i = 0; i < COMPARE_FREQ_CHECKSUM_NUM; i++) {
    phy_pack->check_byte_data_[i] = package->check_sum_[i];
  }
}

void WTLinkPhyPcakgeToLinkPack(const WaveTransComparePhyPackage * package, WaveTransCompareLinkPackage * link_pack)
{
  int i;
  link_pack->real_data_num_ = 0;
  for (i = 0; i < COMPARE_FREQ_DATA_NUM; i++) {
    if (package->byte_data_[i] == COMPARE_FREQ_NONE) {
      break;
    }
    link_pack->byte_data_[link_pack->real_data_num_] = (unsigned char)package->byte_data_[i];
    link_pack->real_data_num_++;
  }
  for (i = 0; i < COMPARE_FREQ_CHECKSUM_NUM; i++) {
    link_pack->check_sum_[i] = (unsigned char)package->check_byte_data_[i];
  }
}

void WTLinkHalfPackageToByte(WaveTransPhyPackage * half_package, WaveTransLinkPackage * package)
{
  int i, j = 0;
  for (i = 0; i < HBYTE_DATA_NUM / 2; i++) {
    if (half_package->half_byte_data_[j] == NONE_MAEK) {
      break;
    }
    package->byte_data_[i] = 0x00;
    package->byte_data_[i] = half_package->half_byte_data_[j];
    package->byte_data_[i] += half_package->half_byte_data_[j + 1]<<4;
    j += 2;
  }
  package->real_data_num_ = i;
  unsigned char *temp = (unsigned char *)(&package->check_sum_);
  j = 0;
  for (i = 0; i < HBYTE_CHECKSUM_NUM / 2; i++) {
    temp[i] = 0x00;
    temp[i] = half_package->check_half_byte_data_[j];
    temp[i] += half_package->check_half_byte_data_[j + 1]<<4;
    j += 2;
  }
}



void WTLinkMuxPackageToByte(WaveTransMixPhyPackage * mux_package, WaveTransMixLinkPackage * package)
{
  int i, j = 0;
  package->real_data_num_ = 0;
  for (i = 0; i < MIXING_BYTE_DATA_NUM; i++) {
    if (MixCheckIsNoneMarks(&mux_package->byte_data_[i])==1){
      break;
    }
    package->byte_data_[package->real_data_num_] = GetByteDataFromPackage(&mux_package->byte_data_[i]);
    package->real_data_num_++;
  }
  unsigned char *temp = (unsigned char *)(&package->check_sum_);
  for (i = 0; i < MIXING_CHECKSUM_NUM; i++) {
    temp[i] = GetByteDataFromPackage(&mux_package->check_byte_data_[i]);
  }
}

int WTLinkCheckStMark(WTPhyFreqMarkType st_mark, int mark_num)
{
  if (st_mark != proto_st_mark_[mark_num]) {
    return 0;
  }
  return 1;
}

int WTLinkCheckStMarkMux(WaveTransMixMarksType * marks, int mark_num)
{
  if (marks->freq_mark_num_ != MIXING_FREQ_NUM) {
    return 0;
  }
  int i;
  for (i = 0; i < MIXING_FREQ_NUM; i++) {
    if (marks->marks_[i] != proto_st_mark_mux_.marks_[i]) {
      return 0;
    }
  }
  return 1;
}

int WTLinkChecksumOk(WaveTransLinkPackage * package)
{
  void *rs_hander = NULL;
  int eras_pos[HBYTE_DATA_NUM / 2 + HBYTE_CHECKSUM_NUM / 2];
  unsigned char buff[HBYTE_DATA_NUM / 2 + HBYTE_CHECKSUM_NUM / 2] = { 0 };
  memset(eras_pos, 0, sizeof(int)*(HBYTE_DATA_NUM / 2) + HBYTE_CHECKSUM_NUM/2);
  rs_hander = init_rs(RS_SYMSIZE, RS_GFPOLY, RS_FCR, RS_PRIM, HBYTE_CHECKSUM_NUM / 2, ((1 << RS_SYMSIZE) - 1 - (package->real_data_num_ + HBYTE_CHECKSUM_NUM / 2)));
  if (rs_hander == NULL) {
    return 1;
  }
  memcpy(buff, package->byte_data_, package->real_data_num_);
  memcpy(buff + package->real_data_num_, &package->check_sum_, HBYTE_CHECKSUM_NUM / 2);
  decode_rs_char(rs_hander, buff, eras_pos, 0);
  memcpy(package->byte_data_, buff, package->real_data_num_);
  free_rs_cache();
  return 1;
}

int WTLinkChecksumOkMux(WaveTransMixLinkPackage * package)
{
  void *rs_hander = NULL;
  int eras_pos[MIXING_BYTE_DATA_NUM + MIXING_CHECKSUM_NUM];
  unsigned char buff[MIXING_BYTE_DATA_NUM + MIXING_CHECKSUM_NUM] = { 0 };
  memset(eras_pos, 0, sizeof(int)*MIXING_BYTE_DATA_NUM + MIXING_CHECKSUM_NUM);
  rs_hander = init_rs(RS_SYMSIZE, RS_GFPOLY, RS_FCR, RS_PRIM, MIXING_CHECKSUM_NUM, ((1 << RS_SYMSIZE) - 1 - (package->real_data_num_ + MIXING_CHECKSUM_NUM)));
  if (rs_hander == NULL) {
    return 1;
  }
  memcpy(buff, package->byte_data_, package->real_data_num_);
  memcpy(buff + package->real_data_num_, &package->check_sum_, MIXING_CHECKSUM_NUM);
  decode_rs_char(rs_hander, buff, eras_pos, 0);
  memcpy(package->byte_data_, buff, package->real_data_num_);
  free_rs_cache();
  return 1;
}
