#include <pjlib.h>
#include <pjmedia.h>
#include <stdlib.h>	
#include <stdio.h>
#include "ansi-utils.h"
#include <pjsua-lib/pjsua.h>

void list_all_codecs(pjmedia_endpt *endpoint) {
    unsigned int count = 100;
    int i;
    char buffer[100];
    pjmedia_codec_mgr *mgr = 0;
    pjmedia_codec_info codec[20];
    pj_str_t codec_id = pj_str("");

    mgr = pjmedia_endpt_get_codec_mgr(endpoint);
    PJ_LOG(3, (__FILE__, "--- mgr = %p", mgr));
    pjmedia_codec_mgr_enum_codecs(mgr, &count, codec, NULL);
    
    PJ_LOG(3, (__FILE__, "count = %p", count));
    for ( i = 0; i < count; i++ ) {
        pj_bzero(buffer, sizeof(buffer));
        pjmedia_codec_info_to_id(&codec[i], buffer, sizeof(buffer));
        PJ_LOG(3, (__FILE__, "Codec : %s", buffer));
    }
}

int main() {
    pj_status_t status;
    pj_uint16_t local_port;
    pj_sockaddr_in remote_addr, remote_addr2;  
    pj_str_t ip, ip2;
    pj_uint16_t port, port2;

    pjmedia_stream_info info;
    const pjmedia_codec_info *codec_info;

    pj_caching_pool cp;
    pj_pool_t *pool;
    pjmedia_endpt *endpoint, *endpoint2;
    pjmedia_transport *transport, *transport2;
    pjmedia_stream *stream, *stream2;
    pjsua_config cfg;
    pjsua_media_config media_cfg;
    pjmedia_codec_mgr *mgr;

    pj_init();
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);
    pool = pj_pool_create(&cp.factory, "pool", 4000, 4000, NULL);

#if 1
    CHECK(__FILE__, pjsua_create());

    pjsua_config_default(&cfg);
    pjsua_media_config_default(&media_cfg);

    CHECK(__FILE__, pjsua_init(&cfg, NULL, NULL));
    //pjsua_set_snd_dev(0 , 2);
    endpoint = pjsua_get_pjmedia_endpt();
    /*pool = pjmedia_endpt_create_pool( pjsua_get_pjmedia_endpt(), "Memory Pool", 256, 1024 );
    if (!pool)
        PJ_LOG(3, (__FILE__,"memoryPool failed" ));*/
#endif
    list_all_codecs(endpoint);
    ip = pj_str("127.0.0.1");
    port = 4321;
    status = pj_sockaddr_in_init(&remote_addr, &ip, port);


    mgr = pjmedia_endpt_get_codec_mgr(endpoint);
    PJ_LOG(3, (__FILE__, "mgr = %p", mgr));

    CHECK(__FILE__, pjmedia_codec_mgr_get_codec_info(mgr , PJMEDIA_RTP_PT_PCMU, &codec_info));

    pj_bzero(&info, sizeof(info));

    info.type = PJMEDIA_TYPE_AUDIO;
    info.dir = PJMEDIA_DIR_ENCODING;

    char buffer[100];
    pjmedia_codec_info_to_id(codec_info, buffer, sizeof(buffer));
    PJ_LOG(3, (__FILE__, "codec_info = %s", buffer));

    pj_memcpy(&(info.fmt), codec_info, sizeof(pjmedia_codec_info));
    pj_memcpy(&(info.rem_addr), &remote_addr, sizeof(pj_sockaddr_in));

    pjmedia_transport_udp_create(endpoint, NULL, 1234, 0, &transport);
    PJ_LOG(3, (__FILE__, "bbb"));

    status = pjmedia_stream_create(endpoint , pool, &info, transport, NULL, &stream);

    //////////////////////////////////
    pjmedia_port *stream_port;
    pjmedia_snd_port *snd_port;

    status = pjmedia_stream_get_port(stream, &stream_port);

    status = pjmedia_snd_port_create_rec(pool, -1,
            PJMEDIA_PIA_SRATE(&stream_port->info),
            PJMEDIA_PIA_CCNT(&stream_port->info),
            PJMEDIA_PIA_SPF(&stream_port->info),
            PJMEDIA_PIA_BITS(&stream_port->info),
            0, &snd_port);

    // Connect sound port to stream
    status = pjmedia_snd_port_connect(snd_port, stream_port);

    pjmedia_stream_start(stream);
    while(1) {
        sleep(10);
    }
}
