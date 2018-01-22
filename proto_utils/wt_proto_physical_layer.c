#include "wt_proto_physical_layer.h"
#include "kiss_fft/kiss_fft.h"
#include "kiss_fft/kiss_fftr.h"
#include "log_utils/wt_log.h"
#include <stdio.h>
#define PI                      3.1415926535897932384626433832795028841971 
#define MAX_FREQ_MISTAKE                       ((int)(1000.0/FREQ_ANALYZE_SAMPLE_TIME_MS))


typedef struct {
  unsigned char           freq_num_;
  int                     freqs_[MIXING_FREQ_NUM];
}MixingFreqsInfo;

typedef struct {
  long        item_;
  int         mark_;
  long        diff_;
}FFTAnalysisSt;

typedef struct {
  int               freq_;
  double            phase_;
}FreqAndPhaseInfo;

typedef struct {
  int                       freq_num_;
  FreqAndPhaseInfo           freq_and_phase_[COMPARE_FREQ_NUM];
}FreqEncodeInfo;


static double theta = 0;

static int GetPcmMaxAmplitudeFreq(const RecvAudioType *pcm_buf, int len, int threshold)
{
  kiss_fft_cpx *in_data = NULL;
  kiss_fft_cpx *out_data = NULL;
  kiss_fft_cfg fft_cfg = NULL;
  in_data = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx)*len);
  if (in_data == NULL) {
    goto error_exit;
  }
  out_data = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx)*len);
  if (out_data == NULL) {
    goto error_exit;
  }
  int i;
  for (i = 0; i < len; i++) {
    in_data[i].r = (float)pcm_buf[i];
    in_data[i].i = 0;
  }
  //printf("################################################\n");
  fft_cfg = kiss_fft_alloc(len, 0, NULL, NULL);
  if (fft_cfg == NULL) {
    goto error_exit;
  }
  kiss_fft(fft_cfg, in_data, out_data);

  int size = len / 2;
  double max_item = 0;
  int max_item_mark = 0;
  int find_left = ((MIN_FREQ - MAX_FREQ_MISTAKE)*len) / RECV_SAMPLE_RATE;
  if (find_left <= 0) {
    find_left = 1;
  }
  int find_right = ((MAX_FREQ + MAX_FREQ_MISTAKE)*len) / RECV_SAMPLE_RATE;
  for (i=find_left; i < find_right; i++) {
    double out_data_item = sqrt(pow(out_data[i].r, 2) + pow(out_data[i].i, 2));
    if (out_data_item > max_item) {
      max_item = out_data_item;
      max_item_mark = i;
    }
  }
  max_item = max_item / (len / 2);
  if (max_item < threshold) {
    goto error_exit;
  }
  free(in_data);
  free(out_data);
  KISS_FFT_FREE(fft_cfg);
  kiss_fft_cleanup();
  return (max_item_mark*RECV_SAMPLE_RATE) / len;

error_exit:
  if (in_data != NULL) {
    free(in_data);
  }
  if (out_data != NULL) {
    free(out_data);
  }
  if (fft_cfg != NULL) {
    KISS_FFT_FREE(fft_cfg);
  }
  kiss_fft_cleanup();
  return -1;
}

