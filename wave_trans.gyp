#
# copyright 2017, sohu inc
# author: chfw
# wave_trans.gyp
# 2017-06-05
#
{   
    'variables': {
    },
    'targets': [
		  {
        'target_name': 'libwavetrans',
        'type': 'static_library',
        'include_dirs': [
          '.',
        ],
        'sources': [
		  'buff_utils/queue.c',
		  'buff_utils/queue.h',
		  'buff_utils/ring_buff.c',
		  'buff_utils/ring_buff.h',
		  'checksum_utils/crc_codec.c',
		  'checksum_utils/crc_codec.h',
#		  'checksum_utils/rs_code.c',
#		  'checksum_utils/rs_code.h',
		  'kiss_fft/_kiss_fft_guts.h',
		  'kiss_fft/kiss_fastfir.c',
		  'kiss_fft/kiss_fastfir.h',
		  'kiss_fft/kiss_fft.c',
		  'kiss_fft/kiss_fft.h',
		  'kiss_fft/kiss_fftr.c',
		  'kiss_fft/kiss_fftr.h',
		  'log_utils/wt_log.h',
		  'proto_utils/wt_proto_common.h',
		  'proto_utils/wt_proto_link_layer.c',
		  'proto_utils/wt_proto_link_layer.h',
		  'proto_utils/wt_proto_physical_layer.c',
		  'proto_utils/wt_proto_physical_layer.h',
		  'transceiver/recv/wt_recv_link_layer.c',
		  'transceiver/recv/wt_recv_link_layer.h',
		  'transceiver/recv/wt_recv_physical_layer.c',
		  'transceiver/recv/wt_recv_physical_layer.h',
		  'transceiver/send/wt_send_link_layer.c',
		  'transceiver/send/wt_send_link_layer.h',
		  'transceiver/send/wt_send_physical_layer.c',
		  'transceiver/send/wt_send_physical_layer.h',
		  'interface/wave_trans_recv.c',
		  'interface/wave_trans_recv.h',
		  'interface/wave_trans_send.c',
		  'interface/wave_trans_send.h',
        ],
      }, # target libwavetrans
      {
        'target_name': 'wave_tran_recv',
        'type': 'executable',
        'include_dirs': [
          './demo',
		  '.',
        ],
        'sources': [
          'demo/recv_test.c',
        ],
        'dependencies': [
          'libwavetrans',
        ],
      }, #target wave_tran_recv
	  {
        'target_name': 'wave_tran_send',
        'type': 'executable',
        'include_dirs': [
          './demo',
		  '.',
        ],
        'sources': [
          'demo/send_test.c',
        ],
        'dependencies': [
          'libwavetrans',
        ],
      }, #target wave_tran_send
    ],
}
