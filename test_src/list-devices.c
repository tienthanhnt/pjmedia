#include <pjmedia.h>
int main() {
    pj_init();
    pjmedia_aud_dev_index dev_idx;

    pjmedia_endpt *ep;

    pj_caching_pool cp;
    pj_caching_pool_init(&cp, 0, 1024);

    pjmedia_endpt_create(&cp.factory, NULL, 1, &ep);
    int dev_cnt;
    dev_cnt = pjmedia_aud_dev_count();
    printf("Got %d audio devices\n", dev_cnt);

    for (dev_idx=0; dev_idx < dev_cnt; ++dev_idx) {
        pjmedia_aud_dev_info info;
        pjmedia_aud_dev_get_info(dev_idx, &info);
        printf("%d. %s (in=%d, out=%d)\n", dev_idx, info.name, info.input_count, info.output_count);
    }
    return 0;
}
