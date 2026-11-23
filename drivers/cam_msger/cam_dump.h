
// MIUI ADD: Camera_CameraOpt
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/dma-buf.h>
#include <linux/fdtable.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/ktime.h>
#include "cam_msger.h"
#ifndef _CAM_MSG_DUMP_H_
#define _CAM_MSG_DUMP_H_

#define PAGES_TO_KB(n_pages)	((n_pages) << (PAGE_SHIFT - 10))
#define TO_KB(bytes)		((bytes) >> 10)
struct proc_meminfo {
	int rss_size;
	int ion_size;
	pid_t pid;
	uid_t uid;
	char task_name[TASK_COMM_LEN];
	struct list_head node_task;
};

void dump_init(void);

#endif
// END Camera_CameraOpt
