/* SPDX-License-Identifier: GPL-2.0 */
/*
 * xm_cam_dev_protection.c
 *
 * This file provides definitions for Xiaomi Camera Device Protection.
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


#include "xm_cam_dev_protection.h"

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/ktime.h>
#include <linux/module.h>

#include "cam_debug_util.h"


// return code define
#define XM_CAM_DEV_SUCCESS		 0	/* No error */
#define	XM_CAM_DEV_EPERM		 1	/* Operation not permitted */
#define	XM_CAM_DEV_ENOENT		 2	/* No such file or directory */
#define	XM_CAM_DEV_ESRCH		 3	/* No such process */
#define	XM_CAM_DEV_EINTR		 4	/* Interrupted system call */
#define	XM_CAM_DEV_EIO			 5	/* I/O error */
#define	XM_CAM_DEV_ENXIO		 6	/* No such device or address */
#define	XM_CAM_DEV_E2BIG		 7	/* Argument list too long */
#define	XM_CAM_DEV_ENOEXEC		 8	/* Exec format error */
#define	XM_CAM_DEV_EBADF		 9	/* Bad file number */
#define	XM_CAM_DEV_ECHILD		10	/* No child processes */
#define	XM_CAM_DEV_EAGAIN		11	/* Try again */
#define	XM_CAM_DEV_ENOMEM		12	/* Out of memory */
#define	XM_CAM_DEV_EACCES		13	/* Permission denied */
#define	XM_CAM_DEV_EFAULT		14	/* Bad address */
#define	XM_CAM_DEV_ENOTBLK		15	/* Block device required */
#define	XM_CAM_DEV_EBUSY		16	/* Device or resource busy */
#define	XM_CAM_DEV_EEXIST		17	/* File exists */
#define	XM_CAM_DEV_EXDEV		18	/* Cross-device link */
#define	XM_CAM_DEV_ENODEV		19	/* No such device */
#define	XM_CAM_DEV_ENOTDIR		20	/* Not a directory */
#define	XM_CAM_DEV_EISDIR		21	/* Is a directory */
#define	XM_CAM_DEV_EINVAL		22	/* Invalid argument */
#define	XM_CAM_DEV_ENFILE		23	/* File table overflow */
#define	XM_CAM_DEV_EMFILE		24	/* Too many open files */
#define	XM_CAM_DEV_ENOTTY		25	/* Not a typewriter */
#define	XM_CAM_DEV_ETXTBSY		26	/* Text file busy */
#define	XM_CAM_DEV_EFBIG		27	/* File too large */
#define	XM_CAM_DEV_ENOSPC		28	/* No space left on device */
#define	XM_CAM_DEV_ESPIPE		29	/* Illegal seek */
#define	XM_CAM_DEV_EROFS		30	/* Read-only file system */
#define	XM_CAM_DEV_EMLINK		31	/* Too many links */
#define	XM_CAM_DEV_EPIPE		32	/* Broken pipe */
#define	XM_CAM_DEV_EDOM			33	/* Math argument out of domain of func */
#define	XM_CAM_DEV_ERANGE		34	/* Math result not representable */
#define XM_CAM_DEV_EOVERFLOW	35	/* Value too large to be stored in data type */

// magic number define to avoid hardcoded values
#define XM_CAM_DEV_UINT32_MAX (4294967295U)
#define XM_CAM_DEV_UINT32_MIN (0U)
#define XM_CAM_DEV_INT32_MAX (2147483647)
#define XM_CAM_DEV_INT32_MIN (-2147483648)
#define XM_CAM_DEV_INT64_MAX (9223372036854775807LL)
#define XM_CAM_DEV_INT64_MIN (-9223372036854775807LL - 1LL)
#define XM_CAM_DEV_UINT64_MAX (18446744073709551615ULL)
#define XM_CAM_DEV_UINT64_MIN (0ULL)
#define XM_CAM_DEV_NSEC_PER_SEC (1000000000L)

// compare result define
#define XM_CAM_DEV_COMPARE_BIGGER 1
#define XM_CAM_DEV_COMPARE_EQUAL 0
#define XM_CAM_DEV_COMPARE_SMALLER -1

// deadlock prevention defines
#define XM_CAM_DEV_MUTEX_RETRY_COUNT (1)    /* Maximum retry attempts */

// xm_cam_dev_mutex_init_status
#define XM_CAM_DEV_MUTEX_INIT_STATUS_INVALID (0)
#define XM_CAM_DEV_MUTEX_INIT_STATUS_INITIALIZING (1)
#define XM_CAM_DEV_MUTEX_INIT_STATUS_INITIALIZED (2)
#define XM_CAM_DEV_MUTEX_INIT_STATUS_DESTROYING (3)
#define XM_CAM_DEV_MUTEX_INIT_STATUS_DESTROYED (4)

// xm_cam_dev_protection_flag
#define XM_CAM_DEV_PROTECTION_FLAG_DISABLE (0x0)
#define XM_CAM_DEV_PROTECTION_FLAG_ENABLE (0xFF0F)

// xm_cam_dev_protection_type_flag
#define XM_CAM_DEV_PROTECTION_TYPE_FLAG_DISABLE (0x0)
#define XM_CAM_DEV_PROTECTION_TYPE_FLAG_ACTUATOR_ENABLE (0x1)
#define XM_CAM_DEV_PROTECTION_TYPE_FLAG_OIS_ENABLE (0x2)
#define XM_CAM_DEV_PROTECTION_TYPE_FLAG_ZOOM_ENABLE (0x4)
#define XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_ENABLE  (0x0100)
#define XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_SKIP_ENABLE (0x0200)
#define XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_CHANGE_RETURN (0x0400)


#define XM_CAM_DEV_XXSKIP_SENSOR_INTERNAL_OP_CODE (15856371)

