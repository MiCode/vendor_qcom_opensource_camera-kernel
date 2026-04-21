/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_VFE_LITE97X_H_
#define _CAM_VFE_LITE97X_H_
#include "cam_vfe_camif_ver3.h"
#include "cam_vfe_top_ver4.h"
#include "cam_vfe_core.h"
#include "cam_vfe_bus_ver3.h"
#include "cam_irq_controller.h"
#include "cam_vfe_lite78x.h"
#include "cam_vfe_lite97x.h"

static struct cam_vfe_hw_info cam_vfe_lite97x_hw_info = {
	.irq_hw_info                   = &vfe_lite98x_irq_hw_info,

	.bus_version                   = CAM_VFE_BUS_VER_3_0,
	.bus_hw_info                   = &vfe_lite98x_bus_hw_info,

	.top_version                   = CAM_VFE_TOP_VER_4_0,
	.top_hw_info                   = &vfe_lite98x_top_hw_info,
};

#endif /* _CAM_VFE_LITE97X_H_ */
