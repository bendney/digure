/*
 *  @file   SyslinkMemMgrDrv.c
 *
 *  @brief  OS-specific implementation of SyslinkMemMgr driver for Linux
 */


/*  Defined to include MACROS EXPORT_SYMBOL. This must be done before including
 *  module.h
 */
#if !defined (EXPORT_SYMTAB)
#define EXPORT_SYMTAB
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/syscalls.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

#include <linux/mm.h>
#include <linux/bootmem.h>
#include <asm/io.h>

#if 1
#define GXV3175
#else
#define GXV3140
#endif

/* DSP reserved DDR space */

#if defined(GXV3175)
#define DSP_DDR_BASE			0x87000000
#define DSP_DDR_SIZE			0x02400000  /* 0x02000000 for previous version */
#elif defined(GXV3140)
#define DSP_DDR_BASE			0x87000000
#define DSP_DDR_SIZE			0x01000000
#endif

/* SHM Config */
#define DSP_SHM_CFG			0x87000080

#if defined(GXV3175)
#define DSP_SHM_BASE			__va(DSP_DDR_BASE + DSP_DDR_SIZE)
#define DSP_SHM_SIZE			0x00A00000   
#elif defined(GXV3140)
#define DSP_SHM_BASE			0xc040d000
#define DSP_SHM_SIZE			0x00500000  
#endif
 
#define FACTORY_DATA_BASE		0x64
#define FACTORY_DATA_SIZE		(SHM_HEAP_BASE - FACTORY_DATA_BASE)

#define SHM_HEAP_BASE           	0x400
#define SHM_HEAP_SIZE           	(DSP_SHM_SIZE - SHM_HEAP_BASE - DSP_LOG_SIZE)

#define RESERVED_HEAP_SIZE		0x28000
#define REAL_HEAP_SIZE			(SHM_HEAP_SIZE - RESERVED_HEAP_SIZE)

#define DSP_LOG_BASE            	(DSP_SHM_BASE + DSP_SHM_SIZE - DSP_LOG_SIZE)
#define DSP_LOG_SIZE 	        	0x400
#define DSP_LOG_MASK			(DSP_LOG_SIZE - 1)


#define DATA_BUFFER_LENGTH		(500 * 160)
#define LOG_BUFFER_LENGTH		20	
#define DATA_FILE_PATH			"/data/sample_data.pcm"

/* Macros for return value to mark */
#define SYS_ERR			-1
#define APP_ERR			-2
#define NORMAL_RESULT		0

typedef struct magic_data {
    unsigned int msg_evt;
    unsigned int message;
    //char log_buffer[10][20];
    short data_buffer[DATA_BUFFER_LENGTH];
    short *buffer_tail;
    short *buffer_head;
} magic_data, *MAGIC_HANDLE;

static MAGIC_HANDLE magic_handle = NULL;
struct timer_list magic_timer;


/*!
 * @brief  Magic initial
 * param   void
 * return  if 0 magic_handle got successfully, if -1 failure
 */
static int magic_initial(void)
{
    magic_handle = (MAGIC_HANDLE)(DSP_SHM_BASE + SHM_HEAP_BASE + REAL_HEAP_SIZE);
    if (NULL == magic_handle) {
	printk("System Error[+1]: Wow, get magic address failure!\n");
	return SYS_ERR;
    }
    
    return NORMAL_RESULT;
}


/*!
 * @brief   Magic finalize
 * param    void 
 * return   magic_handle has been set to NULL 
 */
static int magic_finalize(void)
{
    if (NULL != magic_handle) {
	magic_handle = NULL;
    } else {
	printk("System Error[+1]: Bye, magic has been set to NULL!\n");
    }

    return NORMAL_RESULT;
}


static void magic_handle_print(void)
{
    int i;

    printk("magic_handle->msg_evt:0x%x\n", magic_handle->msg_evt);
    printk("magic_handle->message:0x%x\n", magic_handle->message);
    printk("magic_handle->buffer_tail:0x%p\n", magic_handle->buffer_tail);
    printk("magic_handle->buffer_head:0x%p\n", magic_handle->buffer_head);
    printk("magic_handle->data_buffer:0x%p\n", magic_handle->data_buffer);

    for (i = 0; i < 8; i++)
	printk("%d ", magic_handle->data_buffer[i]);
    printk("\n");
}


