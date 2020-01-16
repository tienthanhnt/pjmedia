#include "my-pjlib-utils.h"
#include "my-pjmedia-utils.h"
#include <unistd.h>
#include <stdlib.h>

int main() {
    pj_caching_pool cp;
    pj_pool_t *pool;
    pjmedia_endpt *ep;
    pjmedia_conf *conf;
    pjmedia_port *fp, *mp;
    pjmedia_snd_port *sp;
    pjmedia_snd_port *mic;
    unsigned slot;

    pj_init();
    pj_srand(12345);
    pj_caching_pool_init(&cp, NULL, 1024);
    pool = pj_pool_create(&cp.factory, "pool1", 1024, 1024, NULL);
    pjmedia_endpt_create(&cp.factory, NULL, 1, &ep);
    pjmedia_conf_create(pool, 3, 8000, 1, 
                        8000 * 1 * 20 / 1000, // Sample per frame = # of samples in 20 ms
                        16, PJMEDIA_CONF_NO_DEVICE, &conf);
    mp = pjmedia_conf_get_master_port(conf);
    CHECK(__FILE__, pjmedia_snd_port_create_player(pool, 1, 8000, 1, 160, 16, 0, &sp));
    printf("sp=%p - mp=%p\n", sp, mp);
    pjmedia_snd_port_connect(sp, mp);

    pjmedia_snd_port_create_rec(pool, 0, 8000, 1, 160, 16, 0, &mic);

    pjmedia_wav_player_port_create(pool, "NHP-mono-pcm-s16le.wav", 0, 0, 0, &fp);

    //pjmedia_wav_writer_port_create(pool, "vnt.wav", 8000, 1, 160, 16, 0, 0, &fp);

    fp = pjmedia_snd_port_get_port(mic);
    pjmedia_conf_add_port(conf, pool, fp, NULL, &slot);
    pjmedia_conf_connect_port(conf, slot, 0, 0);
    while (1) sleep(2);
}