static unsigned long long xm_cam_dev_protection_flag      = XM_CAM_DEV_PROTECTION_FLAG_DISABLE;
static unsigned long long xm_cam_dev_protection_type_flag = XM_CAM_DEV_PROTECTION_TYPE_FLAG_DISABLE;
static int xm_cam_dev_protection_threshold_count = 30;
static unsigned int xm_cam_dev_protection_threshold[30] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int xm_cam_dev_protection_disable_cmd_type_count = 30;
static unsigned int xm_cam_dev_protection_disable_cmd_type[30] = {
	XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static unsigned int xm_cam_dev_probe_skip_switch = 0;
static unsigned int xm_cam_dev_probe_skip_camera_id = 0;

module_param_array(xm_cam_dev_protection_threshold, uint, &xm_cam_dev_protection_threshold_count, 0644);
module_param(xm_cam_dev_protection_flag, ullong, 0644);
module_param(xm_cam_dev_protection_type_flag, ullong, 0644);
module_param_array(xm_cam_dev_protection_disable_cmd_type, uint, &xm_cam_dev_protection_disable_cmd_type_count, 0644);
module_param(xm_cam_dev_probe_skip_switch, uint, 0644);
module_param(xm_cam_dev_probe_skip_camera_id, uint, 0644);


// output functions
uint32_t xm_cam_dev_probe_skip_switch_enable(void)
{
	uint32_t result = 0;

	if (XM_CAM_DEV_PROTECTION_FLAG_ENABLE == xm_cam_dev_probe_skip_switch) {
		result = XM_CAM_DEV_XXSKIP_SENSOR_INTERNAL_OP_CODE;
	}

	return result;
}

// output functions
uint32_t xm_cam_dev_probe_skip_enable(uint32_t camera_id)
{
	uint32_t result = 0;

	if (XM_CAM_DEV_PROTECTION_FLAG_ENABLE != xm_cam_dev_probe_skip_switch) {
		return 0;
	}

	if (0xFF < camera_id)
	{
		return 0;
	}

	if (0xFF00 > xm_cam_dev_probe_skip_camera_id)
	{
		return 0;
	}

	if (0xFF00 != (0xFF00 & xm_cam_dev_probe_skip_camera_id))
	{
		return 0;
	}

	if (camera_id == (0xFF & xm_cam_dev_probe_skip_camera_id))
	{
		result = XM_CAM_DEV_XXSKIP_SENSOR_INTERNAL_OP_CODE;
	}

	return result;
}

bool xm_cam_dev_probe_skip_clear(void)
{
	bool result = false;

	xm_cam_dev_probe_skip_switch = 0;
	xm_cam_dev_probe_skip_camera_id = 0;

	if ((0 == xm_cam_dev_probe_skip_switch) &&
		(0 == xm_cam_dev_probe_skip_camera_id)) {
		result = true;
	}

	return result;
}

// output functions
bool xm_cam_dev_protection_enable(enum xm_cam_dev_type dev_type,
								  enum xm_cam_dev_protection_type protection_type)
{
	bool result = false;

	if (XM_CAM_DEV_PROTECTION_FLAG_ENABLE != xm_cam_dev_protection_flag) {
		return false;
	}

	if ((XM_CAM_DEV_TYPE_ACTUATOR == dev_type) &&
		(XM_CAM_DEV_PROTECTION_TYPE_I2C == protection_type) &&
		(XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_ENABLE == (xm_cam_dev_protection_type_flag & XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_ENABLE)) &&
		(XM_CAM_DEV_PROTECTION_TYPE_FLAG_ACTUATOR_ENABLE == (xm_cam_dev_protection_type_flag & XM_CAM_DEV_PROTECTION_TYPE_FLAG_ACTUATOR_ENABLE))) {
			result = true;
	}

	if ((XM_CAM_DEV_TYPE_OIS == dev_type) &&
		(XM_CAM_DEV_PROTECTION_TYPE_I2C == protection_type) &&
		(XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_ENABLE == (xm_cam_dev_protection_type_flag & XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_ENABLE)) &&
		(XM_CAM_DEV_PROTECTION_TYPE_FLAG_OIS_ENABLE == (xm_cam_dev_protection_type_flag & XM_CAM_DEV_PROTECTION_TYPE_FLAG_OIS_ENABLE))) {
			result = true;
	}

	if ((XM_CAM_DEV_TYPE_ZOOM == dev_type) &&
		(XM_CAM_DEV_PROTECTION_TYPE_I2C == protection_type) &&
		(XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_ENABLE == (xm_cam_dev_protection_type_flag & XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_ENABLE)) &&
		(XM_CAM_DEV_PROTECTION_TYPE_FLAG_ZOOM_ENABLE == (xm_cam_dev_protection_type_flag & XM_CAM_DEV_PROTECTION_TYPE_FLAG_ZOOM_ENABLE))) {
			result = true;
	}

	return result;
}


uint32_t xm_cam_dev_protection_enable2(enum xm_cam_dev_type dev_type,
								       enum xm_cam_dev_protection_type protection_type)
{
	uint32_t result = 0;

	if (XM_CAM_DEV_PROTECTION_FLAG_ENABLE != xm_cam_dev_protection_flag) {
		return 0;
	}

	if (true == xm_cam_dev_protection_enable(dev_type, protection_type)) {
		result = XM_CAM_DEV_XXSKIP_SENSOR_INTERNAL_OP_CODE;
	}

	return result;
}

int32_t xm_cam_dev_timespec64_zero(const struct timespec64 *a)
{
	if (NULL == a) {
		return -XM_CAM_DEV_EINVAL;
	}

	if ((a->tv_sec == 0) && (a->tv_nsec == 0)) {
		return XM_CAM_DEV_RESULT_TRUE;
	}

	return XM_CAM_DEV_RESULT_FALSE;
}


static inline bool xm_dev_timespec64_valid(const struct timespec64 *ts)
{
	if (!ts) {
		return false;
	}


	if ((0 > ts->tv_sec) || (0 > ts->tv_nsec) || (ts->tv_nsec >= NSEC_PER_SEC)) {
		return false;
	}

	return true;
}

static inline int32_t xm_dev_timespec64_compare(const struct timespec64 *lhs,
												const struct timespec64 *rhs)
{
	if ((NULL == lhs) || (NULL == rhs)) {
		return -XM_CAM_DEV_EINVAL;
	}

	if (!xm_dev_timespec64_valid(lhs) || !xm_dev_timespec64_valid(rhs)) {
		return -XM_CAM_DEV_EINVAL;
	}

	if (lhs->tv_sec < rhs->tv_sec) {
		return XM_CAM_DEV_COMPARE_SMALLER;
	}
	if (lhs->tv_sec > rhs->tv_sec) {
		return XM_CAM_DEV_COMPARE_BIGGER;
	}

	if (lhs->tv_nsec < rhs->tv_nsec) {
		return XM_CAM_DEV_COMPARE_SMALLER;
	}
	if (lhs->tv_nsec > rhs->tv_nsec) {
		return XM_CAM_DEV_COMPARE_BIGGER;
	}

	return XM_CAM_DEV_COMPARE_EQUAL;
}

int32_t timespec64_diff(const struct timespec64 *end_time,
						const struct timespec64 *start_time,
						struct timespec64 *result)
{
	if (NULL == result) {
		return -XM_CAM_DEV_EINVAL;
	}

	result->tv_sec = 0;
	result->tv_nsec = 0;

	if ((NULL == end_time) || (NULL == start_time)) {
		return -XM_CAM_DEV_EINVAL;
	}

	if (false == xm_dev_timespec64_valid(end_time) || false == xm_dev_timespec64_valid(start_time)){
		return -XM_CAM_DEV_EINVAL;
	}

	result->tv_sec = 0;
	result->tv_nsec = 0;

	int64_t sec_diff = end_time->tv_sec - start_time->tv_sec;
	long nsec_diff = 0;

	if (end_time->tv_nsec < start_time->tv_nsec) {
		if (sec_diff == XM_CAM_DEV_INT64_MIN) {
			return -XM_CAM_DEV_EOVERFLOW;
		}
		sec_diff -= 1;
		nsec_diff = end_time->tv_nsec + XM_CAM_DEV_NSEC_PER_SEC - start_time->tv_nsec;
	}


	result->tv_sec = sec_diff;
	result->tv_nsec = nsec_diff;

	if (false == xm_dev_timespec64_valid(result)) {
		return -XM_CAM_DEV_EINVAL;
	}

	return XM_CAM_DEV_SUCCESS;
}

void xm_cam_dev_reset_dev_info(struct xm_cam_dev_info *info)
{
	uint32_t  index = 0;
	int32_t   rc    = XM_CAM_DEV_SUCCESS;
	uint32_t  status_reset_count = 0;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args");
		return;
	}

	rc = xm_cam_dev_safe_mutex_lock(info);
	if (rc != XM_CAM_DEV_SUCCESS) {
		CAM_ERR(CAM_SENSOR_IO, "Failed to acquire mutex for reset, rc=%d", rc);
		atomic_set(&(info->untrusted_status_flag), 1);
		atomic_set(&(info->status_reset_flag), 0);
		return;
	}

	for (index = 0; index < XM_CAM_DEV_STATUS_QUEUE_SIZE; index++) {
		info->xm_cam_dev_status[index].xm_cam_dev_operation = XM_CAM_DEV_OPERATION_INVAILD;
		info->xm_cam_dev_status[index].xm_cam_dev_status_code = XM_CAM_DEV_STATUS_CODE_INVAILD;
		info->xm_cam_dev_status[index].xm_cam_dev_time_stamp.tv_sec = 0;
		info->xm_cam_dev_status[index].xm_cam_dev_time_stamp.tv_nsec = 0;
	}

	for (index = 0; index < XM_CAM_DEV_STATUS_QUEUE_SIZE; index++) {
		if ((XM_CAM_DEV_OPERATION_INVAILD == info->xm_cam_dev_status[index].xm_cam_dev_operation) &&
			(XM_CAM_DEV_STATUS_CODE_INVAILD == info->xm_cam_dev_status[index].xm_cam_dev_status_code) &&
			(0 == info->xm_cam_dev_status[index].xm_cam_dev_time_stamp.tv_sec) &&
			(0 == info->xm_cam_dev_status[index].xm_cam_dev_time_stamp.tv_nsec)) {
			status_reset_count++;
		}
	}

	atomic_set(&(info->init_status), XM_CAM_DEV_INIT_STATUS_INVALID);

	if (XM_CAM_DEV_STATUS_QUEUE_SIZE == status_reset_count &&
		XM_CAM_DEV_INIT_STATUS_INVALID == atomic_read(&(info->init_status))) {
		atomic_set(&(info->status_reset_flag), 1);
		atomic_set(&(info->untrusted_status_flag), 0);
	} else {
		atomic_set(&(info->untrusted_status_flag), 1);
		atomic_set(&(info->status_reset_flag), 0);
	}

	xm_cam_dev_safe_mutex_unlock(info);
}

