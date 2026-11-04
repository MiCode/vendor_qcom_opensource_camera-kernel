/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 */
// MIUI ADD: Camera_CameraOpt
#include "cam_msger.h"

#ifndef _CAM_MSG_SCHED_H_
#define _CAM_MSG_SCHED_H_

#include <linux/percpu-defs.h>
#include <linux/spinlock.h>
#include <linux/rcupdate.h>

#define TISOT_DEFAULT	0x00 //isolation type
#define TISOT_DPM	  0x01
#define TISOT_ALLOW	  0x02
#define TISOT_NEVERALLOW 0x04

#define TISOT_ALLOW_STR   "allow"
#define TISOT_NRALLOW_STR "neverallow"
#define TISOT_STR_LEN_MAX 10

#define MONITOR_MAX_PID  5

#define WRITE_CONTENT_MIN_LEN 4
#define WRITE_CONTENT_MAX_LEN 512

#define CPU_NR 10

struct cam_sched_data {
	u8 flag[CPU_NR];
	u8 offset;
	pid_t pids[MONITOR_MAX_PID];
	cpumask_t  ctl_cpus;
	struct rcu_head rcu;
};

void cam_sched_init(void);
int  cam_sched_w_pids(struct file *fp, char *buff, int length, u8 *hit);
void cam_sched_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg, u8 *hit);
void clear_cam_sched_data_entity(struct cam_sched_data *item);
#endif /* _CAM_MSG_SCHED_H_ */
// END Camera_CameraOpt
