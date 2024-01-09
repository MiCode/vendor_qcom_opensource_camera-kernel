/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include "cam_vmrm.h"
#include "cam_cpas_api.h"

#ifndef _CAM_VMRM_INTERFACE_H_
#define _CAM_VMRM_INTERFACE_H_

#ifdef CONFIG_SPECTRA_VMRM
#define CAM_IS_PRIMARY_VM() ((cam_vmrm_intf_get_vmid() == CAM_PVM) \
? true : false)
#define CAM_IS_SECONDARY_VM() ((cam_vmrm_intf_get_vmid() != CAM_PVM) \
? true : false)
#else
#ifdef CONFIG_ARCH_QTI_VM
#define CAM_IS_PRIMARY_VM() false
#define CAM_IS_SECONDARY_VM() true
#else
#define CAM_IS_PRIMARY_VM() true
#define CAM_IS_SECONDARY_VM() false
#endif
#endif

/**
 * cam_vmrm_is_supported()
 *
 * @brief:        VMRM is supported or not
 */
bool cam_vmrm_is_supported(void);

/**
 * cam_vmrm_intf_get_vmid()
 *
 * @brief:        Get camera vm id
 */
uint32_t cam_vmrm_intf_get_vmid(void);

/**
 * cam_vmrm_proxy_clk_rgl_voting_enable()
 *
 * @brief:        Proxy voting clock regulator enable or not
 */
bool cam_vmrm_proxy_clk_rgl_voting_enable(void);

/**
 * cam_vmrm_proxy_icc_voting_enable()
 *
 * @brief:        Proxy voting icc enable or not
 */
bool cam_vmrm_proxy_icc_voting_enable(void);

/**
 * cam_vmvm_populate_hw_instance_info()
 *
 * @brief:        Populate hw instance information to resource manager
 *
 * @soc_info:             Hw soc info
 * @hw_msg_callback:      HW msg callback
 * @hw_msg_callback_data: Hw msg callback data
 */
int cam_vmvm_populate_hw_instance_info(struct cam_hw_soc_info *soc_info,
	msg_cb_func hw_msg_callback, void *hw_msg_callback_data);

/**
 * cam_vmrm_populate_driver_node_info()
 *
 * @brief:        Populate driver node information to resource manager
 *
 * @driver_node:  Driver node
 */
int cam_vmrm_populate_driver_node_info(struct cam_driver_node *driver_node);

/**
 * cam_vmrm_populate_io_resource_info()
 *
 * @brief:         Populate io resource information to resource manager
 */
int cam_vmrm_populate_io_resource_info(void);

/**
 * cam_vmrm_register_gh_callback()
 *
 * @brief:         Register gh callback
 */
int cam_vmrm_register_gh_callback(void);

/**
 * cam_vmrm_unregister_gh_callback()
 *
 * @brief:         Unregister gh callback
 */
int cam_vmrm_unregister_gh_callback(void);

/**
 * cam_vmrm_soc_acquire_resources()
 *
 * @brief:        Acquire resources, such as interrupt, io memory ownership
 *
 * @hw_id:        Hw id
 */
int cam_vmrm_soc_acquire_resources(uint32_t hw_id);

/**
 * cam_vmrm_soc_release_resources()
 *
 * @brief:        Release the resource to resource manager
 *
 * @hw_id:        Hw id
 */
int cam_vmrm_soc_release_resources(uint32_t hw_id);

/**
 * cam_vmrm_send_msg()
 *
 * @brief:        VMRM send message
 *
 * @source_vmid:  Source vm id
 * @des_vmid:     Destination vm id
 * @msg_dst_type: Message destination type
 * @msg_dst_id:   Message destination id
 * @msg_type:     Message type
 * @msg_data:     Message data payload
 * @response_msg  Whether response message or actual message
 * @need_response Whether need response
 * @data_size:    Message data payload size
 * @complete:     Response completion
 */
int cam_vmrm_send_msg(uint32_t source_vmid, uint32_t des_vmid, uint32_t msg_dst_type,
	uint32_t msg_dst_id, uint32_t msg_type, bool response_msg, bool need_response,
	void *msg_data, uint32_t data_size, struct completion *complete);

/**
 * cam_vmrm_soc_enable_disable_resources()
 *
 * @brief:        Vmrm soc enable or disable resources
 *
 * @hw_id:        Hw id
 *
 * @flag:         Enable or disable
 */
int cam_vmrm_soc_enable_disable_resources(uint32_t hw_id, bool flag);

/**
 * cam_vmrm_set_src_clk_rate()
 *
 * @brief:            Vmrm set src clk rate
 *
 * @hw_id:            Hw id
 *
 * @cesta_client_idx: Cesta client index
 *
 * @clk_rate_high:    Clk rate high
 *
 * @clk_rate_low:     Clk rate low
 */
int cam_vmrm_set_src_clk_rate(uint32_t hw_id, int cesta_client_idx,
	unsigned long clk_rate_high, unsigned long clk_rate_low);

/**
 * cam_vmrm_icc_vote()
 *
 * @brief:            Vmrm icc vote
 *
 * @name:             Bus client name
 *
 * @ab:               Arbitrated Bandwidth
 *
 * @ib:               Instantaneous Bandwidth
 */
int cam_vmrm_icc_vote(const char *name, uint64_t ab, uint64_t ib);

/**
 * cam_vmrm_sensor_power_up()
 *
 * @brief:            Vmrm sensor power up
 *
 * @hw_id:            Hw id
 *
 */
int cam_vmrm_sensor_power_up(uint32_t hw_id);

/**
 * cam_vmrm_sensor_power_down()
 *
 * @brief:            Vmrm sensor power down
 *
 * @hw_id:            Hw id
 *
 */
int cam_vmrm_sensor_power_down(uint32_t hw_id);
#endif /* _CAM_VMRM_INTERFACE_H_ */
