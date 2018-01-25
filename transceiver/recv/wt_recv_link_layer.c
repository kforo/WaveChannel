#include "wt_recv_link_layer.h"
#include "proto_utils/wt_proto_link_layer.h"
#include "wt_recv_physical_layer.h"
#include "proto_utils/wt_proto_common.h"

#include <stdlib.h>
#include <string.h>

static WaveTransPhyPackage package_temp_;
static int package_addr_ = 0;

static int GetNextFreqCodePackage(WaveTransLinkPackage *package)
{
  int ret;
  if (package_addr_ >= 0 && package_addr_ < COMPARE_FREQ_ST_NUM) {    //find package start mark
    int loop_num = 10;
    while (loop_num--) {
      WTFreqCodeType temp;
      ret = WTRecvPhuLayerGetData(&temp, 1);
      if (ret <= 0) {
        break;
      }
      if (WTLinkCheckStCode(temp, package_addr_) != 1) {
        package_addr_ = 0;
      }
      else {
        package_temp_.st_mark_[package_addr_] = temp;
        package_addr_++;
      }
      if (package_addr_ == COMPARE_FREQ_ST_NUM) {
        break;
      }
    }
  }
  if (package_addr_ >= COMPARE_FREQ_ST_NUM&&package_addr_ < COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM) {     //read half_byte_data_
    int dst_addr = COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM;
    ret = WTRecvPhuLayerGetData(&package_temp_.byte_data_[package_addr_ - COMPARE_FREQ_ST_NUM], dst_addr - package_addr_);
    if (ret <= 0) {
      return -1;
    }
    package_addr_ += ret;
  }
  /*read crc_half_byte_data_*/
  if (package_addr_ >= (COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM) && package_addr_ < (COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM + COMPARE_FREQ_CHECKSUM_NUM)) {
    int dst_addr = COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM + COMPARE_FREQ_CHECKSUM_NUM;
    int addr_temp = package_addr_ - (COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM);
    ret = WTRecvPhuLayerGetData(&package_temp_.check_byte_data_[addr_temp], dst_addr - package_addr_);
    if (ret <= 0) {
      return -1;
    }
    package_addr_ += ret;
  }
  /*check one package*/
  if (package_addr_ == COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM + COMPARE_FREQ_CHECKSUM_NUM) {
    WaveTransLinkPackage temp;
    WTLinkPackageDecode(&package_temp_, &temp);
    if (WTLinkChecksumDecode(&temp) != 1) {
      package_addr_ = 0;
      return -1;
    }
    memcpy(package, &temp, sizeof(WaveTransLinkPackage));
    package_addr_ = 0;
    return 0;
  }
  return -1;
}

int WTRecvLinkLayerInit()
{
  package_addr_ = 0;
  return 0;
}

void WTRecvLinkLayerExit()
{
  package_addr_ = 1;
}

int WTRecvLinkLayerGetData(void * buf, int buf_len)
{
  WaveTransLinkPackage one_package;
  int buf_w_addr = 0;
  while (buf_len - buf_w_addr >= COMPARE_FREQ_DATA_NUM) {
    if (GetNextFreqCodePackage(&one_package) != 0) {
      break;
    }
    memcpy(((unsigned char *)buf + buf_w_addr), one_package.byte_data_, sizeof(unsigned char)*one_package.real_data_num_);
    buf_w_addr += one_package.real_data_num_;
  }
  return buf_w_addr;
}