static void GetFreqsFromCpx(int nfft, const kiss_fft_cpx *result, MixingFreqsInfo *freqs_info, int threshold)
{
  FFTAnalysisSt *diff_data = NULL;
  int find_left = ((MIN_FREQ - (2 * MAX_FREQ_MISTAKE))*nfft) / RECV_SAMPLE_RATE;
  int find_right = ((MAX_FREQ + (2 * MAX_FREQ_MISTAKE))*nfft) / RECV_SAMPLE_RATE;
  FFTAnalysisSt *max_item[MIXING_FREQ_NUM];
  int diff_data_len = find_right - find_left;
  diff_data = malloc(sizeof(FFTAnalysisSt)*diff_data_len);
  if (diff_data == NULL) {
    return;
  }
  memset(diff_data, 0, sizeof(FFTAnalysisSt)*diff_data_len);
  memset(max_item, 0, sizeof(FFTAnalysisSt *)*MIXING_FREQ_NUM);
  int i;
  for (i = find_left; i < find_right; i++) {
    long out_data_item = (long)(sqrt(pow(result[i].r, 2) + pow(result[i].i, 2)));
    if (i == find_left) {
      diff_data[i - find_left].item_ = out_data_item;
      diff_data[i - find_left].diff_ = out_data_item;
      diff_data[i - find_left].mark_ = i;
    }
    else {
      diff_data[i - find_left].item_ = out_data_item;
      diff_data[i - find_left - 1].diff_ = diff_data[i - find_left].item_ - diff_data[i - find_left - 1].item_;
      diff_data[i - find_left].mark_ = i;
    }
  }
  for (i = 1; i < diff_data_len; i++) {
    if (diff_data[i].diff_ <= 0 && diff_data[i - 1].diff_ > 0) {
      int j;
      for (j = 0; j < MIXING_FREQ_NUM; j++) {
        if (max_item[j] == NULL) {
          max_item[j] = &diff_data[i];
          break;
        }
        else if(diff_data[i].item_>max_item[j]->item_) {
          int temp;
          for (temp = MIXING_FREQ_NUM - 1; temp > j; temp--) {
            max_item[temp] = max_item[temp - 1];
          }
          max_item[j] = &diff_data[i];
          break;
        }
      }
    }
  }
  freqs_info->freq_num_ = 0;
  for (i = 0; i < MIXING_FREQ_NUM; i++) {
    if (max_item[i] != NULL) {
      long amplite = max_item[i]->item_ / (nfft / 2);
      if (amplite >= threshold) {
        freqs_info->freqs_[freqs_info->freq_num_] = (max_item[i]->mark_*RECV_SAMPLE_RATE) / nfft;
        freqs_info->freq_num_++;
      }
    }
  }
  free(diff_data);
}

static int GetPcmFreqs(const RecvAudioType *pcm_buf, int len, int threshold, MixingFreqsInfo *freqs_info)
{
  kiss_fft_cpx *in_data = NULL;
  kiss_fft_cpx *out_data = NULL;
  kiss_fft_cfg fft_cfg = NULL;
  in_data = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx)*len);
  if (in_data == NULL) {
    goto error_exit;
  }
  out_data = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx)*len);
  if (out_data == NULL) {
    goto error_exit;
  }
  int i;
  for (i = 0; i < len; i++) {
    in_data[i].r = (float)pcm_buf[i];
    in_data[i].i = 0;
  }
  //printf("################################################\n");
  fft_cfg = kiss_fft_alloc(len, 0, NULL, NULL);
  if (fft_cfg == NULL) {
    goto error_exit;
  }
  kiss_fft(fft_cfg, in_data, out_data);

  int size = len / 2;
  GetFreqsFromCpx(len, out_data, freqs_info,threshold);
  free(in_data);
  free(out_data);
  KISS_FFT_FREE(fft_cfg);
  kiss_fft_cleanup();
  return 0;

error_exit:
  if (in_data != NULL) {
    free(in_data);
  }
  if (out_data != NULL) {
    free(out_data);
  }
  if (fft_cfg != NULL) {
    KISS_FFT_FREE(fft_cfg);
  }
  kiss_fft_cleanup();
  return -1;
}

static int FreqToFreqMark(int fft_freq, WTPhyFreqMarkType *mark)
{
  int max_freq_mistake = MAX_FREQ_MISTAKE;
  if (fft_freq<freq_to_mark_list_[0].freq_ - max_freq_mistake || fft_freq > freq_to_mark_list_[FREQ_LIST_LEN - 1].freq_ + max_freq_mistake) {
    return -1;
  }
  if (fft_freq < freq_to_mark_list_[0].freq_) {
    *mark = 0;
    return 0;
  }
  if (fft_freq > freq_to_mark_list_[FREQ_LIST_LEN - 1].freq_) {
    *mark = FREQ_LIST_LEN - 1;
    return 0;
  }
  int left = 0;
  int right = FREQ_LIST_LEN - 1;
  while (right - left > 1) {
    int mid = (left + right) / 2;
    if (fft_freq < freq_to_mark_list_[mid].freq_) {
      right = mid;
    }
    else {
      left = mid;
    }
  }
  int left_inval = fft_freq - freq_to_mark_list_[left].freq_;
  int right_inval = freq_to_mark_list_[right].freq_ - fft_freq;
  if (left_inval > max_freq_mistake&&right_inval > max_freq_mistake) {
    return -1;
  }
  if (left_inval < right_inval) {
    *mark = freq_to_mark_list_[left].mark_;
  }
  else {
    *mark = freq_to_mark_list_[right].mark_;
  }
  return 0;
}

