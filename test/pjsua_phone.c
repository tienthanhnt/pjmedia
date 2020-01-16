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

int main(int argc, char *argv[])
{
    pj_caching_pool cp;
    pj_pool_t *pool;
    pj_status_t status;
    char errmsg[PJ_ERR_MSG_SIZE];

    status = pj_init();
    pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 0);

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

	pjsua_start();
    //INIT
	pjsua_transport_config udp_cfg;
	pjsua_transport_config_default(&udp_cfg);
	udp_cfg.port = 6071;

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

    int input;
    while (1)
    {
        printf("0 = register, 1 = un-register\n");
        scanf("%d", &input);
        if (input) {
            pjsua_acc_set_registration(asterisk_acc_id, 1);
        }
        else {
            pjsua_acc_set_registration(asterisk_acc_id, 0);
        }
    }

    return 0;
}
