#include <pjlib.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjmedia/transport_srtp.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
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

int main (int argc, char* argv[]) {
	pj_caching_pool cp;
	pjmedia_endpt *ep;
	pj_pool_t *pool;
	pjmedia_stream *stream = NULL;
	pjmedia_stream *stream2 = NULL;

	pjmedia_master_port *master_port = NULL;
	pj_sockaddr_in remote_addr;
	pj_sockaddr_in remote_addr2;
	pj_uint16_t local_port=4000;
	pj_uint16_t local_port2=4002;
	pjmedia_port *stream_port = NULL;
	pjmedia_port *stream_port2 = NULL;
	pj_status_t status;
	pj_str_t ip = pj_str("127.0.0.1");
	pj_uint16_t port = 6070;
	pj_uint16_t port2 = 6072;

	pjmedia_conf *conf1 = NULL;
	pjmedia_conf *conf2 = NULL;
	pjmedia_port *mp1, *mp2;
	unsigned slot1, slot2, slot3;

	pj_bzero(&remote_addr, sizeof(remote_addr));
	pj_bzero(&remote_addr2, sizeof(remote_addr2));
	int flag = 1;
	const pjmedia_codec_info *codec_info;
	const char *play_file = "vnt.wav";
	pjmedia_port *play_file_port = NULL;
	pjmedia_snd_port *snd_port = NULL;
	pjmedia_snd_port *snd_port2 = NULL;
	if (strcmp(argv[1], "0") == 0) {
		flag = 0;	
	} else {
		flag = 1;	
	}
	setbuf(stdout, NULL);
	pj_init();
	srand(time(NULL));
	pj_caching_pool_init(&cp, NULL, 0);
	status = pjmedia_endpt_create(&cp.factory, NULL, 1, &ep);
	pool = pj_pool_create(&cp.factory, "test_pj_stream", 8000, 8000, NULL);
	status = pjmedia_codec_register_audio_codecs(ep, NULL);
	pjmedia_codec_mgr_get_codec_info( pjmedia_endpt_get_codec_mgr(ep), 0, &codec_info);
	//flag , 0 - send, 1 - recv
	if ( flag == 0) {
		pjmedia_conf_create(pool, 3, 8000, 1,
				8000 * 1 * 20 / 1000, // Sample per frame = # of samples in 20 ms
				16, PJMEDIA_CONF_NO_DEVICE, &conf1);
		pjmedia_conf_create(pool, 3, 8000, 1,
				8000 * 1 * 20 / 1000, // Sample per frame = # of samples in 20 ms
				16, PJMEDIA_CONF_NO_DEVICE, &conf2);
		mp1 = pjmedia_conf_get_master_port(conf1);
		mp2 = pjmedia_conf_get_master_port(conf2);

		status = pj_sockaddr_in_init(&remote_addr, &ip, port);
		status = pj_sockaddr_in_init(&remote_addr2, &ip, port2);
		status = create_stream(pool, ep, codec_info, PJMEDIA_DIR_ENCODING, local_port, &remote_addr, &stream);
		status = create_stream(pool, ep, codec_info, PJMEDIA_DIR_ENCODING, local_port2, &remote_addr2, &stream2);
		pjmedia_stream_get_port(stream, &stream_port);
		pjmedia_stream_get_port(stream2, &stream_port2);
		status = pjmedia_wav_player_port_create(pool, play_file, 0, 0, 0, &play_file_port);

		//connect play_file_port to conf1 and route sound to slot 0 of master port (conf1)
		pjmedia_conf_add_port(conf1, pool, play_file_port, NULL, &slot1);
		pjmedia_conf_connect_port(conf1, slot1, 0, 0);
		//connect stream port to conf2 and route sound from master port slot to slot2 (of stream connected to conf2)
		pjmedia_conf_add_port(conf2, pool, stream_port, NULL, &slot2);
		pjmedia_conf_connect_port(conf2, 0, slot2, 0);
		//add stream port 2 to conf2
		pjmedia_conf_add_port(conf2, pool, stream_port2, NULL, &slot3);
		pjmedia_conf_connect_port(conf2, 0, slot3, 0);
		//create master port to provide master clock to flow stream
		status = pjmedia_master_port_create(pool, mp2, mp1, 0, &master_port);
		status = pjmedia_master_port_start(master_port);
		pjmedia_stream_start(stream);
		pjmedia_stream_start(stream2);
		if (status == PJ_SUCCESS) {
			fprintf(stdout, "--------OK");	
		} else {
			fprintf(stdout, "--------NOK");	
		}
	} else {
		local_port = 6070;
		local_port2 = 6072;
		status = create_stream(pool, ep, codec_info, PJMEDIA_DIR_DECODING, local_port, &remote_addr, &stream);
		status = create_stream(pool, ep, codec_info, PJMEDIA_DIR_DECODING, local_port2, &remote_addr2, &stream2);
		pjmedia_stream_get_port(stream, &stream_port);
		pjmedia_stream_get_port(stream2, &stream_port2);
		status = pjmedia_snd_port_create_player(pool, 5, 8000, 1, 160, 16, 0, &snd_port);
		status = pjmedia_snd_port_create_player(pool, 12, 8000, 1, 160, 16, 0, &snd_port2);
		status = pjmedia_snd_port_connect( snd_port, stream_port );
		status = pjmedia_snd_port_connect( snd_port2, stream_port2 );
		pjmedia_stream_start(stream);
		pjmedia_stream_start(stream2);
	}
	while (1) sleep (2);
	return 0;
}
