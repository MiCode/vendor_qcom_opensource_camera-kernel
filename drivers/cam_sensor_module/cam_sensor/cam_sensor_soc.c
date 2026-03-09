// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2019, 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/of.h>
#include <linux/of_gpio.h>
#include <cam_sensor_cmn_header.h>
#include <cam_sensor_util.h>
#include <cam_sensor_io.h>
#include <cam_req_mgr_util.h>
#include "cam_sensor_soc.h"
#include "cam_soc_util.h"
#include "cam_mem_mgr_api.h"

int32_t cam_sensor_get_sub_module_index(struct device_node *of_node,
	struct cam_sensor_board_info *s_info)
{
	int rc = 0, i = 0;
	uint32_t val = 0;
	struct device_node *src_node = NULL;
	struct cam_sensor_board_info *sensor_info;

	sensor_info = s_info;

	for (i = 0; i < SUB_MODULE_MAX; i++)
		sensor_info->subdev_id[i] = -1;

	src_node = of_parse_phandle(of_node, "actuator-src", 0);
	if (!src_node) {
		CAM_DBG(CAM_SENSOR, "src_node NULL");
	} else {
		rc = of_property_read_u32(src_node, "cell-index", &val);
		CAM_DBG(CAM_SENSOR, "actuator cell index %d, rc %d", val, rc);
		if (rc < 0) {
			CAM_ERR(CAM_SENSOR, "failed %d", rc);
			of_node_put(src_node);
			return rc;
		}
		sensor_info->subdev_id[SUB_MODULE_ACTUATOR] = val;
		of_node_put(src_node);
	}

	src_node = of_parse_phandle(of_node, "ois-src", 0);
	if (!src_node) {
		CAM_DBG(CAM_SENSOR, "src_node NULL");
	} else {
		rc = of_property_read_u32(src_node, "cell-index", &val);
		CAM_DBG(CAM_SENSOR, " ois cell index %d, rc %d", val, rc);
		if (rc < 0) {
			CAM_ERR(CAM_SENSOR, "failed %d",  rc);
			of_node_put(src_node);
			return rc;
		}
		sensor_info->subdev_id[SUB_MODULE_OIS] = val;
		of_node_put(src_node);
	}

	src_node = of_parse_phandle(of_node, "eeprom-src", 0);
	if (!src_node) {
		CAM_DBG(CAM_SENSOR, "eeprom src_node NULL");
	} else {
		rc = of_property_read_u32(src_node, "cell-index", &val);
		CAM_DBG(CAM_SENSOR, "eeprom cell index %d, rc %d", val, rc);
		if (rc < 0) {
			CAM_ERR(CAM_SENSOR, "failed %d", rc);
			of_node_put(src_node);
			return rc;
		}
		sensor_info->subdev_id[SUB_MODULE_EEPROM] = val;
		of_node_put(src_node);
	}

	src_node = of_parse_phandle(of_node, "led-flash-src", 0);
	if (!src_node) {
		CAM_DBG(CAM_SENSOR, " src_node NULL");
	} else {
		rc = of_property_read_u32(src_node, "cell-index", &val);
		CAM_DBG(CAM_SENSOR, "led flash cell index %d, rc %d", val, rc);
		if (rc < 0) {
			CAM_ERR(CAM_SENSOR, "failed %d", rc);
			of_node_put(src_node);
			return rc;
		}
		sensor_info->subdev_id[SUB_MODULE_LED_FLASH] = val;
		of_node_put(src_node);
	}

	rc = of_property_read_u32(of_node, "csiphy-sd-index", &val);
	if (rc < 0)
		CAM_ERR(CAM_SENSOR, "paring the dt node for csiphy rc %d", rc);
	else
		sensor_info->subdev_id[SUB_MODULE_CSIPHY] = val;

	//xiaomi add
	src_node = of_parse_phandle(of_node, "zoom-src", 0);
	if (!src_node) {
		CAM_DBG(CAM_SENSOR, "src_node NULL");
	} else {
		rc = of_property_read_u32(src_node, "cell-index", &val);
		CAM_DBG(CAM_SENSOR, "zoom cell index %d, rc %d", val, rc);
		if (rc < 0) {
			CAM_ERR(CAM_SENSOR, "failed %d", rc);
			of_node_put(src_node);
			return rc;
		}
		sensor_info->subdev_id[SUB_MODULE_ZOOM] = val;
		of_node_put(src_node);
	}
	//xiaomi end

