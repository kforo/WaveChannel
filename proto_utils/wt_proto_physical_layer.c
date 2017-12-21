#include "wt_proto_physical_layer.h"
#include "kiss_fft/kiss_fft.h"
#define PI                      3.1415926535897932384626433832795028841971 
#define MAX_FREQ_MISTAKE                       ((int)(1000.0/FREQ_ANALYZE_SAMPLE_TIME_MS))


static int format_freq_list_[FREQ_LIST_LEN] = FREQ_LIST;
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
    //printf("%d\n", pcm_buf[i]);
    in_data[i].r = (double)pcm_buf[i];
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
  for (i = 0; i < size; i++) {
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

static int CheckFreqAsList(int fft_freq, int *format_freq)
{
  int max_freq_mistake = MAX_FREQ_MISTAKE;
  if (fft_freq<format_freq_list_[0] - max_freq_mistake || fft_freq > format_freq_list_[FREQ_LIST_LEN - 1] + max_freq_mistake) {
    return -1;
  }
  if (fft_freq < format_freq_list_[0]) {
    *format_freq = format_freq_list_[0];
    return 0;
  }
  if (fft_freq > format_freq_list_[FREQ_LIST_LEN - 1]) {
    *format_freq = format_freq_list_[FREQ_LIST_LEN - 1];
    return 0;
  }
  int left = 0;
  int right = FREQ_LIST_LEN - 1;
  while (right - left > 1) {
    int mid = (left + right) / 2;
    if (fft_freq < format_freq_list_[mid]) {
      right = mid;
    }
    else {
      left = mid;
    }
  }
  int left_inval = fft_freq - format_freq_list_[left];
  int right_inval = format_freq_list_[right] - fft_freq;
  if (left_inval > max_freq_mistake&&right_inval > max_freq_mistake) {
    return -1;
  }
  if (left_inval < right_inval) {
    *format_freq = format_freq_list_[left];
  }
  else {
    *format_freq = format_freq_list_[right];
  }
  return 0;

}

static int FormatFreqToMark(int freq, WTPhyFreqMarkType *mark)
{
  int i;
  for (i = 0; i < FREQ_LIST_LEN; i++) {
    if (freq == format_freq_list_[i]) {
      *mark = (unsigned char)i;
      return 0;
    }
  }
  return -1;
}

static int FreqMarkToFreq(WTPhyFreqMarkType freq_mark, int *freq)
{
  if (freq_mark >= 0 && freq_mark < FREQ_LIST_LEN) {
    *freq = format_freq_list_[freq_mark];
    return 0;
  }
  return -1;
}

static int EncodeSound(int freq, SendAudioType *buffer, int buffer_length)
{


  const double amplitude = 29200;
  double theta_increment = 2.0 * PI * (freq) / (SEND_SAMPLE_RATE);
  int frame;

  for (frame = 0; frame < buffer_length; frame++) {

    buffer[frame] = (SendAudioType)(sin(theta) * amplitude);
    theta += theta_increment;

    if (theta > 2.0 * PI) {
      theta -= 2.0 * PI;
    }
  }

  return 0;
}

int WTPhysicalPcmToFreqMark(const RecvAudioType * pcm_buf, int pcm_len, WTPhyFreqMarkType * freq_mark)
{
  int threshold = 50;
  int freq, freq_format;
  freq = GetPcmMaxAmplitudeFreq(pcm_buf, pcm_len, threshold);
  if (freq == -1) {
    return -1;
  }
  if (CheckFreqAsList(freq, &freq_format) != 0) {
    return -1;
  }
  if (FormatFreqToMark(freq_format, freq_mark) != 0) {
    return -1;
  }
  return 0;
}

int WTPhysicalFreqMarkToPcm(WTPhyFreqMarkType freq_mark, SendAudioType * pcm_buf, int pcm_len)
{
  int freq;
  if (FreqMarkToFreq(freq_mark, &freq) != 0) {
    return -1;
  }
  if (EncodeSound(freq, pcm_buf, pcm_len) != 0) {
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
