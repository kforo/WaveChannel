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

typedef struct {
  WaveTransMixMarksType     phy_marks_;
  int                         marks_num_;
  int                         already_num_;
}MixingPhyMarkRef;

#define REF_MARK_INIT_DATA        (30)

#define RING_BUFF_LEN             (1024)
#define MIX_RING_BUFF_LEN         (2048)

static RingBuffFd       *ring_buff_fd_ = NULL;
static OldPhyMarkRef    old_mark_ref_;
static RecvAudioType pcm_buf_[FREQ_ANA_BUF_SIZE];
static int pcm_buf_w_addr_ = 0;

static RingBuffFd       *ring_buff_mix_fd_ = NULL;
static MixingPhyMarkRef old_marks_ref_;
static RecvAudioType pcm_buf_mix_[FREQ_ANA_BUF_SIZE];
static int mix_pcm_buf_w_addr_ = 0;


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

static void MixFreqMarkFormat(WaveTransMixMarksType *data)
{
  WTPhyFreqMarkType temp;
  int i,j;
  for (i = 0; i < data->freq_mark_num_; i++) {
    for (j = 1; j < data->freq_mark_num_ - i; j++) {
      if (data->marks_[j] < data->marks_[j-1]) {
        temp = data->marks_[j];
        data->marks_[j] = data->marks_[j - 1];
        data->marks_[j - 1] = temp;
      }
    }
  }
}

static int CheckTwoWaveTransMixinfFreqMark(WaveTransMixMarksType *data_left, WaveTransMixMarksType *data_right)
{
  if (data_left->freq_mark_num_ != data_left->freq_mark_num_) {
    return 1;
  }
  int i;
  for (i = 0; i < data_left->freq_mark_num_; i++) {
    if (data_left->marks_[i] != data_right->marks_[i]) {
      return 1;
    }
  }
  return 0;
}

