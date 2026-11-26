/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 */

#ifndef _CAM_ZOOM_SOC_H_
#define _CAM_ZOOM_SOC_H_

#include "cam_zoom_dev.h"

/**
 * @z_ctrl: Zoom ctrl structure
 *
 * This API parses zoom device tree
 */
int cam_zoom_parse_dt(struct cam_zoom_ctrl_t *z_ctrl,
	struct device *dev);

#endif /* _CAM_ZOOM_SOC_H_ */
