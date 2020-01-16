#include <pjlib.h>
#include <pjmedia.h>
#include <stdlib.h>	
#include <stdio.h>
#include "ansi-utils.h"
#include "ics-stream.h"

#define CLOCK_RATE 11025
#define NCHANNELS 2
#define SAMPLES_PER_FRAME (NCHANNELS * (CLOCK_RATE * 10 / 1000))
#define BITS_PER_SAMPLE 16

int main() {
    pj_status_t status;
    pj_uint16_t local_port;
    pj_sockaddr_in remote_addr, remote_addr2;  
    pj_str_t ip, ip2;
    pj_uint16_t port, port2;

    pjmedia_stream_info info;
    const pjmedia_codec_info *codec_info;

    pj_caching_pool cp;
    pj_pool_t *pool, *pool2;
    pjmedia_endpt *endpoint, *endpoint2;
    pjmedia_transport *transport, *transport2;
    pjmedia_stream *stream, *stream2;

    pj_init();
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

    pool = pj_pool_create(&cp.factory, "pool", 4000, 4000, NULL);

    ip = pj_str("127.0.0.1");
    port = 4321;
    status = pj_sockaddr_in_init(&remote_addr, &ip,port);

    pjmedia_endpt_create(&cp.factory, NULL, 1, &endpoint);
    pjmedia_endpt_create(&cp.factory, NULL, 1, &endpoint2);

    /////////////////////
    pjmedia_codec_register_audio_codecs(endpoint, NULL);
    pjmedia_codec_mgr_get_codec_info(pjmedia_endpt_get_codec_mgr(endpoint), 0, &codec_info);
    pj_bzero(&info, sizeof(info));

 //   pjmedia_codec_register_audio_codecs(endpoint2, NULL);
   // pjmedia_codec_mgr_get_codec_info(pjmedia_endpt_get_codec_mgr(endpoint2), 0, &codec_info);
   // pj_bzero(&info, sizeof(info));

    /////////////////////

    info.type = PJMEDIA_TYPE_AUDIO;
    info.dir = PJMEDIA_DIR_ENCODING;

    pj_memcpy(&info.fmt, codec_info, sizeof(pjmedia_codec_info));
    pj_memcpy(&info.rem_addr, &remote_addr, sizeof(pj_sockaddr_in));



    CHECK(__FILE__, pjmedia_transport_udp_create(endpoint, NULL, 1234, 0, &transport));

    status = pjmedia_stream_create(endpoint, pool, &info, transport, NULL, &stream);

    CHECK(__FILE__, pjmedia_transport_udp_create(endpoint, NULL, 12345, 0, &transport2));
    status = pjmedia_stream_create(endpoint, pool, &info, transport2, NULL, &stream2);

    pjmedia_port *file_port, *file_port2;
    pjmedia_snd_port *snd_port, *snd_port2;

      /* Create WAVE file writer port. */
    status = pjmedia_wav_writer_port_create( pool, "a.wav", CLOCK_RATE, NCHANNELS, SAMPLES_PER_FRAME, BITS_PER_SAMPLE, 0, 0, &file_port);
    status = pjmedia_wav_writer_port_create( pool, "b.wav", CLOCK_RATE, NCHANNELS, SAMPLES_PER_FRAME, BITS_PER_SAMPLE, 0, 0, &file_port2);

    /* Create sound player port. */
    status = pjmedia_snd_port_create_rec(pool, -1,
            PJMEDIA_PIA_SRATE(&file_port->info),
            PJMEDIA_PIA_CCNT(&file_port->info),
            PJMEDIA_PIA_SPF(&file_port->info), 
            PJMEDIA_PIA_BITS(&file_port->info),
            0,
            &snd_port);

    status = pjmedia_snd_port_create_rec(pool, -1,
            PJMEDIA_PIA_SRATE(&file_port->info),
            PJMEDIA_PIA_CCNT(&file_port->info),
            PJMEDIA_PIA_SPF(&file_port->info), 
            PJMEDIA_PIA_BITS(&file_port->info),
            0,
            &snd_port2);

    status = pjmedia_snd_port_connect( snd_port, file_port);
    status = pjmedia_snd_port_connect( snd_port2, file_port2);

    while(1) sleep(1);
}