void xm_cam_dev_init_dev_info(struct xm_cam_dev_info *info, enum xm_cam_dev_type dev_type)
{
	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return;
	}

	info->dev_type = dev_type;

	if (XM_CAM_DEV_MUTEX_INIT_STATUS_INVALID == atomic_read(&(info->xm_cam_dev_mutex_init_flag))) {
		atomic_set(&(info->xm_cam_dev_mutex_init_flag), XM_CAM_DEV_MUTEX_INIT_STATUS_INITIALIZING);
		mutex_init(&info->xm_cam_dev_mutex);
		atomic_set(&(info->xm_cam_dev_mutex_init_flag), XM_CAM_DEV_MUTEX_INIT_STATUS_INITIALIZED);
	}

	atomic_set(&(info->untrusted_status_flag), 0);
	atomic_set(&(info->status_reset_flag), 0);
	atomic_set(&(info->init_status), XM_CAM_DEV_INIT_STATUS_INVALID);
	info->xm_cam_dev_init_flag = 0;

	xm_cam_dev_reset_dev_info(info);
}

void xm_cam_dev_destroy_dev_info(struct xm_cam_dev_info *info)
{
	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return;
	}

	atomic_set(&(info->untrusted_status_flag), 0);
	atomic_set(&(info->status_reset_flag), 0);
	atomic_set(&(info->init_status), XM_CAM_DEV_INIT_STATUS_INVALID);
	info->xm_cam_dev_init_flag = 0;

	xm_cam_dev_reset_dev_info(info);

	if (XM_CAM_DEV_MUTEX_INIT_STATUS_INITIALIZED == atomic_read(&(info->xm_cam_dev_mutex_init_flag))) {
		atomic_set(&(info->xm_cam_dev_mutex_init_flag), XM_CAM_DEV_MUTEX_INIT_STATUS_DESTROYING);
		mutex_destroy(&info->xm_cam_dev_mutex);
		atomic_set(&(info->xm_cam_dev_mutex_init_flag), XM_CAM_DEV_MUTEX_INIT_STATUS_DESTROYED);
	} else {
		CAM_ERR(CAM_SENSOR_IO, "Mutex is not initialized, skip destroy");
	}
}

