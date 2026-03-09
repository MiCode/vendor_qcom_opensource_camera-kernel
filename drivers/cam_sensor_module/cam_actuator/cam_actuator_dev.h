/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2019, 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, 2024-2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */


#ifndef _CAM_ACTUATOR_DEV_H_
#define _CAM_ACTUATOR_DEV_H_

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
#include "cam_parklens_thread.h" //xiaomi add
/* xiaomi dev protection add*/
#include  "xm_cam_dev_protection.h"
/* xiaomi dev protection add*/

#define NUM_MASTERS 2
#define NUM_QUEUES 2

#define ACTUATOR_DRIVER_I2C                    "cam-i2c-actuator"
#define CAMX_ACTUATOR_DEV_NAME                 "cam-actuator-driver"
#define ACTUATOR_DRIVER_I3C                    "i3c_camera_actuator"


#define MSM_ACTUATOR_MAX_VREGS (10)
#define ACTUATOR_MAX_POLL_COUNT 10

enum cam_actuator_apply_state_t {
	ACT_APPLY_SETTINGS_NOW,
	ACT_APPLY_SETTINGS_LATER,
};

enum cam_actuator_state {
	CAM_ACTUATOR_INIT,
	CAM_ACTUATOR_ACQUIRE,
	CAM_ACTUATOR_CONFIG,
	CAM_ACTUATOR_START,
	CAM_ACTUATOR_PARKLENS, //xiaomi add
};

/**
 * struct cam_actuator_i2c_info_t - I2C info
 * @slave_addr      :   slave address
 * @i2c_freq_mode   :   i2c frequency mode
 */
struct cam_actuator_i2c_info_t {
	uint16_t slave_addr;
	uint8_t i2c_freq_mode;
};

struct cam_actuator_soc_private {
	struct cam_actuator_i2c_info_t i2c_info;
	struct cam_sensor_power_ctrl_t power_info;
};

/**
 * struct actuator_intf_params
 * @device_hdl: Device Handle
 * @session_hdl: Session Handle
 * @ops: KMD operations
 * @crm_cb: Callback API pointers
 */
struct actuator_intf_params {
	int32_t device_hdl;
	int32_t session_hdl;
	int32_t link_hdl;
	struct cam_req_mgr_kmd_ops ops;
	struct cam_req_mgr_crm_cb *crm_cb;
};

/**
 * struct cam_actuator_ctrl_t
 * @device_name           : Device name
 * @i2c_driver            : I2C device info
 * @pdev                  : Platform device
 * @io_master_info        : Information about the communication master
 * @actuator_mutex        : Actuator mutex
 * @act_apply_state       : Actuator settings aRegulator config
 * @id                    : Cell Index
 * @res_apply_state       : Actuator settings apply state
 * @cam_act_state         : Actuator state
 * @gconf                 : GPIO config
 * @pinctrl_info          : Pinctrl information
 * @v4l2_dev_str          : V4L2 device structure
 * @i2c_data              : I2C register settings structure
 * @act_info              : Sensor query cap structure
 * @of_node               : Node ptr
 * @last_flush_req        : Last request to flush
 * @workq                 : work queue for actuator
 * @actuator_park_mutex   : Mutex for actuator park
 * @cam_act_park_state    : Actuator park state
 * @is_deferred_park_lens : Flag to specify deferred park lens
 * @park_lens_complete    : Indicator for park lens complete
 * @read_buf_list         : Actuator register read cmd buffer handle list
 * @read_buf_lock         : Actuator register read cmd buffer mutex
 * @cci_debug: Sensor debugfs info and entry
 */
struct cam_actuator_ctrl_t {
	char device_name[CAM_CTX_DEV_NAME_MAX_LENGTH];
	struct i2c_driver *i2c_driver;
	struct camera_io_master io_master_info;
	struct cam_hw_soc_info soc_info;
	struct mutex actuator_mutex;
	uint32_t id;
	enum cam_actuator_apply_state_t setting_apply_state;
	enum cam_actuator_state cam_act_state;
	uint8_t cam_pinctrl_status;
	struct cam_subdev v4l2_dev_str;
	struct i2c_data_settings i2c_data;
	struct cam_actuator_query_cap act_info;
	struct actuator_intf_params bridge_intf;
	uint32_t last_flush_req;
	struct cam_req_mgr_core_workq *workq;
	struct mutex actuator_park_mutex;
	bool is_deferred_park_lens;
	struct completion park_lens_complete;
	struct list_head read_buf_list;
	struct mutex read_buf_lock;
	/* xiaomi add for cci debug */
	void *cci_debug;
	struct cam_actuator_parklens_ctrl_t parklens_ctrl;
	/* xiaomi add for cci debug */
	/* xiaomi dev protection add*/
	struct xm_cam_dev_info xm_cam_dev_info_data;
	/* xiaomi dev protection add*/
};

/**
 * @brief : API to register Actuator hw to platform framework.
 * @return struct platform_device pointer on on success, or ERR_PTR() on error.
 */
int cam_actuator_driver_init(void);

/**
 * @brief : API to remove Actuator Hw from platform framework.
 */
void cam_actuator_driver_exit(void);

/* xiaomi dev protection add*/
struct xm_cam_dev_info* get_actuator_xm_cam_dev_info(struct cam_actuator_ctrl_t *a_ctrl);
/* xiaomi dev protection add*/
#endif /* _CAM_ACTUATOR_DEV_H_ */
