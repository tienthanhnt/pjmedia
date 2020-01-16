#include "my-pjlib-utils.h"
#include "my-pjmedia-utils.h"
#include "endpoint.h"
#include "ansi-utils.h"

endpoint_t receiver;

int usage(char *app) {
    SHOW_LOG(1, "%s <device_idx> <host> <port>\n", app);  
    exit(-1);
}

int main(int argc, char *argv[]) {
    char temp[10];
    pj_caching_pool cp;
    pj_pool_t *pool;
    pjmedia_endpt *ep;
    pjmedia_stream *stream;
    pjmedia_codec_info *ci;

    if (argc < 4) {
        usage(argv[0]);
    }

    int lport = atoi(argv[3]);
    char *mcast = argv[2];

    int dev_idx;

    pj_init();

    pj_log_set_level(2);
    pj_caching_pool_init(&cp, NULL, 1024);
    pool = pj_pool_create(&cp.factory, "pool1", 1024, 1024, NULL);
    pjmedia_endpt_create(&cp.factory, NULL, 1, &ep);
    pjmedia_codec_g711_init(ep);

    receiver_init(&receiver, ep, pool, 2);
    receiver_config_stream(&receiver, mcast, lport, 0);

    dev_idx =atoi(argv[1]);
    fprintf(stdout, "=========== Using device idx:%d ===========\n",dev_idx);

    receiver_config_dev_sink(&receiver, dev_idx);
    receiver_start(&receiver);

    while(1) {
        pj_thread_sleep(5*100);
    }

    return 0;
}
