#include "wt_recv_physical_layer.h"
#include "buff_utils/ring_buff.h"

#include <stdio.h>
#include <string.h>
#define FREQ_ANA_BUF_SIZE               time_ms_to_length(FREQ_ANALYZE_SAMPLE_TIME_MS,RECV_SAMPLE_RATE)

#ifndef WIN32
#include <pthread.h>
static pthread_mutex_t buff_mutex_ = PTHREAD_MUTEX_INITIALIZER;
#define BUF_LOCK_INIT()               pthread_mutex_init(&buff_mutex_,NULL)
#define BUF_LOCK_EXIT()               pthread_mutex_destroy(&buff_mutex_)
#define BUF_LOCK()                    pthread_mutex_lock(&buff_mutex_)
#define BUF_UNLOCK()                  pthread_mutex_unlock(&buff_mutex_)
#else
#define BUF_LOCK_INIT()         
#define BUF_LOCK_EXIT()            
#define BUF_LOCK()                   
#define BUF_UNLOCK() 
#endif


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
  BUF_LOCK();
  while ((ret = RingBuffReadData(ring_buff_fd_, &temp, sizeof(WTPhyFreqMarkType))) != 0) {
    if (temp == old_mark_ref_.phy_mark_) {
      old_mark_ref_.mark_num_++;
      int old_num = WTPhyAnalysisNumToRealNum(old_mark_ref_.mark_num_);
      if (old_num > old_mark_ref_.already_num_) {
        *data = temp;
        old_mark_ref_.already_num_++;
        BUF_UNLOCK();
        return 0;
      }
    }
    else {
      old_mark_ref_.phy_mark_ = temp;
      old_mark_ref_.already_num_ = 0;
      old_mark_ref_.mark_num_ = 1;
    }
  }
  BUF_UNLOCK();
  return -1;
}


int WTRecvPhyLayerInit(void)
{
  ring_buff_fd_ = RingBuffCreate(RING_BUFF_LEN);
  if (ring_buff_fd_ == NULL) {
    return -1;
  }
  BUF_LOCK_INIT();
  old_mark_ref_.phy_mark_ = REF_MARK_INIT_DATA;
  old_mark_ref_.already_num_ = 0;
  old_mark_ref_.mark_num_ = 0;
  return 0;
}

void WTRecvPhyLayerExit(void)
{
  BUF_LOCK();
  RingBuffDestroy(ring_buff_fd_);
  BUF_UNLOCK();
  BUF_LOCK_EXIT();
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
  
  if (pcm_buf_w_addr_ != 0) {
    if (pcm_len < FREQ_ANA_BUF_SIZE - pcm_buf_w_addr_) {
      memcpy(&pcm_buf_[pcm_buf_w_addr_], pcm, sizeof(RecvAudioType)*pcm_len);
      pcm_buf_w_addr_ += pcm_len;
      return;
    }
    memcpy(&pcm_buf_[pcm_buf_w_addr_], pcm, sizeof(RecvAudioType)*(FREQ_ANA_BUF_SIZE - pcm_buf_w_addr_));
    pcm_r_addr += FREQ_ANA_BUF_SIZE - pcm_buf_w_addr_;
    pcm_buf_w_addr_ = 0;
    if (WTPhysicalPcmToFreqMark(pcm_buf_, FREQ_ANA_BUF_SIZE, &mark) == 0) {
      BUF_LOCK();
      RingBuffWriteData(ring_buff_fd_, &mark, sizeof(WTPhyFreqMarkType));
      BUF_UNLOCK();
    }
   // printf(" %d ", mark);
  }
  while (pcm_len - pcm_r_addr >= FREQ_ANA_BUF_SIZE) {
    if (WTPhysicalPcmToFreqMark(pcm + pcm_r_addr, FREQ_ANA_BUF_SIZE, &mark) != 0) {
      pcm_r_addr += FREQ_ANA_BUF_SIZE;
      continue;
    }
   // printf(" %d ", mark);
    pcm_r_addr += FREQ_ANA_BUF_SIZE;
    BUF_LOCK();
    RingBuffWriteData(ring_buff_fd_, &mark, sizeof(WTPhyFreqMarkType));
    BUF_UNLOCK();
  }

  if(pcm_r_addr<pcm_len){
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
