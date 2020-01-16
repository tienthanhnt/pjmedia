#include <pjlib.h>
#include <pjsua-lib/pjsua.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjmedia/transport_srtp.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

static pj_status_t create_stream( pj_pool_t *pool, pjmedia_endpt *med_endpt, const pjmedia_codec_info *codec_info, pjmedia_dir dir, pj_uint16_t local_port, const pj_sockaddr_in *rem_addr, pjmedia_stream **p_stream );

typedef struct app_s app_t;

struct app_s {
	pj_caching_pool cp;
	pj_pool_t *pool;
	pjmedia_endpt *ep;
	pjmedia_endpt *ep2;
	pj_thread_t *sip_thread;
};


int sip_worker_thread(app_t *app) {
	const pjmedia_codec_info *codec_info;
	pj_status_t status;
	pj_uint16_t local_port;
	pj_uint16_t local_port2;
	pj_sockaddr_in remote_addr;
	pj_sockaddr_in remote_addr2;
	pjmedia_stream *stream = NULL;
	pjmedia_stream *stream2 = NULL;
	pjmedia_port *stream_port = NULL;
	pjmedia_port *stream_port2 = NULL;
	pjmedia_snd_port *snd_port = NULL;
	pjmedia_snd_port *snd_port2 = NULL;

	pj_bzero(&remote_addr, sizeof(remote_addr));
	pj_bzero(&remote_addr2, sizeof(remote_addr2));

	status = pjmedia_endpt_create(&app->cp.factory, NULL, 1, &app->ep2);
	status = pjmedia_codec_register_audio_codecs(app->ep2, NULL);
	pjmedia_codec_mgr_get_codec_info( pjmedia_endpt_get_codec_mgr(app->ep2), 0, &codec_info);
    local_port = 6070;
    local_port2 = 6072;
/*
    status = create_stream(app->pool, app->ep2, codec_info, PJMEDIA_DIR_DECODING, local_port, &remote_addr, &stream);
    status = create_stream(app->pool, app->ep2, codec_info, PJMEDIA_DIR_DECODING, local_port2, &remote_addr2, &stream2);
    pjmedia_stream_get_port(stream, &stream_port);
    pjmedia_stream_get_port(stream2, &stream_port2);
    status = pjmedia_snd_port_create_player(app->pool, 6, 8000, 1, 160, 16, 0, &snd_port);
    status = pjmedia_snd_port_create_player(app->pool, 9, 8000, 1, 160, 16, 0, &snd_port2);
    status = pjmedia_snd_port_connect( snd_port, stream_port );
    status = pjmedia_snd_port_connect( snd_port2, stream_port2 );
    pjmedia_stream_start(stream);
    pjmedia_stream_start(stream2);
*/
}

