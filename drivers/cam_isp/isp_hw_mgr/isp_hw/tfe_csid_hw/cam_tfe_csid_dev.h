/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2019-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_TFE_CSID_DEV_H_
#define _CAM_TFE_CSID_DEV_H_

#include "cam_isp_hw.h"

irqreturn_t cam_tfe_csid_irq(int irq_num, void *data);

int cam_tfe_csid_probe(struct platform_device *pdev);

#if KERNEL_VERSION(6, 10, 0) > LINUX_VERSION_CODE
int cam_tfe_csid_remove(struct platform_device *pdev);
#else
void cam_tfe_csid_remove(struct platform_device *pdev);
#endif

#endif /*_CAM_TFE_CSID_DEV_H_ */
