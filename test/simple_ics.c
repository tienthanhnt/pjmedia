#include <pjsua-lib/pjsua.h>
#include <pjlib.h>
#include "processing-port.h"

int main(int argc, char *argv[]) {
	pj_pool_t *pool;
    pj_status_t status;
    pjmedia_port *sc;
    pjmedia_port* rev;
    char errmsg[PJ_ERR_MSG_SIZE];
    int slot;

    if (argc < 2) {
        printf("./simple_ics <file_name> <device_id>\n");
        return 0;
    }

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
	pjmedia_port *conf = pjsua_set_no_snd_dev();

    //INIT FILE PLAYBACK
    char playbackName[256];
	pjsua_player_id playback_id;
    pj_str_t file = pj_str(argv[1]);

    pjsua_player_create(&file, PJMEDIA_FILE_NO_LOOP, &playback_id);
    if (status != PJ_SUCCESS) {
        pj_strerror(status, errmsg, sizeof(errmsg));
        PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
        return 0;
    }
    else {
        PJ_LOG(1, (__FILE__, "pjsua_player_create...OK"));
    }

    //CREATE SOUND PORT
    pjmedia_snd_port *snd_port;
	status = pjmedia_snd_port_create(pool, atoi(argv[2]), atoi(argv[2]), 
        PJMEDIA_PIA_SRATE(&conf->info),
        2 /* stereo */,
        2 * PJMEDIA_PIA_SPF(&conf->info),
        PJMEDIA_PIA_BITS(&conf->info),
        0, &snd_port);
    if (status != PJ_SUCCESS) {
        pj_strerror(status, errmsg, sizeof(errmsg));
        PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
        return 0;
    }
    else {
        PJ_LOG(1, (__FILE__, "pjmedia_snd_port_create...OK"));
    }

    // Create stereo-mono splitter/combiner
    status = pjmedia_splitcomb_create(pool, 
            PJMEDIA_PIA_SRATE(&conf->info),
            2 /* stereo */,
            2 * PJMEDIA_PIA_SPF(&conf->info),
            PJMEDIA_PIA_BITS(&conf->info),
            0, &sc);
    if (status != PJ_SUCCESS) {
        pj_strerror(status, errmsg, sizeof(errmsg));
        PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
        return 0;
    }
    else {
        PJ_LOG(1, (__FILE__, "pjmedia_splitcomb_create OK"));
    }

    /* Connect channel0 (left channel?) to conference port slot0 */
    status = pjmedia_splitcomb_set_channel(sc, 1/* ch0 */,0 /*options*/, conf);
    if (status != PJ_SUCCESS) {
        pj_strerror(status, errmsg, sizeof(errmsg));
        PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
        return 0;
    }
    else {
        PJ_LOG(1, (__FILE__, "pjmedia_splitcomb_set_channel OK"));
    }

    /* Create reverse channel for channel1 (right channel?)... */
    status = pjmedia_splitcomb_create_rev_channel(pool, sc, 0, 0, &rev);
    if (status != PJ_SUCCESS) {
        pj_strerror(status, errmsg, sizeof(errmsg));
        PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
        return 0;
    }
    else {
        PJ_LOG(1, (__FILE__, "pjmedia_splitcomb_create_rev_channel...OK"));
    }

    pjsua_conf_add_port(pool, rev, &slot);

    pjsua_conf_port_id conf_port_playback = pjsua_player_get_conf_port(playback_id);
    pjsua_conf_connect(conf_port_playback, slot);
	pjmedia_snd_port_connect(snd_port, sc);
 
    pj_thread_sleep(100);

    printf("Playing...\n");
    puts("");
    puts("Press <ENTER> to stop playing and quit");

    char tmp[10];
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
    /* Done. */
    return 0;
}