static int FreqMarkToFreq(WTPhyFreqMarkType freq_mark, int *freq)
{
  int i;
  for (i = 0; freq_to_mark_list_[i].freq_ != -1; i++) {
    if (freq_mark == freq_to_mark_list_[i].mark_) {
      *freq = freq_to_mark_list_[i].freq_;
      return 0;
    }
  }
  return -1;
}

static void CreatePluralForFreqs(int nfft,const MixingFreqsInfo *freqs_info, kiss_fft_cpx *plural_data,int sample_rate,int amplitude)
{
  //memset(plural_data, 0, sizeof(kiss_fft_cpx)*((nfft / 2) + 1));
  memset(plural_data, 0, sizeof(kiss_fft_cpx)*nfft);
  float temp;
  int n;
  int i;
  for (i = 0; i < freqs_info->freq_num_; i++) {
    temp = ((freqs_info->freqs_[i] * (nfft - 1) * 2.0) / sample_rate);
    n = (int)temp;
    if (temp - n > 0.5) {
      n++;
    }
    plural_data[n].r = amplitude/8;
   // plural_data[nfft - n - 1].r = amplitude / 8;
  }
}

static int EncodeSoundMixing(const MixingFreqsInfo *freqs_info, void *buffer, int buffer_length, int sample_bit, int sample_rate)
{
  kiss_fftr_cfg fftr_cfg = NULL;
  kiss_fft_cpx *in_data = NULL;
  float *out_data = NULL;
  int pcm_num = buffer_length / (sample_bit / 8);
  int in_data_num = (pcm_num / 2) + 1;
  fftr_cfg = kiss_fftr_alloc(pcm_num, 1, NULL, NULL);
  if (fftr_cfg == NULL) {
    return -1;
  }
  in_data = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx)*in_data_num);
  if (in_data == NULL) {
    KISS_FFT_FREE(fftr_cfg);
    return -1;
  }
  int amplitude;
  switch (sample_bit) {
    case 8:amplitude = (127 * AUDIO_AMPLITUDE_SCALE) / 100; break;
    case 16:amplitude = (32767 * AUDIO_AMPLITUDE_SCALE) / 100; break;
    default:
      free(in_data);
      KISS_FFT_FREE(fftr_cfg);
      return -1;
  }
  CreatePluralForFreqs(in_data_num, freqs_info, in_data, sample_rate,amplitude);
  out_data = (float*)malloc(sizeof(float)*pcm_num);
  if (out_data == NULL) {
    free(in_data);
    KISS_FFT_FREE(fftr_cfg);
    return -1;
  }
  kiss_fftri(fftr_cfg, in_data, out_data);
  int i;
  for (i = 0; i < pcm_num; i++) {
    switch (sample_bit) {
      case 8: ((signed char *)buffer)[i] = (signed char)out_data[i]; break;
      case 16: ((short *)buffer)[i] = (short)out_data[i]; break;
      default:
        free(out_data);
        free(in_data);
        KISS_FFT_FREE(fftr_cfg);
        return -1;
    }
  }
  free(in_data);
  free(out_data);
  KISS_FFT_FREE(fftr_cfg);
  return 0;
}

static int WTPhyEncodeSound(int freq, void *buffer, int buffer_length,int sample_bit,int sample_rate)
{


  double amplitude;
  double theta_increment = 2.0 * PI * (freq) / (sample_rate);
  int frame;
  switch (sample_bit) {
    case 8:amplitude = (double)((127 * AUDIO_AMPLITUDE_SCALE) / 100); break;
    case 16:amplitude = (double)((32767 * AUDIO_AMPLITUDE_SCALE) / 100); break;
    default:return -1;
  }

  for (frame = 0; frame < (buffer_length/(sample_bit/8)); frame++) {

    switch (sample_bit) {
      case 8:
        ((signed char *)buffer)[frame] = (signed char)(sin(theta) * amplitude); break;
      case 16:
        ((signed short *)buffer)[frame] = (short)(sin(theta) * amplitude); break;
      default:
        return -1;
    }
    theta += theta_increment;

    if (theta > 2.0 * PI) {
      theta -= 2.0 * PI;
    }
  }

  return 0;
}

