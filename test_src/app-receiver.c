#include "my-pjlib-utils.h"
#include "my-pjmedia-utils.h"
#include "endpoint.h"

endpoint_t receiver;

int main() {
    char temp[10];
    pj_caching_pool cp;
    pj_pool_t *pool;
    pjmedia_endpt *ep;
    pjmedia_stream *stream;
    pjmedia_codec_info *ci;

    int lport = 4321;
    int lport2 = 5321;
    char *file = "NHP-mono.wav";
    char *mcast = "239.1.0.1";
    char *mcast2 = "239.1.0.2";

    pj_init();

    pj_log_set_level(2);
    pj_caching_pool_init(&cp, NULL, 1024);
    pool = pj_pool_create(&cp.factory, "pool1", 1024, 1024, NULL);
    pjmedia_endpt_create(&cp.factory, NULL, 1, &ep);
    pjmedia_codec_g711_init(ep);

    //receiver_init(&receiver, ep, pool);
    receiver_init(&receiver, ep, pool, 3);
    receiver_config_dev_sink(&receiver, 2);
    receiver_config_stream(&receiver, mcast, lport, 0);
    receiver_config_stream(&receiver, mcast2, lport2, 1);
    receiver_start(&receiver);

    int volume;
    pjmedia_aud_stream *aud_stream;
    unsigned vol;
    int idx;

    while(1) {
        fprintf(stdout, "s=Stop - r=Resume:\n");
        fflush(stdout);
        if (fgets(temp, sizeof(temp), stdin) == NULL)
            exit(-1);
        fprintf(stdout, "%s\n",temp);
        switch(temp[0]) {
        case 's':
        case 'S':
            receiver_stop(&receiver, 0);
            break;
        case 'r':
        case 'R':
            receiver_start(&receiver);
            break;
        case '+':
            lport++;
            receiver_stop(&receiver, 0);
            receiver_config_stream(&receiver, mcast, lport, 0);
            receiver_start(&receiver);
            break;
        case '-':
            lport--;
            receiver_stop(&receiver, 0);
            receiver_config_stream(&receiver, mcast, lport, 0);
            receiver_start(&receiver);
            break;
        case 'v':
            receiver_update_stats(&receiver);
            fprintf(stdout, "rtt:%d - delay:%d - pkt:%d - lost: %d - discard:%d\n",
                            receiver.streams[0].delay.mean_rtt_us, receiver.streams[0].delay.mean_delay_ms,
                            receiver.streams[0].drop.pkt, receiver.streams[0].drop.lost, receiver.streams[0].drop.discard);
            break;
        case 'm':
            vol = atoi(&temp[4]);
            vol = (vol * 256)/100 - 128;
            idx = atoi(&temp[2]);

            printf("vol = %d, idx = %d\n", vol, idx);           
            receiver_adjust_volume(&receiver, idx, vol);
            break;
        case 'd':
            receiver_dump_streams(&receiver);
            break;
        }
        pj_thread_sleep(5*100);
    }

    return 0;
}
