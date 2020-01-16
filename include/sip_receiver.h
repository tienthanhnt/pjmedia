#ifndef __SIP_RECEIVER__
#define __SIP_RECEIVER__

#define SIP_PORT 6050
#define MAX_RADIO 4
#define ERR_MAX_CONNECTION -1
#define ERR_SIP_USER_NOT_FOUND -2
#define ERR_SUBJECT_FIELD_NOT_ACCEPTED -3

typedef struct app_s {
    pj_caching_pool cp;
    pj_pool_t *pool;
    pjsip_endpoint *endpt;
    pj_str_t local_addr;
    pj_thread_t *sip_thread;
    pj_mutex_t *mutex;
} app_t;

#endif
