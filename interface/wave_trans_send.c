#include "wave_trans_send.h"
#include "transceiver/send/wt_send_link_layer.h"
#include "transceiver/send/wt_send_physical_layer.h"

int WaveTransSendInit()
{
  if (WTSendPhyLayerInit() != 0) {
    return -1;
  }
  if (WTSendLinkLayerInit() != 0) {
    return -1;
  }
  return 0;
}

void WaveTransSendExit()
{
  WTSendLinkLayerExit();
  WTSendPhyLayerExit();
}

void WaveTransSendSetContext(const void * context, int context_len)
{
  WTSendLinkLayerSetData(context, context_len);
}

int WaveTransSendGetPcm(SendAudioType * pcm_buf, int buf_len)
{
  return WTSendPhyLayerGetPcm(pcm_buf, buf_len);
}