static int GetFFTResult(const RecvAudioType *pcm_buf, int len, kiss_fft_cpx *out_data)
{
  kiss_fft_cpx *in_data = NULL;
  kiss_fft_cfg fft_cfg = NULL;
  in_data = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx)*len);
  if (in_data == NULL) {
    goto error_exit;
  }
  int i;
  for (i = 0; i < len; i++) {
    in_data[i].r = (float)pcm_buf[i];
    in_data[i].i = 0;
  }
  //printf("################################################\n");
  fft_cfg = kiss_fft_alloc(len, 0, NULL, NULL);
  if (fft_cfg == NULL) {
    goto error_exit;
  }
  kiss_fft(fft_cfg, in_data, out_data);
  free(in_data);
  KISS_FFT_FREE(fft_cfg);
  kiss_fft_cleanup();
  return 0;

error_exit:
  if (in_data != NULL) {
    free(in_data);
  }
  if (fft_cfg != NULL) {
    KISS_FFT_FREE(fft_cfg);
  }
  kiss_fft_cleanup();
  return -1;
}

static void GetCpxForFreqAndPhase(int amplitude, double phase, kiss_fft_cpx *cpx)
{
  WtLog("input phase:%llf,amplitude:%d\n", phase, amplitude);
  //phase = PI/2;
  double slope = tan(phase);
  WtLog("slope:%llf\n", slope);
  if (phase > (PI*0.5) && phase < PI*1.5) {
    cpx->i = (float)sqrt(amplitude / (1 + pow(slope, 2)));
  }
  else {
    cpx->i = -(float)sqrt(amplitude / (1 + pow(slope, 2)));
  }
  cpx->r = (float)(cpx->i * slope);
}

static void CreatePluralFreqsForFreqCode(int nfft, const FreqEncodeInfo *freqs_info, kiss_fft_cpx *plural_data, int sample_rate, int amplitude)
{
  //memset(plural_data, 0, sizeof(kiss_fft_cpx)*((nfft / 2) + 1));
  memset(plural_data, 0, sizeof(kiss_fft_cpx)*nfft);
  float temp;
  int n;
  int i;
  for (i = 0; i < freqs_info->freq_num_; i++) {
    temp = ((freqs_info->freq_and_phase_[i].freq_ * (nfft - 1) * 2.0) / sample_rate);
    n = (int)temp;
    if (temp - n > 0.5) {
      n++;
    }
    //plural_data[n].r = amplitude / 18;
    GetCpxForFreqAndPhase(amplitude*nfft, freqs_info->freq_and_phase_[i].phase_, &plural_data[n]);
    //GetCpxForFreqAndPhase(amplitude*nfft, freqs_info->freq_and_phase_[i].phase_, &plural_data[nfft-n]);
    // plural_data[nfft - n - 1].r = amplitude / 8;
  }
}

static int EncodeSoundMixingForFreqCode(const FreqEncodeInfo *freqs_info, void *buffer, int buffer_length, int sample_bit, int sample_rate)
{
  kiss_fftr_cfg fftr_cfg = NULL;
  kiss_fft_cpx *in_data = NULL;
  float *out_data = NULL;
  int pcm_num = buffer_length / (sample_bit / 8);
  int in_data_num = (pcm_num / 2) + 1;
  fftr_cfg = kiss_fftr_alloc(pcm_num, 1, NULL, NULL);
  if (fftr_cfg == NULL) {
    return -1;
  }
  in_data = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx)*in_data_num);
  if (in_data == NULL) {
    KISS_FFT_FREE(fftr_cfg);
    return -1;
  }
  int amplitude;
  switch (sample_bit) {
    case 8:amplitude = (127 * AUDIO_AMPLITUDE_SCALE) / 100; break;
    case 16:amplitude = (32767 * AUDIO_AMPLITUDE_SCALE) / 100; break;
    default:
      free(in_data);
      KISS_FFT_FREE(fftr_cfg);
      return -1;
  }
  CreatePluralFreqsForFreqCode(in_data_num, freqs_info, in_data, sample_rate, amplitude);
  out_data = (float*)malloc(sizeof(float)*pcm_num);
  if (out_data == NULL) {
    free(in_data);
    KISS_FFT_FREE(fftr_cfg);
    return -1;
  }
  kiss_fftri(fftr_cfg, in_data, out_data);
  int i;
  for (i = 0; i < pcm_num; i++) {
    switch (sample_bit) {
      case 8: ((signed char *)buffer)[i] = (signed char)out_data[i]; break;
      case 16: ((short *)buffer)[i] = (short)out_data[i]; break;
      default:
        free(out_data);
        free(in_data);
        KISS_FFT_FREE(fftr_cfg);
        return -1;
    }
  }
  free(in_data);
  free(out_data);
  KISS_FFT_FREE(fftr_cfg);
  return 0;
}