	return rc;
}

static int32_t cam_sensor_init_bus_params(struct cam_sensor_ctrl_t *s_ctrl)
{
	/* Validate input parameters */
	if (!s_ctrl) {
		CAM_ERR(CAM_SENSOR, "failed: invalid params s_ctrl %pK",
			s_ctrl);
		return -EINVAL;
	}

	CAM_DBG(CAM_SENSOR,
		"master_type: %d", s_ctrl->io_master_info.master_type);
	/* Initialize cci_client */
	if (s_ctrl->io_master_info.master_type == CCI_MASTER) {
		s_ctrl->io_master_info.cci_client = CAM_MEM_ZALLOC(sizeof(
			struct cam_sensor_cci_client), GFP_KERNEL);
		if (!(s_ctrl->io_master_info.cci_client)) {
			CAM_ERR(CAM_SENSOR, "Memory allocation failed");
			return -ENOMEM;
		}
	} else if (s_ctrl->io_master_info.master_type == I2C_MASTER) {
		if (!(s_ctrl->io_master_info.qup_client))
			return -EINVAL;
	} else if (s_ctrl->io_master_info.master_type == I3C_MASTER) {
		CAM_DBG(CAM_SENSOR, "I3C Master Type");
	} else {
		CAM_ERR(CAM_SENSOR,
			"Invalid master / Master type Not supported : %d",
				s_ctrl->io_master_info.master_type);
		return -EINVAL;
	}

	return 0;
}

