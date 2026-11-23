/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef __CAM_BUS_COMMON_QUERY_V1_H__
#define __CAM_BUS_COMMON_QUERY_V1_H__

#define CAM_NUM_BUS_WR_CLIENT_V1         64
#define CAM_NUM_BUS_WR_RESERVED_V1       8
#define CAM_NUM_BUS_WR_MSF_INFO_V1       8
#define CAM_NUM_BUS_WR_SUB_GRP_INFO_V1   16
#define CAM_NUM_BUS_WR_SRC_INFO_V1       16

/**
 * struct cam_bus_wr_debug_feature_mask - Bit fields for debug features.
 *
 * @cons_violation        : Constraint Violation supported  :BIT_0
 * @ccif_violation        : CCIF Violation supported :BIT_1
 * @image_size_violation  : Image Size Violation supported: BIT_2
 * @reserved              : reserved: BIT_3:BIT_31
 *
 */
struct cam_bus_wr_debug_feature_mask {
	uint32_t cons_violation        : 1;
	uint32_t ccif_violation        : 1;
	uint32_t image_size_violation  : 1;
	uint32_t reserved              : 29;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_debug_feature - Place holder for debug features.
 *
 * @bits:                Bit fields for debug features.
 * @dword:               dword
 *
 */
union cam_bus_wr_debug_feature {
	struct cam_bus_wr_debug_feature_mask  bits;
	uint32_t                              dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_wrapper_feature_mask - Bit fields for Wrapper features.
 *
 * @framedrop_present     : Frame drop feature   :BIT_0
 * @frameheader_present   : Frame header feature :BIT_1
 * @virtualframe_present  : virtual frame: BIT_2
 * @vfr_present           : VFR: BIT_3
 * @dba_present           : dynamic buffer allocation cross contexts: BIT_4
 * @mmu_prefetch_present  : MMU Fetch: BIT_5_
 * @reserved              : reserved: BIT_6-BIT_31
 *
 */
struct cam_bus_wr_wrapper_feature_mask {
	uint32_t framedrop_present     : 1;
	uint32_t frameheader_present   : 1;
	uint32_t virtualframe_present  : 1;
	uint32_t vfr_present           : 1;
	uint32_t dba_present           : 1;
	uint32_t mmu_prefetch_present  : 1;
	uint32_t reserved              : 26;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_wrapper_feature - Place holder for wrapper features.
 *
 * @bits:                Bit fields for wrapper features.
 * @dword:               dword
 *
 */
union cam_bus_wr_wrapper_feature {
	struct cam_bus_wr_wrapper_feature_mask  bits;
	uint32_t                                dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_msf_ipcc_cfg_mask - Bit fields for IPCC MSF ports.
 *
 * @port_id  : MSF port id:  BIT_0:BIT_7
 * @sid      : SID           BIT_8:BIT_12
 * @reserved :               BIT_13-BIT_31
 *
 */
struct cam_bus_wr_msf_ipcc_cfg_mask {
	uint32_t port_id  : 8;
	uint32_t sid      : 5;
	uint32_t reserved : 19;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_msf_ipcc_cfg - Place holder for IPCC MSF ports.
 *
 * @bits:                Bit fields for the MSF for IPCC.
 * @dword:               dword
 *
 */
union cam_bus_wr_msf_ipcc_cfg {
	struct cam_bus_wr_msf_ipcc_cfg_mask  bits;
	uint32_t                             dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_msf_info - Place holder for MSF Ports info.
 *
 * @msf_port_client_present_0:  Clients connected to a MSF port. Range: 0-31.
 *                              Each bit represents one client
 * @msf_port_client_present_1:  Clients connected to a MSF port. Range: 32-63.
 *                              Each bit represents one client
 *
 */
struct cam_bus_wr_msf_info {
	uint32_t    msf_port_client_present_0;
	uint32_t    msf_port_client_present_1;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_sub_grp_info - Place holder for Sub groups info.
 *
 * @sub_grp_client_present_0:  Clients present in a sub-group. Range: 0-31.
 *                             Each bit represents one client
 * @sub_grp_client_present_1:  Clients present in a sub-group. Range: 32-63.
 *                             Each bit represents one client
 *
 */
struct cam_bus_wr_sub_grp_info {
	uint32_t    sub_grp_client_present_0;
	uint32_t    sub_grp_client_present_1;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_src_grp_context1_mask - Bit fields for contexts in source grp.
 *
 * @src8_context  : Number of contexts in src_grp_8:  BIT_0:BIT_2
 * @src9_context  : Number of contexts in src_grp_9:  BIT_3:BIT_5
 * @src10_context : Number of contexts in src_grp_10: BIT_6:BIT_8
 * @src11_context : Number of contexts in src_grp_11: BIT_9:BIT_11
 * @src12_context : Number of contexts in src_grp_12: BIT_12:BIT_14
 * @src13_context : Number of contexts in src_grp_13: BIT_15:BIT_17
 * @src14_context : Number of contexts in src_grp_14: BIT_18:BIT_20
 * @src15_context : Number of contexts in src_grp_15: BIT_21:BIT_23
 * @reserved      : reserved: BIT_24-BIT_31
 *
 */
struct cam_bus_wr_src_grp_context1_mask {
	uint32_t src8_context  : 3;
	uint32_t src9_context  : 3;
	uint32_t src10_context : 3;
	uint32_t src11_context : 3;
	uint32_t src12_context : 3;
	uint32_t src13_context : 3;
	uint32_t src14_context : 3;
	uint32_t src15_context : 3;
	uint32_t reserved      : 8;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_src_grp_context1 - Place holder for avaiable contexts for source grp 8-15.
 *
 * @bits:                Bit fields for the context info.
 * @dword:               dword
 *
 */
union cam_bus_wr_src_grp_context1 {
	struct  cam_bus_wr_src_grp_context1_mask bits;
	uint32_t                                 dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_src_grp_context0_mask - Bit fields for contexts in source grp.
 *
 * @src0_context  : Number of contexts in src_grp_0: BIT_0:BIT_2
 * @src1_context  : Number of contexts in src_grp_1: BIT_3:BIT_5
 * @src2_context  : Number of contexts in src_grp_2: BIT_6:BIT_8
 * @src3_context  : Number of contexts in src_grp_3: BIT_9:BIT_11
 * @src4_context  : Number of contexts in src_grp_4: BIT_12:BIT_14
 * @src5_context  : Number of contexts in src_grp_5: BIT_15:BIT_17
 * @src6_context  : Number of contexts in src_grp_6: BIT_18:BIT_20
 * @src7_context  : Number of contexts in src_grp_7: BIT_21:BIT_23
 * @reserved      : reserved: BIT_24-BIT_31
 *
 */
struct cam_bus_wr_src_grp_context0_mask {
	uint32_t src0_context  : 3;
	uint32_t src1_context  : 3;
	uint32_t src2_context  : 3;
	uint32_t src3_context  : 3;
	uint32_t src4_context  : 3;
	uint32_t src5_context  : 3;
	uint32_t src6_context  : 3;
	uint32_t src7_context  : 3;
	uint32_t reserved      : 8;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_src_grp_context0 - Place holder for avaiable contexts for source grp 0-7.
 *
 * @bits:                Bit fields for the context info.
 * @dword:               dword
 *
 */
union cam_bus_wr_src_grp_context0 {
	struct  cam_bus_wr_src_grp_context0_mask bits;
	uint32_t                                 dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_src_info - Place holder for source info.
 *
 * @src_client_present_0:  Clients in a src grp. Range from 0-31. Each bit represents one client
 * @src_client_present_1:  Clients in a src grp. Range from 32-63. Each bit represents one client
 *
 */
struct cam_bus_wr_src_info {
	uint32_t    src_client_present_0;
	uint32_t    src_client_present_1;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_info_mask - Bit fields for client features.
 *
 * @ubwc_en                 : BIT_0
 * @ipcc_slice_en           : BIT_1
 * @rcs_en                  : BIT_2
 * @index_based_en          : BIT_3
 * @byte_align_en           : BIT_4
 * @early_done_irq_en       : BIT_5
 * @irq_sub_sample_en       : BIT_6
 * @reserved_0              : BIT_7-BIT_10
 * @client_cfg_num_context  : BIT_11-BIT_15
 * @client_cfg_ipcc_id      : BIT_16-BIT_20
 * @reserved_1              : BIT_21-BIT_31
 *
 */
struct cam_bus_wr_info_mask {
	uint32_t ubwc_en                 : 1;
	uint32_t ipcc_slice_en           : 1;
	uint32_t rcs_en                  : 1;
	uint32_t index_based_en          : 1;
	uint32_t byte_align_en           : 1;
	uint32_t early_done_irq_en       : 1;
	uint32_t irq_sub_sample_en       : 1;
	uint32_t reserved_0              : 4;
	uint32_t client_cfg_num_context  : 5;
	uint32_t client_cfg_ipcc_id      : 5;
	uint32_t reserved_1              : 11;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_linear_size - Place holder for client info.
 *
 * @bits:                Bit fields for the client info.
 * @dword:               dword
 *
 */
union cam_bus_wr_info {
	struct cam_bus_wr_info_mask  bits;
	uint32_t                     dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_linear_size_mask - Place holder for Linear size.
 *
 * @max_width:           Max width: BIT_0-BIT_15.
 * @max_height:          Max width: BIT_16-BIT_31.
 *
 */
struct cam_bus_wr_linear_size_mask {
	uint32_t max_width   : 16;
	uint32_t max_height  : 16;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_linear_size - Place holder for linear size.
 *
 * @bits:                Bit fields for the dimensions.
 * @dword:               dword
 *
 */
union cam_bus_wr_linear_size {
	struct cam_bus_wr_linear_size_mask  bits;
	uint32_t                            dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_ubwc_size_mask - Place holder for UBWC size.
 *
 * @max_width:           Max width: BIT_0-BIT_15.
 * @max_height:          Max width: BIT_16-BIT_31.
 *
 */
struct cam_bus_wr_ubwc_size_mask {
	uint32_t max_width   : 16;
	uint32_t max_height  : 16;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_ubwc_size - Place holder for UBWC size.
 *
 * @bits:                Bit fields for the dimensions.
 * @dword:               dword
 *
 */
union cam_bus_wr_ubwc_size {
	struct cam_bus_wr_ubwc_size_mask  bits;
	uint32_t                              dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_linear_format_mask - Bit fields for Linear formats.
 *
 * @plain128          :  BIT_0
 * @plain8_mipi8      :  BIT_1
 * @plain8_swap       :  BIT_2
 * @plain8_shift      :  BIT_3
 * @plain8_shift_swap :  BIT_4
 * @plain16_10        :  BIT_5
 * @plain16_12        :  BIT_6
 * @plain16_14        :  BIT_7
 * @plain16_16        :  BIT_8
 * @plain32           :  BIT_9
 * @plain64           :  BIT_10
 * @tp10              :  BIT_11
 * @mipi10            :  BIT_12
 * @mipi12            :  BIT_13
 * @mipi14            :  BIT_14
 * @mipi20            :  BIT_15
 * @plain32_20        :  BIT_16
 * @plain24           :  BIT_17
 * @plain48           :  BIT_18
 * @reserved          :  BIT_31:BIT_19
 */
struct cam_bus_wr_linear_format_mask {
	uint32_t plain128           : 1;
	uint32_t plain8_mipi8       : 1;
	uint32_t plain8_swap        : 1;
	uint32_t plain8_shift       : 1;
	uint32_t plain8_shift_swap  : 1;
	uint32_t plain16_10         : 1;
	uint32_t plain16_12         : 1;
	uint32_t plain16_14         : 1;
	uint32_t plain16_16         : 1;
	uint32_t plain32            : 1;
	uint32_t plain64            : 1;
	uint32_t tp10               : 1;
	uint32_t mipi10             : 1;
	uint32_t mipi12             : 1;
	uint32_t mipi14             : 1;
	uint32_t mipi20             : 1;
	uint32_t plain32_20         : 1;
	uint32_t plain24            : 1;
	uint32_t plain48            : 1;
	uint32_t reserved           : 13;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_linear_format - Place holder for Linear formats.
 *
 * @bits:                Bit fields for the formats.
 * @dword:               dword
 *
 */
union cam_bus_wr_linear_format {
	struct cam_bus_wr_linear_format_mask  bits;
	uint32_t                              dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_ubwc_format_mask - Bit fields for UBWC formats.
 *
 * @nv12_y    :  BIT_0
 * @nv12_uv   :  BIT_1
 * @tp10_y    :  BIT_2
 * @tp10_uv   :  BIT_3
 * @nv12_4r_Y :  BIT_4
 * @nv12_4r_UV:  BIT_5
 * @p010_y    :  BIT_6
 * @p010_uv   :  BIT_7
 * @p016_y    :  BIT_8
 * @p016_uv   :  BIT_9
 * @bayer_tP10:  BIT_10
 * @p012_y    :  BIT_11
 * @p012_uv   :  BIT_12
 * @g016_y    :  BIT_13
 * @g016_uv   :  BIT_14
 * @bayer16   :  BIT_15
 * @reserved  :  BIT_31:BIT_16
 *
 */
struct cam_bus_wr_ubwc_format_mask {
	uint32_t nv12_y      : 1;
	uint32_t nv12_uv     : 1;
	uint32_t tp10_y      : 1;
	uint32_t tp10_uv     : 1;
	uint32_t nv12_4r_Y   : 1;
	uint32_t nv12_4r_UV  : 1;
	uint32_t p010_y      : 1;
	uint32_t p010_uv     : 1;
	uint32_t p016_y      : 1;
	uint32_t p016_uv     : 1;
	uint32_t bayer_tP10  : 1;
	uint32_t p012_y      : 1;
	uint32_t p012_uv     : 1;
	uint32_t g016_y      : 1;
	uint32_t g016_uv     : 1;
	uint32_t bayer16     : 1;
	uint32_t reserved    : 16;
} __attribute((aligned(4)));

/**
 * union cam_bus_wr_ubwc_format - Place holder for UBWC formats.
 *
 * @bits:                Bit fields for the formats.
 * @dword:               dword
 *
 */
union cam_bus_wr_ubwc_format {
	struct cam_bus_wr_ubwc_format_mask  bits;
	uint32_t                        dword;
} __attribute((aligned(4)));

/**
 * struct cam_bus_wr_client - Place holder for Bus Clients.
 *
 * @info:                 Info about the client.
 * @linear_size:          Size info for Linear formats
 * @ubwc_size:            UBWC  and height info for Linear formats.
 * @client_sid_0_used:    SID0 information for client.
 * @client_sid_1_used:    SID1 information for client.
 * @linear_formats:       Linear Format supported.
 * @ubwc_formats:         UBWC Formats supported.
 *
 */
struct cam_bus_wr_client {
	union cam_bus_wr_info            info;
	union cam_bus_wr_linear_size     linear_size;
	union cam_bus_wr_ubwc_size       ubwc_size;
	uint32_t                         client_sid_0_used;
	uint32_t                         client_sid_1_used;
	union cam_bus_wr_linear_format   linear_format;
	union cam_bus_wr_ubwc_format     ubwc_format;
} __attribute((aligned(4)));

/**
 * struct cam_bus_hw_query_info - Place holder for querying the bus hw capabilities
 *
 * @client_present_0:        Valid Clients present; for the client id 0:31. BIT(i)-> Client_i
 * @client_present_1:        Valid Clients present; for the client id 32:63 BIT(i)-> Client_i
 * @src_present:             Source IDs present in a target. Range from 0-31, BIT(i)-> Src_i
 * @sub_grp_present:         Sub groups present in a target. Range from 0-31, BIT(i)-> sub_grp_i
 * @src_info:                Source information.
 * @src_grp_context0:        Multi context information for source group. Range from Source 0:7
 * @src_grp_context1:        Multi context information for source group. Range from Source 8:15
 * @sub_grp_info:            Sub group info for each sub group.
 * @num_msf_ports:           Number of MSF ports in a target for that IP.
 * @msf_info:                MSF info.
 * @msf_cfg_addr_width:      MSF addr width.
 * @msf_ipcc_cfg:            MSF information for IPCC.
 * @ipcc_present_0_id:       IPCC IDs present. Each bit represents one id. Range from 0:32.
 * @ipcc_present_1_id:       IPCC IDs present. Each bit represents one id. Range from 32:63.
 * @debug_feature:           Debug feature.
 * @wrapper_feature:         Wrapper feature.
 * @reserved:                reserved.
 * @client:                  Client Information.
 *
 */
struct cam_wr_bus_hw_query_info_v1 {
	uint32_t                                  client_present_0;
	uint32_t                                  client_present_1;
	uint32_t                                  src_present;
	uint32_t                                  sub_grp_present;
	struct cam_bus_wr_src_info                src_info[CAM_NUM_BUS_WR_SRC_INFO_V1];
	union cam_bus_wr_src_grp_context0         src_grp_context0;
	union cam_bus_wr_src_grp_context1         src_grp_context1;
	struct cam_bus_wr_sub_grp_info            sub_grp_info[CAM_NUM_BUS_WR_SUB_GRP_INFO_V1];
	uint32_t                                  num_msf_ports;
	struct cam_bus_wr_msf_info                msf_info[CAM_NUM_BUS_WR_MSF_INFO_V1];
	uint32_t                                  msf_cfg_addr_width;
	union cam_bus_wr_msf_ipcc_cfg             msf_ipcc_cfg;
	uint32_t                                  ipcc_present_0_id;
	uint32_t                                  ipcc_present_1_id;
	union cam_bus_wr_debug_feature            debug_feature;
	union cam_bus_wr_wrapper_feature          wrapper_feature;
	uint32_t                                  reserved[CAM_NUM_BUS_WR_RESERVED_V1];
	struct cam_bus_wr_client                  client[CAM_NUM_BUS_WR_CLIENT_V1];
} __attribute((aligned(4)));

#endif /* __CAM_BUS_COMMON_QUERY_V1_H__ */