// output functions
int32_t xm_cam_dev_set_status_info(struct xm_cam_dev_info *info, uint32_t operation, uint32_t status_code)
{
	int32_t rc = XM_CAM_DEV_SUCCESS;

	uint32_t  index = 0;
	uint32_t  status_empty_index = XM_CAM_DEV_UINT32_MAX;
	uint32_t  oldest_status_index = 0;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return -XM_CAM_DEV_EINVAL;
	}

	if ((XM_CAM_DEV_OPERATION_INIT != operation) && (XM_CAM_DEV_OPERATION_UPDATE != operation)) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid operation: %u", operation);
		return -XM_CAM_DEV_EINVAL;
	}

	rc = xm_cam_dev_safe_mutex_lock(info);
	if (XM_CAM_DEV_SUCCESS != rc) {
		atomic_set(&(info->untrusted_status_flag), 1);
		CAM_ERR(CAM_SENSOR_IO, "Failed to acquire mutex safely, rc=%d", rc);
		return rc;
	}

	for (index = 0; index < XM_CAM_DEV_STATUS_QUEUE_SIZE; index++) {
		if ((false == xm_dev_timespec64_valid(&info->xm_cam_dev_status[index].xm_cam_dev_time_stamp)) ||
			(XM_CAM_DEV_OPERATION_INVAILD == info->xm_cam_dev_status[index].xm_cam_dev_operation) ||
			(XM_CAM_DEV_STATUS_CODE_INVAILD == info->xm_cam_dev_status[index].xm_cam_dev_status_code)) {
			info->xm_cam_dev_status[index].xm_cam_dev_operation = XM_CAM_DEV_OPERATION_INVAILD;
			info->xm_cam_dev_status[index].xm_cam_dev_status_code = XM_CAM_DEV_STATUS_CODE_INVAILD;
			info->xm_cam_dev_status[index].xm_cam_dev_time_stamp.tv_sec = 0;
			info->xm_cam_dev_status[index].xm_cam_dev_time_stamp.tv_nsec = 0;
		}
	}

	for (index = 0; index < XM_CAM_DEV_STATUS_QUEUE_SIZE; index++) {
		if ((XM_CAM_DEV_OPERATION_INVAILD == info->xm_cam_dev_status[index].xm_cam_dev_operation) && (XM_CAM_DEV_UINT32_MAX == status_empty_index)) {
			status_empty_index = index;
		}
		if (XM_CAM_DEV_OPERATION_INVAILD != info->xm_cam_dev_status[index].xm_cam_dev_operation) {
			if (XM_CAM_DEV_STATUS_QUEUE_SIZE > oldest_status_index) {
				if (xm_dev_timespec64_compare(&info->xm_cam_dev_status[index].xm_cam_dev_time_stamp,
											  &info->xm_cam_dev_status[oldest_status_index].xm_cam_dev_time_stamp) == XM_CAM_DEV_COMPARE_SMALLER) {
					oldest_status_index = index;
				}
			}
		}
	}

	if (XM_CAM_DEV_STATUS_QUEUE_SIZE > status_empty_index) {
		info->xm_cam_dev_status[status_empty_index].xm_cam_dev_operation = operation;
		info->xm_cam_dev_status[status_empty_index].xm_cam_dev_status_code = status_code;
		ktime_get_boottime_ts64(&(info->xm_cam_dev_status[status_empty_index].xm_cam_dev_time_stamp));
	} else if (XM_CAM_DEV_STATUS_QUEUE_SIZE > oldest_status_index) {
		info->xm_cam_dev_status[oldest_status_index].xm_cam_dev_operation = operation;
		info->xm_cam_dev_status[oldest_status_index].xm_cam_dev_status_code = status_code;
		ktime_get_boottime_ts64(&(info->xm_cam_dev_status[oldest_status_index].xm_cam_dev_time_stamp));
	} else {
		CAM_ERR(CAM_SENSOR_IO, "Failed to set status info");
		rc = -XM_CAM_DEV_EINVAL;
	}

	xm_cam_dev_safe_mutex_unlock(info);

	return rc;
}

