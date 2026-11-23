/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */


#ifndef _CAM_VFE_LITE_COMMON_REG_V1_H_
#define _CAM_VFE_LITE_COMMON_REG_V1_H_
#include "cam_vfe_camif_ver3.h"
#include "cam_vfe_top_ver4.h"
#include "cam_vfe_core.h"
#include "cam_vfe_bus_ver3.h"
#include "cam_irq_controller.h"

static struct cam_vfe_top_ver4_module_desc vfe_lite_common_reg_v1_ipp_mod_desc[] = {
	{
		.id = 0,
		.desc = "RESERVED",
	},
	{
		.id = 1,
		.desc = "CLC_BUS_WR",
	},
	{
		.id = 2,
		.desc = "CLC_CSID",
	},
	{
		.id = 3,
		.desc = "CLC_BLS",
	},
	{
		.id = 4,
		.desc = "CLC_GLUT",
	},
	{
		.id = 5,
		.desc = "CLC_STATS_BG",
	},
};

/*
 * Top HM registers, Offsets w.r.t top_hm_base. If top_hm_base is 0,
 * make these offsets relative core start address.
 */
static struct cam_irq_register_set vfe_lite_common_reg_v1_top_irq_reg_set[2] = {
	{
		.mask_reg_offset   = 0x164,
		.clear_reg_offset  = 0x174,
		.status_reg_offset = 0x154,
		.set_reg_offset    = 0x184,
		.test_set_val      = BIT(0),
		.test_sub_val      = BIT(0),
	},
	{
		.mask_reg_offset   = 0x168,
		.clear_reg_offset  = 0x178,
		.status_reg_offset = 0x158,
	},
};

static struct cam_irq_controller_reg_info vfe_lite_common_reg_v1_top_irq_reg_info = {
	.num_registers = 2,
	.irq_reg_set = vfe_lite_common_reg_v1_top_irq_reg_set,
	.global_irq_cmd_offset = 0x188,
	.global_clear_bitmask  = 0x1,
	.global_set_bitmask    = 0x10,
	.clear_all_bitmask     = 0xFFFFFFFF,
};

static uint32_t vfe_lite_common_reg_v1_top_debug_reg[] = {
	0x38C,
	0x390,
	0x394,
	0x398,
	0x39C,
};

static struct cam_vfe_top_ver4_reg_offset_common vfe_lite_common_reg_v1_common_reg = {
	/*
	 * Top HM registers, Offsets w.r.t top_hm_base. If top_hm_base is 0,
	 * make these offsets relative core start address.
	 */
	.hw_version               = 0x0,
	.hw_capability            = 0x0,
	.core_cgc_ovd_0           = 0x104,
	.ahb_cgc_ovd              = 0x108,
	.core_cfg_0               = 0x114,
	.diag_config              = 0x254,
	.diag_config_1            = 0x258,
	.diag_sensor_status       = {0x25C, 0x260},
	.diag_frm_cnt_status      = {0x264, 0x268},
	.ipp_violation_status     = 0x2A4,
	.top_debug_cfg            = 0x3DC,
	.num_top_debug_reg        = 5,
	.top_debug                = vfe_lite_common_reg_v1_top_debug_reg,
	/*
	 * Bus Wr registers, w.r.t bus_wr_base. If bus_wr_base is 0,
	 * make these offsets relative core start address.
	 */
	.bus_violation_status     = 0xB0,
	.bus_overflow_status      = 0xB8,
};

static struct cam_vfe_ver4_path_reg_data vfe_lite_common_reg_v1_ipp_reg_data = {
	.sof_irq_mask                    = 0x1,
	.eof_irq_mask                    = 0x2,
	.error_irq_mask                  = 0x6,
	.enable_diagnostic_hw            = 0x1,
	.ipp_violation_mask              = 0x2,
	.diag_violation_mask             = 0x4,
	.diag_sensor_sel_mask            = 0x40,
	.diag_frm_count_mask_1           = 0x100,
	.top_debug_cfg_en                = 0x3,
};