int main (int argc, char* argv[]) {
    char errmsg[PJ_ERR_MSG_SIZE];
	pj_status_t status;

    app_t app;

	pj_sockaddr_in remote_addr;
	pj_sockaddr_in remote_addr2;
	const pjmedia_codec_info *codec_info;
	pj_str_t ip = pj_str("127.0.0.1");
	pj_uint16_t port = 6070;
	pj_uint16_t port2 = 6072;
	pj_uint16_t local_port=4000;
	pj_uint16_t local_port2=4002;
	pjmedia_stream *stream = NULL;
	pjmedia_stream *stream2 = NULL;
	pjmedia_port *stream_port = NULL;
	pjmedia_port *stream_port2 = NULL;
	pjmedia_port *mp1, *mp2;
	const char *play_file = "vnt.wav";
	pjmedia_port *play_file_port = NULL;
	unsigned slot1, slot2, slot3;
	pjmedia_conf *conf2 = NULL;
	pjmedia_master_port *master_port = NULL;

    //INIT
	pj_init();
	srand(time(NULL));
	pj_caching_pool_init(&app.cp, NULL, 1045);
	app.pool = pj_pool_create(&app.cp.factory, "test_pj_stream", 8000, 8000, NULL);

    //END

    //CREATE NEW THREAD
/*
	pj_thread_create(app.pool,
                    "app",
                    (pj_thread_proc*)&sip_worker_thread,
                    &app,
                    PJ_THREAD_DEFAULT_STACK_SIZE,
                    0,
                    &app.sip_thread);
*/
    //CREATE MEDIA ENDPOINT
    if (1) {
        status = pjmedia_endpt_create(&app.cp.factory, NULL, 1, &app.ep);
        status = pjmedia_codec_register_audio_codecs(app.ep, NULL);
        pjmedia_codec_mgr_get_codec_info( pjmedia_endpt_get_codec_mgr(app.ep), 0, &codec_info);


        status = pj_sockaddr_in_init(&remote_addr, &ip, port);
        status = pj_sockaddr_in_init(&remote_addr2, &ip, port2);
        status = create_stream(app.pool, app.ep, codec_info, PJMEDIA_DIR_ENCODING, local_port, &remote_addr, &stream);
        status = create_stream(app.pool, app.ep, codec_info, PJMEDIA_DIR_ENCODING, local_port2, &remote_addr2, &stream2);
        pjmedia_stream_get_port(stream, &stream_port);
        pjmedia_stream_get_port(stream2, &stream_port2);

        status = pjsua_create();

        pjsua_config cfg;
        pjsua_logging_config log_cfg;

        pjsua_config_default(&cfg);
        cfg.cb.on_incoming_call = NULL;
        cfg.cb.on_call_media_state = NULL;
        cfg.cb.on_call_state = NULL;
        pjsua_logging_config_default(&log_cfg);
        log_cfg.console_level = 4;

        status = pjsua_init(&cfg, &log_cfg, NULL);
        mp1 = pjsua_set_no_snd_dev();

        pjmedia_conf_create(app.pool, 3, 16000, 1,
                16000 * 2 * 10 / 1000, // Sample per frame = # of samples in 20 ms
                16, PJMEDIA_CONF_NO_DEVICE, &conf2);
        //mp1 = pjmedia_conf_get_master_port(conf1);
        mp2 = pjmedia_conf_get_master_port(conf2);


        pjsua_conf_add_port(app.pool, play_file_port, &slot1);
        pjsua_conf_connect(slot1, 0);
        //pjmedia_conf_connect_port(conf1, slot1, 0, 0);
        
        //connect stream port to conf2 and route sound from master port slot to slot2 (of stream connected to conf2)
        pjmedia_conf_add_port(conf2, app.pool, stream_port, NULL, &slot2);
        pjmedia_conf_connect_port(conf2, 0, slot2, 0);
        //add stream port 2 to conf2
        pjmedia_conf_add_port(conf2, app.pool, stream_port2, NULL, &slot3);
        pjmedia_conf_connect_port(conf2, 0, slot3, 0);
        //create master port to provide master clock to flow stream
        pjmedia_audio_format_detail *u_afd, *d_afd;
        u_afd = pjmedia_format_get_audio_format_detail(&mp2->info.fmt, PJ_TRUE);
        d_afd = pjmedia_format_get_audio_format_detail(&mp1->info.fmt, PJ_TRUE);
        PJ_LOG(1, (__FILE__,"MP2 clock :%d\n", u_afd->clock_rate));
        PJ_LOG(1, (__FILE__,"MP1 clock :%d\n", d_afd->clock_rate));
        status = pjmedia_master_port_create(app.pool, mp2, mp1, 0, &master_port);
        if (status != PJ_SUCCESS) {
            pj_strerror(status, errmsg, sizeof(errmsg));
            PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
            return 0;
        }

        status = pjmedia_master_port_start(master_port);
        if (status != PJ_SUCCESS) {
            pj_strerror(status, errmsg, sizeof(errmsg));
            PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
            return 0;
        }

        pjmedia_stream_start(stream);
        pjmedia_stream_start(stream2);
        if (status == PJ_SUCCESS) {
            fprintf(stdout, "--------OK");	
        } else {
            fprintf(stdout, "--------NOK");	
        }
    }

    pj_thread_join(app.sip_thread);
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