int32_t xm_cam_dev_get_failure_rate_time_interval(struct xm_cam_dev_info *info,
												  struct timespec64 time_interval,
												  uint32_t status_code,
												  uint32_t *total_events,
												  uint32_t *failure_rate)
{
	int32_t rc = XM_CAM_DEV_SUCCESS;
	struct timespec64 time_now = {0, 0};
	struct timespec64 time_threshold = {0, 0};

	uint32_t failure_events = 0;
	uint32_t index = 0;
	uint32_t calculated_failure_rate = 0;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return -XM_CAM_DEV_EINVAL;
	}

	if (NULL == failure_rate || NULL == total_events) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: failure_rate or total_events is NULL");
		return -XM_CAM_DEV_EINVAL;
	}

	*total_events = 0;
	*failure_rate = 0;

	if (status_code == XM_CAM_DEV_STATUS_CODE_INVAILD) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid status_code: %u", status_code);
		return -XM_CAM_DEV_EINVAL;
	}

	if ((false == xm_dev_timespec64_valid(&time_interval)) ||
		(XM_CAM_DEV_RESULT_FALSE != xm_cam_dev_timespec64_zero(&time_interval))) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid time interval");
		return -XM_CAM_DEV_EINVAL;
	}

	rc = xm_cam_dev_safe_mutex_lock(info);
	if (rc != XM_CAM_DEV_SUCCESS) {
		atomic_set(&(info->untrusted_status_flag), 1);
		CAM_ERR(CAM_SENSOR_IO, "Failed to acquire mutex safely, rc=%d", rc);
		return rc;
	}

	ktime_get_boottime_ts64(&time_now);

	if (false == xm_dev_timespec64_valid(&time_now)) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid time now");
		xm_cam_dev_safe_mutex_unlock(info);
		return -XM_CAM_DEV_EINVAL;
	}

	if (time_now.tv_sec < time_interval.tv_sec ||
		(time_now.tv_sec == time_interval.tv_sec && time_now.tv_nsec < time_interval.tv_nsec)) {
		time_threshold.tv_sec = 0;
		time_threshold.tv_nsec = 0;
	} else {
		rc = timespec64_diff(&time_now, &time_interval, &time_threshold);
		if (rc != XM_CAM_DEV_SUCCESS) {
			CAM_ERR(CAM_SENSOR_IO, "Failed to calculate time threshold, rc=%d", rc);
			xm_cam_dev_safe_mutex_unlock(info);
			return rc;
		}
	}

	if (false == xm_dev_timespec64_valid(&time_threshold)) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid time threshold");
		xm_cam_dev_safe_mutex_unlock(info);
		return -XM_CAM_DEV_EINVAL;
	}

	for (index = 0; index < XM_CAM_DEV_STATUS_QUEUE_SIZE; index++) {
		if (info->xm_cam_dev_status[index].xm_cam_dev_operation == XM_CAM_DEV_OPERATION_INVAILD) {
			continue;
		}

		if (false == xm_dev_timespec64_valid(&info->xm_cam_dev_status[index].xm_cam_dev_time_stamp)) {
			continue;
		}

		int compare_result = xm_dev_timespec64_compare(&info->xm_cam_dev_status[index].xm_cam_dev_time_stamp,
													   &time_threshold);
		if (compare_result < 0) {
			CAM_ERR(CAM_SENSOR_IO, "Time comparison failed for index %u", index);
			continue;
		}

		if (compare_result >= XM_CAM_DEV_COMPARE_EQUAL) {
			if ((*total_events) == XM_CAM_DEV_UINT32_MAX) {
				CAM_ERR(CAM_SENSOR_IO, "Total events counter overflow");
				rc = -XM_CAM_DEV_EOVERFLOW;
				break;
			}
			(*total_events)++;

			if (info->xm_cam_dev_status[index].xm_cam_dev_status_code == status_code) {
				if (failure_events == XM_CAM_DEV_UINT32_MAX) {
					CAM_ERR(CAM_SENSOR_IO, "Failure events counter overflow");
					rc = -XM_CAM_DEV_EOVERFLOW;
					break;
				}
				failure_events++;
			}
		}
	}

	if (rc == XM_CAM_DEV_SUCCESS) {
		if ((*total_events) > 0) {
			if (failure_events > (XM_CAM_DEV_UINT32_MAX / 100)) {
				CAM_ERR(CAM_SENSOR_IO, "Integer overflow in failure rate calculation");
				rc = -XM_CAM_DEV_EOVERFLOW;
			} else {
				calculated_failure_rate = (failure_events * 100) / (*total_events);
				*failure_rate = calculated_failure_rate;
			}
		} else {
			*failure_rate = 0;
		}
	}

	xm_cam_dev_safe_mutex_unlock(info);

	if (rc == XM_CAM_DEV_SUCCESS) {
		CAM_DBG(CAM_SENSOR_IO, "Failure rate calculation: failure evnets: %u, total events: %u, failure rate: %u, in time interval %lld",
				failure_events, *total_events, *failure_rate, time_interval.tv_sec);
	} else {
		CAM_ERR(CAM_SENSOR_IO, "Failure rate calculation failed with error: %d", rc);
	}

	return rc;
}

int32_t xm_cam_dev_is_device_damaged(struct xm_cam_dev_info *info,
									 uint32_t status_code,
									 uint32_t *is_damaged)
{
	int32_t rc = XM_CAM_DEV_SUCCESS;
	struct timespec64 time_interval = {0, 0};
	uint32_t total_events = 0;
	uint32_t failure_rate = 0;

	int64_t temp_time_interval = 0;
	int64_t temp_total_events = 0;
	int64_t temp_failure_rate = 0;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return -XM_CAM_DEV_EINVAL;
	}

	if (NULL == is_damaged) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: is_damaged is NULL");
		return -XM_CAM_DEV_EINVAL;
	}

	*is_damaged = XM_CAM_DEV_RESULT_FALSE;

	if (false == xm_cam_dev_protection_enable(info->dev_type, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		CAM_DBG(CAM_SENSOR_IO, "Protection flag is 0, skip protection");
		*is_damaged = XM_CAM_DEV_RESULT_FALSE;
		return XM_CAM_DEV_SUCCESS;
	}

	if ((XM_CAM_DEV_TYPE_MAX <= info->dev_type) || (XM_CAM_DEV_TYPE_INVAILD >= info->dev_type)) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid device type: %d", info->dev_type);
		return -XM_CAM_DEV_EINVAL;
	}

	temp_total_events = xm_cam_dev_protection_threshold[info->dev_type * 3];
	temp_failure_rate = xm_cam_dev_protection_threshold[info->dev_type * 3 + 1];
	temp_time_interval = xm_cam_dev_protection_threshold[info->dev_type * 3 + 2];

	if ((0 >= temp_total_events) || (XM_CAM_DEV_INT32_MAX < temp_total_events)) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid total events: %lld", temp_total_events);
		return -XM_CAM_DEV_EINVAL;
	}

	if ((0 >= temp_failure_rate) || (100 < temp_failure_rate)) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid failure rate: %lld", temp_failure_rate);
		return -XM_CAM_DEV_EINVAL;
	}

	if ((0 >= temp_time_interval) || (XM_CAM_DEV_INT32_MAX < temp_time_interval)) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid time interval: %lld", temp_time_interval);
		return -XM_CAM_DEV_EINVAL;
	}

	time_interval.tv_sec = temp_time_interval;

	rc = xm_cam_dev_get_failure_rate_time_interval(info,
		 time_interval,
		 status_code,
		 &total_events,
		 &failure_rate);

	if (XM_CAM_DEV_SUCCESS != rc) {
		CAM_ERR(CAM_SENSOR_IO, "Failed to get failure rate, rc=%d", rc);
		return rc;
	}

	if ((0 < total_events) &&
		(0 < failure_rate) &&
		(100 >= failure_rate) &&
		(total_events >= temp_total_events) &&
		(failure_rate >= temp_failure_rate)) {
		CAM_ERR(CAM_SENSOR_IO, "Failure rate calculation: total events: %u, failure rate: %u, in time interval %lld",
				total_events, failure_rate, time_interval.tv_sec);
		*is_damaged = XM_CAM_DEV_RESULT_TRUE;
	}

	return rc;
}

