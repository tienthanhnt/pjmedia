#include <pjsua-lib/pjsua.h>
#include <pjlib.h>

int main(int argc, char *argv[]) {
    pjsua_acc_id local_acc_id;
    pj_status_t status;
    pj_caching_pool cp;
    pjmedia_endpt *med_endpt;
    char errmsg[PJ_ERR_MSG_SIZE];

    //INIT PJSUA
    pj_init();
    pj_caching_pool_init(&cp, NULL, 1024);
    pjmedia_endpt_create(&cp.factory, NULL, 1, &med_endpt);

    pjsua_config cfg;
    pjsua_logging_config log_cfg;
    pjsua_call_id *call_id;
    pjsua_msg_data msg_data;
	pjsip_generic_string_hdr ed137_hdr;
	pjsip_generic_string_hdr ed137_prio_hdr;
	pjsip_generic_string_hdr ed137_subj_hdr;
	char tmp[256];

    status = pjsua_create();

    pjsua_config_default(&cfg);
    pjsua_logging_config_default(&log_cfg);

    status = pjsua_init(&cfg, &log_cfg, NULL);

    pj_pool_t *pool = pjsua_pool_create(NULL, 4000, 4000);

	pjsua_transport_config udp_cfg;
	pjsua_transport_config_default(&udp_cfg);
	udp_cfg.port = 6070;

	pjsua_transport_id transport_id;
	pjsua_transport_create(PJSIP_TRANSPORT_UDP, &udp_cfg, &transport_id);
	pjsua_acc_add_local(transport_id, PJ_TRUE, &local_acc_id);

    pjsua_start();

	/* Add ED137B header */
	pjsua_msg_data_init(&msg_data);
	pj_str_t hdr_hname = pj_str((char*)"WG67-Version");
	pj_str_t hdr_hvalue = pj_str((char*)"radio.01");;
	pjsip_generic_string_hdr_init2(&ed137_hdr, &hdr_hname, &hdr_hvalue);

	pj_str_t prio_hname = pj_str((char*)"Priority");
	pj_ansi_sprintf(tmp, "%s", "emergency");
	pj_str_t prio_hvalue = pj_str(tmp);
	pjsip_generic_string_hdr_init2(&ed137_prio_hdr, &prio_hname, &prio_hvalue);

	pj_str_t subj_hname = pj_str((char*)"Subject");
	pj_str_t subj_hvalue = pj_str((char*)"radio");
	pjsip_generic_string_hdr_init2(&ed137_subj_hdr, &subj_hname, &subj_hvalue);

	pj_list_push_back(&msg_data.hdr_list, &ed137_hdr);
	pj_list_push_back(&msg_data.hdr_list, &ed137_prio_hdr);
	pj_list_push_back(&msg_data.hdr_list, &ed137_subj_hdr);

    char uri[100];
    pj_ansi_sprintf(uri, "%s", "<sip:192.168.2.51:6050>");
    pj_str_t uri_str = pj_str(uri);

    status = pjsua_call_make_call(local_acc_id, &uri_str, 0, NULL, &msg_data, call_id);
    if (status != PJ_SUCCESS) {
        pj_strerror(status, errmsg, sizeof(errmsg));
        PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
        return 1;
    } else {
        PJ_LOG(1, (__FILE__, "making call %d", status));
    }
    
    while(1) {
        pj_thread_sleep(1000);
    }

    return 0;
}
