/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_IFE_CSID_1190_H_
#define _CAM_IFE_CSID_1190_H_

#include <linux/module.h>
#include "cam_ife_csid_dev.h"
#include "camera_main.h"
#include "cam_ife_csid_common.h"
#include "cam_ife_csid_hw_ver2.h"
#include "cam_irq_controller.h"
#include "cam_isp_hw_mgr_intf.h"

int cam_ife_csid_1190_override_cb(void *hw_info)
{
	struct cam_ife_csid_core_info *core_info = (struct cam_ife_csid_core_info *)hw_info;
	struct cam_ife_csid_ver2_reg_info *csid_reg = NULL;

	csid_reg = (struct cam_ife_csid_ver2_reg_info *)core_info->csid_reg;
	core_info->csid_reg = csid_reg->base;

	/* Override values over the base values */
	csid_reg = csid_reg->base;
	csid_reg->cmn_reg->rd_ctxt_sel_shift = 0;
	csid_reg->cmn_reg->capabilities |= CAM_IFE_CSID_CAP_SPLIT_CTXT_RD_WR_SEL;

	return 0;
}

static struct cam_ife_csid_ver2_reg_info cam_ife_csid_1190_reg_info = {
	.override_cb = cam_ife_csid_1190_override_cb,
	.base = &cam_ife_csid_common_reg_v1_reg_info,
};
#endif /*_CAM_IFE_CSID_1190_H_ */
