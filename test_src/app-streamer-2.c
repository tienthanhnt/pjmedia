#include "my-pjlib-utils.h"
#include "my-pjmedia-utils.h"
#include "endpoint.h"

endpoint_t streamer;

void usage(char *app) {
    printf("%s <lport> <file_name>\n", app);
    exit(-1);
}

int main(int argc, char *argv[]) {
    char temp[10];
    pj_caching_pool cp;
    pj_pool_t *pool;
    pjmedia_endpt *ep;
    pjmedia_stream *stream;
    pjmedia_codec_info *ci;

    int rport = 5321;
    char *rhost = "239.1.0.2";
    //char *rhost = "192.168.2.50";

    if (argc < 3) {
        usage(argv[0]);
    }

#if 1
    int lport;

    lport = atoi(argv[1]);
    char *file = argv[2];
#endif

#if 0
    int lport = 2345;
#endif
  
    pj_init();

    pj_log_set_level(5);
    pj_caching_pool_init(&cp, NULL, 1024);
    streamer.pool = pj_pool_create(&cp.factory, "pool1", 1024, 1024, NULL);
    pjmedia_endpt_create(&cp.factory, NULL, 1, &streamer.ep);
    pjmedia_codec_g711_init(streamer.ep);

    streamer_init(&streamer, streamer.ep, streamer.pool);
    streamer_config_stream(&streamer, lport, rhost, rport);
    streamer_config_file_source(&streamer, file);
    //streamer_config_dev_source(&streamer, 2);
    streamer_start(&streamer);
    fprintf(stdout, "Local port: %d, File: %s\n", lport, file);
    
   
    while(1) {
        fprintf(stdout, "s=Stop - r=Resume: ");
        fflush(stdout);
        if (fgets(temp, sizeof(temp), stdin) == NULL)
            exit(-1);
        fprintf(stdout, "%s\n",temp);
        switch(temp[0]) {
        case 's':
        case 'S':
            streamer_stop(&streamer);
            break;
        case 'r':
        case 'R':
            streamer_start(&streamer);
            break;
        case '+':
            rport++;
            streamer_stop(&streamer);
            streamer_config_stream(&streamer, lport, rhost, rport);
            streamer_start(&streamer);
            break;
        case '-':
            rport--;
            streamer_stop(&streamer);
            streamer_config_stream(&streamer, lport, rhost, rport);
            streamer_start(&streamer);
            break;
        case 'v':
            streamer_update_stats(&streamer);
            fprintf(stdout, "rtt:%d - delay:%d - pkt:%d - lost: %d - discard:%d\n",
                            streamer.streams[0].delay.mean_rtt_us, streamer.streams[0].delay.mean_delay_ms,
                            streamer.streams[0].drop.pkt, streamer.streams[0].drop.lost, streamer.streams[0].drop.discard);
            break;
        }
        pj_thread_sleep(5*100);
    }

    return 0;
}