static int GetNextDataMixing(WaveTransMixMarksType *data)
{
  WaveTransMixMarksType temp;
  int ret;
  BUF_LOCK();
  while ((ret = RingBuffReadData(ring_buff_mix_fd_, &temp, sizeof(WaveTransMixMarksType))) != 0) {
    if (temp.freq_mark_num_ != MIXING_FREQ_NUM) {
      old_marks_ref_.already_num_ = 0;
      old_marks_ref_.marks_num_ = 0;
      old_marks_ref_.phy_marks_.marks_[0] = REF_MARK_INIT_DATA;
      continue;
    }
    MixFreqMarkFormat(&temp);
    ret = CheckTwoWaveTransMixinfFreqMark(&temp, &old_marks_ref_.phy_marks_);
    if (ret == 0) {
      old_marks_ref_.marks_num_++;
      int old_num = WTPhyAnalysisNumToRealNum(old_marks_ref_.marks_num_);
      if (old_num > old_marks_ref_.already_num_) {
        memcpy(data, &temp, sizeof(WaveTransMixMarksType));
        old_marks_ref_.already_num_++;
        BUF_UNLOCK();
        return 0;
      }
    }
    else {
      old_marks_ref_.already_num_ = 0;
      old_marks_ref_.marks_num_ = 1;
      memcpy(&old_marks_ref_.phy_marks_, &temp, sizeof(WaveTransMixMarksType));
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
      //printf(" %d ", mark);
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
    pcm_r_addr += FREQ_ANA_BUF_SIZE;
    BUF_LOCK();
    //printf(" %d ", mark);
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

int WTRecvPhyLayerInitForMixing(void)
{
  ring_buff_mix_fd_ = RingBuffCreate(MIX_RING_BUFF_LEN);
  if (ring_buff_mix_fd_ == NULL) {
    return -1;
  }
  BUF_LOCK_INIT();
  old_marks_ref_.already_num_ = 0;
  old_marks_ref_.marks_num_ = 0;
  old_marks_ref_.phy_marks_.freq_mark_num_ = 4;
  int i;
  for (i = 0; i < old_marks_ref_.phy_marks_.freq_mark_num_; i++) {
    old_marks_ref_.phy_marks_.marks_[i] = REF_MARK_INIT_DATA;
  }
  return 0;
}

void WTRecvPhyLayerExitForMixing(void)
{
  BUF_LOCK();
  RingBuffDestroy(ring_buff_mix_fd_);
  BUF_UNLOCK();
  BUF_LOCK_EXIT();
  ring_buff_mix_fd_ = NULL;
  old_marks_ref_.already_num_ = 0;
  old_marks_ref_.marks_num_ = 0;
  old_marks_ref_.phy_marks_.freq_mark_num_ = 0;
  int i;
  for (i = 0; i < old_marks_ref_.phy_marks_.freq_mark_num_; i++) {
    old_marks_ref_.phy_marks_.marks_[i] = REF_MARK_INIT_DATA;
  }
  mix_pcm_buf_w_addr_ = 0;
}

void WTRecvPhyLayerSendPcmForMixing(const RecvAudioType * pcm, int pcm_len)
{
  int pcm_r_addr = 0;
  WaveTransMixMarksType mark;

  if (mix_pcm_buf_w_addr_ != 0) {
    if (pcm_len < FREQ_ANA_BUF_SIZE - mix_pcm_buf_w_addr_) {
      memcpy(&pcm_buf_mix_[mix_pcm_buf_w_addr_], pcm, sizeof(RecvAudioType)*pcm_len);
      mix_pcm_buf_w_addr_ += pcm_len;
      return;
    }
    memcpy(&pcm_buf_mix_[mix_pcm_buf_w_addr_], pcm, sizeof(RecvAudioType)*(FREQ_ANA_BUF_SIZE - mix_pcm_buf_w_addr_));
    pcm_r_addr += FREQ_ANA_BUF_SIZE - mix_pcm_buf_w_addr_;
    mix_pcm_buf_w_addr_ = 0;
    if (WTPhysicalPcmToFreqMarks(pcm_buf_mix_, FREQ_ANA_BUF_SIZE,&mark) == 0) {
      BUF_LOCK();
      //printf("%d %d %d %d \n", mark.marks_[0], mark.marks_[1], mark.marks_[2], mark.marks_[3]);
      RingBuffWriteData(ring_buff_mix_fd_, &mark, sizeof(WaveTransMixMarksType));
      BUF_UNLOCK();
    }
    // printf(" %d ", mark);
  }
  while (pcm_len - pcm_r_addr >= FREQ_ANA_BUF_SIZE) {
    if (WTPhysicalPcmToFreqMarks(pcm + pcm_r_addr, FREQ_ANA_BUF_SIZE, &mark) != 0) {
      pcm_r_addr += FREQ_ANA_BUF_SIZE;
      continue;
    }
    //printf("%d %d %d %d \n", mark.marks_[0], mark.marks_[1], mark.marks_[2], mark.marks_[3]);
    pcm_r_addr += FREQ_ANA_BUF_SIZE;
    BUF_LOCK();
    //printf(" %d ", mark);
    RingBuffWriteData(ring_buff_mix_fd_, &mark, sizeof(WaveTransMixMarksType));
    BUF_UNLOCK();
  }

  if (pcm_r_addr<pcm_len) {
    memcpy(pcm_buf_mix_, pcm + pcm_r_addr, sizeof(RecvAudioType)*(pcm_len - pcm_r_addr));
    mix_pcm_buf_w_addr_ = pcm_len - pcm_r_addr;
  }
}

int WTRecvPhuLayerGetDataForMixing(WaveTransMixMarksType * data_buf, int buf_len)
{
  int i;
  for (i = 0; i < buf_len; i++) {
    if (GetNextDataMixing(&data_buf[i]) != 0) {
      break;
    }
  }
  return i;
}
