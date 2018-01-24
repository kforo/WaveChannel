#ifndef __WT_PROTO_COMMON_H__
#define __WT_PROTO_COMMON_H__

/*global config*/
//#define FREQ_MODE_MUX
#define FREQ_MODE_COMPARE
typedef unsigned char WTPhyFreqMarkType;
typedef struct {
  int                   freq_;
  WTPhyFreqMarkType     mark_;
}FreqAsMark;

typedef unsigned short WTFreqCodeType;

static const FreqAsMark freq_to_mark_list_[] = {
  { 740,0 },
  { 988,1 },
  { 1480,2 },
  { 1760,3 },
  { 1976,4 },
  { 2217,5 },
  { 2489,6 },
  { 2794,7 },
  { 2960,8 },
  { 3520,9 },
  { 3729,10 },
  { 3951,11 },
  { 4186,12 },
  { 4435,13 },
  { 4699,14 },
  { 4978,15 },
  { 5274,16 },
  { 5588,17 },
  { -1,-1 },  //list end
};
#define MAX_FREQ                (5588)
#define MIN_FREQ                (740)
//#define FREQ_LIST               {1765,1986,2211,2492,2799,3243,3482,3751,3987,4192,4430,4794,5000,5449,5900,6262,6627,7004}
#define FREQ_LIST_LEN           (18)
#define START_FREQ_NUM                  (2)
#define HBYTE_DATA_NUM                  (8)
#define HBYTE_CHECKSUM_NUM              (4)
#define START_FREQ_MARK                  {16,17}
#define NONE_MAEK                        (17)
#define ONE_FREQ_TIME_MS                 (24)
#define time_ms_to_length(time_ms,sample_rate)          (((time_ms)*(sample_rate))/1000)

#define MIXING_FREQ_NUM                   (4)
#define MIXING_MARK_PROGRESSIVE           (4)
#define MIXING_BYTE_ST_NUM                (1)    
#define MIXING_BYTE_DATA_NUM              (4)
#define MIXING_CHECKSUM_NUM               (2)

#define COMPARE_FREQ_ST_NUM                 (2)
#define COMPARE_FREQ_DATA_NUM               (4)
#define COMPARE_FREQ_CHECKSUM_NUM           (2)
#define COMPARE_FREQ_NUM                    (18)
#define COMPARE_FREQ_BIT                    (9)
#define COMPARE_FREQ_PARITY_BIT              (1)
#define COMPARE_FREQ_NONE                    (0x0103)
#define COMPARE_ST_CODE                     {0x1aa,0x155}
typedef struct {
  int left_freq_;
  int right_freq_;
  int bool_;
}CompareFreqNode;

extern CompareFreqNode compare_freq_list_[];


/*recv side config*/
#define RECV_SAMPLE_BIT                         (16)
#define RECV_SAMPLE_RATE                        (16000)
#define FREQ_ANALYZE_SAMPLE_TIME_MS             (8)
#if (RECV_SAMPLE_BIT==8)
typedef char    RecvAudioType;
#endif
#if (RECV_SAMPLE_BIT==16)
typedef short   RecvAudioType;
#endif



/*send side config*/
#define  AUDIO_AMPLITUDE_SCALE            (40)  //AUDIO_AMPLITUDE_SCALE/100 * max value
#define  AUDIO_NONE_TIME_MS               (200)
#define  AUDIO_NONE_LEN(sample_rate)      ((AUDIO_NONE_TIME_MS*(sample_rate))/1000)




#endif
