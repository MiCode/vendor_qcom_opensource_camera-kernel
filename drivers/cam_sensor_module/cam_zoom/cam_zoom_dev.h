/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2019, 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */


#ifndef _CAM_ZOOM_DEV_H_
#define _CAM_ZOOM_DEV_H_

#include <cam_sensor_io.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/iommu.h>
#include <linux/timer.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-subdev.h>
#include <cam_cci_dev.h>
#include <cam_sensor_cmn_header.h>
#include <cam_subdev.h>
#include "cam_sensor_util.h"
#include "cam_soc_util.h"
#include "cam_debug_util.h"
#include "cam_context.h"
/* xiaomi add begin */
#include  "xm_cam_dev_protection.h"
/* xiaomi add end */

#define NUM_MASTERS 2
#define NUM_QUEUES 2

#define ZOOM_DRIVER_I2C                    "cam-i2c-zoom"
#define CAMX_ZOOM_DEV_NAME                 "cam-zoom-driver"
#define ZOOM_DRIVER_I3C                    "i3c_camera_zoom"

#define MSM_ZOOM_MAX_VREGS (10)
#define ZOOM_MAX_POLL_COUNT 10
#define ZOOM_NAME_LEN 64

enum cam_zoom_apply_state_t {
	ZOOM_APPLY_SETTINGS_NOW,
	ZOOM_APPLY_SETTINGS_LATER,
};

enum cam_zoom_state {
	CAM_ZOOM_INIT,
	CAM_ZOOM_ACQUIRE,
	CAM_ZOOM_CONFIG,
	CAM_ZOOM_START,
};

/**
 * struct cam_zoom_i2c_info_t - I2C info
 * @slave_addr      :   slave address
 * @i2c_freq_mode   :   i2c frequency mode
 */
struct cam_zoom_i2c_info_t {
	uint16_t slave_addr;
	uint8_t i2c_freq_mode;
};

struct cam_zoom_soc_private {
	struct cam_zoom_i2c_info_t i2c_info;
	struct cam_sensor_power_ctrl_t power_info;
};

/**
 * struct zoom_intf_params
 * @device_hdl: Device Handle
 * @session_hdl: Session Handle
 * @ops: KMD operations
 * @crm_cb: Callback API pointers
 */
struct zoom_intf_params {
	int32_t device_hdl;
	int32_t session_hdl;
	int32_t link_hdl;
	struct cam_req_mgr_kmd_ops ops;
	struct cam_req_mgr_crm_cb *crm_cb;
};

/**
 * struct cam_zoom_ctrl_t
 * @device_name: Device name
 * @i2c_driver: I2C device info
 * @pdev: Platform device
 * @cci_i2c_master: I2C structure
 * @io_master_info: Information about the communication master
 * @zoom_mutex: Zoom mutex
 * @is_i3c_device : A Flag to indicate whether this zoom is I3C device
 * @zoom_apply_state: Zoom settings aRegulator config
 * @id: Cell Index
 * @res_apply_state: Zoom settings apply state
 * @cam_zoom_state:   Zoom state
 * @gconf: GPIO config
 * @pinctrl_info: Pinctrl information
 * @v4l2_dev_str: V4L2 device structure
 * @i2c_data: I2C register settings structure
 * @zoom_info: Sensor query cap structure
 * @of_node: Node ptr
 * @last_flush_req: Last request to flush
 * @cci_debug: Sensor debugfs info and entry
 */
struct cam_zoom_ctrl_t {
	char device_name[CAM_CTX_DEV_NAME_MAX_LENGTH];
	struct platform_device *pdev;
	struct i2c_driver *i2c_driver;
	enum cci_i2c_master_t cci_i2c_master;
	struct camera_io_master io_master_info;
	struct cam_hw_soc_info soc_info;
	struct mutex zoom_mutex;
	uint32_t id;
	enum cam_zoom_apply_state_t setting_apply_state;
	enum cam_zoom_state cam_zoom_state;
	uint8_t cam_pinctrl_status;
	struct cam_subdev v4l2_dev_str;
	struct i2c_data_settings i2c_data;
	struct cam_zoom_query_cap zoom_info;
	struct zoom_intf_params bridge_intf;
	uint32_t last_flush_req;
	/* xiaomi add for cci debug start */
	char zoom_name[ZOOM_NAME_LEN];
	uint32_t zoom_3polefw_version;
	uint32_t zoom_5polefw_version;
	uint32_t zoom_hw_version;
	uint8_t  noupdate_fw_flash;
	void *cci_debug;
	/* xiaomi add for cci debug end */
	/* xiaomi add begin */
	struct xm_cam_dev_info xm_cam_dev_info_data;
	/* xiaomi add end */
};

/**
 * @brief : API to register Zoom hw to platform framework.
 * @return struct platform_device pointer on on success, or ERR_PTR() on error.
 */
int cam_zoom_driver_init(void);

/**
 * @brief : API to remove Zoom Hw from platform framework.
 */
void cam_zoom_driver_exit(void);

/* xiaomi dev protection add*/
struct xm_cam_dev_info* get_zoom_xm_cam_dev_info(struct cam_zoom_ctrl_t *z_ctrl);
/* xiaomi dev protection add*/

#endif /* _CAM_ZOOM_DEV_H_ */
