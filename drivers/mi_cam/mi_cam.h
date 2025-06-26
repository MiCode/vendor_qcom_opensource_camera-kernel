// MIUI ADD: Camera_CamSched
/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 */
#ifndef MICAM_H
#define MICAM_H
#include <linux/types.h>
#include <linux/kfifo.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/workqueue.h>
#include <linux/ioctl.h>
#include <linux/sched/walt.h>

#define MI_CAM_DEV_NAME  "micam"
#define MI_CAM_SCHED   "mi_cam_sched"

#define MI_CAM_SCHED_ENABLE  _IO(240, 0X0)
#define MI_CAM_SCHED_DISABLE _IO(240, 0X1)
#define MI_CAM_SCHED_WRITE  _IOW(240, 0X2, unsigned int)
#define MI_CAM_SCHED_READ  _IOR(240, 0X3, unsigned int)
#define MI_CAM_SCHED_SET_WDOG _IO(240, 0X4)

#define CAM_CPUBG_DEFAULT_TL_THRESH (1024)
#define CAM_CPUBG_DEFAULT_MAX_FREQ (2438400)

void micam_exception_process(struct timer_list *timer);

#ifdef CONFIG_MI_CAM_WALT
typedef u64 (*micam_cpu_cycles_calibration_f)(u64 wallclock, int cpu, u64 cycles_delta);
extern void register_micam_cpu_cycles_calibration_hook(micam_cpu_cycles_calibration_f f);
extern void unregister_micam_cpu_cycles_calibration_hook(void);
#endif

static bool cam_cpubg_pressure_enable = false;
static int cam_cpubg_target_load_thresh = CAM_CPUBG_DEFAULT_TL_THRESH;
static unsigned long cam_cpubg_max_freq = CAM_CPUBG_DEFAULT_MAX_FREQ;

struct micam_amu_data {
	int cpu;
	u64 mem_stall;
	u64 mem_stall_delta;
	u64 amu_cyc;
	u64 amu_cyc_delta;
	u64 last_update;
};

DECLARE_PER_CPU(struct micam_amu_data, micam_amu_data);

struct micam_dev {
	dev_t dev;
	struct device *micam_device;
	struct mutex  micam_lock;
	unsigned int  enable_feature;
	rwlock_t f_lock;
	struct timer_list  crm_wdog;
};
#endif
// END Camera_CamSched
