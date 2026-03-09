/* SPDX-License-Identifier: GPL-2.0 */
/*
 * xm_cam_dev_protection.h
 *
 * This header provides definitions for Xiaomi Camera Device Protection.
 *
 * Copyright (c) 2025 Xiaomi Technologies Co., Ltd.
 *
 * Author: zhengjiacheng@xiaomi.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <https://www.gnu.org/licenses/gpl-2.0.html>.
 */
#ifndef _XM_CAM_DEV_PROTECTION_H_
#define _XM_CAM_DEV_PROTECTION_H_

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/time64.h>
#include <linux/atomic.h>

#include "cam_sensor_cmn_header.h"

// xm_cam_dev_status queue size define
#define XM_CAM_DEV_STATUS_QUEUE_SIZE 10

#define XM_CAM_DEV_XXSKIP_SENSOR_OP_CODE (15856371)

// xm_cam_dev_operation define
#define XM_CAM_DEV_OPERATION_INVAILD 0
#define XM_CAM_DEV_OPERATION_INIT 1
#define XM_CAM_DEV_OPERATION_UPDATE 2

// xm_cam_dev_status_code code define
#define XM_CAM_DEV_STATUS_CODE_INVAILD				0 /* invalid status code */
#define XM_CAM_DEV_STATUS_CODE_I2C_ERROR			1 /* i2c error */
#define XM_CAM_DEV_STATUS_CODE_I2C_WRITE_ERROR		2 /* i2c write error */
#define XM_CAM_DEV_STATUS_CODE_I2C_READ_ERROR		3 /* i2c read error */
#define XM_CAM_DEV_STATUS_CODE_I2C_POLL_ERROR		4 /* i2c poll error */
#define XM_CAM_DEV_STATUS_CODE_I2C_WRITE_SUCCESS	5 /* i2c write success */

// xm_cam_dev_type
enum xm_cam_dev_type
{
	XM_CAM_DEV_TYPE_INVAILD = 0,
	XM_CAM_DEV_TYPE_ACTUATOR,
	XM_CAM_DEV_TYPE_OIS,
	XM_CAM_DEV_TYPE_ZOOM,
	XM_CAM_DEV_TYPE_MAX,
};

// xm_cam_dev_protection_type
enum xm_cam_dev_protection_type
{
	XM_CAM_DEV_PROTECTION_TYPE_INVAILD = 0,
	XM_CAM_DEV_PROTECTION_TYPE_I2C,
};

// xm_cam_dev_init_status
#define XM_CAM_DEV_INIT_STATUS_INVALID 0
#define XM_CAM_DEV_INIT_STATUS_SUCCESS 1
#define XM_CAM_DEV_INIT_STATUS_FAILURE 2

// xm_cam_dev_i2c_cmd_type
enum xm_cam_dev_i2c_cmd_type
{
	XM_CMD_DEV_I2C_CMD_TYPE_DEFAULT = 0,
	XM_CMD_DEV_I2C_ACTUATOR_CMD_TYPE_INIT,
	XM_CMD_DEV_I2C_ACTUATOR_CMD_TYPE_CONFIG,
	XM_CMD_DEV_I2C_ACTUATOR_CMD_TYPE_CRM_CONFIG,
	XM_CMD_DEV_I2C_ACTUATOR_CMD_TYPE_PARK_LENS,
	XM_CMD_DEV_I2C_ACTUATOR_CMD_TYPE_READ,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_INIT,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_CONFIG,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_WRITE_TIME,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_CALIBRATION,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_POSTINIT,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_MOTION,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_OIS_CONTROL,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_PARK_LENS,
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_READ,
	XM_CMD_DEV_I2C_ZOOM_CMD_TYPE_CRM_CONGIG,
	XM_CMD_DEV_I2C_ZOOM_CMD_TYPE_INIT,
	XM_CAM_DEV_I2C_STATUS_CODE_MAX
};


// bool result define
#define XM_CAM_DEV_RESULT_TRUE 1
#define XM_CAM_DEV_RESULT_FALSE 0