// output functions
uint32_t xm_cam_dev_need_skip_i2c_operation(struct xm_cam_dev_info *info,
											struct i2c_settings_list *settings_list,
											enum xm_cam_dev_i2c_cmd_type i2c_cmd_type)
{
	uint32_t result = 0;
	uint32_t is_damaged = XM_CAM_DEV_RESULT_FALSE;
	int32_t rc = XM_CAM_DEV_SUCCESS;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return 0;
	}

	if (NULL == settings_list) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: settings_list is NULL");
		return 0;
	}

	if (false == xm_cam_dev_protection_enable(info->dev_type, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		CAM_DBG(CAM_SENSOR_IO, "I2C protection is disabled, skip protection");
		return 0;
	}

	if (0 == info->xm_cam_dev_init_flag) {
		CAM_DBG(CAM_SENSOR_IO, "Device is not initialized, skip protection");
		return 0;
	}

	if (1 == atomic_read(&(info->untrusted_status_flag))) {
		CAM_DBG(CAM_SENSOR_IO, "Untrusted status flag is 1, skip protection");
		return 0;
	}

	if (1 != atomic_read(&(info->status_reset_flag))) {
		CAM_DBG(CAM_SENSOR_IO, "Status reset flag is not 1, skip protection");
		return 0;
	}

	if (false == xm_cam_dev_is_need_protection_i2c_cmd(info, i2c_cmd_type)) {
		CAM_DBG(CAM_SENSOR_IO, "I2C command type is not need protection, skip protection");
		return 0;
	}

	if (xm_cam_dev_is_i2c_write_cmd(settings_list) || xm_cam_dev_is_i2c_read_cmd(settings_list)) {
		rc = xm_cam_dev_is_device_damaged(info, XM_CAM_DEV_STATUS_CODE_I2C_WRITE_ERROR, &is_damaged);
		if (XM_CAM_DEV_SUCCESS != rc) {
			CAM_ERR(CAM_SENSOR_IO, "Failed to check if device is damaged, rc=%d", rc);
			return 0;
		}

		if ((XM_CAM_DEV_RESULT_TRUE == is_damaged) &&
			(XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_SKIP_ENABLE == (XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_SKIP_ENABLE & xm_cam_dev_protection_type_flag))) {
			CAM_ERR(CAM_SENSOR_IO, "Device is damaged, skip i2c operation");
			result = XM_CAM_DEV_XXSKIP_SENSOR_INTERNAL_OP_CODE;
		}
	}

	return result;
}

// output functions
uint32_t xm_cam_dev_need_skip_i2c_operation2(struct xm_cam_dev_info *info,
											struct i2c_settings_array *i2c_set,
											enum xm_cam_dev_i2c_cmd_type i2c_cmd_type)
{
	uint32_t result = 0;
	uint32_t is_damaged = XM_CAM_DEV_RESULT_FALSE;
	int32_t rc = XM_CAM_DEV_SUCCESS;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return 0;
	}

	if (NULL == i2c_set) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: i2c_set is NULL");
		return 0;
	}

	if (false == xm_cam_dev_protection_enable(info->dev_type, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		CAM_DBG(CAM_SENSOR_IO, "I2C protection is disabled, skip protection");
		return 0;
	}

	if (0 == info->xm_cam_dev_init_flag) {
		CAM_DBG(CAM_SENSOR_IO, "Device is not initialized, skip protection");
		return 0;
	}

	if (1 == atomic_read(&(info->untrusted_status_flag))) {
		CAM_DBG(CAM_SENSOR_IO, "Untrusted status flag is 1, skip protection");
		return 0;
	}

	if (1 != atomic_read(&(info->status_reset_flag))) {
		CAM_DBG(CAM_SENSOR_IO, "Status reset flag is not 1, skip protection");
		return 0;
	}

	if (false == xm_cam_dev_is_need_protection_i2c_cmd(info, i2c_cmd_type)) {
		CAM_DBG(CAM_SENSOR_IO, "I2C command type is not need protection, skip protection");
		return 0;
	}

	if (xm_cam_dev_has_i2c_write_cmd(i2c_set) || xm_cam_dev_has_i2c_read_cmd(i2c_set)) {
		rc = xm_cam_dev_is_device_damaged(info, XM_CAM_DEV_STATUS_CODE_I2C_WRITE_ERROR, &is_damaged);
		if (XM_CAM_DEV_SUCCESS != rc) {
			CAM_ERR(CAM_SENSOR_IO, "Failed to check if device is damaged, rc=%d", rc);
			return 0;
		}

		if ((XM_CAM_DEV_RESULT_TRUE == is_damaged) &&
			(XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_SKIP_ENABLE == (XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_SKIP_ENABLE & xm_cam_dev_protection_type_flag))) {
			CAM_ERR(CAM_SENSOR_IO, "Device is damaged, skip i2c operation");
			result = XM_CAM_DEV_XXSKIP_SENSOR_INTERNAL_OP_CODE;
		}
	}

	return result;
}


// output functions
uint32_t xm_cam_dev_need_skip_i2c_operation3(struct xm_cam_dev_info *info,
	enum xm_cam_dev_i2c_cmd_type i2c_cmd_type)
{
	uint32_t result = 0;
	uint32_t is_damaged = XM_CAM_DEV_RESULT_FALSE;
	int32_t rc = XM_CAM_DEV_SUCCESS;
	uint32_t index = 0;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return 0;
	}

	if (false == xm_cam_dev_protection_enable(info->dev_type, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		CAM_DBG(CAM_SENSOR_IO, "I2C protection is disabled, skip protection");
		return 0;
	}

	if (0 == info->xm_cam_dev_init_flag) {
		CAM_DBG(CAM_SENSOR_IO, "Device is not initialized, skip protection");
		return 0;
	}

	if (1 == atomic_read(&(info->untrusted_status_flag))) {
		CAM_DBG(CAM_SENSOR_IO, "Untrusted status flag is 1, skip protection");
		return 0;
	}

	if (1 != atomic_read(&(info->status_reset_flag))) {
		CAM_DBG(CAM_SENSOR_IO, "Status reset flag is not 1, skip protection");
		return 0;
	}

	if (false == xm_cam_dev_is_need_protection_i2c_cmd(info, i2c_cmd_type)) {
		CAM_DBG(CAM_SENSOR_IO, "I2C command type is not need protection, skip protection");
		return 0;
	}

	rc = xm_cam_dev_is_device_damaged(info, XM_CAM_DEV_STATUS_CODE_I2C_WRITE_ERROR, &is_damaged);
	if (XM_CAM_DEV_SUCCESS != rc) {
		CAM_ERR(CAM_SENSOR_IO, "Failed to check if device is damaged, rc=%d", rc);
		return 0;
	}

	if ((XM_CAM_DEV_RESULT_TRUE == is_damaged) &&
		(XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_SKIP_ENABLE == (XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_SKIP_ENABLE & xm_cam_dev_protection_type_flag))) {
		for (index = 0; index < XM_CAM_DEV_I2C_STATUS_CODE_MAX; index++) {
			if (index < xm_cam_dev_protection_disable_cmd_type_count) {
				if (xm_cam_dev_protection_disable_cmd_type[index] == i2c_cmd_type) {
					return 0;
				}
			}
		}
		CAM_ERR(CAM_SENSOR_IO, "Device is damaged, skip i2c operation");
		return XM_CAM_DEV_XXSKIP_SENSOR_INTERNAL_OP_CODE;
	}

	return result;
}

