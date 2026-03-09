/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_ZOOM_CORE_H_
#define _CAM_ZOOM_CORE_H_

#include "cam_zoom_dev.h"

/**
 * @power_info: power setting info to control the power
 *
 * This API construct the default zoom power setting.
 *
 * @return Status of operation. Negative in case of error. Zero otherwise.
 */
int32_t cam_zoom_construct_default_power_setting(
	struct cam_sensor_power_ctrl_t *power_info);

/**
 * @apply: Req mgr structure for applying request
 *
 * This API applies the request that is mentioned
 */
int32_t cam_zoom_apply_request(struct cam_req_mgr_apply_request *apply);

/**
 * @info: Sub device info to req mgr
 *
 * This API publish the subdevice info to req mgr
 */
int32_t cam_zoom_publish_dev_info(struct cam_req_mgr_device_info *info);

/**
 * @flush: Req mgr structure for flushing request
 *
 * This API flushes the request that is mentioned
 */
int cam_zoom_flush_request(struct cam_req_mgr_flush_request *flush);


/**
 * @link: Link setup info
 *
 * This API establishes link zoom subdevice with req mgr
 */
int32_t cam_zoom_establish_link(
	struct cam_req_mgr_core_dev_link_setup *link);

/**
 * @z_ctrl: Zoom ctrl structure
 * @arg:    Camera control command argument
 *
 * This API handles the camera control argument reached to zoom
 */
int32_t cam_zoom_driver_cmd(struct cam_zoom_ctrl_t *z_ctrl, void *arg);

/**
 * @z_ctrl: Zoom ctrl structure
 *
 * This API handles the shutdown ioctl/close
 */
void cam_zoom_shutdown(struct cam_zoom_ctrl_t *z_ctrl);

struct completion *cam_zoom_get_i3c_completion(uint32_t index);


#endif /* _CAM_ZOOM_CORE_H_ */