static void magic_handle_reset(const MAGIC_HANDLE magic_handle)
{
    //magic_handle->lock();
    magic_handle->msg_evt = 0x0;
    magic_handle->message = 0x0;
    memset(magic_handle->data_buffer, 0x0, DATA_BUFFER_LENGTH * sizeof(short));
    magic_handle->buffer_tail = magic_handle->data_buffer + DATA_BUFFER_LENGTH;
    magic_handle->buffer_head = magic_handle->data_buffer;
    //magic_handle->unlock();
}


static int dsp2arm_data(const MAGIC_HANDLE magic_handle)
{
    unsigned int i;
    mm_segment_t old_fs;
    struct file *outfile  = NULL;
    unsigned int voice_length;

    voice_length = (unsigned int)DATA_BUFFER_LENGTH;

    outfile = filp_open(DATA_FILE_PATH, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (IS_ERR(outfile)) {
	printk("APP_ERROR[+2]: Wow, we can't create your voice file!\n");
	return APP_ERR;
    }

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    /* It must be devide voice buffer into peice to save voice */
    for (i = 0; i < voice_length; i++) {
	outfile->f_op->write(outfile, (void *)(magic_handle->data_buffer + i),
	       sizeof(short), &outfile->f_pos);
    }
    set_fs(old_fs);	

    filp_close(outfile, NULL);

    return NORMAL_RESULT;
}


static void system_printk(const MAGIC_HANDLE magic_handle)
{
#if 0
    unsigned int buffer_cycle = 0;
    
    while (!magic_handle->log_buffer[buffer_cycle]) {
	buffer_cycle++;
    }
    /* log buffer is not null */
    if (magic_handle->log_buffer[buffer_cycle][0] != 0) {
	printk("%s\n", magic_handle->log_buffer[buffer_cycle]);
	memset(magic_handle->log_buffer[buffer_cycle], 0, 20 * sizeof(char));
	buffer_cycle++;
    }
#endif
}

/**
 * notify_pend
 */
static void notify_pend(unsigned long arg)
{
    if (magic_handle->msg_evt == 1) {
	//system_printk(magic_handle);
	//printk("%s\n", magic_handle->log_buffer);
	printk("message_event:%d\n", magic_handle->msg_evt);
	magic_handle->msg_evt = 0;
  //  } else if (23 == magic_handle->msg_evt) {
	//printk("dsptest message:%d\n", magic_handle->msg_evt);
    }else {
	//printk("test\n");
    }

    mod_timer(&magic_timer, jiffies + (HZ / 100));
    //del_timer(&notify_timer);
}

static int notify_timer_startup(void)
{
    init_timer(&magic_timer);
    magic_timer.expires = jiffies + (HZ / 100);
    magic_timer.data = 0;
    magic_timer.function = notify_pend;

    add_timer(&magic_timer);

    return NORMAL_RESULT;
}

static int notify_timer_stop(void)
{
    del_timer(&magic_timer);

    return NORMAL_RESULT;
}

/** ============================================================================
 *  Functions required for multiple .ko modules configuration
 *  ============================================================================
 */

/*!
 *  @brief  Module initialization function for Linux driver.
 */
static int __init MylinkMemMgrDrv_initializeModule(void)
{
    int result = 0;

    printk("MylinkMemMgrDrv_initializeModule\n");
    
    /* magic handle initialize */
    result = magic_initial();

    /* Start a kernel timer to wait message event */
    notify_timer_startup();

    magic_handle_print();

    printk("MylinkMemMgrDrv_initializeModule 0x%x\n", result);

    return result;
}


/*!
 *  @brief  Linux driver function to finalize the driver module.
 */
static void __exit MylinkMemMgrDrv_finalizeModule(void)
{

    printk("Entered MylinkMemMgrDrv_finalizeModule\n");

    notify_timer_stop();
    /* magic handle finalization */ 
    magic_finalize();

    printk("Leaving MylinkMemMgrDrv_finalizeModule\n");

    return;
}


/*!
 *  @brief  Macro calls that indicate initialization and finalization functions
 *          to the kernel.
 */
MODULE_LICENSE("GPL");

module_init (MylinkMemMgrDrv_initializeModule);
module_exit (MylinkMemMgrDrv_finalizeModule);

