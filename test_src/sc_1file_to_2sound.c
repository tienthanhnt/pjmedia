#include <pjmedia.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <stdio.h>
#include <stdlib.h>

void usage(char *app) {
    printf("%s file_name snd_dev_idx1 snd_dev_idx2\n", app);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
        usage(argv[0]);

    pj_caching_pool cp;
    pjmedia_endpt *med_endpt;
    pj_pool_t *pool;
    pjmedia_port *file_port;
    pjmedia_port *file_port_2;
    pjmedia_port *mp1;
    pjmedia_snd_port *snd_port;
    pjmedia_snd_port *snd_port_2;
    char tmp[10];
    pj_status_t status;
	pjmedia_conf *conf = NULL;
	unsigned file_slot;
	unsigned sc_slot;
	unsigned rev_slot_1;
	unsigned rev_slot_2;
    pjmedia_master_port *master_port;

    status = pj_init();
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);
    status = pjmedia_endpt_create(&cp.factory, NULL, 1, &med_endpt);

    pool = pj_pool_create( &cp.factory, /* pool factory */
            "wav", /* pool name. */
            4000, /* init size */
            4000, /* increment size */
            NULL /* callback on error */
            );

    status = pjmedia_wav_player_port_create( pool, /* memory pool */
            argv[1], /* file to play */
            20, /* ptime. */
            0, /* flags */
            0, /* default buffer */
            &file_port/* returned port */
            );

    status = pjmedia_snd_port_create_player(
            pool, /* pool */
            atoi(argv[2]), /* use default dev. */
            PJMEDIA_PIA_SRATE(&file_port->info),/* clock rate. */
            PJMEDIA_PIA_CCNT(&file_port->info),/* # of channels. */
            PJMEDIA_PIA_SPF(&file_port->info), /* samples per frame. */
            PJMEDIA_PIA_BITS(&file_port->info),/* bits per sample. */
            0, /* options */
            &snd_port /* returned port */
            );

    status = pjmedia_snd_port_create_player(
            pool, /* pool */
            atoi(argv[3]), /* use default dev. */
            PJMEDIA_PIA_SRATE(&file_port->info),/* clock rate. */
            PJMEDIA_PIA_CCNT(&file_port->info),/* # of channels. */
            PJMEDIA_PIA_SPF(&file_port->info), /* samples per frame. */
            PJMEDIA_PIA_BITS(&file_port->info),/* bits per sample. */
            0, /* options */
            &snd_port_2 /* returned port */
            );

    pjmedia_conf_create(pool, 10, 8000, 1, 160, 16, PJMEDIA_CONF_NO_DEVICE, &conf);
    mp1 = pjmedia_conf_get_master_port(conf);

    pjmedia_port *sc, *rev_1, *rev_2;
    // Create stereo-mono splitter/combiner
    status = pjmedia_splitcomb_create(pool, 
            PJMEDIA_PIA_SRATE(&file_port->info),
            2 /* stereo */,
            2 * PJMEDIA_PIA_SPF(&file_port->info),
            PJMEDIA_PIA_BITS(&file_port->info),
            0, &sc);

    status = pjmedia_splitcomb_create_rev_channel(pool, sc, 0, 0, &rev_1);
    status = pjmedia_splitcomb_create_rev_channel(pool, sc, 1, 0, &rev_2);

    pjmedia_conf_add_port(conf, pool, sc, NULL, &sc_slot);
    pjmedia_conf_connect_port(conf, 0, sc_slot, 0);

    pjmedia_master_port_create(pool, file_port, mp1, 0, &master_port);

    pjmedia_snd_port_connect(snd_port, rev_1);
    pjmedia_snd_port_connect(snd_port_2, rev_2);

    pjmedia_master_port_start(master_port);

    while(1) {
        pj_thread_sleep(2);
    }
    return 0;
}
