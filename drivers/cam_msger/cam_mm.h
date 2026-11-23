/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 */
// MIUI ADD: Camera_CameraOpt
#include "cam_msger.h"
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/fs.h>

#ifndef _CAM_MSG_MM_H_
#define _CAM_MSG_MM_H_

#define WATHERMARK_TYPE_MIN  0
#define WATHERMARK_TYPE_LOW  1
#define WATHERMARK_TYPE_HIGH 2
#define MEMORY_OPERATION_TYPE_DROP_FILE_CACHE 3

struct inode_entry {
	struct inode *inode;
	struct list_head list;
};

void cam_mm_init(void);
int  cam_mm_rw(struct file *fp, char *buff, int length, u8 *hit);
void cam_mm_ioctl(struct file *filp, unsigned int cmd,  unsigned long arg, u8 *hit);
#endif /* _CAM_MSG_MM_H_ */
// END Camera_CameraOpt
