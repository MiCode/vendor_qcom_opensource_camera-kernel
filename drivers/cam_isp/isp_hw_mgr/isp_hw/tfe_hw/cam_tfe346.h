/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */


#ifndef _CAM_TFE346_H_
#define _CAM_TFE346_H_
#include "cam_tfe_core.h"
#include "cam_tfe_bus.h"
#include "cam_tfe530.h"

struct cam_tfe_hw_info cam_tfe346 = {
	.top_irq_mask = {
		0x00001034,
		0x00001038,
		0x0000103C,
	},
	.top_irq_clear = {
		0x00001040,
		0x00001044,
		0x00001048,
	},
	.top_irq_status = {
		0x0000104C,
		0x00001050,
		0x00001054,
	},
	.top_irq_cmd                       = 0x00001030,
	.global_clear_bitmask              = 0x00000001,

	.bus_irq_mask = {
		0x00001A18,
		0x00001A1C,
	},
	.bus_irq_clear = {
		0x00001A20,
		0x00001A24,
	},
	.bus_irq_status = {
		0x00001A28,
		0x00001A2C,
	},
	.bus_irq_cmd = 0x00001A30,
	.bus_violation_reg = 0x00001A64,
	.bus_overflow_reg = 0x00001A68,
	.bus_image_size_vilation_reg = 0x1A70,
	.bus_overflow_clear_cmd = 0x1A60,
	.debug_status_top = 0x1AD8,

	.reset_irq_mask = {
		0x00000001,
		0x00000000,
		0x00000000,
	},
	.error_irq_mask = {
		0x000F0F00,
		0x00000000,
		0x0000003F,
	},
	.bus_reg_irq_mask = {
		0x00000002,
		0x00000000,
	},
	.bus_error_irq_mask = {
		0xC0000000,
		0x00000000,
	},

	.num_clc = 14,
	.clc_hw_status_info            = tfe530_clc_hw_info,
	.bus_version                   = CAM_TFE_BUS_1_0,
	.bus_hw_info                   = &tfe530_bus_hw_info,

	.top_version                   = CAM_TFE_TOP_1_0,
	.top_hw_info                   = &tfe530_top_hw_info,
};

#endif /* _CAM_TFE346_H_ */
