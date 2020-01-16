#include <pjmedia.h>
#include <pjsua-lib/pjsua.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <stdio.h>
#include <stdlib.h>

void on_call_media_state (pjsua_call_id call_id) {
	pjsua_call_info info;
	pjsua_call_get_info(call_id, &info);

}


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
    char errmsg[PJ_ERR_MSG_SIZE];

    status = pj_init();
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);
    status = pjmedia_endpt_create(&cp.factory, NULL, 1, &med_endpt);

    pool = pj_pool_create( &cp.factory, /* pool factory */
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
    log_cfg.console_level = 5;

    status = pjsua_init(&cfg, &log_cfg, NULL);

    //GET CONF
	pjmedia_port *pjsua_conf = pjsua_set_no_snd_dev();

	pjsua_start();
    //INIT
	pjsua_transport_config udp_cfg;
	pjsua_transport_config_default(&udp_cfg);
	udp_cfg.port = 6070;

	pjsua_transport_id transport_id;
	pjsua_transport_create(PJSIP_TRANSPORT_UDP, &udp_cfg, &transport_id);

	pjsua_acc_config acfg;
	pjsua_acc_config_default(&acfg);
    pjsua_acc_id asterisk_acc_id;

	acfg.reg_timeout = 5;
	acfg.reg_retry_interval = 5;
	acfg.reg_retry_random_interval = 3;

	char id[256], reg_uri[256], username[256], password[256];
	pj_ansi_sprintf(username, "%s", "111");
	pj_ansi_sprintf(password, "%s", "123456");
	pj_ansi_sprintf(id, "<sip:%s@%s>", username, "192.168.2.28");
	pj_ansi_sprintf(reg_uri, "<sip:%s:%d>", "192.168.2.28", 5060);

	acfg.id = pj_str(id);
	acfg.reg_uri = pj_str(reg_uri);
	acfg.cred_count = 1;
	acfg.cred_info[0].realm = pj_str((char*)"*");
	acfg.cred_info[0].scheme = pj_str((char*)"digest");
	acfg.cred_info[0].username = pj_str(username);
	acfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
	acfg.cred_info[0].data = pj_str(password);

	status = pjsua_acc_add(&acfg, PJ_FALSE, &asterisk_acc_id);
    if (status != PJ_SUCCESS) {
        pj_strerror(status, errmsg, sizeof(errmsg));
        PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
        return 0;
    }
    else {
        PJ_LOG(1, (__FILE__, "pjsua_player_create...OK"));
    }

    PJ_LOG(1, (__FILE__, "Make call now"));
    pjsua_call_id call_id = PJSUA_INVALID_ID;
    char phone_c[100];
    pjsua_msg_data msg_data;
    pj_ansi_sprintf(phone_c, "<sip:%s@%s:%d>", "102", "192.168.2.28", 5060);
    pj_str_t dst_uri = pj_str(phone_c);

    pjsua_msg_data_init(&msg_data);
    pjsua_call_make_call(asterisk_acc_id, &dst_uri, 0, NULL, &msg_data, &call_id);

    PJ_LOG(1, (__FILE__, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ: %d", call_id));
    PJ_LOG(1, (__FILE__, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ: %d", pjsua_call_is_active(call_id)));
    pjsua_conf_port_id source_slot = pjsua_call_get_conf_port(call_id);
    PJ_LOG(1, (__FILE__, "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ: %d", source_slot));
    pjsua_conf_connect(source_slot, 0);

    //CREATE SND PORT
    status = pjmedia_snd_port_create_player(pool, atoi(argv[1]), 8000, 1, 160, 16, 0, &snd_port);
    status = pjmedia_snd_port_create_player(pool, atoi(argv[2]), 8000, 1, 160, 16, 0, &snd_port_2);
    
    pjmedia_conf_create(pool, 3, 16000, 1, 320, 16, PJMEDIA_CONF_NO_DEVICE, &conf);
    mp1 = pjmedia_conf_get_master_port(conf);

    pjmedia_port *sc, *rev_1, *rev_2;
    // Create stereo-mono splitter/combiner
    status = pjmedia_splitcomb_create(pool, 8000, 2, 320, 16, 0 ,&sc);

    status = pjmedia_splitcomb_create_rev_channel(pool, sc, 0, 0, &rev_1);
    status = pjmedia_splitcomb_create_rev_channel(pool, sc, 1, 0, &rev_2);

    pjmedia_conf_add_port(conf, pool, sc, NULL, &sc_slot);
    pjmedia_conf_connect_port(conf, 0, sc_slot, 0);

    pjmedia_master_port_create(pool, pjsua_conf, mp1, 0, &master_port);

    pjmedia_snd_port_connect(snd_port, rev_1);
    pjmedia_snd_port_connect(snd_port_2, rev_2);

    pjmedia_master_port_start(master_port);

    while(1) {
        pj_thread_sleep(2);
    }
    return 0;
}
