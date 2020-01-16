#include <pjlib.h>
#include <pjsua-lib/pjsua.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjmedia/transport_srtp.h>
#include "processing-port.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

static pj_status_t create_stream( pj_pool_t *pool, pjmedia_endpt *med_endpt, const pjmedia_codec_info *codec_info, pjmedia_dir dir, pj_uint16_t local_port, const pj_sockaddr_in *rem_addr, pjmedia_stream **p_stream );

int main (int argc, char* argv[]) {
    char errmsg[PJ_ERR_MSG_SIZE];
	pj_status_t status;

	pj_sockaddr_in remote_addr;
	pj_sockaddr_in remote_addr2;
	pj_sockaddr_in remote_addr3;
	pj_sockaddr_in remote_addr4;
	const pjmedia_codec_info *codec_info;
	pj_str_t ip = pj_str("127.0.0.1");
	pj_uint16_t port = 6070;
	pj_uint16_t port2 = 6072;
	pjmedia_stream *stream = NULL;
	pjmedia_stream *stream2 = NULL;
	pjmedia_stream *stream3 = NULL;
	pjmedia_stream *stream4 = NULL;
	pjmedia_port *stream_port = NULL;
	pjmedia_port *stream_port2 = NULL;
	pjmedia_port *stream_port3 = NULL;
	pjmedia_port *stream_port4 = NULL;
	pjmedia_port *mp1, *mp2;
	const char *play_file = "vnt.wav";
	pjmedia_port *play_file_port = NULL;
	unsigned slot1, slot2, slot3;
	pjmedia_conf *conf2 = NULL;
	pjmedia_master_port *master_port = NULL;
	pjmedia_snd_port *snd_port = NULL;
	pjmedia_snd_port *snd_port2 = NULL;
	pjmedia_snd_port *snd_port3 = NULL;
	pjmedia_snd_port *snd_port4 = NULL;
	pj_caching_pool cp;
	pj_pool_t *pool;
	pjmedia_endpt *ep;
	pjmedia_endpt *ep2;
	pj_thread_t *sip_thread;

    //INIT
	pj_init();
	srand(time(NULL));
	pj_caching_pool_init(&cp, NULL, 0);
	pool = pj_pool_create(&cp.factory, "test_pj_stream", 8000, 8000, NULL);

    //CREATE MEDIA ENDPOINT
	status = pjmedia_endpt_create(&cp.factory, NULL, 1, &ep);
	status = pjmedia_codec_register_audio_codecs(ep, NULL);
	pjmedia_codec_mgr_get_codec_info( pjmedia_endpt_get_codec_mgr(ep), 0, &codec_info);

    //CREATE STREAM
    pj_bzero(&remote_addr, sizeof(remote_addr));
    pj_bzero(&remote_addr2, sizeof(remote_addr2));
    pj_bzero(&remote_addr3, sizeof(remote_addr2));
    pj_bzero(&remote_addr4, sizeof(remote_addr2));

    if (1) {
        status = pj_sockaddr_in_init(&remote_addr, &ip, port);
        status = create_stream(pool, ep, codec_info, PJMEDIA_DIR_ENCODING, 4000, &remote_addr, &stream);
        pjmedia_stream_get_port(stream, &stream_port);
        PJ_LOG(1, (__FILE__, "ENCODING 1"));
    }

    if (1) {
        status = pj_sockaddr_in_init(&remote_addr3, &ip, port2);
        status = create_stream(pool, ep, codec_info, PJMEDIA_DIR_ENCODING, 4000, &remote_addr3, &stream3);
        pjmedia_stream_get_port(stream3, &stream_port3);
        PJ_LOG(1, (__FILE__, "ENCODING 2"));
    }

    if (1) {
        status = create_stream(pool, ep, codec_info, PJMEDIA_DIR_DECODING, port, &remote_addr2, &stream2);
        pjmedia_stream_get_port(stream2, &stream_port2);
        status = pjmedia_snd_port_create_player(pool, atoi(argv[2]), 8000, 1, 160, 16, 0, &snd_port2);
        status = pjmedia_snd_port_connect( snd_port2, stream_port2 );
        pjmedia_stream_start(stream2);
        PJ_LOG(1, (__FILE__, "DECODING 1"));
    }

    if (0) {
        status = create_stream(pool, ep, codec_info, PJMEDIA_DIR_DECODING, port2, &remote_addr4, &stream4);
        pjmedia_stream_get_port(stream4, &stream_port4);
        status = pjmedia_snd_port_create_player(pool, atoi(argv[3]), 8000, 1, 160, 16, 0, &snd_port4);
        status = pjmedia_snd_port_connect( snd_port4, stream_port4);
        pjmedia_stream_start(stream4);
        PJ_LOG(1, (__FILE__, "DECODING 2"));
    }

    //INIT PJSUA
    pjsua_config cfg;
    pjsua_logging_config log_cfg;

    status = pjsua_create();
    pjsua_config_default(&cfg);
    cfg.cb.on_incoming_call = NULL;
    cfg.cb.on_call_media_state = NULL;
    cfg.cb.on_call_state = NULL;
    pjsua_logging_config_default(&log_cfg);
    log_cfg.console_level = 4;
    status = pjsua_init(&cfg, &log_cfg, NULL);
	pjmedia_port *conf = pjsua_set_no_snd_dev();

    //INIT FILE PLAYBACK
    char playbackName[256];
	pjsua_player_id playback_id;
    pj_str_t file = pj_str(argv[1]);
    pjsua_player_create(&file, PJMEDIA_FILE_NO_LOOP, &playback_id);
    pjsua_conf_port_id conf_port_playback = pjsua_player_get_conf_port(playback_id);
    pjsua_conf_connect(conf_port_playback, 0);

    //INIT CONF
    pjmedia_conf_create(pool, 3, 16000, 1,
            16000 * 1 * 20 / 1000, // Sample per frame = # of samples in 20 ms
            16, PJMEDIA_CONF_NO_DEVICE, &conf2);
    mp2 = pjmedia_conf_get_master_port(conf2);
    pjmedia_conf_add_port(conf2, pool, stream_port, NULL, &slot1);
    pjmedia_conf_add_port(conf2, pool, stream_port3, NULL, &slot3);
    pjmedia_conf_connect_port(conf2, 0, slot1, 0);
    pjmedia_conf_connect_port(conf2, 0, slot3, 0);

    //CONNECT
    status = pjmedia_master_port_create(pool, mp2, conf, 0, &master_port);
    status = pjmedia_master_port_start(master_port);
    
    pjmedia_stream_start(stream);
	while (1) sleep (2);

	return 0;
}

