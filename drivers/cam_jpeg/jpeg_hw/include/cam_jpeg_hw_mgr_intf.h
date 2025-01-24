/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022,2024 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef CAM_JPEG_HW_MGR_INTF_H
#define CAM_JPEG_HW_MGR_INTF_H

#include <linux/of.h>
#include <media/cam_jpeg.h>
#include <media/cam_defs.h>

/* Xiaomi: enlarge from 24 to 48 */
#define CAM_JPEG_CTX_MAX        48

enum cam_jpeg_hw_type {
	CAM_JPEG_DEV_ENC,
	CAM_JPEG_DEV_DMA,
	CAM_JPEG_DEV_MAX,
};

/**
 * struct cam_jpeg_request_data - Jpeg request data received from command buffers
 * @dev_type                 : Jpeg device type(ENC vs DMA)
 * @request_id               : Request ID
 * @thumbnail_threshold_size : Threshold size for thumbnail image
 * @out_size_mem_handle      : handle to the buffer to share encoded output size with userspace
 * @out_size_offset          : offset to memory where out_size_mem_handle is stored
 */
struct cam_jpeg_request_data {
	uint32_t                            dev_type;
	uint64_t                            request_id;
	uint32_t                            thumbnail_threshold_size;
	__s32				    out_size_mem_handle;
	uint32_t			    out_size_offset;
};

typedef void (*cam_jpeg_mini_dump_cb)(void *priv, void *dst);

int cam_jpeg_hw_mgr_init(struct device_node *of_node,
	uint64_t *hw_mgr_hdl, int *iommu_hdl,
	cam_jpeg_mini_dump_cb mini_dump_cb);

#endif /* CAM_JPEG_HW_MGR_INTF_H */
