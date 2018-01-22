#include "wt_recv_link_layer.h"
#include "proto_utils/wt_proto_link_layer.h"
#include "wt_recv_physical_layer.h"
#include "proto_utils/wt_proto_common.h"

#include <stdlib.h>
#include <string.h>

static WaveTransPhyPackage package_temp_;
static int package_temp_addr_ = 0;

static WaveTransMixPhyPackage mux_package_temp_;
static int package_mux_addr_ = 0;

static WaveTransComparePhyPackage compare_package_temp_;
static int package_compare_addr_ = 0;


static int GetNextPackage(WaveTransLinkPackage *package)
{
  int ret;
  if (package_temp_addr_ >= 0 && package_temp_addr_ < START_FREQ_NUM) {    //find package start mark
re_read_st: while((ret = WTRecvPhyLayerGetData(&package_temp_.st_mark_[package_temp_addr_], START_FREQ_NUM - package_temp_addr_))>0){
       int i;
       for (i = 0; i < ret; i++) {
         if (WTLinkCheckStMark(package_temp_.st_mark_[package_temp_addr_ + i], package_temp_addr_ + i) != 1) {
           package_temp_addr_ = 0;
           goto re_read_st;
         }
       }
       package_temp_addr_ += ret;
       break;
    }
  }
  if (package_temp_addr_ >= START_FREQ_NUM&&package_temp_addr_ < START_FREQ_NUM + HBYTE_DATA_NUM) {     //read half_byte_data_
    int dst_addr = START_FREQ_NUM + HBYTE_DATA_NUM;
    ret = WTRecvPhyLayerGetData(&package_temp_.half_byte_data_[package_temp_addr_ - START_FREQ_NUM], dst_addr - package_temp_addr_);
    if (ret <= 0) {
      return -1;
    }
    package_temp_addr_ += ret;
  }
  /*read crc_half_byte_data_*/
  if (package_temp_addr_ >= (START_FREQ_NUM + HBYTE_DATA_NUM) && package_temp_addr_ < (START_FREQ_NUM + HBYTE_DATA_NUM + HBYTE_CHECKSUM_NUM)) {
    int dst_addr = START_FREQ_NUM + HBYTE_DATA_NUM + HBYTE_CHECKSUM_NUM;
    int addr_temp = package_temp_addr_ - (START_FREQ_NUM + HBYTE_DATA_NUM);
    ret = WTRecvPhyLayerGetData(&package_temp_.check_half_byte_data_[addr_temp], dst_addr - package_temp_addr_);
    if (ret <= 0) {
      return -1;
    }
    package_temp_addr_ += ret;
  }
  /*check one package*/
  if (package_temp_addr_ == START_FREQ_NUM + HBYTE_DATA_NUM + HBYTE_CHECKSUM_NUM) {
    WaveTransLinkPackage temp;
    WTLinkHalfPackageToByte(&package_temp_, &temp);
    if (WTLinkChecksumOk(&temp) != 1) {
      package_temp_addr_ = 0;
      return -1;
    }
    memcpy(package, &temp, sizeof(WaveTransLinkPackage));
    package_temp_addr_ = 0;
    return 0;
  }
  return -1;
}

static int GetNextPackageMux(WaveTransMixLinkPackage *package)
{
  int ret;
  if (package_mux_addr_ >= 0 && package_mux_addr_ < MIXING_BYTE_ST_NUM) {    //find package start mark
  re_read_st: while ((ret = WTRecvPhuLayerGetDataForMixing(&mux_package_temp_.st_mark_[package_mux_addr_], MIXING_BYTE_ST_NUM - package_mux_addr_))>0) {
    int i;
    for (i = 0; i < ret; i++) {
      if (WTLinkCheckStMarkMux(&mux_package_temp_.st_mark_[package_mux_addr_ + i], package_mux_addr_ + i) != 1) {
        package_mux_addr_ = 0;
        goto re_read_st;
      }
    }
    package_mux_addr_ += ret;
    break;
  }
  }
  if (package_mux_addr_ >= MIXING_BYTE_ST_NUM&&package_mux_addr_ < MIXING_BYTE_ST_NUM + MIXING_BYTE_DATA_NUM) {     //read half_byte_data_
    int dst_addr = MIXING_BYTE_ST_NUM + MIXING_BYTE_DATA_NUM;
    ret = WTRecvPhuLayerGetDataForMixing(&mux_package_temp_.byte_data_[package_mux_addr_ - MIXING_BYTE_ST_NUM], dst_addr - package_mux_addr_);
    if (ret <= 0) {
      return -1;
    }
    package_mux_addr_ += ret;
  }
  /*read crc_half_byte_data_*/
  if (package_mux_addr_ >= (MIXING_BYTE_ST_NUM + MIXING_BYTE_DATA_NUM) && package_mux_addr_ < (MIXING_BYTE_ST_NUM + MIXING_BYTE_DATA_NUM + MIXING_CHECKSUM_NUM)) {
    int dst_addr = MIXING_BYTE_ST_NUM + MIXING_BYTE_DATA_NUM + MIXING_CHECKSUM_NUM;
    int addr_temp = package_mux_addr_ - (MIXING_BYTE_ST_NUM + MIXING_BYTE_DATA_NUM);
    ret = WTRecvPhuLayerGetDataForMixing(&mux_package_temp_.check_byte_data_[addr_temp], dst_addr - package_mux_addr_);
    if (ret <= 0) {
      return -1;
    }
    package_mux_addr_ += ret;
  }
  /*check one package*/
  if (package_mux_addr_ == MIXING_BYTE_ST_NUM + MIXING_BYTE_DATA_NUM + MIXING_CHECKSUM_NUM) {
    WaveTransMixLinkPackage temp;
    WTLinkMuxPackageToByte(&mux_package_temp_, &temp);
    if (WTLinkChecksumOkMux(&temp) != 1) {
      package_mux_addr_ = 0;
      return -1;
    }
    memcpy(package, &temp, sizeof(WaveTransMixLinkPackage));
    package_mux_addr_ = 0;
    return 0;
  }
  return -1;
}

