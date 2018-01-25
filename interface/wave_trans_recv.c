#include "wave_trans_recv.h"
#include "transceiver/recv/wt_recv_link_layer.h"
#include "transceiver/recv/wt_recv_physical_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

int WaveTransRecvInit(void)
{
  if (WTRecvLinkLayerInitForCompare() != 0) {
    return -1;
  }
  if (WTRecvPhyLayerInitForCompare() != 0) {
    WTRecvLinkLayerExitForCompare();
    return -1;
  }
  return 0;
}

void WaveTransRecvExit(void)
{
  WTRecvPhyLayerExitForCompare();
  WTRecvLinkLayerExitForCompare();
}

void WaveTransRecvSetPcm(const RecvAudioType * pcm, int pcm_len)
{
  WTRecvPhyLayerSendPcmForCompare(pcm,pcm_len);
}

int WaveTransRecvGetContext(void * context, int context_len)
{
  return WTRecvLinkLayerGetDataForCompare(context, context_len);
}


#ifdef __cplusplus
}
#endif