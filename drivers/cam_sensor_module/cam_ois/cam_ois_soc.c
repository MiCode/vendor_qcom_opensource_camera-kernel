// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/of.h>
#include <linux/of_gpio.h>
#include <cam_sensor_cmn_header.h>
#include <cam_sensor_util.h>
#include <cam_sensor_io.h>
#include <cam_req_mgr_util.h>

#include "cam_ois_soc.h"
#include "cam_debug_util.h"

/**
 * @o_ctrl: ctrl structure
 *
 * This function is called from cam_ois_platform/i2c_driver_probe, it parses
 * the ois dt node.
 */
int cam_ois_driver_soc_init(struct cam_ois_ctrl_t *o_ctrl)
{
	int                             rc = 0;
	struct cam_hw_soc_info         *soc_info = &o_ctrl->soc_info;
	struct cam_ois_soc_private     *soc_private =
		(struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t *power_info = &soc_private->power_info;
	struct device_node             *of_node = NULL;

	if (!soc_info->dev) {
		CAM_ERR(CAM_OIS, "soc_info is not initialized");
		return -EINVAL;
	}

	of_node = soc_info->dev->of_node;
	if (!of_node) {
		CAM_ERR(CAM_OIS, "dev.of_node NULL");
		return -EINVAL;
	}

	rc = cam_sensor_util_parse_and_request_resources(&(o_ctrl->io_master_info),
		soc_info);
	if (rc < 0) {
		CAM_ERR(CAM_EEPROM, "Failed in parse_and_request_resources rc : %d", rc);
		return rc;
	}

	if (!soc_info->gpio_data) {
		CAM_INFO(CAM_OIS, "No GPIO found");
		goto end;
	}

	if (!soc_info->gpio_data->cam_gpio_common_tbl_size) {
		CAM_INFO(CAM_OIS, "No GPIO found");
		rc = -EINVAL;
		goto release_resources;
	}

	rc = cam_sensor_util_init_gpio_pin_tbl(soc_info,
		&power_info->gpio_num_info);
	if ((rc < 0) || (!power_info->gpio_num_info)) {
		CAM_ERR(CAM_OIS, "No/Error OIS GPIOs");
		rc = -EINVAL;
		goto release_resources;
	}
	goto end;

release_resources:
	cam_sensor_util_release_resources(&(o_ctrl->io_master_info), soc_info);

end:
	memset(&o_ctrl->fw_info, 0, sizeof(struct cam_ois_fw_info));

	INIT_LIST_HEAD(&(o_ctrl->i2c_init_data.list_head));
	INIT_LIST_HEAD(&(o_ctrl->i2c_calib_data.list_head));
	INIT_LIST_HEAD(&(o_ctrl->i2c_fwinit_data.list_head));

	INIT_LIST_HEAD(&(o_ctrl->i2c_fw_version_data.list_head));
	INIT_LIST_HEAD(&(o_ctrl->i2c_mode_data.list_head));
	INIT_LIST_HEAD(&(o_ctrl->i2c_time_data.list_head));
	INIT_LIST_HEAD(&(o_ctrl->read_buf_list));
	mutex_init(&(o_ctrl->read_buf_lock));
	INIT_LIST_HEAD(&(o_ctrl->i2c_postinit_data.list_head)); // xiaomi add
	INIT_LIST_HEAD(&(o_ctrl->i2c_motion_data.list_head)); // xiaomi add

	return rc;
}
