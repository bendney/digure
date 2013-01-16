
#ifndef HOST_SAMPLE_H
#define HOST_SAMPLE_H

#include "host/shm_config.h"
#include "codec/voice/voc.h"


#ifndef SAMPLE_DATA_DSP
#define SAMPLE_DATA_DSP
#endif  /* #define SAMPLE_DATA_DSP */

#define ENABLE_VOICE_SAMPLE		1

#define RESERVED_HEAP_SIZE		0x28000   /* Reserved heap size */

#if (ENABLE_VOICE_SAMPLE)
#define VOICE_BUFFER_LENGHT		500 * 160 /* 250 voice frame size */
#else
#define VIDEO_BUFFER_LENGHT		500 * 160 /* for video frame size */
#endif


void hst_sample_init(void);

void sample_handle_init(void);

void system_printf(const char *format, ...);

void notify_shm_post(void);

int record_voice_data(short * const frame_buf, unsigned short frame_size);

#endif  /* #ifdef HOST_SAMPLE_H */
