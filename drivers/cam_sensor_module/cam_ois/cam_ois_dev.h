/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#ifndef _CAM_OIS_DEV_H_
#define _CAM_OIS_DEV_H_

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <media/v4l2-event.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ioctl.h>
#include <media/cam_sensor.h>
#include <cam_sensor_i2c.h>
#include <cam_sensor_spi.h>
#include <cam_sensor_io.h>
#include <cam_cci_dev.h>
#include <cam_req_mgr_util.h>
#include <cam_req_mgr_interface.h>
#include <cam_mem_mgr.h>
#include <cam_subdev.h>
#include "cam_soc_util.h"
#include "cam_context.h"
#include "cam_parklens_thread.h" //xiaomi add
/* xiaomi add begin */
#include  "xm_cam_dev_protection.h"
/* xiaomi add end */

#define DEFINE_MSM_MUTEX(mutexname) \
	static struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

#define OIS_DRIVER_I2C "cam-i2c-ois"
#define OIS_DRIVER_I3C "i3c_camera_ois"
#define MAX_FW_COUNT   5 //xiaomi add
enum cam_ois_state {
	CAM_OIS_INIT,
	CAM_OIS_ACQUIRE,
	CAM_OIS_CONFIG,
	CAM_OIS_START,
};

/**
 * struct cam_ois_i2c_info_t - I2C info
 * @slave_addr      :   slave address
 * @i2c_freq_mode   :   i2c frequency mode
 *
 */
struct cam_ois_i2c_info_t {
	uint16_t slave_addr;
	uint8_t i2c_freq_mode;
};

/**
 * struct cam_ois_soc_private - ois soc private data structure
 * @ois_name        :   ois name
 * @i2c_info        :   i2c info structure
 * @power_info      :   ois power info
 *
 */
struct cam_ois_soc_private {
	const char *ois_name;
	struct cam_ois_i2c_info_t i2c_info;
	struct cam_sensor_power_ctrl_t power_info;
};


/* xiaomi add for fw store start */
struct fw_store_info
{
	int  fw_count;
	char *fw_name[MAX_FW_COUNT];
	struct firmware *fw[MAX_FW_COUNT];
};
/* xiaomi add for fw store end */

/**
 * struct cam_ois_intf_params - bridge interface params
 * @device_hdl   : Device Handle
 * @session_hdl  : Session Handle
 * @ops          : KMD operations
 * @crm_cb       : Callback API pointers
 */
struct cam_ois_intf_params {
	int32_t device_hdl;
	int32_t session_hdl;
	int32_t link_hdl;
	struct cam_req_mgr_kmd_ops ops;
	struct cam_req_mgr_crm_cb *crm_cb;
};

/**
 * struct cam_ois_intf_params - bridge interface params
 * @version         :       version info
 *                          NOTE: if struct cam_cmd_ois_fw_param is updated,
 *                          version here needs to be updated too.
 * @reserved        :       reserved
 * @cmd_type        :       Explains type of command
 * @fw_count        :       firmware count
 * @endianness      :       endianness combo:
 *                          bit[3:0] firmware data's endianness
 *                          bit[7:4] endian type of input parameter to ois driver, say QTime
 * @fw_param        :       includes firmware parameters
 */
struct cam_ois_fw_info {
	__u32                           version;
	__u8                            reserved;
	__u8                            cmd_type;
	__u8                            fw_count;
	__u8                            endianness;
	struct cam_cmd_ois_fw_param*    fw_param;
} __attribute__((packed));


/**
 * struct cam_ois_ctrl_t - OIS ctrl private data
 * @device_name     :   ois device_name
 * @pdev            :   platform device
 * @ois_mutex       :   ois mutex
 * @soc_info        :   ois soc related info
 * @io_master_info  :   Information about the communication master
 * @v4l2_dev_str    :   V4L2 device structure
 * @bridge_intf     :   bridge interface params
 * @i2c_fwinit_data :   ois i2c firmware init settings
 * @i2c_init_data   :   ois i2c init settings
 * @i2c_mode_data   :   ois i2c mode settings
 * @i2c_time_data   :   ois i2c time write settings
 * @i2c_calib_data  :   ois i2c calib settings
 * @cam_ois_state   :   ois_device_state
 * @ois_fw_flag     :   flag for firmware download
 * @is_ois_calib    :   flag for Calibration data
 * @opcode          :   ois opcode
 * @device_name     :   Device name
 * @read_buf_list   :   ois register read cmd buffer handle list
 * @read_buf_lock   :   ois register read cmd buffer mutex
 *
 * @cci_debug       :   OIS debugfs info and entry
 */
struct cam_ois_ctrl_t {
	char device_name[CAM_CTX_DEV_NAME_MAX_LENGTH];
	struct platform_device *pdev;
	struct mutex ois_mutex;
	struct cam_hw_soc_info soc_info;
	struct camera_io_master io_master_info;
	struct cam_subdev v4l2_dev_str;
	struct cam_ois_intf_params bridge_intf;
	struct i2c_settings_array i2c_fwinit_data;
	struct i2c_settings_array i2c_init_data;
	struct i2c_settings_array i2c_calib_data;
	struct i2c_settings_array i2c_postinit_data; // xiaomi add
	struct i2c_settings_array i2c_motion_data; // xiaomi add
	struct i2c_settings_array i2c_mode_data;
	struct i2c_settings_array i2c_time_data;
	enum cam_ois_state cam_ois_state;
	char ois_name[64];//xiaomi change from 32 to 64
	uint8_t ois_fw_flag;
	uint8_t is_ois_calib;
	struct cam_ois_opcode opcode;
	struct cam_ois_fw_info fw_info;
	struct i2c_settings_array* i2c_fw_init_data;
	struct i2c_settings_array* i2c_fw_finalize_data;
	struct i2c_settings_array i2c_fw_version_data;
	struct list_head read_buf_list;
	struct mutex read_buf_lock;

	struct fw_store_info fw_store; //xiaomi add
	/* xiaomi add for cci debug */
	void *cci_debug;
	/* xiaomi add for cci debug */
	struct cam_ois_parklens_ctrl_t parklens_ctrl; //xiaomi add
	struct i2c_settings_array i2c_parklens_data;  // xiaomi add
	/* xiaomi add begin */
	struct xm_cam_dev_info xm_cam_dev_info_data;
	/* xiaomi add end */
};

/**
 * @brief : API to register OIS hw to platform framework.
 * @return struct platform_device pointer on on success, or ERR_PTR() on error.
 */
int cam_ois_driver_init(void);

/**
 * @brief : API to remove OIS Hw from platform framework.
 */
void cam_ois_driver_exit(void);

/* xiaomi dev protection add*/
struct xm_cam_dev_info* get_ois_xm_cam_dev_info(struct cam_ois_ctrl_t *o_ctrl);
/* xiaomi dev protection add*/
#endif /*_CAM_OIS_DEV_H_ */
