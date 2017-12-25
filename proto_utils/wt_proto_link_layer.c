#include "wt_proto_link_layer.h"
#include "checksum_utils/crc_codec.h.h"
#include <stdlib.h>
#include <string.h>

static WTPhyFreqMarkType proto_st_mark_[START_FREQ_NUM] = START_FREQ_MARK;

static WaveTransPackageHalf package_temp;
static int package_temp_addr_ = 0;

static void SetPackageStMark(WTPhyFreqMarkType *mark_data)
{
  int i;
  for (i = 0; i < START_FREQ_NUM; i++) {
    mark_data[i] = proto_st_mark_[i];
  }
}

void WTLinkGetDataChecksum(WaveTransPackage *package)
{
  package->check_sum_ = crc_16(package->byte_data_, package->real_data_num_);
}

void WTLinkPackageToHalf(const WaveTransPackage * package, WaveTransPackageHalf * half_package)
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

void WTLinkHalfPackageToByte(WaveTransPackageHalf * half_package, WaveTransPackage * package)
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

int WTLinkCheckStMark(WTPhyFreqMarkType * st_mark)
{
  int i;
  for (i = 0; i < START_FREQ_NUM; i++) {
    if (st_mark[i] != proto_st_mark_[i]) {
      return 0;
    }
  }
  return 1;
}

int WTLinkChecksumOk(WaveTransPackage * package)
{
  unsigned short package_checksum = crc_16(package->byte_data_, package->real_data_num_);
  if (package_checksum != package->check_sum_) {
    return 0;
  }
  return 1;
}