/**
 * struct xm_cam_dev_i2c_cmd_info - This structure is used to store I2C command information
 * @is_valid: is valid
 * @has_write_cmd: has write command
 * @has_read_cmd: has read command
 * @has_poll_cmd: has poll command
 */
struct xm_cam_dev_i2c_cmd_info
{
	bool is_valid;
	bool has_write_cmd;
	bool has_read_cmd;
	bool has_poll_cmd;
};

/**
 * struct xm_cam_dev_status - This structure is used to store camera device status
 * @xm_cam_dev_operation: camera device flag
 * @xm_cam_dev_time_stamp: camera device status time stamp
 * @xm_cam_dev_status_code: camera device status
 */
struct xm_cam_dev_status
{
	uint32_t xm_cam_dev_operation;
	struct timespec64 xm_cam_dev_time_stamp;
	uint32_t xm_cam_dev_status_code;
};

/**
 * struct xm_cam_dev_info - This structure is used to store camera device information
 * @xm_cam_dev_mutex: camera device mutex
 * @xm_cam_dev_status: camera device status
 * @xm_cam_dev_mutex_init_flag: xm_cam_dev_mutex init flag
 * @untrusted_status_flag: untrusted status flag
 * @status_reset_flag: status reset flag
 * @init_status: init status
 * @xm_cam_dev_init_flag: init flag
 */
struct xm_cam_dev_info
{
	struct mutex xm_cam_dev_mutex;
	struct xm_cam_dev_status xm_cam_dev_status[XM_CAM_DEV_STATUS_QUEUE_SIZE];
	enum xm_cam_dev_type dev_type;
	atomic_t xm_cam_dev_mutex_init_flag;
	atomic_t untrusted_status_flag;
	atomic_t status_reset_flag;
	atomic_t init_status;
	uint32_t xm_cam_dev_init_flag;
};

/**
 * xm_cam_dev_probe_skip_switch_enable - Check if probe skip switch is enabled
 */
uint32_t xm_cam_dev_probe_skip_switch_enable(void);

/**
 * xm_cam_dev_probe_skip_enable - Check if probe skip is enabled
 * @camera_id: camera id
 */
uint32_t xm_cam_dev_probe_skip_enable(uint32_t camera_id);

/**
 * xm_cam_dev_probe_skip_clear - Check if probe skip is clear
 */
bool xm_cam_dev_probe_skip_clear(void);

/**
 * xm_cam_dev_protection_enable - Check if protection is enabled
 * @dev_type: device type
 * @protection_type: protection type
 *
 * Return: true if protection is enabled, false otherwise
 */
bool xm_cam_dev_protection_enable(enum xm_cam_dev_type dev_type,
								  enum xm_cam_dev_protection_type protection_type);

/**
 * xm_cam_dev_protection_enable2 - Check if protection is enabled
 * @dev_type: device type
 * @protection_type: protection type
 *
 * Return: true if protection is enabled, false otherwise
 */
uint32_t xm_cam_dev_protection_enable2(enum xm_cam_dev_type dev_type,
								       enum xm_cam_dev_protection_type protection_type);

/**
 * xm_cam_dev_timespec64_zero - Check if timespec64 is zero
 * @a: pointer to timespec64 structure
 * Return: XM_CAM_DEV_RESULT_TRUE if zero, XM_CAM_DEV_RESULT_FALSE if not, negative error code on failure
 */
int32_t xm_cam_dev_timespec64_zero(const struct timespec64 *a);

/**
 * timespec64_diff - Calculate difference between two timespec64 values
 * @end_time: pointer to end time
 * @start_time: pointer to start time
 * @result: pointer to store the result
 * Return: 0 on success, negative error code on failure
 */
int32_t timespec64_diff(const struct timespec64 *end_time,
                    const struct timespec64 *start_time,
                    struct timespec64 *result);

/**
 * xm_cam_dev_reset_dev_info - This function is used to reset camera device information
 * @info: pointer to camera device info structure
 */
