/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_IFE_CSID_975_H_
#define _CAM_IFE_CSID_975_H_

#include <linux/module.h>
#include "cam_ife_csid_dev.h"
#include "camera_main.h"
#include "cam_ife_csid_common.h"
#include "cam_ife_csid_hw_ver2.h"
#include "cam_irq_controller.h"
#include "cam_isp_hw_mgr_intf.h"
#include "cam_ife_csid980.h"

#define CAM_CSID_VERSION_V975                 0x90070050

static struct cam_ife_csid_ver2_reg_info cam_ife_csid_975_reg_info = {
	.top_irq_reg_info      = cam_ife_csid_980_top_irq_reg_info,
	.rx_irq_reg_info       = cam_ife_csid_980_rx_irq_reg_info,
	.path_irq_reg_info     = {
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_0],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_1],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_2],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_3],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_4],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_IPP],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_PPP],
		NULL,
		NULL,
		NULL,
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_IPP_1],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_IPP_2],
	},
	.buf_done_irq_reg_info = &cam_ife_csid_980_buf_done_irq_reg_info,
	.cmn_reg                              = &cam_ife_csid_980_cmn_reg_info,
	.csi2_reg                             = &cam_ife_csid_980_csi2_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_IPP]   = &cam_ife_csid_980_ipp_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_IPP_1] = &cam_ife_csid_980_ipp_1_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_IPP_2] = &cam_ife_csid_980_ipp_2_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_PPP]   = &cam_ife_csid_980_ppp_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_0] = &cam_ife_csid_980_rdi_0_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_1] = &cam_ife_csid_980_rdi_1_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_2] = &cam_ife_csid_980_rdi_2_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_3] = &cam_ife_csid_980_rdi_3_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_4] = &cam_ife_csid_980_rdi_4_reg_info,
	.ipp_mc_reg                           = &cam_ife_csid_980_ipp_mc_reg_info,
	.need_top_cfg = 0x0,
	.dynamic_drv_supported = true,
	.top_irq_desc        = &cam_ife_csid_980_top_irq_desc,
	.rx_irq_desc         = &cam_ife_csid_980_rx_irq_desc,
	.path_irq_desc       = cam_ife_csid_980_path_irq_desc,
	.num_top_err_irqs    = cam_ife_csid_980_num_top_irq_desc,
	.num_rx_err_irqs     = cam_ife_csid_980_num_rx_irq_desc,
	.num_path_err_irqs   = ARRAY_SIZE(cam_ife_csid_980_path_irq_desc),
	.top_debug_mask      = &cam_ife_csid_980_top_debug_mask,
	.rx_debug_mask       = &cam_ife_csid_980_rx_debug_mask,
	.num_top_regs        = 2,
	.num_rx_regs         = 2,
};
#endif /*_CAM_IFE_CSID_975_H_ */
