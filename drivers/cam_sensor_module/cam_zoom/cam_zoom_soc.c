// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/of.h>
#include <linux/of_gpio.h>
#include <cam_sensor_cmn_header.h>
#include <cam_sensor_util.h>
#include <cam_sensor_io.h>
#include <cam_req_mgr_util.h>
#include "cam_zoom_soc.h"
#include "cam_soc_util.h"

int32_t cam_zoom_parse_dt(struct cam_zoom_ctrl_t *z_ctrl,
	struct device *dev)
{
	int32_t                         rc = 0;
	struct cam_hw_soc_info          *soc_info = &z_ctrl->soc_info;
	struct cam_zoom_soc_private *soc_private =
		(struct cam_zoom_soc_private *)z_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t  *power_info = &soc_private->power_info;

	/* Initialize mutex */
	mutex_init(&(z_ctrl->zoom_mutex));

	rc = cam_sensor_util_parse_and_request_resources(&(z_ctrl->io_master_info),
		soc_info);
	if (rc < 0) {
		CAM_ERR(CAM_ZOOM, "Failed in parse_and_request_resources rc : %d", rc);
		return rc;
	}

	if (!soc_info->gpio_data) {
		CAM_DBG(CAM_ZOOM, "No GPIO found");
		rc = 0;
		return rc;
	}

	if (!soc_info->gpio_data->cam_gpio_common_tbl_size) {
		CAM_DBG(CAM_ZOOM, "No GPIO found");
		return -EINVAL;
	}

	rc = cam_sensor_util_init_gpio_pin_tbl(soc_info,
		&power_info->gpio_num_info);
	if ((rc < 0) || (!power_info->gpio_num_info)) {
		CAM_ERR(CAM_ZOOM, "No/Error Zoom GPIOs");
		return -EINVAL;
	}
	return rc;
}
