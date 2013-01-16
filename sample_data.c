
#include <string.h>

#include "host/sample_data.h" 
#include "platform/app_log.h"

typedef struct shm_data {
    unsigned int msg_evt;
    unsigned int message;
    char log_buffer[10][20];
#if (ENABLE_VOICE_SAMPLE)
    short voice_buffer[VOICE_BUFFER_LENGHT];
#else
    short video_buffer[VIDEO_BUFFER_LENGHT];
#endif
    short *data_buf_tail;
    short *idle_buffer_address;
} shm_data, *shm_data_handle_t;

shm_data_handle_t shm_data_h;


void hst_sample_init(void)
{
    shm_data_h = (shm_data_handle_t)(shm_buf_h->heap + SHM_HEAP_SIZE
	    - RESERVED_HEAP_SIZE);
}

void sample_handle_init(void)
{
    shm_data_h->msg_evt = 0;
    shm_data_h->message = 2;

    memset(shm_data_h->log_buffer, 0, 20 * 10 * sizeof(char));

#if (ENABLE_VOICE_SAMPLE)
    memset(shm_data_h->voice_buffer, 0x0, VOICE_BUFFER_LENGHT * sizeof(short));
    shm_data_h->data_buf_tail = shm_data_h->voice_buffer + VOICE_BUFFER_LENGHT;
    shm_data_h->idle_buffer_address = shm_data_h->voice_buffer;
#else
    memset(shm_data_h->video_buffer, 0x0, VIDEO_BUFFER_LENGHT * sizeof(short));
    shm_data_h->data_buf_tail = shm_data_h->video_buffer + VIDEO_BUFFER_LENGTH;
    shm_data_h->idle_buffer_address = shm_data_h->video_buffer;
#endif
}

void system_printf(const char *format, ...)
{
    static int count = 0;
    int buffer_cycle = 9; 

    if (count == 3)
	return;

    while (buffer_cycle > 0) {
	if (shm_data_h->log_buffer[buffer_cycle][0]) {
	    //buffer_cycle ++;
	    break;
	}
	if (buffer_cycle == 0)
	    break;
	buffer_cycle --;
    }

    memcpy(shm_data_h->log_buffer[buffer_cycle] + 1, format, 20 * sizeof(char));
    shm_data_h->log_buffer[buffer_cycle][0] = 1;

    GS_LOG1(1, 1, 1, shm_data_h->log_buffer[0] + 1);
    GS_LOG1(1, 1, 2, shm_data_h->log_buffer[1] + 1);
    GS_LOG1(1, 1, 3, shm_data_h->log_buffer[2] + 1);
//    memcpy(shm_data_h->log_buffer, format, 20 * sizeof(char));
    shm_data_h->msg_evt = 1;
    count++;
}

void notify_shm_post(void)
{
    while (shm_data_h->msg_evt);	
    shm_data_h->msg_evt = 1;
}

#if ENABLE_VOICE_SAMPLE
int send_voice_to_host(short * const frame_buffer, unsigned short frame_size)
{
    short *voice_buffer = NULL;

    if (shm_data_h->msg_evt)
	return 0;

    voice_buffer = shm_data_h->idle_buffer_address;

    memcpy(voice_buffer, frame_buffer, frame_size * sizeof(short));
#if 0
    unsigned int i;
    GS_LOG1(LOG_AREA_VOC, LOG_LVL_INFO, LOG_EVT_VOC_INFO, "length:%d==========\n", frame_size);
    if (shm_data_h->buf_curr_ptr != shm_data_h->data_buf)
	GS_LOG1(LOG_AREA_VOC, LOG_LVL_INFO, LOG_EVT_VOC_INFO, "print frame 2 length:%d\n", frame_size);
    else
	GS_LOG1(LOG_AREA_VOC, LOG_LVL_INFO, LOG_EVT_VOC_INFO, "print frame 1 length:%d\n", frame_size);
    for (i = 0; i < frame_size; i++) {
	GS_LOG1(LOG_AREA_VOC, LOG_LVL_INFO, LOG_EVT_VOC_INFO, "0x%x\n", frame_buf[i]);
	GS_LOG1(LOG_AREA_VOC, LOG_LVL_INFO, LOG_EVT_VOC_INFO, "0x%x\n", shm_data_h->buf_curr_ptr[i]);
    }
    //shm_data_h->buf_curr_ptr += frame_size;
#endif
    //memcpy(data_buf_p, frame_buf, frame_size * sizeof(short));
    voice_buffer += frame_size;

    shm_data_h->idle_buffer_address = voice_buffer;

    if (voice_buffer == shm_data_h->data_buf_tail)
	return 1;

    return 0;
}

#else 
int send_video_to_host(short * const frame_buffer, unsigned short frame_size)
{
    short *video_buffer = NULL;

    if (shm_data_h->msg_evt)
	return 0;

    video_buffer = shm_data_h->idle_buffer_address;
    memcpy(video_buffer, frame_buffer, frame_size * sizeof(short));
    video_buffer += frame_size;
    shm_data_h->idle_buffer_address = video_buffer;

    if (video_buffer == shm_data_h->data_buf_tail)
	return 1;

    return 0;
}
#endif /* if ENABLE_VOICE_SAMPLE */
