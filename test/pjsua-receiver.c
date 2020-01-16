#include "node.h"
#include "my-pjlib-utils.h"
#include "my-pjmedia-utils.h"
#include "ansi-utils.h"
#include "endpoint.h"
#include "ics.h"

typedef struct {
    ics_t ics;
    node_t node;

    endpoint_t streamer;
    endpoint_t receiver;
} app_data_t;

void usage(char *app) {
    SHOW_LOG(1, "%s <device_idx>\n", app);
    exit(-1);
}

int main(int argc, char *argv[]) {
    pjmedia_endpt *ep;
    app_data_t app_data;
    int dev_idx;

    if (argc < 2) {
        usage(argv[0]);
    }

    int rport = 4321;
    char *rhost = "239.1.0.1";
    //char *rhost = "192.168.2.50";

    int lport = 2345;

    dev_idx = atoi(argv[1]);

	ics_pool_init(&app_data.ics);
	ics_pjsua_init(&app_data.ics); 
	ics_init(&app_data.ics);

	node_media_config(&app_data.node, &app_data.streamer, &app_data.receiver);
	app_data.node.streamer->pool = app_data.node.receiver->pool = app_data.ics.pool;
    app_data.node.streamer->ep = app_data.node.receiver->ep = pjsua_get_pjmedia_endpt();
    //pjmedia_endpt_create(&app_data.ics.cp.factory, NULL, 1, &app_data.node.streamer->ep);
    //app_data.node.receiver->ep = app_data.node.streamer->ep;

	pjmedia_codec_g711_init(app_data.node.streamer->ep);
	pjmedia_codec_g711_init(app_data.node.receiver->ep);

	streamer_init(app_data.node.streamer, app_data.node.streamer->ep, app_data.node.streamer->pool);
	receiver_init(app_data.node.receiver, app_data.node.receiver->ep, app_data.node.receiver->pool, 2);

    //streamer_config_stream(app_data.node.streamer, lport, rhost, rport);
    //receiver_config_stream(app_data.node.receiver, rhost, lport, 0);

	streamer_config_dev_source(app_data.node.streamer, dev_idx);
	receiver_config_dev_sink(app_data.node.receiver, dev_idx);

    //streamer_start(app_data.node.streamer);
    //receiver_start(app_data.node.receiver);

    while(1) {
        pj_thread_sleep(5*100);
    }

    return 0;
}
