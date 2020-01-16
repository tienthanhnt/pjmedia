#include <pjlib.h>
#include <pjlib-util.h>
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjmedia/transport_srtp.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int main (int argc, char* argv[]) {
	pj_caching_pool cp;
	pj_pool_t *pool;
	pjmedia_stream *stream = NULL;

	pjmedia_master_port *master_port = NULL;
	pj_status_t status;

	pjmedia_conf *conf1 = NULL;
	pjmedia_port *mp1;
	unsigned slot1;
	pjmedia_snd_port *snd_port = NULL;
	pjmedia_port *file_port = NULL;

	pj_caching_pool_init(&cp, NULL, 0);
	pool = pj_pool_create(&cp.factory, "test_pj_stream", 8000, 8000, NULL);

    pjmedia_conf_create(pool, 3, 8000, 1,
            8000 * 1 * 20 / 1000, // Sample per frame = # of samples in 20 ms
            16, PJMEDIA_CONF_NO_DEVICE, &conf1);
    mp1 = pjmedia_conf_get_master_port(conf1);

    status = pjmedia_wav_player_port_create(pool, argv[1], 20, 0, 0, &file_port);

    //connect file_port to conf1 and route sound to slot 0 of master port (conf1)
    pjmedia_conf_add_port(conf1, pool, file_port, NULL, &slot1);
    pjmedia_conf_connect_port(conf1, slot1, 0, 0);

    status = pjmedia_snd_port_create_player(pool, atoi(argv[2]), 
            PJMEDIA_PIA_SRATE(&file_port->info),/* clock rate. */
            PJMEDIA_PIA_CCNT(&file_port->info),/* # of channels. */
            PJMEDIA_PIA_SPF(&file_port->info), /* samples per frame. */
            PJMEDIA_PIA_BITS(&file_port->info),/* bits per sample. */
            0, &snd_port);
    pjmedia_snd_port_connect(snd_port, mp1);

    pj_thread_sleep(100);

	while (1) sleep (2);

	return 0;
}