WTFreqCodeType GetFreqCodeFromFFTData(const kiss_fft_cpx *fft_data, int fft_len, int sample_time_ms)
{
  int i;
  int step = 1000 / sample_time_ms;
  //FILE *fp = fopen("result.txt", "w");
  WTFreqCodeType temp = 0x0000;
  for (i = 0; compare_freq_list_[i].left_freq_ != -1; i++) {
    int fft_left, fft_right;
    fft_left = compare_freq_list_[i].left_freq_ / step;
    fft_right = compare_freq_list_[i].right_freq_ / step;
    long left_item, right_item;
    left_item = (long)sqrt(pow(fft_data[fft_left].r, 2)+pow(fft_data[fft_left].i, 2));
    right_item = (long)sqrt(pow(fft_data[fft_right].r, 2)+pow(fft_data[fft_right].i, 2));
    if (left_item > right_item) {
      if (compare_freq_list_[i].bool_) {
        temp |= (0x0001) << i;
      }
    }
    else {
      if (!compare_freq_list_[i].bool_) {
        temp |= (0x0001) << i;
      }
    }
  }
  WtLog(" %d ", temp);
  //fclose(fp);
  //printf("temp:%d\n", temp);
  return temp;
}

int WTPhysicalPcmToFreqMark(const RecvAudioType * pcm_buf, int pcm_len, WTPhyFreqMarkType * freq_mark)
{
  int threshold = 50;
  int freq;
  freq = GetPcmMaxAmplitudeFreq(pcm_buf, pcm_len, threshold);
  if (freq == -1) {
    return -1;
  }
  if (FreqToFreqMark(freq, freq_mark) != 0) {
    return -1;
  }
  return 0;
}

int WTPhysicalFreqMarkToPcm(WTPhyFreqMarkType freq_mark, void  *pcm_buf, int pcm_len, int sample_bit,int sample_rate)
{
  int freq;
  if (FreqMarkToFreq(freq_mark, &freq) != 0) {
    return -1;
  }
  if (WTPhyEncodeSound(freq, pcm_buf, pcm_len,sample_bit, sample_rate) != 0) {
    return -1;
  }
  return 0;
}

int WTPhysicalPcmToFreqMarks(const RecvAudioType * pcm_buf, int pcm_len, WaveTransMixMarksType * freq_marks)
{
  int threshold = 50;
  MixingFreqsInfo freq_info;
  if (GetPcmFreqs(pcm_buf, pcm_len, threshold, &freq_info) != 0) {
    return -1;
  }
  if (freq_info.freq_num_ != MIXING_FREQ_NUM) {
    return -1;
  }
  int i;
  freq_marks->freq_mark_num_ = 0;
  for (i = 0; i < freq_info.freq_num_; i++) {
    WTPhyFreqMarkType temp;
    if (FreqToFreqMark(freq_info.freqs_[i], &temp) != 0) {
      return -1;
    }
    freq_marks->marks_[freq_marks->freq_mark_num_] = temp;
    freq_marks->freq_mark_num_++;
  }
  return 0;
}

int WTPhysicalFreqMarksToPcm(const WaveTransMixMarksType * freq_marks, void * pcm_buf, int pcm_len, int sample_bit, int sample_rate)
{
  MixingFreqsInfo freq_info;
  int i;
  freq_info.freq_num_ = freq_marks->freq_mark_num_;
  for (i = 0; i < freq_marks->freq_mark_num_; i++) {
    if (FreqMarkToFreq(freq_marks->marks_[i], &freq_info.freqs_[i]) != 0) {
      return -1;
    }
  }
  if (EncodeSoundMixing(&freq_info, pcm_buf, pcm_len, sample_bit, sample_rate) != 0) {
    return -1;
  }
  return 0;
}

int WTPhyAnalysisNumToRealNum(int ana_num)
{
  int remainder = ana_num % (ONE_FREQ_TIME_MS / FREQ_ANALYZE_SAMPLE_TIME_MS);
  int divsor = ana_num / (ONE_FREQ_TIME_MS / FREQ_ANALYZE_SAMPLE_TIME_MS);

  if (remainder >= ((ONE_FREQ_TIME_MS / FREQ_ANALYZE_SAMPLE_TIME_MS) - 1)) {
    remainder = 1;
  }
  else {
    remainder = 0;
  }
  return remainder + divsor;
}