void xm_cam_dev_reset_dev_info(struct xm_cam_dev_info *info);

/**
 * xm_cam_dev_init_dev_info - This function is used to initialize camera device information
 * @info: pointer to camera device info structure
 * @dev_type: device type
 */
void xm_cam_dev_init_dev_info(struct xm_cam_dev_info *info, enum xm_cam_dev_type dev_type);

/**
 * xm_cam_dev_destroy_dev_info - This function is used to destroy camera device information
 * @info: pointer to camera device info structure
 */
void xm_cam_dev_destroy_dev_info(struct xm_cam_dev_info *info);

/**
 * xm_cam_dev_set_status_info - This function is used to set camera device status information
 * @info: pointer to camera device info structure
 * @operation: camera device operation
 * @status_code: camera device status code
 * Return: 0 on success, negative error code on failure
 */
int32_t xm_cam_dev_set_status_info(struct xm_cam_dev_info *info, uint32_t operation, uint32_t status_code);

/**
 * xm_cam_dev_get_failure_rate_time_interval - This function calculates failure rate within time interval
 * @info: pointer to camera device info structure
 * @time_interval: time interval to calculate failure rate for
 * @status_code: status code to calculate failure rate for
 * @total_events: pointer to store the total events
 * @failure_rate: pointer to store the calculated failure rate percentage (0-100)
 * Return: 0 on success, negative error code on failure
 */
int32_t xm_cam_dev_get_failure_rate_time_interval(struct xm_cam_dev_info *info,
												  struct timespec64 time_interval,
												  uint32_t status_code,
												  uint32_t *total_events,
												  uint32_t *failure_rate);

/**
 * xm_cam_dev_safe_mutex_lock - Safe mutex lock with timeout and deadlock detection
 * @info: pointer to camera device info structure
 * Return: 0 on success, negative error code on failure
 */
int32_t xm_cam_dev_safe_mutex_lock(struct xm_cam_dev_info *info);

/**
 * xm_cam_dev_safe_mutex_unlock - Safe mutex unlock with validation
 * @info: pointer to camera device info structure
 * Return: 0 on success, negative error code on failure
 */
int32_t xm_cam_dev_safe_mutex_unlock(struct xm_cam_dev_info *info);

/**
 * xm_cam_dev_is_device_damaged - Check if device is damaged based on failure rate
 * @info: pointer to camera device info structure
 * @status_code: status code to check failure rate for
 * @is_damaged: pointer to store the result
 * Return: 0 on success, negative error code on failure
 */
int32_t xm_cam_dev_is_device_damaged(struct xm_cam_dev_info *info,
                                     uint32_t status_code,
                                     uint32_t *is_damaged);

/**
 * xm_cam_dev_need_skip_i2c_operation - Check if I2C operation should be skipped
 * @info: pointer to camera device info structure
 * @settings_list: pointer to I2C settings list
 * Return: true if should skip, false otherwise
 */
uint32_t xm_cam_dev_need_skip_i2c_operation(struct xm_cam_dev_info *info,
											struct i2c_settings_list *settings_list,
											enum xm_cam_dev_i2c_cmd_type i2c_cmd_type);

/**
 * xm_cam_dev_need_skip_i2c_operation2 - Check if I2C operation should be skipped
 * @info: pointer to camera device info structure
 * @i2c_set: pointer to I2C settings set
 * Return: true if should skip, false otherwise
 */
uint32_t xm_cam_dev_need_skip_i2c_operation2(struct xm_cam_dev_info *info,
											struct i2c_settings_array *i2c_set,
											enum xm_cam_dev_i2c_cmd_type i2c_cmd_type);

/**
 * xm_cam_dev_need_skip_i2c_operation3 - Check if I2C operation should be skipped
 * @info: pointer to camera device info structure
 * @i2c_set: pointer to I2C settings set
 * Return: true if should skip, false otherwise
 */
uint32_t xm_cam_dev_need_skip_i2c_operation3(struct xm_cam_dev_info *info,
											enum xm_cam_dev_i2c_cmd_type i2c_cmd_type);


