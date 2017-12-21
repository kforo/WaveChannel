#include "wt_send_link_layer.h"
#include "buff_utils/ring_buff.h"
#include <stdio.h>
#include <string.h>

#define SEND_BUFF_LEN                   (2048)
static RingBuffFd * send_data_buf_fd_ = NULL;

static WTPhyFreqMarkType one_mark_temp_[ONE_PACKAGE_NUM];
static int temp_w_addr_ = 0;

static int GetNextPackage(WaveTransPackageHalf *package)
{
  int ret;
  ret = RingBuffReadData(send_data_buf_fd_, &one_mark_temp_[temp_w_addr_], ONE_PACKAGE_NUM - temp_w_addr_);
  if (ret <= 0) {
    return -1;
  }
  temp_w_addr_ += ret;
  if (temp_w_addr_ < ONE_PACKAGE_NUM) {
    return -1;
  }
  memcpy(package, one_mark_temp_, sizeof(WTPhyFreqMarkType)*ONE_PACKAGE_NUM);
  temp_w_addr_ = 0;
  return 0;
}

int WTSendLinkLayerInit()
{
  send_data_buf_fd_ = RingBuffCreate(SEND_BUFF_LEN);
  if (send_data_buf_fd_ == NULL) {
    return -1;
  }
  return 0;
}

void WTSendLinkLayerExit()
{
  temp_w_addr_ = 0;
  RingBuffDestroy(send_data_buf_fd_);
}

void WTSendLinkLayerSetData(const void * buf, int buf_len)
{
  WaveTransPackageS *packages = NULL;
  packages = WTLinkGetPackageS(buf, buf_len);
  if (packages == NULL) {
    return;
  }
  int i;
  for (i = 0; i < packages->package_num_; i++) {
    RingBuffWriteData(send_data_buf_fd_, &packages->package_[i], sizeof(WaveTransPackageHalf));
  }
  WTLinkReleasePackageS(packages);
}

int WTSendLinkGetPackage(WaveTransPackageHalf * package, int package_len)
{
  int i;
  for (i = 0; i < package_len; i++) {
    if (GetNextPackage(&package[i]) != 0) {
      break;
    }
  }
  return i;
}
