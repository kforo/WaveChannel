#ifndef __WINDOWS_AUDIO_INPUT_H__
#define __WINDOWS_AUDIO_INPUT_H__

typedef void(*audioInCallBack)(const void *pcm_buf, const int pcm_len);

void WinAudioStart(audioInCallBack call_back);

void WinAudioStop();




#endif
