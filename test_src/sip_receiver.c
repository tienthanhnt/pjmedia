#include <pjsua-lib/pjsua.h>
#include <pjlib.h>
#include "sip_receiver.h"

static pj_bool_t on_rx_request( pjsip_rx_data *rdata )
{
    PJ_LOG(1, (__FILE__, "on_rx_request..."));
	app_t *app;
	app = (app_t*)rdata->endpt_info.mod_data[0];
/*
    if (app) {
        PJ_LOG(1, (__FILE__, "Process app\n"));
        pj_mutex_lock(app->mutex);
        pj_str_t reason = pj_str("Unsupported Operation");
        if (rdata->msg_info.msg->line.req.method.id == PJSIP_ACK_METHOD) {
            pj_mutex_unlock(app->mutex);
            return PJ_FALSE;
        }

        if (rdata->msg_info.msg->line.req.method.id != PJSIP_INVITE_METHOD) {
            reason = pj_str("Unsupported Operation");
            pjsip_endpt_respond_stateless(app->endpt, rdata, 500, &reason, NULL, NULL);
            pj_mutex_unlock(app->mutex);
            return PJ_TRUE;
        }
        reason = pj_str("Trying");
        pjsip_endpt_respond_stateless(app->endpt, rdata, 100, &reason, NULL, NULL);
        process_incoming_call(rdata, app);
        pj_mutex_unlock(app->mutex);
    }
    else {
        PJ_LOG(1, (__FILE__, "app NULL\n"));
    }
*/
	return PJ_TRUE;
}

pjsip_module mod_siprtp =
{
	NULL, NULL,			    /* prev, next.		*/
	{ "mod-siprtp", 13 },	    /* Name.			*/
	-1,				    /* Id			*/
	PJSIP_MOD_PRIORITY_APPLICATION, /* Priority			*/
	NULL,			    /* load()			*/
	NULL,			    /* start()			*/
	NULL,			    /* stop()			*/
	NULL,			    /* unload()			*/
	&on_rx_request,		    /* on_rx_request()		*/
	NULL,			    /* on_rx_response()		*/
	NULL,			    /* on_tx_request.		*/
	NULL,			    /* on_tx_response()		*/
	NULL,			    /* on_tsx_state()		*/
};

int sip_worker_thread(app_t *app) {
    static pj_thread_desc s_desc;
    static pj_thread_t *s_thread;
    pj_thread_register("sip_worker_thread", s_desc, &s_thread);
    int sq_timer_counter = 0;
    while (1) {
        pj_time_val timeout = {0, 10};
        pjsip_endpt_handle_events(app->endpt, &timeout);
    }
    return 0;
}

void pjsip_init(app_t *app) {
	pj_init(); 
	pj_caching_pool_init(&app->cp, &pj_pool_factory_default_policy, 0);
	app->pool = pj_pool_create(&app->cp.factory, "siprtp", 4000, 4000, NULL);
	pjsip_endpt_create(&app->cp.factory, "pjsip_siprtp", &app->endpt);
    pj_mutex_create_recursive(app->pool, "app_mutex", &app->mutex);
}

void create_udp_transport(app_t *app) {
    char *local_addr_c = malloc(128*sizeof(char));
    pj_status_t status;
	pj_sockaddr_in addr;
	pjsip_host_port addr_name;
	pjsip_transport *tp;
    pj_sockaddr addr2;
    //

	pj_gethostip(pj_AF_INET(), &addr2);
	strcpy(local_addr_c, pj_inet_ntoa(addr2.ipv4.sin_addr));
	app->local_addr = pj_str(local_addr_c);
    //
	pj_bzero(&addr, sizeof(pj_sockaddr_in));
	addr.sin_family = pj_AF_INET();
	addr.sin_addr.s_addr = 0;
	addr.sin_port = pj_htons(SIP_PORT);

	addr_name.host = app->local_addr;
	addr_name.port = SIP_PORT;
	status = pj_sockaddr_in_init(&addr, &app->local_addr, SIP_PORT);
	if (status != PJ_SUCCESS) {
		PJ_LOG(1, (__FILE__, "Unable to resolve hostname %s", app->local_addr));
	}
	status = pjsip_udp_transport_start(app->endpt, &addr, &addr_name, 1, &tp);
	if (status != PJ_SUCCESS) {
		PJ_LOG(1, (__FILE__, "Unable to create UDP transport"));
	}

}

int main(int argc, char *argv[]) {
    app_t app;
	pj_status_t status;

    //init
    pjsip_init(&app);

	//create UDP transport
    create_udp_transport(&app);

	//initialize invite session module
	status = pjsip_endpt_register_module( app.endpt, &mod_siprtp);
	pjsip_endpt_add_user_data(app.endpt, &app, 0);

    //start worker thread
    pj_thread_create(app.pool, "app", (pj_thread_proc*)&sip_worker_thread, &app, PJ_THREAD_DEFAULT_STACK_SIZE, 0, &app.sip_thread);

    while (1) {
        pj_thread_sleep(1000);
    }
    return 0;
}
