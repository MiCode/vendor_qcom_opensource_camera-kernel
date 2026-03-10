/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_TFE_CSID_346_H_
#define _CAM_TFE_CSID_346_H_

#include "cam_tfe_csid_core.h"
#include "cam_tfe_csid530.h"


#define CAM_TFE_CSID_VERSION_V346                 0x30406000

static struct cam_tfe_csid_hw_info cam_tfe_csid346_hw_info = {
	.csid_reg = &cam_tfe_csid_530_reg_offset,
	.hw_dts_version = CAM_TFE_CSID_VERSION_V346,
};

int cam_tfe_csid346_init_module(void);
void cam_tfe_csid346_exit_module(void);

#endif /*_CAM_TFE_CSID_346_H_ */
