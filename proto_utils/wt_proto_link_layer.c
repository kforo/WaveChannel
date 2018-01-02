#include "wt_proto_link_layer.h"
#include "checksum_utils/crc_codec.h"
#include <stdlib.h>
#include <string.h>

static WTPhyFreqMarkType proto_st_mark_[START_FREQ_NUM] = START_FREQ_MARK;

static WaveTransMixFreqMark proto_st_mark_mux_ = {
  4,
  {0,3,16,17},
};

static WaveTransPackageHalf package_temp;
static int package_temp_addr_ = 0;

static void SetPackageStMark(WTPhyFreqMarkType *mark_data)
{
  int i;
  for (i = 0; i < START_FREQ_NUM; i++) {
    mark_data[i] = proto_st_mark_[i];
  }
}

static int CheckIsNoneMarks(WaveTransMixFreqMark *marks)
{

}

static unsigned char GetByteDataFromPackage(WaveTransMixFreqMark *marks)
{
  int i;
  unsigned char data = 0;
  for (i = 0; i < MIXING_FREQ_NUM; i++) {
    data += (marks->marks_[i] & 0x0003)<<(i*2);
  }
  return data;
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



void WTLinkMuxPackageToByte(WaveTransPackageMix * mux_package, WaveTransPackageMux * package)
{
  int i, j = 0;
  package->real_data_num_ = 0;
  for (i = 0; i < MIXING_BYTE_DATA_NUM; i++) {
    if (CheckIsNoneMarks(&mux_package->check_byte_data_[i] == 1)){
      break;
    }
    package->byte_data_[package->real_data_num_] = GetByteDataFromPackage(&mux_package->byte_data_[i]);
    package->real_data_num_++;
  }
  unsigned char *temp = &package->check_sum_;
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

int WTLinkCheckStMarkMux(WaveTransMixFreqMark * marks, int mark_num)
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

int WTLinkChecksumOk(WaveTransPackage * package)
{
  unsigned short package_checksum = crc_16(package->byte_data_, package->real_data_num_);
  if (package_checksum != package->check_sum_) {
    return 0;
  }
  return 1;
}

int WTLinkChecksumOkMux(WaveTransPackageMux * package)
{
  unsigned short package_checksum = crc_16(package->byte_data_, package->real_data_num_);
  if (package_checksum != package->check_sum_) {
    return 0;
  }
  return 1;
}
