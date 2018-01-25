#include "wt_proto_link_layer.h"
#include "checksum_utils/crc_codec.h"
#include "checksum_utils/rs_code.h"
#include <stdlib.h>
#include <string.h>

#define RS_SYMSIZE			8
#define RS_GFPOLY			0x11d
#define RS_FCR				1
#define RS_PRIM				1


static WTFreqCodeType freq_code_st_mark_[COMPARE_FREQ_ST_NUM] = COMPARE_ST_CODE;

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
