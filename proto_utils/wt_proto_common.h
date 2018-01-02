#ifndef __WT_PROTO_COMMON_H__
#define __WT_PROTO_COMMON_H__

/*global config*/
#define FREQ_MODE_MUX
#define FREQ_LIST               {1765,1986,2211,2492,2799,3243,3482,3751,3987,4192,4430,4794,5000,5449,5900,6262,6627,7004}
#define FREQ_LIST_LEN           (18)
#define MAX_FREQ                (7004)
#define MIN_FREQ                (1765)
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
#define  AUDIO_AMPLITUDE_SCALE            (85)  //AUDIO_AMPLITUDE_SCALE/100 * max value





#endif