int WTPhysicalPcmDecode(const RecvAudioType * pcm_buf, int pcm_len, WTFreqCodeType * code)
{
  kiss_fft_cpx *fft_out_data = NULL;
  fft_out_data = (kiss_fft_cpx *)malloc(sizeof(kiss_fft_cpx)*pcm_len);
  if (fft_out_data == NULL) {
    return -1;
  }
  if (GetFFTResult(pcm_buf, pcm_len, fft_out_data) != 0) {
    free(fft_out_data);
    return -1;
  }
  int sample_time_ms = (1000 * pcm_len) / RECV_SAMPLE_RATE;
  *code = GetFreqCodeFromFFTData(fft_out_data, pcm_len, sample_time_ms);
  free(fft_out_data);
  return 0;
}

static double GetNextPhaseAsCurent(double cur_phase, int freq, int sample_rate, int sample_num)
{
  double phase_step = (2 * PI*freq) / sample_rate;
  double end_phase = cur_phase + phase_step*sample_num;
  long temp = (long)(end_phase / (2 * PI));
  end_phase = end_phase - (2 * PI)*temp;
  return end_phase;
}

int WTPhysicalPcmEncode(WTFreqCodeType code, void * pcm_buf, int pcm_len, RefPhaseInfo *ref_phase, int sample_bit, int sample_rate)
{
  FreqEncodeInfo freq_code_info;
  freq_code_info.freq_num_ = 0;
  int i;
  for (i = 0; i < COMPARE_FREQ_BIT; i++) {
    if ((0x0001 << i)&code) {
      if (compare_freq_list_[i].bool_) {
        freq_code_info.freq_and_phase_[freq_code_info.freq_num_].freq_ = compare_freq_list_[i].left_freq_;
        freq_code_info.freq_and_phase_[freq_code_info.freq_num_].phase_ = ref_phase->left_phase_[i];
        ref_phase->left_phase_[i] = GetNextPhaseAsCurent(freq_code_info.freq_and_phase_[freq_code_info.freq_num_].phase_,
          freq_code_info.freq_and_phase_[freq_code_info.freq_num_].freq_, sample_rate, pcm_len / (sample_bit / 8));
        ref_phase->right_phase_[i] = 0.0;
      }
      else {
        freq_code_info.freq_and_phase_[freq_code_info.freq_num_].freq_ = compare_freq_list_[i].right_freq_;
        freq_code_info.freq_and_phase_[freq_code_info.freq_num_].phase_ = ref_phase->right_phase_[i];
        ref_phase->right_phase_[i] = GetNextPhaseAsCurent(freq_code_info.freq_and_phase_[freq_code_info.freq_num_].phase_,
          freq_code_info.freq_and_phase_[freq_code_info.freq_num_].freq_, sample_rate, pcm_len / (sample_bit / 8));
        ref_phase->left_phase_[i] = 0.0;
      }
    }
    else {
      if (!compare_freq_list_[i].bool_) {
        freq_code_info.freq_and_phase_[freq_code_info.freq_num_].freq_ = compare_freq_list_[i].left_freq_;
        freq_code_info.freq_and_phase_[freq_code_info.freq_num_].phase_ = ref_phase->left_phase_[i];
        ref_phase->left_phase_[i] = GetNextPhaseAsCurent(freq_code_info.freq_and_phase_[freq_code_info.freq_num_].phase_,
          freq_code_info.freq_and_phase_[freq_code_info.freq_num_].freq_, sample_rate, pcm_len / (sample_bit / 8));
        ref_phase->right_phase_[i] = 0.0;
      }
      else {
        freq_code_info.freq_and_phase_[freq_code_info.freq_num_].freq_ = compare_freq_list_[i].right_freq_;
        freq_code_info.freq_and_phase_[freq_code_info.freq_num_].phase_ = ref_phase->right_phase_[i];
        ref_phase->right_phase_[i] = GetNextPhaseAsCurent(freq_code_info.freq_and_phase_[freq_code_info.freq_num_].phase_,
          freq_code_info.freq_and_phase_[freq_code_info.freq_num_].freq_, sample_rate, pcm_len / (sample_bit / 8));
        ref_phase->left_phase_[i] = 0.0;
      }
    }
    freq_code_info.freq_num_++;
  }
  if (EncodeSoundMixingForFreqCode(&freq_code_info, pcm_buf, pcm_len, sample_bit, sample_rate) != 0) {
    return -1;
  }
  return 0;
}



