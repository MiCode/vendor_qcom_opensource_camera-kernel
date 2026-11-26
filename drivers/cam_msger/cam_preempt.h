/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 */
// MIUI ADD: Camera_CameraOpt
#include "cam_msger.h"

#ifndef _CAM_MSG_PREEMPT_H_
#define _CAM_MSG_PREEMPT_H_
void cam_preempt_init(void);
int  cam_preempt_write(struct file *fp, char *buff, int length, u8 *hit);
void cam_preempt_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg, u8 *hit);
void  cam_preempt_is_cpu_allowed(void *unused, struct task_struct *p, int cpu, bool *allowed);
#endif /* _CAM_MSG_PREEMPT_H_ */
// END Camera_CameraOpt