// output functions
bool xm_cam_dev_need_change_i2c_rc(struct xm_cam_dev_info *info,
								   enum xm_cam_dev_i2c_cmd_type i2c_cmd_type)
{
	bool result = false;
	uint32_t is_damaged = XM_CAM_DEV_RESULT_FALSE;
	int32_t rc = XM_CAM_DEV_SUCCESS;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return false;
	}

	if (false == xm_cam_dev_protection_enable(info->dev_type, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		CAM_DBG(CAM_SENSOR_IO, "I2C protection is disabled, skip protection");
		return false;
	}

	if (0 == info->xm_cam_dev_init_flag) {
		CAM_DBG(CAM_SENSOR_IO, "Device is not initialized, skip protection");
		return false;
	}

	if (1 == atomic_read(&(info->untrusted_status_flag))) {
		CAM_INFO(CAM_SENSOR_IO, "Untrusted status flag is 1, skip protection");
		return false;
	}

	if (1 != atomic_read(&(info->status_reset_flag))) {
		CAM_INFO(CAM_SENSOR_IO, "Status reset flag is not 1, skip protection");
		return false;
	}

	if (false == xm_cam_dev_is_need_protection_i2c_cmd(info, i2c_cmd_type)) {
		CAM_DBG(CAM_SENSOR_IO, "I2C command type is not need protection, skip protection");
		return false;
	}

	rc = xm_cam_dev_is_device_damaged(info, XM_CAM_DEV_STATUS_CODE_I2C_WRITE_ERROR, &is_damaged);
	if (XM_CAM_DEV_SUCCESS != rc) {
		CAM_ERR(CAM_SENSOR_IO, "Failed to check if device is damaged, rc=%d", rc);
		return false;
	}

	if ((XM_CAM_DEV_RESULT_TRUE == is_damaged) &&
		(XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_CHANGE_RETURN == (XM_CAM_DEV_PROTECTION_TYPE_FLAG_I2C_SKIP_ENABLE & xm_cam_dev_protection_type_flag))) {
		result = true;
	}

	return result;
}

int32_t xm_cam_dev_safe_mutex_lock(struct xm_cam_dev_info *info)
{
	int32_t retry_count = 0;
	int32_t rc = -XM_CAM_DEV_EBUSY;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return -XM_CAM_DEV_EINVAL;
	}

	if (XM_CAM_DEV_MUTEX_INIT_STATUS_INITIALIZED != atomic_read(&(info->xm_cam_dev_mutex_init_flag))) {
		CAM_ERR(CAM_SENSOR_IO, "Mutex not initialized");
		return -XM_CAM_DEV_EINVAL;
	}

	for (retry_count = 0; retry_count < XM_CAM_DEV_MUTEX_RETRY_COUNT; retry_count++) {
		if (mutex_trylock(&info->xm_cam_dev_mutex)) {
			return XM_CAM_DEV_SUCCESS;
		}
	}
	CAM_ERR(CAM_SENSOR_IO, "Failed to acquire mutex after %d retries",
			XM_CAM_DEV_MUTEX_RETRY_COUNT);
	return rc;
}

int32_t xm_cam_dev_safe_mutex_unlock(struct xm_cam_dev_info *info)
{
	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return -XM_CAM_DEV_EINVAL;
	}

	if (XM_CAM_DEV_MUTEX_INIT_STATUS_INITIALIZED != atomic_read(&(info->xm_cam_dev_mutex_init_flag))) {
		CAM_ERR(CAM_SENSOR_IO, "Mutex not initialized");
		return -XM_CAM_DEV_EINVAL;
	}

	mutex_unlock(&info->xm_cam_dev_mutex);
	return XM_CAM_DEV_SUCCESS;
}

bool xm_cam_dev_is_dev_init_failed(struct xm_cam_dev_info *info)
{
	bool result = false;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return false;
	}

	if (false == xm_cam_dev_protection_enable(info->dev_type, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		return false;
	}

	if (1 == atomic_read(&(info->untrusted_status_flag))) {
		CAM_INFO(CAM_SENSOR_IO, "Untrusted status flag is 1, skip protection");
		return false;
	}

	if (1 != atomic_read(&(info->status_reset_flag))) {
		CAM_INFO(CAM_SENSOR_IO, "Status reset flag is not 1, skip protection");
		return false;
	}

	if (XM_CAM_DEV_INIT_STATUS_FAILURE == atomic_read(&(info->init_status))) {
		CAM_INFO(CAM_SENSOR_IO, "Dev init failed flag is 1");
		result = true;
	}

	return result;
}

void xm_cam_dev_get_i2c_cmd_info_by_list(struct i2c_settings_list *i2c_set, struct xm_cam_dev_i2c_cmd_info *i2c_cmd_info)
{
	if ((NULL == i2c_set) || (NULL == i2c_cmd_info)) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: is NULL");
		return;
	}

	i2c_cmd_info->is_valid = false;
	i2c_cmd_info->has_write_cmd = false;
	i2c_cmd_info->has_read_cmd = false;
	i2c_cmd_info->has_poll_cmd = false;

	if ((CAM_SENSOR_I2C_WRITE_RANDOM == i2c_set->op_code) ||
		(CAM_SENSOR_I2C_WRITE_BURST == i2c_set->op_code)  ||
		(CAM_SENSOR_I2C_WRITE_SEQ == i2c_set->op_code)) {
		i2c_cmd_info->is_valid = true;
		i2c_cmd_info->has_write_cmd = true;
	} else if ((CAM_SENSOR_I2C_READ_RANDOM == i2c_set->op_code) ||
			   (CAM_SENSOR_I2C_READ_SEQ == i2c_set->op_code)) {
		i2c_cmd_info->is_valid = true;
		i2c_cmd_info->has_read_cmd = true;
	} else if (CAM_SENSOR_I2C_POLL == i2c_set->op_code) {
		i2c_cmd_info->is_valid = true;
		i2c_cmd_info->has_poll_cmd = true;
	}

	return;
}