/**
 * xm_cam_dev_need_change_i2c_rc - Check if I2C return code should be changed
 * @info: pointer to camera device info structure
 * Return: true if should change, false otherwise
 */
bool xm_cam_dev_need_change_i2c_rc(struct xm_cam_dev_info *info,
									enum xm_cam_dev_i2c_cmd_type i2c_cmd_type);

/**
 * xm_cam_dev_is_dev_init_failed - Check if device initialization failed
 * @info: pointer to camera device info structure
 * Return: true if initialization failed, false otherwise
 */
bool xm_cam_dev_is_dev_init_failed(struct xm_cam_dev_info *info);

/**
 * xm_cam_dev_get_i2c_cmd_info_by_list - Get I2C command information by settings list
 * @i2c_set: pointer to I2C settings list
 * Return: I2C command information
 */
void xm_cam_dev_get_i2c_cmd_info_by_list(struct i2c_settings_list *i2c_set, struct xm_cam_dev_i2c_cmd_info *i2c_cmd_info);

/**
 * xm_cam_dev_get_i2c_cmd_info - Get I2C command information
 * @i2c_set: pointer to I2C settings array
 * Return: I2C command information
 */
void xm_cam_dev_get_i2c_cmd_info(struct i2c_settings_array *i2c_set, struct xm_cam_dev_i2c_cmd_info *i2c_cmd_info);

/**
 * xm_cam_dev_has_i2c_write_cmd - Check if I2C write command
 * @i2c_cmd_info: pointer to I2C command information
 * Return: true if is I2C write command, false otherwise
 */
bool xm_cam_dev_has_i2c_write_cmd(struct i2c_settings_array *i2c_set);

/**
 * xm_cam_dev_has_i2c_read_cmd - Check if I2C read command
 * @i2c_cmd_info: pointer to I2C command information
 * Return: true if is I2C read command, false otherwise
 */
bool xm_cam_dev_has_i2c_read_cmd(struct i2c_settings_array *i2c_set);

/**
 * xm_cam_dev_has_i2c_poll_cmd - Check if I2C poll command
 * @i2c_cmd_info: pointer to I2C command information
 * Return: true if is I2C poll command, false otherwise
 */
bool xm_cam_dev_has_i2c_poll_cmd(struct i2c_settings_array *i2c_set);

/**
 * xm_cam_dev_is_i2c_write_cmd - Check if I2C write command
 * @i2c_cmd_info: pointer to I2C command information
 * Return: true if is I2C write command, false otherwise
 */
bool xm_cam_dev_is_i2c_write_cmd(struct i2c_settings_list *i2c_set);

/**
 * xm_cam_dev_is_i2c_read_cmd - Check if I2C read command
 * @i2c_cmd_info: pointer to I2C command information
 * Return: true if is I2C read command, false otherwise
 */
bool xm_cam_dev_is_i2c_read_cmd(struct i2c_settings_list *i2c_set);

/**
 * xm_cam_dev_is_i2c_poll_cmd - Check if I2C poll command
 * @i2c_cmd_info: pointer to I2C command information
 * Return: true if is I2C poll command, false otherwise
 */
bool xm_cam_dev_is_i2c_poll_cmd(struct i2c_settings_list *i2c_set);

/**
 * xm_cam_dev_is_need_protection_i2c_cmd - Check if I2C command needs protection
 * @info: pointer to camera device info structure
 * @i2c_cmd_type: I2C command type
 * Return: true if needs protection, false otherwise
 */
bool xm_cam_dev_is_need_protection_i2c_cmd(struct xm_cam_dev_info *info, enum xm_cam_dev_i2c_cmd_type i2c_cmd_type);

/**
 * xm_cam_dev_set_init_result - Set initialization result
 * @info: pointer to camera device info structure
 * @result: initialization result
 */
void xm_cam_dev_set_init_result(struct xm_cam_dev_info *info, int32_t result);

#endif  /* _XM_CAM_DEV_PROTECTION_H_ */