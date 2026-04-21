// MIUI ADD: Camera_CameraOpt
#ifndef _CAM_MSG_H_
#define _CAM_MSG_H_
#include <linux/types.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/input.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/timer.h>
#include <linux/notifier.h>
#include <linux/fb.h>
#include <linux/pid.h>
#include <linux/poll.h>
#include <linux/preempt.h>
#include <linux/string.h>

#define LTYPE_CPUSET 1226
#define LTYPE_PID 1227
#define LTYPE_COC 1228 
#define LTYPE_SCHED 1229

#define MY_IOCTL_TYPE 'M'
#define IOC_CMD_PID_DEBUG_ON    _IOW(MY_IOCTL_TYPE, LTYPE_PID, int)
#define IOC_CMD_COC_DEBUG_ON    _IOW(MY_IOCTL_TYPE, LTYPE_COC, int)
#define IOC_CMD_SCHED_DEBUG_ON  _IOW(MY_IOCTL_TYPE, LTYPE_SCHED, int)

#define IOC_CMD_DEBUG_OFF _IOW(MY_IOCTL_TYPE, 1230, int)

#define IOC_CMD_DUMP _IOW(MY_IOCTL_TYPE, 1231, int)
#define IOC_CMD_SCHED_DUMP _IOW(MY_IOCTL_TYPE, 1232, int)
#define IOC_CMD_MSGER_SWITCHES_DUMP _IOW(MY_IOCTL_TYPE, 1233, int)

#define READER_COUNT_MAX 7
#define MSG_COUNT_MAX 16
#define MSG_LEN_MAX 2048
#define READER_ID_MAX 60
#define DEV_NAME_LEN_MAX 25

#define READER_DEATH_OMEN_LEN_MAX     256
#define READER_DEATH_CAMMIND_MSG_CODE 1015
#define READED_DEATH_LIST_COUNT_MAX 16
#define READER_DEATH_OMEN_TEMPLATE "snd|%s|%d|{\"id\":5, \"what\":%d, \"time\":%llu, \"replay\":0, \"data\":{\"tid\":%d, \"pid\":%d, \"name\":%s}}"

#define CMDLINE_MAX_LEN  60 
#define SPLIT_DELIM "|"

#define m_pr(type, fmt, args...) \
    if(debug(type)) { \
       printk(KERN_INFO fmt, ##args); \
    }
//COC-cam_msger.h SCD-cam_sched.h 
//PID-cam_pid.h   BID-cam_binder.h
#define COC_ENABLE 0x0001
#define SCD_ENABLE 0x0002
#define PID_ENABLE 0x0004
#define BID_ENABLE 0x0008

extern unsigned int msger_siwtches;

#define switch_enable_and_exec(swh) \
    if(msger_siwtches & swh)        \

#define switch_disable_and_exec(swh) \
    if(!(msger_siwtches & swh))      \

typedef struct {
    int used;
    char from[READER_ID_MAX];
    char   to[READER_ID_MAX];
    char   data[MSG_LEN_MAX];
} m_item;//message item

typedef struct {
    int   pid;
    int  tgid;
    int  used;
    int  rear;
    int front;
    struct file *fp;
    char   name[READER_ID_MAX];
    m_item msgs[MSG_COUNT_MAX];
    wait_queue_head_t wait_queue;
} r_entity;//reader entity

typedef struct cammsger_dev {
   char    name[DEV_NAME_LEN_MAX];
   char version[DEV_NAME_LEN_MAX];
   struct class   *cammsger_class;
   unsigned int    cammsger_major;

   dev_t devt;
   struct device *device;
   struct file_operations dev_fops;

   struct mutex g_lock;//global Lock
   int r_entity_count;
   r_entity readers[READER_COUNT_MAX];

   int debug;
   char d_cache[MSG_LEN_MAX];
} cammsger_dev;

typedef void (*init_f)(void);
typedef int  (*write_f)(struct file *fp, char *data, int length, u8* hit);
typedef int  (*read_f)(struct file *fp, char __user *buff, int length, u8* hit);
typedef void (*release_f)(struct inode *inode, struct file *filp);
typedef void (*ioctl_f)(struct file *filp, unsigned int cmd,  unsigned long arg, u8 *hit);
typedef unsigned int (*poll_f)(struct file *file, struct poll_table_struct *poll_table, u8 *hit);

typedef struct {
    init_f  init;
    write_f write;
    read_f  read;
    poll_f  poll;
    ioctl_f ioctl;
    release_f release;
} submodule;

u8 debug(int log_type);

#endif /* _CAM_MSG_H_ */
// END Camera_CameraOpt
