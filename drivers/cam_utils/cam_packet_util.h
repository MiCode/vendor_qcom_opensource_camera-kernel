/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2025, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_PACKET_UTIL_H_
#define _CAM_PACKET_UTIL_H_

#include <media/cam_defs.h>
#include "cam_hw_mgr_intf.h"

#define CAM_UNIQUE_SRC_HDL_MAX 50
#define CAM_UNIQUE_DST_HDL_MAX 50
#define CAM_PRESIL_UNIQUE_HDL_MAX 50

/**
 * @brief:                 Unique buf handle table to accelerate patching
 *
 * @hdl:                   buf handle
 * @buf_size:              Offset from the start of the buffer
 * @flags:                 Flag
 * @iova:                  IO virtual address
 * @kva:                   Kernel virtual address
 *
 */
struct cam_patch_unique_buf_tbl {
	int32_t       hdl;
	size_t        buf_size;
	uint32_t      flags;
	union {
		dma_addr_t iova;
		uintptr_t  kva;
	} u;
};

/**
 * @brief                  KMD scratch buffer information
 *
 * @handle:                Memory handle
 * @cpu_addr:              Cpu address
 * @offset:                Offset from the start of the buffer
 * @size:                  Size of the buffer in bytes
 * @used_bytes:            Used memory in bytes
 *
 */
struct cam_kmd_buf_info {
	int        handle;
	uint32_t  *cpu_addr;
	uint32_t   offset;
	uint32_t   size;
	uint32_t   used_bytes;
};

/* Generic Cmd Buffer blob callback function type */
typedef int (*cam_packet_generic_blob_handler)(void *user_data,
	uint32_t blob_type, uint32_t blob_size, uint8_t *blob_data);

/**
 * cam_packet_util_get_cmd_mem_addr()
 *
 * @brief                  Get command buffer address
 *
 * @handle:                Command buffer memory handle
 * @buf_addr:              Command buffer cpu mapped address
 * @len:                   Command buffer length
 *
 * @return:                0 for success
 *                         -EINVAL for Fail
 */
int cam_packet_util_get_cmd_mem_addr(int handle, uint32_t **buf_addr,
	size_t *len);

/**
 * cam_packet_util_validate_packet()
 *
 * @brief                  Validate the packet
 *
 * @packet:                Packet to be validated
 *
 * @remain_len:            CPU buff length after config offset
 *
 * @return:                0 for success
 *                         -EINVAL for Fail
 */
int cam_packet_util_validate_packet(struct cam_packet *packet,
	size_t remain_len);

int cam_packet_util_copy_pkt_to_kmd(struct cam_packet *packet_u,
	struct cam_packet **packet, size_t remain_len);
/**
 * cam_packet_util_validate_cmd_desc()
 *
 * @brief                  Validate the packet
 *
 * @cmd_desc:              Command descriptor to be validated
 *
 * @return:                0 for success
 *                         -EINVAL for Fail
 */
int cam_packet_util_validate_cmd_desc(struct cam_cmd_buf_desc *cmd_desc);

/**
 * cam_packet_util_get_kmd_buffer()
 *
 * @brief                  Get the kmd buffer from the packet command descriptor
 *
 * @packet:                Packet data
 * @kmd_buf:               Extracted the KMD buffer information
 *
 * @return:                0 for success
 *                         -EINVAL for Fail
 */
int cam_packet_util_get_kmd_buffer(struct cam_packet *packet,
	struct cam_kmd_buf_info *kmd_buf_info);

/**
 * cam_packet_util_dump_patch_info()
 *
 * @brief:              Dump patch info in case of page fault
 *
 * @packet:             Input packet containing Command Buffers and Patches
 * @iommu_hdl:          IOMMU handle of the HW Device that received the packet
 * @sec_iommu_hdl:      Secure IOMMU handle of the HW Device that
 *                      received the packet
 * @pf_args:            Page fault arguments
 *
 */
void cam_packet_util_dump_patch_info(struct cam_packet *packet,
	int32_t iommu_hdl, int32_t sec_iommu_hdl, struct cam_hw_dump_pf_args *pf_args);

/**
 * cam_packet_util_get_unique_tbl()
 *
 * @brief:              Get unique src/dst table to accelerate patching process
 *
 * @src_tbl:            Output pointer for unique src table
 * @dst_tbl:            Output pointer for unique dst table
 *
 * @return:             0: Success
 *                      Negative: Failure
 *
 */