static pj_status_t create_stream( pj_pool_t *pool, pjmedia_endpt *med_endpt, const pjmedia_codec_info *codec_info, pjmedia_dir dir, pj_uint16_t local_port, const pj_sockaddr_in *rem_addr, pjmedia_stream **p_stream )
{
	pjmedia_stream_info info;
	pjmedia_transport *transport = NULL;
	pj_status_t status;

	/* Reset stream info. */
	pj_bzero(&info, sizeof(info));

	/* Initialize stream info formats */
	info.type = PJMEDIA_TYPE_AUDIO;
	info.dir = dir;
	pj_memcpy(&info.fmt, codec_info, sizeof(pjmedia_codec_info));
	info.tx_pt = codec_info->pt;
	info.rx_pt = codec_info->pt;
	info.ssrc = pj_rand();

	/* Copy remote address */
	pj_memcpy(&info.rem_addr, rem_addr, sizeof(pj_sockaddr_in));

	/* If remote address is not set, set to an arbitrary address
	 * (otherwise stream will assert).
	 */
	if (info.rem_addr.addr.sa_family == 0) {
		const pj_str_t addr = pj_str("127.0.0.1");
		pj_sockaddr_in_init(&info.rem_addr.ipv4, &addr, 0);
	}

	/* Create media transport */
	status = pjmedia_transport_udp_create(med_endpt, NULL, local_port, 0, &transport);
	if (status != PJ_SUCCESS)
		return status;

	/* Now that the stream info is initialized, we can create the 
	 * stream.
	 */

	status = pjmedia_stream_create( med_endpt, pool, &info, transport, NULL, p_stream);

	if (status != PJ_SUCCESS) {
		pjmedia_transport_close(transport);
		return status;
	}
	return PJ_SUCCESS;
}

