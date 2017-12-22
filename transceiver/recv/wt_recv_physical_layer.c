#include "wt_recv_physical_layer.h"
#include "buff_utils/ring_buff.h"

#include <stdio.h>
#include <string.h>
#define FREQ_ANA_BUF_SIZE               time_ms_to_length(FREQ_ANALYZE_SAMPLE_TIME_MS,SEND_SAMPLE_RATE)


typedef struct {
  WTPhyFreqMarkType   phy_mark_;
  int                     mark_num_;
  int                     already_num_;
}OldPhyMarkRef;
#define REF_MARK_INIT_DATA        (30)

#define RING_BUFF_LEN             (1024)

static RingBuffFd *ring_buff_fd_ = NULL;
static OldPhyMarkRef old_mark_ref_;

static RecvAudioType pcm_buf_[FREQ_ANA_BUF_SIZE];
static int pcm_buf_w_addr_ = 0;

static int GetNextData(WTPhyFreqMarkType *data)
{
  WTPhyFreqMarkType temp;
  int ret;
  while ((ret = RingBuffReadData(ring_buff_fd_, &temp, sizeof(WTPhyFreqMarkType))) != 0) {
    if (temp == old_mark_ref_.phy_mark_) {
      old_mark_ref_.mark_num_++;
      int old_num = WTPhyAnalysisNumToRealNum(old_mark_ref_.mark_num_);
      if (old_num > old_mark_ref_.already_num_) {
        *data = temp;
        old_mark_ref_.already_num_++;
        return 0;
      }
    }
    else {
      old_mark_ref_.phy_mark_ = temp;
      old_mark_ref_.already_num_ = 0;
      old_mark_ref_.mark_num_ = 1;
    }
  }
  return -1;
}


int WTRecvPhyLayerInit()
{
  ring_buff_fd_ = RingBuffCreate(RING_BUFF_LEN);
  if (ring_buff_fd_ == NULL) {
    return -1;
  }
  old_mark_ref_.phy_mark_ = REF_MARK_INIT_DATA;
  old_mark_ref_.already_num_ = 0;
  old_mark_ref_.mark_num_ = 0;
  return 0;
}

void WTRecvPhyLayerExit()
{
  RingBuffDestroy(ring_buff_fd_);
  ring_buff_fd_ = NULL;
  old_mark_ref_.phy_mark_ = REF_MARK_INIT_DATA;
  old_mark_ref_.already_num_ = 0;
  old_mark_ref_.mark_num_ = 0;
  pcm_buf_w_addr_ = 0;
}

void WTRecvPhyLayerSendPcm(const RecvAudioType * pcm, int pcm_len)
{
  int pcm_r_addr = 0;
  WTPhyFreqMarkType mark;
  
  if (IF_LIKELY(pcm_buf_w_addr_ != 0)) {
    if (IF_UNLIKELY(pcm_len < FREQ_ANA_BUF_SIZE - pcm_buf_w_addr_)) {
      memcpy(&pcm_buf_[pcm_buf_w_addr_], pcm, sizeof(RecvAudioType)*pcm_len);
      pcm_buf_w_addr_ += pcm_len;
      return;
    }
    memcpy(&pcm_buf_[pcm_buf_w_addr_], pcm, sizeof(RecvAudioType)*(FREQ_ANA_BUF_SIZE - pcm_buf_w_addr_));
    pcm_r_addr += FREQ_ANA_BUF_SIZE - pcm_buf_w_addr_;
    pcm_buf_w_addr_ = 0;
    if (IF_UNLIKELY(WTPhysicalPcmToFreqMark(pcm_buf_, FREQ_ANA_BUF_SIZE, &mark) != 0)) {
      RingBuffWriteData(ring_buff_fd_, &mark, sizeof(WTPhyFreqMarkType));
    }
  }
  while (pcm_len - pcm_r_addr >= FREQ_ANA_BUF_SIZE) {
    if (IF_LIKELY(WTPhysicalPcmToFreqMark(pcm + pcm_r_addr, FREQ_ANA_BUF_SIZE, &mark) != 0)) {
      pcm_r_addr += FREQ_ANA_BUF_SIZE;
      continue;
    }
    pcm_r_addr += FREQ_ANA_BUF_SIZE;
    RingBuffWriteData(ring_buff_fd_, &mark, sizeof(WTPhyFreqMarkType));
  }

  if(IF_LIKELY(pcm_r_addr<pcm_len)){
    memcpy(pcm_buf_, pcm + pcm_r_addr, sizeof(RecvAudioType)*(pcm_len - pcm_r_addr));
    pcm_buf_w_addr_ = pcm_len - pcm_r_addr;
  }
}

int WTRecvPhyLayerGetData(WTPhyFreqMarkType *data_buf, int buf_len)
{
  int i;
  for (i = 0; i < buf_len; i++) {
    if (GetNextData(&data_buf[i]) != 0) {
      break;
    }
  }
  return i;
}
