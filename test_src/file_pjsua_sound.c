#include <pjmedia.h>
#include "processing-port.h"
#include <pjsua-lib/pjsua.h>
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
	unsigned slot;
	unsigned file_slot;
	unsigned sc_slot;
	unsigned rev_slot_1;
	unsigned rev_slot_2;
    pjmedia_master_port *master_port;

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
    pool = pjsua_pool_create("pool", 512, 512);

    //GET CONF
	pjmedia_port *pjsua_conf = pjsua_set_no_snd_dev();

	pjsua_player_id playback_id;
    pj_str_t file = pj_str(argv[1]);

    pjsua_player_create(&file, PJMEDIA_FILE_NO_LOOP, &playback_id);

    status = pjmedia_snd_port_create(pool, atoi(argv[2]), atoi(argv[2]), 
        8000, 1, 160, 16,
        0, &snd_port);

    pjsua_conf_port_id conf_port_playback = pjsua_player_get_conf_port(playback_id);
    pjsua_conf_connect(conf_port_playback, 0);

	processing_port_t proc_port;
	processing_port_init(&proc_port, 8000, 1, NULL, NULL);
	processing_port_set_putthrought_frame(&proc_port);
	processing_port_connect_downstream(&proc_port, pjsua_conf);
	processing_port_connect_upstream(&proc_port, pjsua_conf);
	pjmedia_snd_port_connect(snd_port,(pjmedia_port *)&proc_port);
	//pjmedia_snd_port_connect(snd_port_1, conf_port);
	processing_port_turn_agc(&proc_port, 1);

	//pjmedia_snd_port_connect(snd_port, pjsua_conf);

    while(1) {
        pj_thread_sleep(2);
    }
    return 0;
}