static int GetNextFreqCodePackage(WaveTransCompareLinkPackage *package)
{
  int ret;
  if (package_compare_addr_ >= 0 && package_compare_addr_ < COMPARE_FREQ_ST_NUM) {    //find package start mark
    int loop_num = 10;
    while (loop_num--) {
      WTFreqCodeType temp;
      ret = WTRecvPhuLayerGetDataForCompare(&temp, 1);
      if (ret <= 0) {
        break;
      }
      if (WTLinkCheckStCode(temp, package_compare_addr_) != 1) {
        package_compare_addr_ = 0;
      }
      else {
        compare_package_temp_.st_mark_[package_compare_addr_] = temp;
        package_compare_addr_++;
      }
      if (package_compare_addr_ == COMPARE_FREQ_ST_NUM) {
        break;
      }
    }
  }
  if (package_compare_addr_ >= COMPARE_FREQ_ST_NUM&&package_compare_addr_ < COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM) {     //read half_byte_data_
    int dst_addr = COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM;
    ret = WTRecvPhuLayerGetDataForCompare(&compare_package_temp_.byte_data_[package_compare_addr_ - COMPARE_FREQ_ST_NUM], dst_addr - package_compare_addr_);
    if (ret <= 0) {
      return -1;
    }
    package_compare_addr_ += ret;
  }
  /*read crc_half_byte_data_*/
  if (package_compare_addr_ >= (COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM) && package_compare_addr_ < (COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM + COMPARE_FREQ_CHECKSUM_NUM)) {
    int dst_addr = COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM + COMPARE_FREQ_CHECKSUM_NUM;
    int addr_temp = package_compare_addr_ - (COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM);
    ret = WTRecvPhuLayerGetDataForCompare(&compare_package_temp_.check_byte_data_[addr_temp], dst_addr - package_compare_addr_);
    if (ret <= 0) {
      return -1;
    }
    package_compare_addr_ += ret;
  }
  /*check one package*/
  if (package_compare_addr_ == COMPARE_FREQ_ST_NUM + COMPARE_FREQ_DATA_NUM + COMPARE_FREQ_CHECKSUM_NUM) {
    WaveTransCompareLinkPackage temp;
    WTLinkPhyPcakgeToLinkPack(&compare_package_temp_, &temp);
    if (WTLinkChecksumDecode(&temp) != 1) {
      package_compare_addr_ = 0;
      return -1;
    }
    memcpy(package, &temp, sizeof(WaveTransCompareLinkPackage));
    package_compare_addr_ = 0;
    return 0;
  }
  return -1;
}

int WTRecvLinkLayerInit(void)
{
  package_temp_addr_ = 0;
  return 0; 
}

void WTRecvLinkLayerExit(void)
{
  package_temp_addr_ = 0;
}

int WTRecvLinkLayerGetData(void * buf, int buf_len)
{
  WaveTransLinkPackage one_package;
  int buf_w_addr = 0;
  while (buf_len - buf_w_addr >= HBYTE_DATA_NUM / 2) {
    if (GetNextPackage(&one_package) != 0) {
      break;
    }
    memcpy(((unsigned char *)buf + buf_w_addr), one_package.byte_data_, sizeof(unsigned char)*one_package.real_data_num_);
    buf_w_addr += one_package.real_data_num_;
  }
  return buf_w_addr;
}

int WTRecvLinkLayerInitForMix(void)
{
  package_mux_addr_ = 0;
  return 0;
}

void WTRecvLinkLayerExitForMix(void)
{
  package_mux_addr_ = 0;
}

int WTRecvLinkLayerGetDataForMix(void * buf, int buf_len)
{
  WaveTransMixLinkPackage one_package;
  int buf_w_addr = 0;
  while (buf_len - buf_w_addr >= MIXING_BYTE_DATA_NUM) {
    if (GetNextPackageMux(&one_package) != 0) {
      break;
    }
    memcpy(((unsigned char *)buf + buf_w_addr), one_package.byte_data_, sizeof(unsigned char)*one_package.real_data_num_);
    buf_w_addr += one_package.real_data_num_;
  }
  return buf_w_addr;
}

int WTRecvLinkLayerInitForCompare()
{
  package_compare_addr_ = 0;
  return 0;
}

void WTRecvLinkLayerExitForCompare()
{
  package_compare_addr_ = 1;
}

int WTRecvLinkLayerGetDataForCompare(void * buf, int buf_len)
{
  WaveTransCompareLinkPackage one_package;
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