int32_t cam_sensor_parse_dt(struct cam_sensor_ctrl_t *s_ctrl)
{
	int32_t rc = 0;
	int i = 0;
	struct cam_sensor_board_info *sensordata = NULL;
	struct device_node *of_node = s_ctrl->of_node;
	struct cam_hw_soc_info *soc_info = &s_ctrl->soc_info;

	s_ctrl->sensordata = CAM_MEM_ZALLOC(sizeof(*sensordata), GFP_KERNEL);
	if (!s_ctrl->sensordata)
		return -ENOMEM;

	sensordata = s_ctrl->sensordata;

	rc = cam_sensor_init_bus_params(s_ctrl);
	if (rc < 0) {
		CAM_ERR(CAM_SENSOR,
			"Failed in Initialize Bus params, rc %d", rc);
		goto free_sensor_data;
	}

	rc = cam_sensor_util_parse_and_request_resources(&(s_ctrl->io_master_info),
		soc_info);
	if (rc < 0) {
		CAM_ERR(CAM_EEPROM, "Failed in parse_and_request_resources rc : %d", rc);
		return rc;
	}

	CAM_INFO(CAM_SENSOR,
		"master: %d (1-CCI, 2-I2C, 3-SPI, 4-I3C) pm_ctrl_client_enable: %d",
		s_ctrl->io_master_info.master_type,
		(!s_ctrl->io_master_info.qup_client) ? 0 :
			s_ctrl->io_master_info.qup_client->pm_ctrl_client_enable);

	rc =  cam_sensor_util_init_gpio_pin_tbl(soc_info,
			&sensordata->power_info.gpio_num_info);
	if (rc < 0) {
		CAM_ERR(CAM_SENSOR, "Failed to read gpios %d", rc);
		goto release_resources;
	}

	s_ctrl->id = soc_info->index;

	/* Validate cell_id */
	if (s_ctrl->id >= MAX_CAMERAS) {
		CAM_ERR(CAM_SENSOR, "Failed invalid cell_id %d", s_ctrl->id);
		rc = -EINVAL;
		goto release_resources;
	}

	/* Store the index of BoB regulator if it is available */
	for (i = 0; i < soc_info->num_rgltr; i++) {
		if (!strcmp(soc_info->rgltr_name[i],
			"cam_bob")) {
			CAM_DBG(CAM_SENSOR,
				"i: %d cam_bob", i);
			s_ctrl->bob_reg_index = i;
			soc_info->rgltr[i] = devm_regulator_get(soc_info->dev,
				soc_info->rgltr_name[i]);
			if (IS_ERR_OR_NULL(soc_info->rgltr[i])) {
				CAM_WARN(CAM_SENSOR,
					"Regulator: %s get failed",
					soc_info->rgltr_name[i]);
				soc_info->rgltr[i] = NULL;
			} else {
				if (!of_property_read_bool(of_node,
					"pwm-switch")) {
					CAM_DBG(CAM_SENSOR,
					"No BoB PWM switch param defined");
					s_ctrl->bob_pwm_switch = false;
				} else {
					s_ctrl->bob_pwm_switch = true;
				}
			}
		}
	}

	/* Read subdev info */
	rc = cam_sensor_get_sub_module_index(of_node, sensordata);
	if (rc < 0) {
		CAM_ERR(CAM_SENSOR, "failed to get sub module index, rc=%d",
			 rc);
		goto release_resources;
	}

	if (of_property_read_u32(of_node, "sensor-position-pitch",
		&sensordata->pos_pitch) < 0) {
		CAM_DBG(CAM_SENSOR, "Invalid sensor position");
		sensordata->pos_pitch = 360;
	}
	if (of_property_read_u32(of_node, "sensor-position-roll",
		&sensordata->pos_roll) < 0) {
		CAM_DBG(CAM_SENSOR, "Invalid sensor position");
		sensordata->pos_roll = 360;
	}
	if (of_property_read_u32(of_node, "sensor-position-yaw",
		&sensordata->pos_yaw) < 0) {
		CAM_DBG(CAM_SENSOR, "Invalid sensor position");
		sensordata->pos_yaw = 360;
	}

	if (of_property_read_u32(of_node, "aon-camera-id", &s_ctrl->aon_camera_id)) {
		CAM_DBG(CAM_SENSOR, "cell_idx: %d is not used for AON usecase", soc_info->index);
		s_ctrl->aon_camera_id = NOT_AON_CAM;
	} else {
		CAM_INFO(CAM_SENSOR,
			"AON Sensor detected in cell_idx: %d aon_camera_id: %d phy_index: %d",
			soc_info->index, s_ctrl->aon_camera_id,
			s_ctrl->sensordata->subdev_id[SUB_MODULE_CSIPHY]);
		if ((s_ctrl->sensordata->subdev_id[SUB_MODULE_CSIPHY] == 2) &&
			(s_ctrl->aon_camera_id != AON_CAM2)) {
			CAM_ERR(CAM_SENSOR, "Incorrect AON camera id for cphy_index %d",
				s_ctrl->sensordata->subdev_id[SUB_MODULE_CSIPHY]);
			s_ctrl->aon_camera_id = NOT_AON_CAM;
		}
		if ((s_ctrl->sensordata->subdev_id[SUB_MODULE_CSIPHY] == 4) &&
			(s_ctrl->aon_camera_id != AON_CAM1)) {
			CAM_ERR(CAM_SENSOR, "Incorrect AON camera id for cphy_index %d",
				s_ctrl->sensordata->subdev_id[SUB_MODULE_CSIPHY]);
			s_ctrl->aon_camera_id = NOT_AON_CAM;
		}
	}

	rc = cam_sensor_util_aon_registration(
		s_ctrl->sensordata->subdev_id[SUB_MODULE_CSIPHY],
		s_ctrl->aon_camera_id);
	if (rc) {
		CAM_ERR(CAM_SENSOR, "Aon registration failed, rc: %d", rc);
		goto release_resources;
	}

	if (!of_property_read_bool(of_node, "hw-no-ops"))
		s_ctrl->hw_no_ops = false;
	else
		s_ctrl->hw_no_ops = true;

	/* Initialize mutex */
	mutex_init(&(s_ctrl->cam_sensor_mutex));
	mutex_init(&(s_ctrl->read_buf_lock));
	INIT_LIST_HEAD(&(s_ctrl->read_buf_list));

	if (s_ctrl->io_master_info.master_type == CCI_MASTER) {
		init_power_sync_mutex(s_ctrl->io_master_info.cci_client, s_ctrl->io_master_info.cci_client->cci_i2c_master);//xiaomi add
	} else if (s_ctrl->io_master_info.master_type == I2C_MASTER) {
		init_power_sync_mutex(s_ctrl->io_master_info.cci_client, CCI_MASTER_0);
	} else {
		CAM_ERR(CAM_SENSOR,
			"Invalid master / Master type Not supported : %d",
				s_ctrl->io_master_info.master_type);
	}

	return rc;

release_resources:
	cam_sensor_util_release_resources(&(s_ctrl->io_master_info), soc_info);

free_sensor_data:
	CAM_MEM_FREE(sensordata);
	s_ctrl->sensordata = NULL;

	return rc;
}

