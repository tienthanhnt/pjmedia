#include "my-pjlib-utils.h"
#include <pjmedia.h>
#include <pjmedia-codec.h>
void list_codecs(pjmedia_endpt *ep) {
    unsigned int count = 100;
    int i;
    char buffer[100];
    pjmedia_codec_mgr *mgr = 0;
    pjmedia_codec_info codec[20];

    mgr = pjmedia_endpt_get_codec_mgr(ep);
    pjmedia_codec_mgr_enum_codecs(mgr, &count, codec, NULL);
    
    PJ_LOG(3, (__FILE__, "THERE ARE %d codecs", count));
    for ( i = 0; i < count; i++ ) {
        pj_bzero(buffer, sizeof(buffer));
        pjmedia_codec_info_to_id(&codec[i], buffer, sizeof(buffer));
        PJ_LOG(3, (__FILE__, "Codec : %s", buffer));
    }
}

int main() {
    pj_caching_pool cp;
    pj_caching_pool_init(&cp, NULL, 4000);
    CHECK(__FILE__, pj_init());
    pjmedia_endpt *endpoint;
    pjmedia_endpt_create(&cp.factory, NULL, 1, &endpoint);
    //pjmedia_codec_register_audio_codecs(endpoint, NULL);
    pjmedia_codec_g711_init(endpoint);
    list_codecs(endpoint);
    pjmedia_codec_g711_deinit();
    list_codecs(endpoint);
    return 0;
}
