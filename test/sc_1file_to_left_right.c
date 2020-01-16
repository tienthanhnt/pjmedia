#include <pjmedia.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <stdio.h>
#include <stdlib.h>

void usage(char *app) {
    printf("%s file_name snd_dev\n", app);
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
    pjmedia_snd_port *snd_port;
    pjmedia_snd_port *snd_port_2;
    char tmp[10];
    pj_status_t status;
	pjmedia_conf *conf = NULL;
	unsigned slot1;
	unsigned rev_slot;
	unsigned slot3;
	unsigned file_slot;
	unsigned slot5;

    pjmedia_port *conf_port;

    /* Must init PJLIB first: */
    status = pj_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Must create a pool factory before we can allocate any memory. */
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

    status = pjmedia_endpt_create(&cp.factory, NULL, 1, &med_endpt);

    /* Create memory pool for our file player */
    pool = pj_pool_create( &cp.factory, /* pool factory */
            "wav", /* pool name. */
            4000, /* init size */
            4000, /* increment size */
            NULL /* callback on error */
            );

    pjmedia_conf_create(pool, 10, 8000, 1, 160, 16, PJMEDIA_CONF_NO_DEVICE, &conf);
    conf_port = pjmedia_conf_get_master_port(conf);

    pjmedia_port *sc, *rev_1, *rev_2;
    // Create stereo-mono splitter/combiner
    status = pjmedia_splitcomb_create(pool, 
            PJMEDIA_PIA_SRATE(&conf_port->info),
            2 /* stereo */,
            2 * PJMEDIA_PIA_SPF(&conf_port->info),
            PJMEDIA_PIA_BITS(&conf_port->info),
            0, &sc);
    status = pjmedia_splitcomb_set_channel(sc, 0 /* ch0 */, 0 /*options*/, conf_port);
    status = pjmedia_splitcomb_create_rev_channel(pool, sc, 1  /* ch1 */, 0  /* options */,&rev_1);

    pjmedia_conf_add_port(conf, pool, rev_1, NULL, &rev_slot);

    /* Create sound player port. */
    status = pjmedia_snd_port_create_player(
            pool, /* pool */
            atoi(argv[2]), /* use default dev. */
            PJMEDIA_PIA_SRATE(&conf_port->info),/* clock rate. */
            PJMEDIA_PIA_CCNT(&conf_port->info),/* # of channels. */
            PJMEDIA_PIA_SPF(&conf_port->info), /* samples per frame. */
            PJMEDIA_PIA_BITS(&conf_port->info),/* bits per sample. */
            0, /* options */
            &snd_port /* returned port */
            );

    pjmedia_snd_port_connect(snd_port, sc);

    /* Create file media port from the WAV file */
    status = pjmedia_wav_player_port_create( pool, /* memory pool */
            argv[1], /* file to play */
            20, /* ptime. */
            0, /* flags */
            0, /* default buffer */
            &file_port/* returned port */
            );
    pjmedia_conf_add_port(conf, pool, file_port, NULL, &file_slot);
    pjmedia_conf_connect_port(conf, file_slot, rev_slot, 0);

    pj_thread_sleep(100);

    while(1) {
        sleep(2);
    }

    printf("Playing %s..\n", argv[1]);
    puts("");
    puts("Press <ENTER> to stop playing and quit");

    if (fgets(tmp, sizeof(tmp), stdin) == NULL) {
        puts("EOF while reading stdin, will quit now..");
    }


    /* Start deinitialization: */

    /* Disconnect sound port from file port */
    status = pjmedia_snd_port_disconnect(snd_port);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /* Without this sleep, Windows/DirectSound will repeteadly
     * play the last frame during destroy.
     */
    pj_thread_sleep(100);

	if (status == PJ_SUCCESS) {
		fprintf(stdout, "OK, DESTROY COMPLETED\n");	
		fflush(stdout);
	} else {
		fprintf(stdout, "NOT OK\n");	
		fflush(stdout);
	}
    /* Destroy sound device */
    status = pjmedia_snd_port_destroy( snd_port );
	if (status == PJ_SUCCESS) {
		fprintf(stdout, "OK, DESTROY COMPLETED 2\n");	
		fflush(stdout);
	} else {
		fprintf(stdout, "NOT OK\n");	
		fflush(stdout);
	}
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Destroy file port */
    status = pjmedia_port_destroy( file_port );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /* Release application pool */
    pj_pool_release( pool );

    /* Destroy media endpoint. */
    pjmedia_endpt_destroy( med_endpt );

    /* Destroy pool factory */
    pj_caching_pool_destroy( &cp );

    /* Shutdown PJLIB */
    pj_shutdown();


    /* Done. */
    return 0;
}
