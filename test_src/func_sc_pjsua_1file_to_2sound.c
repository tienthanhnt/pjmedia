#include <pjmedia.h>
#include <pjsua-lib/pjsua.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct app_s app_t;

struct app_s {
    pj_caching_pool cp;
    pjmedia_endpt *med_endpt;
    pj_pool_t *pool;
    pjmedia_port *file_port;
    pjmedia_port *file_port_2;
    pjmedia_port *mp1;
    pjmedia_snd_port *snd_port;
    pjmedia_snd_port *snd_port_2;
	pjmedia_conf *conf;
	unsigned slot;
	unsigned file_slot;
	unsigned sc_slot;
	unsigned rev_slot_1;
	unsigned rev_slot_2;
    pjmedia_master_port *master_port;
    pjmedia_port *sc, *rev_1, *rev_2;
	pjsua_player_id ring_id;
};

void usage(char *app) {
    printf("%s file_name snd_dev_idx1 snd_dev_idx2\n", app);
    exit(1);
}

void init(app_t *app);
void doStream(app_t *app, char *file_name, int idx, int idx2);

int main(int argc, char *argv[])
{
    if (argc < 4) {
        usage(argv[0]);
    }

    app_t app;

    init(&app);

    PJ_LOG(1, (__FILE__, "file_name: %s\n", argv[1]));
    PJ_LOG(1, (__FILE__, "idx1: %d\n", atoi(argv[2])));
    PJ_LOG(1, (__FILE__, "idx2: %d\n", atoi(argv[3])));

    doStream(&app, argv[1], atoi(argv[2]), atoi(argv[3]));

    while(1) {
        pj_thread_sleep(2);
    }
    return 0;
}

void init(app_t *app) {
    pj_status_t status;
    status = pj_init();
    pj_caching_pool_init(&app->cp, &pj_pool_factory_default_policy, 0);
    status = pjmedia_endpt_create(&app->cp.factory, NULL, 1, &app->med_endpt);

    app->pool = pj_pool_create( &app->cp.factory, /* pool factory */
            "wav", /* pool name. */
            4000, /* init size */
            4000, /* increment size */
            NULL /* callback on error */
            );

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

	char ringFile[256];
	pj_ansi_sprintf(ringFile, "%s", "ring.wav");
	pj_str_t ringStr = pj_str(ringFile);
	pjsua_player_create(&ringStr, 0, &app->ring_id);
}

void doStream(app_t *app, char *file_name, int idx1, int idx2) {
    pj_status_t status;

    PJ_LOG(1, (__FILE__, "idx1: %d\n", idx1));
    PJ_LOG(1, (__FILE__, "idx2: %d\n", idx2));
    PJ_LOG(1, (__FILE__, "file_name: %s\n", file_name));
    //GET CONF
	pjmedia_port *pjsua_conf = pjsua_set_no_snd_dev();

    status = pjmedia_wav_player_port_create( app->pool, /* memory pool */
            file_name, /* file to play */
            20, /* ptime. */
            0, /* flags */
            0, /* default buffer */
            &app->file_port/* returned port */
            );
    status = pjmedia_snd_port_create_player(
            app->pool, /* pool */
            idx1, /* use default dev. */
            PJMEDIA_PIA_SRATE(&app->file_port->info),/* clock rate. */
            PJMEDIA_PIA_CCNT(&app->file_port->info),/* # of channels. */
            PJMEDIA_PIA_SPF(&app->file_port->info), /* samples per frame. */
            PJMEDIA_PIA_BITS(&app->file_port->info),/* bits per sample. */
            0, /* options */
            &app->snd_port /* returned port */
            );

    status = pjmedia_snd_port_create_player(
            app->pool, /* pool */
            idx2, /* use default dev. */
            PJMEDIA_PIA_SRATE(&app->file_port->info),/* clock rate. */
            PJMEDIA_PIA_CCNT(&app->file_port->info),/* # of channels. */
            PJMEDIA_PIA_SPF(&app->file_port->info), /* samples per frame. */
            PJMEDIA_PIA_BITS(&app->file_port->info),/* bits per sample. */
            0, /* options */
            &app->snd_port_2 /* returned port */
            );

    pjmedia_conf_create(app->pool, 3, 16000, 1, 320, 16, PJMEDIA_CONF_NO_DEVICE, &app->conf);
    app->mp1 = pjmedia_conf_get_master_port(app->conf);

    // Create stereo-mono splitter/combiner
    status = pjmedia_splitcomb_create(app->pool, 
            PJMEDIA_PIA_SRATE(&app->file_port->info),
            2 /* stereo */,
            2 * PJMEDIA_PIA_SPF(&app->file_port->info),
            PJMEDIA_PIA_BITS(&app->file_port->info),
            0, &app->sc);

    status = pjmedia_splitcomb_create_rev_channel(app->pool, app->sc, 0, 0, &app->rev_1);
    status = pjmedia_splitcomb_create_rev_channel(app->pool, app->sc, 1, 0, &app->rev_2);

    pjmedia_conf_add_port(app->conf, app->pool, app->sc, NULL, &app->sc_slot);
    pjmedia_conf_connect_port(app->conf, 0, app->sc_slot, 0);

    //pjsua_conf_add_port(app->pool, app->file_port, &app->slot);
    //pjsua_conf_connect(app->slot, 0);

    pjsua_conf_port_id ring_port_id = pjsua_player_get_conf_port(app->ring_id);
    pjsua_conf_connect(ring_port_id, 0);
    pjsua_player_set_pos(app->ring_id, 0);

    pjmedia_master_port_create(app->pool, pjsua_conf, app->mp1, 0, &app->master_port);

    pjmedia_snd_port_connect(app->snd_port, app->rev_1);
    pjmedia_snd_port_connect(app->snd_port_2, app->rev_2);

    pjmedia_master_port_start(app->master_port);   
}
