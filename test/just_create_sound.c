#include <pjmedia.h>
#include <pjsua-lib/pjsua.h>
#include <pjlib-util.h>
#include <pjlib.h>
#include <stdio.h>
#include <stdlib.h>

#define DEV_NAME "sysdefault:CARD=Device"
#define MAX_SOUND 4

typedef struct app_s app_t;
typedef struct device_s device_t;

struct device_s {
	pjmedia_snd_port *snd_port;
	char aud_dev_name[64];
};

struct app_s {
	pj_caching_pool cp;
	pj_pool_t *pool;
	device_t device[MAX_SOUND];
	pjsip_endpoint *endpt;
	pjmedia_endpt *media_endpt;
};

int get_audio_dev_idx(char *dev) {
	int dev_idx = 0;
	pj_status_t status;
    char errmsg[PJ_ERR_MSG_SIZE];
	pjmedia_aud_dev_refresh();
	status = pjmedia_aud_dev_lookup("alsa", dev, &dev_idx);
	if (status != PJ_SUCCESS) {
        pj_strerror(status, errmsg, sizeof(errmsg));
        PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
		return -1;
	}
	return dev_idx;
}

void init_pjsip(app_t *app) {
    pj_status_t status;

    pj_init();
    pj_caching_pool_init(&app->cp, &pj_pool_factory_default_policy, 0);
    app->pool = pj_pool_create(&app->cp.factory, "siprtp", 4000, 4000, NULL);
	status = pjmedia_endpt_create(&app->cp.factory, NULL, 1, &app->media_endpt);
    pjmedia_codec_register_audio_codecs(app->media_endpt, NULL);
}

int main(int argc, char *argv[]) {
    pj_status_t status;
    char errmsg[PJ_ERR_MSG_SIZE];
    app_t app;

    init_pjsip(&app);

    /* Create sound player port. */
    int idx;
	for (idx=0; idx < MAX_SOUND; idx++) {
		device_t *device = &app.device[idx];
		switch (idx) {
			case 0:
				pj_ansi_sprintf(device->aud_dev_name, "%s_%d", DEV_NAME, 1);
				break;
			case 1:
				pj_ansi_sprintf(device->aud_dev_name, "%s", DEV_NAME);
				break;
			case 2:
				pj_ansi_sprintf(device->aud_dev_name, "%s_%d", DEV_NAME, 3);
				break;
			case 3:
				pj_ansi_sprintf(device->aud_dev_name, "%s_%d", DEV_NAME, 2);
				break;
		}
		PJ_LOG(1, (__FILE__, "===== %s", device->aud_dev_name));
		int dev_idx = get_audio_dev_idx(device->aud_dev_name);
		if (dev_idx >= -1) {
			PJ_LOG(1, (__FILE__, "radio idx: %d", dev_idx));
			status = pjmedia_snd_port_create(app.pool, dev_idx, dev_idx, 8000, 1, 160, 16, 0, &device->snd_port);
			if (status != PJ_SUCCESS) {
				pj_strerror(status, errmsg, sizeof(errmsg));
				PJ_LOG(1, (__FILE__, "Status %d: %s\n", status, errmsg));
			}
		}
	}

    while (1) {
        pj_thread_sleep(2);
    }
    /* Done. */
    return 0;
}