int cam_packet_util_get_unique_tbl(struct cam_patch_unique_buf_tbl **src_tbl,
	struct cam_patch_unique_buf_tbl **dst_tbl);

/**
 * cam_packet_util_put_unique_tbl()
 *
 * @brief:              Put unique src/dst table and free the space
 *
 * @src_tbl:            Input pointer for unique src table that needs to be freed
 * @dst_tbl:            Input pointer for unique dst table that needs to be freed
 *
 */
void cam_packet_util_put_unique_tbl(struct cam_patch_unique_buf_tbl *src_tbl,
	struct cam_patch_unique_buf_tbl *dst_tbl);

/**
 * cam_packet_util_process_patches()
 *
 * @brief:              Replace the handle in Packet to Address using the
 *                      information from patches.
 *
 * @packet:             Input packet containing Command Buffers and Patches
 * @mapped_io_list:     List in to add patches/buffers to for reference counting
 * @iommu_hdl:          IOMMU handle of the HW Device that received the packet
 * @sec_iommu_hdl:      Secure IOMMU handle of the HW Device that
 *                      received the packet
 * @exp_mem:            Boolean to know if patched address is in expanded memory range
 *                      or within default 32-bit address space.
 * @in_src_tbl:         Unique buf handle table for src buffers
 * @in_dst_tbl:         Unique buf handle table for dst buffers
 *
 * @return:             0: Success
 *                      Negative: Failure
 */
int cam_packet_util_process_patches(struct cam_packet *packet,
	struct list_head *mapped_io_list,  int32_t iommu_hdl, int32_t sec_mmu_hdl,
	bool exp_mem, struct cam_patch_unique_buf_tbl *in_src_tbl,
	struct cam_patch_unique_buf_tbl *in_dst_tbl);

/**
 * cam_packet_util_dump_io_bufs()
 *
 * @brief:              Search for faulted io buffer in packet and print io buffers info
 *
 * @packet:             Input packet containing io buffers
 * @iommu_hdl:          IOMMU handle of the HW Device that received the packet
 * @sec_iommu_hdl:      Secure IOMMU handle of the HW Device that
 *                      received the packet
 * @pf_args:            Pault Fault related info
 * @res_id_support:     if the specific device has knowledge of the resource id for hw
 */

void cam_packet_util_dump_io_bufs(struct cam_packet *packet,
	int32_t iommu_hdl, int32_t sec_mmu_hdl,
	struct cam_hw_dump_pf_args *pf_args, bool res_id_support);

/**
 * cam_packet_util_process_generic_cmd_buffer()
 *
 * @brief:              Process Generic Blob command buffer. This utility
 *                      function process the command buffer and calls the
 *                      blob_handle_cb callback for each blob that exists
 *                      in the command buffer.
 *
 * @cmd_buf:            Generic Blob Cmd Buffer handle
 * @blob_handler_cb:    Callback pointer to call for each blob exists in the
 *                      command buffer
 * @user_data:          User data to be passed while callback
 *
 * @return:             0: Success
 *                      Negative: Failure
 */
int cam_packet_util_process_generic_cmd_buffer(
	struct cam_cmd_buf_desc *cmd_buf,
	cam_packet_generic_blob_handler blob_handler_cb, void *user_data);

/**
 * @brief :            API to retrieve image buffers from presil after processing is
 *                     done,using packet from request
 *
 * @packet:            Packet pointer for current request
 * @iommu_hdl:         IOMMU hdl for Image buffers
 * @out_res_id:        Resource ID corresponding to the output buffer
 *
 * @return:            Success or Failure
 */
int cam_presil_retrieve_buffers_from_packet(struct cam_packet *packet, int iommu_hdl,
	int out_res_id);

/**
 * @brief : API to send relevant buffers to presil
 *
 * @packet :            Packet pointer for current request
 * @img_iommu_hdl:      IOMMU hdl for Image buffers
 * @cdm_iommu_hdl:      IOMMU hdl for cdm buffers
 *
 */
int cam_presil_send_buffers_from_packet(struct cam_packet *packet, int img_iommu_hdl,
	int cdm_iommu_hdl);

/**
 * @brief : API to handle the blob data to get blob type and size
 *
 * @length :            length of the blob
 * @blob_ptr:           blob base address
 * @blob_handler_cb:    blob handler call back
 * @user_data:          user data information
 */
int cam_packet_util_process_generic_blob(uint32_t length, uint32_t *blob_ptr,
	cam_packet_generic_blob_handler blob_handler_cb, void *user_data);


#endif /* _CAM_PACKET_UTIL_H_ */
