/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */
#ifndef _CAM_OIS_CORE_H_
#define _CAM_OIS_CORE_H_

#include <linux/cma.h>
#include "cam_ois_dev.h"
#include "cam_req_mgr_dev.h" // xiaomi add

#define OIS_NAME_LEN                            32

#define FIRMWARE_UPDATE_FORCED                  1
#define FIRMWARE_UPDATE_EVERY_TIMES             2

#define FIRMWARE_UPDATE_RETRY_TIMES             1
/**
 * @power_info: power setting info to control the power
 *
 * This API construct the default ois power setting.
 *
 * @return Status of operation. Negative in case of error. Zero otherwise.
 */
int32_t cam_ois_construct_default_power_setting(
	struct cam_sensor_power_ctrl_t *power_info);


int cam_ois_driver_cmd(struct cam_ois_ctrl_t *e_ctrl, void *arg);

/**
 * @o_ctrl: OIS ctrl structure
 *
 * This API handles the shutdown ioctl/close
 */
void cam_ois_shutdown(struct cam_ois_ctrl_t *o_ctrl);

struct completion *cam_ois_get_i3c_completion(uint32_t index);

// xiaomi add

/**
 * @info: Sub device info to req mgr
 *
 * This API publish the subdevice info to req mgr
 */
int32_t cam_ois_publish_dev_info(struct cam_req_mgr_device_info *info);

/**
 * @link: Link setup info
 *
 * This API establishes link ois subdevice with req mgr
 */
int32_t cam_ois_establish_link(
	struct cam_req_mgr_core_dev_link_setup *link);

/**
 * @apply: Req mgr structure for applying request
 *
 * This API applies the request that is mentioned
 */
int32_t cam_ois_apply_request(struct cam_req_mgr_apply_request *apply);

/**
 * @flush: Req mgr structure for flushing request
 *
 * This API flushes the request that is mentioned
 */
int cam_ois_flush_request(struct cam_req_mgr_flush_request *flush);

/**
 * cam_ois_update_req_mgr - camera ois update reg manager
 * @o_ctrl:        camera ois controller
 * @csl_packet:    camera packet
 *
 * Returns success or failure
 */
int cam_ois_update_req_mgr(
	struct cam_ois_ctrl_t *o_ctrl,
	struct cam_packet *csl_packet);


/**
 * cam_ois_do_frame_skip - cam ois frame skip
 */
bool cam_ois_do_frame_skip(
	int64_t req_id,
	int32_t dev_hdl);


// xiaomi add

#endif
/* _CAM_OIS_CORE_H_ */
