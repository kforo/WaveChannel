#include "wt_send_physical_layer.h"
#include "wt_send_link_layer.h"
#include "proto_utils/wt_proto_common.h"
#include "proto_utils/wt_proto_physical_layer.h"

static WTPhyFreqMarkType  data_temp_[ONE_PACKAGE_NUM];
static int data_temp_r_addr_ = ONE_PACKAGE_NUM;

int WTSendPhyLayerInit()
{
  return 0; 
}

void WTSendPhyLayerExit()
{
  data_temp_r_addr_ = ONE_PACKAGE_NUM;
}

int WTSendPhyLayerGetPcm(SendAudioType * pcm, int pcm_len)
{
  int one_mark_pcm_len = time_ms_to_length(ONE_FREQ_TIME_MS, SEND_SAMPLE_RATE);
  int pcm_w_addr = 0;
  while (pcm_len - pcm_w_addr >= one_mark_pcm_len) {
    if (data_temp_r_addr_ >= ONE_PACKAGE_NUM) {
      int ret = WTSendLinkGetPackage((WaveTransPackageHalf *)data_temp_, 1);
      if (ret <= 0) {
        break;
      }
      data_temp_r_addr_ = 0;
      continue;
    }
    else {
      WTPhysicalFreqMarkToPcm(data_temp_[data_temp_r_addr_], pcm + pcm_w_addr, one_mark_pcm_len);
      data_temp_r_addr_++;
      pcm_w_addr += one_mark_pcm_len;
    }
  }
  return pcm_w_addr;
}

//void WTSendPhyLayerSetData(const void *data_buf, int buf_len)
//{
//  WTSendLinkLayerSetData(data_buf, buf_len);
//}