void xm_cam_dev_get_i2c_cmd_info(struct i2c_settings_array *i2c_set, struct xm_cam_dev_i2c_cmd_info *i2c_cmd_info)
{
	struct xm_cam_dev_i2c_cmd_info i2c_cmd_info_tmp = {false, false, false, false};
	struct i2c_settings_list *i2c_list = NULL;

	if ((NULL == i2c_set) || (NULL == i2c_cmd_info)) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: is NULL");
		return;
	}

	list_for_each_entry(i2c_list,
		&(i2c_set->list_head), list) {
		xm_cam_dev_get_i2c_cmd_info_by_list(i2c_list, &i2c_cmd_info_tmp);
		if (i2c_cmd_info_tmp.is_valid) {
			i2c_cmd_info->is_valid = true;
			if (i2c_cmd_info_tmp.has_write_cmd) {
				i2c_cmd_info->has_write_cmd = true;
			}
			if (i2c_cmd_info_tmp.has_read_cmd) {
				i2c_cmd_info->has_read_cmd = true;
			}
			if (i2c_cmd_info_tmp.has_poll_cmd) {
				i2c_cmd_info->has_poll_cmd = true;
			}
		}
	}

	return;
}

bool xm_cam_dev_has_i2c_write_cmd(struct i2c_settings_array *i2c_set)
{
	bool result = false;
	struct xm_cam_dev_i2c_cmd_info i2c_cmd_info = {false, false, false, false};

	if (NULL == i2c_set) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: i2c_set is NULL");
		return false;
	}

	xm_cam_dev_get_i2c_cmd_info(i2c_set, &i2c_cmd_info);
	if ((true == i2c_cmd_info.is_valid) && (true == i2c_cmd_info.has_write_cmd)) {
		result = true;
	}

	return result;
}

bool xm_cam_dev_has_i2c_read_cmd(struct i2c_settings_array *i2c_set)
{
	bool result = false;
	struct xm_cam_dev_i2c_cmd_info i2c_cmd_info = {false, false, false, false};

	if (NULL == i2c_set) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: i2c_set is NULL");
		return false;
	}

	xm_cam_dev_get_i2c_cmd_info(i2c_set, &i2c_cmd_info);
	if ((true == i2c_cmd_info.is_valid) && (true == i2c_cmd_info.has_read_cmd)) {
		result = true;
	}

	return result;
}

bool xm_cam_dev_has_i2c_poll_cmd(struct i2c_settings_array *i2c_set)
{
	bool result = false;
	struct xm_cam_dev_i2c_cmd_info i2c_cmd_info = {false, false, false, false};

	if (NULL == i2c_set) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: i2c_set is NULL");
		return false;
	}

	xm_cam_dev_get_i2c_cmd_info(i2c_set, &i2c_cmd_info);
	if ((true == i2c_cmd_info.is_valid) && (true == i2c_cmd_info.has_poll_cmd)) {
		result = true;
	}

	return result;
}

// output functions
bool xm_cam_dev_is_i2c_write_cmd(struct i2c_settings_list *i2c_set)
{
	bool result = false;
	struct xm_cam_dev_i2c_cmd_info i2c_cmd_info = {false, false, false, false};

	if (NULL == i2c_set) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: i2c_set is NULL");
		return false;
	}

	xm_cam_dev_get_i2c_cmd_info_by_list(i2c_set, &i2c_cmd_info);
	if ((true == i2c_cmd_info.is_valid) && (true == i2c_cmd_info.has_write_cmd)) {
		result = true;
	}

	return result;
}

bool xm_cam_dev_is_i2c_read_cmd(struct i2c_settings_list *i2c_set)
{
	bool result = false;
	struct xm_cam_dev_i2c_cmd_info i2c_cmd_info = {false, false, false, false};

	if (NULL == i2c_set) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: i2c_set is NULL");
		return false;
	}

	xm_cam_dev_get_i2c_cmd_info_by_list(i2c_set, &i2c_cmd_info);
	if ((true == i2c_cmd_info.is_valid) && (true == i2c_cmd_info.has_read_cmd)) {
		result = true;
	}

	return result;
}

bool xm_cam_dev_is_i2c_poll_cmd(struct i2c_settings_list *i2c_set)
{
	bool result = false;
	struct xm_cam_dev_i2c_cmd_info i2c_cmd_info = {false, false, false, false};

	if (NULL == i2c_set) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: i2c_set is NULL");
		return false;
	}

	xm_cam_dev_get_i2c_cmd_info_by_list(i2c_set, &i2c_cmd_info);
	if ((true == i2c_cmd_info.is_valid) && (true == i2c_cmd_info.has_poll_cmd)) {
		result = true;
	}

	return result;
}

bool xm_cam_dev_is_need_protection_i2c_cmd(struct xm_cam_dev_info *info,
										   enum xm_cam_dev_i2c_cmd_type i2c_cmd_type)
{
	bool result = false;

	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return false;
	}

	if (false == xm_cam_dev_protection_enable(info->dev_type, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		CAM_DBG(CAM_SENSOR_IO, "I2C protection is disabled, skip i2c operation");
		return false;
	}

	if ((XM_CMD_DEV_I2C_ACTUATOR_CMD_TYPE_CONFIG == i2c_cmd_type) ||
		(XM_CMD_DEV_I2C_ACTUATOR_CMD_TYPE_CRM_CONFIG == i2c_cmd_type) ||
		(XM_CMD_DEV_I2C_ACTUATOR_CMD_TYPE_PARK_LENS == i2c_cmd_type)) {
		result = true;
	} else {
		result = false;
	}

	return result;
}

// output functions
void xm_cam_dev_set_init_result(struct xm_cam_dev_info *info, int32_t result)
{
	if (NULL == info) {
		CAM_ERR(CAM_SENSOR_IO, "Invalid Args: info is NULL");
		return;
	}

	if (false == xm_cam_dev_protection_enable(info->dev_type, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		return;
	}

	if (XM_CAM_DEV_INIT_STATUS_SUCCESS == result ||
		XM_CAM_DEV_INIT_STATUS_FAILURE == result) {
		atomic_set(&(info->init_status), result);
	}
}