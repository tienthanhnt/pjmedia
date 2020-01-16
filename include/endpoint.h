#ifndef __ENDPOINT_H__
#define __ENDPOINT_H__
#include <pjmedia.h>
#include <pjmedia-codec.h>
#include <pjlib.h>
#include "processing-port.h"
#define LOCAL_IP_SOUND "127.0.0.1"
/*
 * the maximum of streams can be attached to loopback transport is 8, 
 * if you need more, please modify pjproject (pjmedia/src/pjmedia/transport_loop.c)
 * and then rebuild it.
*/
#define MAX_SOUND_DEVICE 4 
#define MAX_RECV_STREAM 11//need to be equal with MAX_STREAMER
#define MAX_STREAMER 11
typedef enum streamer_type_e {
    EPT_UNKNOWN = -1,
    EPT_FILE = 1,
    EPT_DEV = 2
} endpoint_type;

typedef enum streamer_state_e {
    EPS_UNINIT = -1,
    EPS_START = 1,
    EPS_STOP = 2
} endpoint_state;

typedef struct endpoint_s endpoint_t;

typedef struct endpoint_stream_s {
    volatile int slot;
    int lock; //do not process until we done with this stream
    pjmedia_stream *stream;

    pj_in_addr peer_ip; // join multicast group for receiver OR destination multicast group for streamer
    int peer_port;      // binding port for receiver OR destination port for streamer
	volatile int isRxBlocked;		//indicate this port is Rx blocked
	volatile int isSelected;		//do not emit audio util we select the radio or oiu
	float volume;
	int pjsuaCallId[MAX_RECV_STREAM];  //save call id of call in patching list
	volatile int inUse;// for streamer only

    int stream_idx;
    endpoint_t *endpoint;

    struct {
        int mean_rtt_us;
        int max_rtt_us;
        int min_rtt_us;
        int mean_delay_ms;
        int max_delay_ms;
        int min_delay_ms;
    } delay;
    struct {
        unsigned int pkt;
        unsigned lost;
        unsigned discard;
    } drop;
} endpoint_stream_t;

typedef void (*vad_state_changed_callback)(int stream_idx, int signal, int tx, int rx);

struct endpoint_s {
    endpoint_type type;
    endpoint_state state;
    pjmedia_endpt *ep;
    pj_pool_t *pool;
    pjmedia_codec_info *ci;
    
    int nstreams;
    endpoint_stream_t *streams;
	
	//int recvStreamIdx[MAX_RECV_STREAM];  //recv stream idx of oiuc
    union {
        struct {
            union {
                pjmedia_master_port *mport;
                pjmedia_snd_port *snd_port;
            };
        } aout;
        union {
            pjmedia_master_port *mport;
            pjmedia_snd_port *snd_port;
        } ain;
    };
    int idx;

    processing_port_t processing_port;
    int node_idx;
    vad_state_changed_callback on_vad_state_changed;
};

void streamer_init(endpoint_t *streamer, pjmedia_endpt *ep, pj_pool_t *pool, vad_state_changed_callback callback);
pj_status_t streamer_config_stream(endpoint_t *streamer, int lport, char *rhost, int rport, int idx);
//pj_status_t streamer_config_processing(endpoint_t *streamer, int stream_idx, int agc, int vad);
void streamer_config_file_source(endpoint_t *streamer, char *file_name);
void streamer_config_dev_source(endpoint_t *streamer, int idx);
void streamer_start(endpoint_t *streamer, int idx);
void streamer_start_sound(endpoint_t *streamer);
void streamer_start_streaming(endpoint_t *streamer, int idx);
void streamer_update_stats(endpoint_t *streamer);
void streamer_stop(endpoint_t *streamer);
void streamer_stop_sound(endpoint_t *streamer);
void streamer_stop_streaming(endpoint_t *streamer);
void streamer_adjust_master_volume(endpoint_t *receiver, endpoint_t *streamer, float incremental);
void streamer_bind_to_node(endpoint_t *streamer, int node_idx);

void receiver_init(endpoint_t *receiver, pjmedia_endpt *ep, pj_pool_t *pool, int nchans, vad_state_changed_callback callback);
pj_status_t receiver_config_stream(endpoint_t *receiver, char *mcast, int lport, int idx);
//pj_status_t receiver_config_processing(endpoint_t *receiver, int stream_idx, int agc, int vad);
void receiver_config_file_sink(endpoint_t *receiver, char *file_name);
void receiver_config_dev_sink(endpoint_t *receiver, int idx);
void receiver_start(endpoint_t *receiver, int idx);
void receiver_start_sound(endpoint_t *receiver);
void receiver_start_streaming(endpoint_t *receiver, int idx);
void receiver_update_stats(endpoint_t *receiver);
void receiver_stop(endpoint_t *receiver);
void receiver_stop_sound(endpoint_t *receiver);
void receiver_stop_streaming(endpoint_t *receiver);
void receiver_reset_volume(endpoint_t *receiver);
void receiver_mute_volume(endpoint_t *receiver);
void receiver_dump_streams(endpoint_t *receiver);

void receiver_turn_vad_on(endpoint_t *receiver);
void receiver_turn_vad_off(endpoint_t *receiver);
void receiver_turn_agc_on(endpoint_t *receiver);
void receiver_turn_agc_off(endpoint_t *receiver);
void receiver_bind_to_node(endpoint_t *receiver, int node_idx);

int agc_volume_get(endpoint_t *endpoint);
void agc_volume_set(endpoint_t *endpoint, int level);
void agc_volume_inc(endpoint_t *endpoint);
void agc_volume_dec(endpoint_t *endpoint);

/* use for oiuc only */
void receiver_start_2(endpoint_t *receiver, int idx);
void receiver_stop_2(endpoint_t *receiver, int idx);
void streamer_start_2(endpoint_t *streamer, int idx);
void streamer_stop_2(endpoint_t *streamer, int idx);
void receiver_adjust_volume(endpoint_t *receiver, int stream_idx, float incremental);
void receiver_adjust_master_volume(endpoint_t *receiver, float incremental);
void receiver_rx_block(endpoint_t *receiver, int stream_idx, int flag);
void receiver_rx_select(endpoint_t *receiver, int stream_idx, int flag);
#endif