static struct cam_vfe_ver4_path_reg_data vfe_lite_common_reg_v1_rdi_reg_data[4] = {

	{
		.sof_irq_mask                    = 0x10000,
		.eof_irq_mask                    = 0x20000,
		.error_irq_mask                  = 0x0,
		.diag_sensor_sel_mask            = 0x0,
		.diag_frm_count_mask_0           = 0x80,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x40000,
		.eof_irq_mask                    = 0x80000,
		.error_irq_mask                  = 0x0,
		.diag_sensor_sel_mask            = 0x2,
		.diag_frm_count_mask_0           = 0x100,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x100000,
		.eof_irq_mask                    = 0x200000,
		.error_irq_mask                  = 0x0,
		.diag_sensor_sel_mask            = 0x4,
		.diag_frm_count_mask_0           = 0x200,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
	{
		.sof_irq_mask                    = 0x400000,
		.eof_irq_mask                    = 0x800000,
		.error_irq_mask                  = 0x0,
		.diag_sensor_sel_mask            = 0x6,
		.diag_frm_count_mask_0           = 0x400,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 0x3,
	},
};

static struct cam_vfe_ver4_path_hw_info
	vfe_lite_common_reg_v1_rdi_hw_info[] = {
	{
		.common_reg     = &vfe_lite_common_reg_v1_common_reg,
		.reg_data       = &vfe_lite_common_reg_v1_rdi_reg_data[0],
	},
	{
		.common_reg     = &vfe_lite_common_reg_v1_common_reg,
		.reg_data       = &vfe_lite_common_reg_v1_rdi_reg_data[1],
	},
	{
		.common_reg     = &vfe_lite_common_reg_v1_common_reg,
		.reg_data       = &vfe_lite_common_reg_v1_rdi_reg_data[2],
	},
	{
		.common_reg     = &vfe_lite_common_reg_v1_common_reg,
		.reg_data       = &vfe_lite_common_reg_v1_rdi_reg_data[3],
	},
};

static struct cam_vfe_top_ver4_debug_reg_info vfe_lite_common_reg_v1_dbg_reg_info[][8] = {
	VFE_DBG_INFO_ARRAY_4bit(
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved"
	),
	{
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
		VFE_DBG_INFO(32, "non-CLC info"),
	},
	VFE_DBG_INFO_ARRAY_4bit(
		"PP_THROTTLE",
		"STATS_BG_THROTTLE",
		"STATS_BG",
		"BLS",
		"GLUT",
		"unused",
		"unused",
		"unused"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"RDI_0",
		"RDI_1",
		"RDI_2",
		"RDI_3",
		"PP_STATS_BG",
		"PP_GLUT",
		"PP_STATS_BG",
		"PP_GLUT"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"unused",
		"unused",
		"unused",
		"unused",
		"unused",
		"unused",
		"unused",
		"unused"
	),
};

static struct cam_vfe_top_ver4_diag_reg_info vfe_lite_common_reg_v1_diag_reg_info[] = {
	{
		.bitmask = 0x3FFF,
		.name    = "SENSOR_HBI",
	},
	{
		.bitmask = 0x4000,
		.name    = "SENSOR_NEQ_HBI",
	},
	{
		.bitmask = 0x8000,
		.name    = "SENSOR_HBI_MIN_ERROR",
	},
	{
		.bitmask = 0xFFFFFF,
		.name    = "SENSOR_VBI",
	},
	{
		.bitmask = 0xFF,
		.name    = "FRAME_CNT_RDI_0_PIPE",
	},
	{
		.bitmask = 0xFF00,
		.name    = "FRAME_CNT_RDI_1_PIPE",
	},
	{
		.bitmask = 0xFF0000,
		.name    = "FRAME_CNT_RDI_2_PIPE",
	},
	{
		.bitmask = 0xFF000000,
		.name    = "FRAME_CNT_RDI_3_PIPE",
	},
	{
		.bitmask = 0xFF,
		.name    = "FRAME_CNT_IPP_PIPE",
	},
};

static struct cam_vfe_top_ver4_diag_reg_fields vfe_lite_common_reg_v1_diag_sensor_field[] = {
	{
		.num_fields = 3,
		.field      = &vfe_lite_common_reg_v1_diag_reg_info[0],
	},
	{
		.num_fields = 1,
		.field      = &vfe_lite_common_reg_v1_diag_reg_info[3],
	},
};

static struct cam_vfe_top_ver4_diag_reg_fields vfe_lite_common_reg_v1_diag_frame_field[] = {
	{
		.num_fields = 4,
		.field      = &vfe_lite_common_reg_v1_diag_reg_info[4],
	},
	{
		.num_fields = 1,
		.field      = &vfe_lite_common_reg_v1_diag_reg_info[8],
	},
};

static struct cam_vfe_top_ver4_hw_info vfe_lite_common_reg_v1_top_hw_info = {
	.common_reg = &vfe_lite_common_reg_v1_common_reg,
	.rdi_hw_info = vfe_lite_common_reg_v1_rdi_hw_info,
	.vfe_full_hw_info = {
		.common_reg     = &vfe_lite_common_reg_v1_common_reg,
		.reg_data       = &vfe_lite_common_reg_v1_ipp_reg_data,
	},
	.ipp_module_desc        = vfe_lite_common_reg_v1_ipp_mod_desc,
	.num_mux = 5,
	.mux_type = {
		CAM_VFE_CAMIF_VER_4_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
	},
	.top_debug_reg_info = &vfe_lite_common_reg_v1_dbg_reg_info,
	.num_rdi        = ARRAY_SIZE(vfe_lite_common_reg_v1_rdi_hw_info),
	.diag_sensor_info = vfe_lite_common_reg_v1_diag_sensor_field,
	.diag_frame_info  = vfe_lite_common_reg_v1_diag_frame_field,
	.top_hm_base                      = 0x9000,
	.bus_wr_base                      = 0x9800,

};

/*
 * Bus Wr registers, w.r.t bus_wr_base. If bus_wr_base is 0,
 * make these offsets relative core start address.
 */
static struct cam_irq_register_set vfe_lite_common_reg_v1_bus_irq_reg[1] = {
	{
		.mask_reg_offset   = 0x10,
		.clear_reg_offset  = 0x24,
		.status_reg_offset = 0x38,
	},
};

static struct cam_vfe_bus_ver3_hw_info vfe_lite_common_reg_v1_bus_hw_info = {
	.common_reg = {
		/*
		 * Bus Wr registers, w.r.t bus_wr_base. If bus_wr_base is 0,
		 * make these offsets relative core start address.
		 */
		.hw_version                       = 0x0,
		.cgc_ovd                          = 0x4,
		.pwr_iso_cfg                      = 0xA8,
		.overflow_status_clear            = 0xAC,
		.ccif_violation_status            = 0xB0,
		.overflow_status                  = 0xB8,
		.image_size_violation_status      = 0xC0,
		.debug_status_top_cfg             = 0x12C,
		.debug_status_top                 = 0x130,
		.test_bus_ctrl                    = 0x144,
		.wm_mode_shift                    = 16,
		.wm_mode_val                      = { 0x0, 0x1, 0x2 },
		.wm_en_shift                      = 0,
		.frmheader_en_shift               = 2,
		.virtual_frm_en_shift             = 1,
		.irq_reg_info = {
			.num_registers            = 1,
			.irq_reg_set              = vfe_lite_common_reg_v1_bus_irq_reg,
			.global_irq_cmd_offset    = 0x48,
			.global_clear_bitmask     = 0x1,
		},
	},
	.bus_wr_base                              = 0x9800,
	.num_client = 6,
	.support_dyn_offset                       = true,
	/*
	 * client_base is w.r.t bus_wr_base. If bus_wr_base is 0,
	 * make client_base relative core start address.
	 */
	.client_base                              = 0x1000,
	.client_reg_size                          = 0x200,
	.client_offsets = {
			.cfg                      = 0x0,
			.image_addr               = 0x4,
			.frame_incr               = 0x8,
			.image_cfg_0              = 0xC,
			.image_cfg_1              = 0x10,
			.image_cfg_2              = 0x14,
			.packer_cfg               = 0x18,
			.frame_header_addr        = 0x20,
			.frame_header_incr        = 0x24,
			.frame_header_cfg         = 0x28,
			.line_done_cfg            = 0x2C,
			.irq_subsample_period     = 0x30,
			.irq_subsample_pattern    = 0x34,
			.mmu_prefetch_cfg         = 0x64,
			.mmu_prefetch_max_offset  = 0x68,
			.system_cache_cfg         = 0x6C,
			.addr_cfg                 = 0x70,
			.debug_status_cfg         = 0x78,
			.debug_status_0           = 0x7C,
			.debug_status_1           = 0x80,
			.addr_status_0            = 0x8C,
			.addr_status_1            = 0x90,
			.addr_status_2            = 0x94,
			.addr_status_3            = 0x98,
		},
	.bus_client_reg = {
		/* BUS Client 0 RDI0 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_1,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "LITE_RDI_0",
			.line_based               = 1,
			.mid                      = {32},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI0,
			.pid_mask                 = BIT_ULL(17) | BIT_ULL(18),
		},
		/* BUS Client 1 RDI1 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "LITE_RDI_1",
			.line_based               = 1,
			.mid                      = {33},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI1,
			.pid_mask                 = BIT_ULL(17) | BIT_ULL(18),
		},
		/* BUS Client 2 RDI2 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "LITE_RDI_2",
			.line_based               = 1,
			.mid                      = {34},
			.num_mid                  = 1,
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI2,
			.pid_mask                 = BIT_ULL(17) | BIT_ULL(18),
		},
		/* BUS Client 3 RDI3 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_4,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "LITE_RDI_3",
			.line_based               = 1,
			.mid                      = {35},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI3,
			.pid_mask                 = BIT_ULL(17) | BIT_ULL(18),
		},
		/* BUS Client 4 Gamma */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_MIPI12) |
				BIT_ULL(PACKER_FMT_VER3_MIPI14) |
				BIT_ULL(PACKER_FMT_VER3_MIPI20) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "PREPROCESS_RAW",
			.mid                      = {36},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_PREPROCESS_RAW,
			.pid_mask                 = BIT_ULL(17) | BIT_ULL(18),
		},
		/* BUS Client 5 Stats BE */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "STATS_BG",
			.mid                      = {37},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_BG,
			.pid_mask                 = BIT_ULL(17) | BIT_ULL(18),
		},
	},
	.valid_wm_mask   = 0x1F,
	.num_comp_grp    = 5,
	.support_consumed_addr = true,
	.comp_done_mask = {
		(BIT(0) | BIT(1) | BIT(2)), BIT(16), BIT(17), BIT(18), BIT(19),
	},
	.top_irq_shift   = 0,
	.max_out_res = CAM_ISP_IFE_OUT_RES_BASE + 34,
};

static struct cam_vfe_irq_hw_info vfe_lite_common_reg_v1_irq_hw_info = {
	.reset_mask    = 0,
	.supported_irq = CAM_VFE_HW_IRQ_CAP_LITE_EXT_CSID,
	.top_irq_reg   = &vfe_lite_common_reg_v1_top_irq_reg_info,
};

static struct cam_vfe_hw_info cam_vfe_lite_common_reg_v1_hw_info = {
	.irq_hw_info                   = &vfe_lite_common_reg_v1_irq_hw_info,

	.bus_version                   = CAM_VFE_BUS_VER_3_0,
	.bus_hw_info                   = &vfe_lite_common_reg_v1_bus_hw_info,

	.top_version                   = CAM_VFE_TOP_VER_4_0,
	.top_hw_info                   = &vfe_lite_common_reg_v1_top_hw_info,
};

#endif /* _CAM_VFE_LITE_COMMON_REG_V1_H_ */
