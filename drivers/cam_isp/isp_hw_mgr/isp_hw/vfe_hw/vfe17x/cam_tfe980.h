/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023-2025, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_TFE980_H_
#define _CAM_TFE980_H_
#include "cam_vfe_top_ver4.h"
#include "cam_vfe_core.h"
#include "cam_vfe_bus_ver3.h"
#include "cam_irq_controller.h"

#define CAM_TFE_980_NUM_TOP_DBG_REG          17
#define CAM_TFE_980_NUM_BAYER_DBG_REG        10
#define CAM_TFE_BUS_VER3_980_MAX_CLIENTS     28

static struct cam_vfe_top_ver4_module_desc tfe980_ipp_mod_desc[] = {
	{
		.id = 0,
		.desc = "CLC_STATS_AWB_BG_TINTLESS",
	},
	{
		.id  = 1,
		.desc = "CLC_STATS_AWB_BG_AE",
	},
	{
		.id = 2,
		.desc = "CLC_STATS_BHIST_AEC",
	},
	{
		.id = 3,
		.desc = "CLC_STATS_RS",
	},
	{
		.id = 4,
		.desc = "CLC_STATS_BFW_AWB",
	},
	{
		.id = 5,
		.desc = "CLC_STATS_AWB_BG_AWB",
	},
	{
		.id = 6,
		.desc = "CLC_STATS_BHIST_AF",
	},
	{
		.id = 7,
		.desc = "CLC_STATS_AWB_BG_ALSC",
	},
	{
		.id = 8,
		.desc = "CLC_STATS_BHIST_TMC",
	},
	{
		.id = 9,
		.desc = "CLC_COMPDECOMP_FD",
	},
	{
		.id = 10,
		.desc = "CLC_COLOR_CORRECT",
	},
	{
		.id = 11,
		.desc = "CLC_GTM",
	},
	{
		.id = 12,
		.desc = "CLC_GLUT",
	},
	{
		.id = 13,
		.desc = "CLC_COLOR_XFORM",
	},
	{
		.id = 14,
		.desc = "CLC_DOWNSCALE_MN_Y",
	},
	{
		.id  = 15,
		.desc = "CLC_DOWNSCALE_MN_C",
	},
	{
		.id = 16,
		.desc = "CLC_CROP_RND_CLAMP_FD_Y",
	},
	{
		.id = 17,
		.desc = "CLC_CROP_RND_CLAMP_FD_C",
	},
	{
		.id = 18,
		.desc = "CLC_BDS2_DEMO",
	},
	{
		.id = 19,
		.desc = "CLC_PUNCH_BDS2",
	},
	{
		.id = 20,
		.desc = "CLC_PUNCH_DS4_MUX",
	},
	{
		.id = 21,
		.desc = "CLC_BAYER_DS_4_DS4",
	},
	{
		.id = 22,
		.desc = "CLC_CROP_RND_CLAMP_DS4"
	},
	{
		.id = 23,
		.desc = "CLC_PUNCH_DS16"
	},
	{
		.id = 24,
		.desc = "CLC_BAYER_DS_4_DS16",
	},
	{
		.id = 25,
		.desc = "CLC_CROP_RND_CLAMP_DS16",
	},
	{
		.id = 26,
		.desc = "CLC_CROP_RND_CLAMP_DS2",
	},
	{
		.id = 27,
		.desc = "CLC_RCS_DS2",
	},
	{
		.id = 28,
		.desc = "CLC_CROP_RND_CLAMP_FULL_OUT",
	},
	{
		.id = 29,
		.desc = "CLC_COMPDECOMP_BYPASS",
	},
	{
		.id = 30,
		.desc = "CLC_CROP_RND_CLAMP_BYPASS",
	},
	{
		.id = 31,
		.desc = "CLC_RCS_FULL_OUT",
	},
};

struct cam_vfe_top_ver4_module_desc tfe980_bayer_mod_desc[] = {
	{
		.id = 0,
		.desc = "CLC_DEMUX",
	},
	{
		.id = 1,
		.desc = "CLC_BPC_PDPC_GIC",
	},
	{
		.id = 2,
		.desc = "CLC_PDPC_BPC_1D",
	},
	{
		.id = 3,
		.desc = "CLC_ABF_BINC",
	},
	{
		.id = 4,
		.desc = "CLC_CHANNEL_GAINS",
	},
	{
		.id = 5,
		.desc = "CLC_LSC",
	},
	{
		.id = 6,
		.desc = "CLC_FCG",
	},
	{
		.id = 7,
		.desc = "CLC_WB_GAIN",
	},
	{
		.id = 8,
		.desc = "CLC_COMPDECOMP_BAYER",
	},
	{
		.id = 9,
		.desc = "CLC_CROP_RND_CLAMP_WIRC",
	},
};

static struct cam_vfe_top_ver4_top_err_irq_desc tfe980_top_irq_err_desc[] = {
	{
		.bitmask = BIT(2),
		.err_name = "BAYER_HM violation",
		.desc = "CLC CCIF Violation",
	},
	{
		.bitmask = BIT(24),
		.err_name = "DYNAMIC PDAF SWITCH VIOLATION",
		.desc =
			"HAF RDI exposure select changes dynamically, the common vbi is insufficient",
	},
	{
		.bitmask = BIT(25),
		.err_name  = "HAF violation",
		.desc = "CLC_HAF Violation",
	},
	{
		.bitmask = BIT(26),
		.err_name = "PP VIOLATION",
		.desc = "CCIF protocol violation",
	},
	{
		.bitmask  = BIT(27),
		.err_name = "DIAG VIOLATION",
		.desc = "Sensor: The HBI at TFE input is less than the spec (64 cycles)",
		.debug = "Check sensor config",
	},
};

static struct cam_vfe_top_ver4_pdaf_violation_desc tfe980_haf_violation_desc[] = {
	{
		.bitmask = BIT(0),
		.desc = "Sim monitor 1 violation - SAD output",
	},
	{
		.bitmask = BIT(1),
		.desc = "Sim monitor 2 violation - pre-proc output",
	},
	{
		.bitmask = BIT(2),
		.desc = "Sim monitor 3 violation - parsed output",
	},
	{
		.bitmask = BIT(3),
		.desc = "Sim monitor 4 violation - CAF output",
	},
	{
		.bitmask  = BIT(4),
		.desc = "PDAF constraint violation",
	},
	{
		.bitmask = BIT(5),
		.desc = "CAF constraint violation",
	},
};

/*
 * Top HM registers, Offsets w.r.t top_hm_base. If top_hm_base is 0,
 * make these offsets relative core start address.
 */
static struct cam_irq_register_set tfe980_top_irq_reg_set = {
	.mask_reg_offset   = 0x00000080,
	.clear_reg_offset  = 0x00000084,
	.status_reg_offset = 0x00000088,
	.set_reg_offset    = 0x0000008C,
	.test_set_val      = BIT(0),
	.test_sub_val      = BIT(0),
};

static struct cam_irq_controller_reg_info tfe980_top_irq_reg_info = {
	.num_registers = 1,
	.irq_reg_set = &tfe980_top_irq_reg_set,
	.global_irq_cmd_offset = 0x0000007C,
	.global_clear_bitmask  = 0x00000001,
	.global_set_bitmask    = 0x00000010,
	.clear_all_bitmask     = 0xFFFFFFFF,
};

static uint32_t tfe980_top_debug_reg[] = {
	0x00000190,
	0x00000194,
	0x00000198,
	0x0000019C,
	0x000001A0,
	0x000001A4,
	0x000001A8,
	0x000001AC,
	0x000001B0,
	0x000001B4,
	0x000001B8,
	0x000001BC,
	0x000001C0,
	0x000001C4,
	0x000001C8,
	0x000001CC,
	0x000001D0,
};

static struct cam_vfe_top_ver4_debug_reg_info tfe980_top_dbg_reg_info[
	CAM_TFE_980_NUM_TOP_DBG_REG][8] = {
	VFE_DBG_INFO_ARRAY_4bit("test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved"
	),
	{
		VFE_DBG_INFO_WITH_IDLE(0, "STATS_AWB_BG_TINTLESS",
			0x000001D4, (BIT(0) | BIT(1) | BIT(2))),
		VFE_DBG_INFO_WITH_IDLE(4, "STATS_AWB_BG_AE",
			0x000001D4, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(8, "STATS_BHIST_AEC",
			0x000001D4, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(12, "STATS_RS",
			0x000001D4, (BIT(9) | BIT(10) | BIT(11))),
		VFE_DBG_INFO_WITH_IDLE(16, "STATS_BFW_AWB",
			0x000001D4, (BIT(12) | BIT(13) | BIT(14))),
		VFE_DBG_INFO_WITH_IDLE(20, "STATS_AWB_BG_AWB",
			0x000001D4, (BIT(15) | BIT(16) | BIT(17))),
		VFE_DBG_INFO_WITH_IDLE(24, "STATS_BHIST_AF",
			0x000001D4, (BIT(18) | BIT(19) | BIT(20))),
		VFE_DBG_INFO_WITH_IDLE(28, "STATS_AWB_BG_ALSC",
			0x000001D4, (BIT(21) | BIT(22) | BIT(23))),
	},
	{
		VFE_DBG_INFO_WITH_IDLE(0, "STATS_BHIST_TMC",
			0x000001D4, (BIT(24) | BIT(25) | BIT(26))),
		VFE_DBG_INFO_WITH_IDLE(4, "compdecomp_fd",
			0x000001D4, BIT(27)),
		VFE_DBG_INFO_WITH_IDLE(8, "color_correct",
			0x000001D4, BIT(28)),
		VFE_DBG_INFO_WITH_IDLE(12, "gtm",
			0x000001D4, BIT(29)),
		VFE_DBG_INFO_WITH_IDLE(16, "glut",
			0x000001D4, BIT(30)),
		VFE_DBG_INFO_WITH_IDLE(20, "color_xform",
			0x000001D4, BIT(31)),
		VFE_DBG_INFO_WITH_IDLE(24, "downscale_mn_y",
			0x000001D8, BIT(0)),
		VFE_DBG_INFO_WITH_IDLE(28, "downscale_mn_c",
			0x000001D8, BIT(1)),
	},
	{
		VFE_DBG_INFO_WITH_IDLE(0, "crop_rnd_clamp_fd_y",
			0x000001D8, BIT(2)),
		VFE_DBG_INFO_WITH_IDLE(4, "crop_rnd_clamp_fd_c",
			0x000001D8, BIT(3)),
		VFE_DBG_INFO_WITH_IDLE(8, "bds2_demo",
			0x000001D8, (BIT(4) | BIT(5) | BIT(6))),
		VFE_DBG_INFO_WITH_IDLE(12, "punch_bds2",
			0x000001D8, (BIT(7) | BIT(8) | BIT(9))),
		VFE_DBG_INFO_WITH_IDLE(16, "punch_ds4_mux",
			0x000001D8, (BIT(10) | BIT(11) | BIT(12))),
		VFE_DBG_INFO_WITH_IDLE(20, "bayer_ds_4_ds4",
			0x000001D8, (BIT(13) | BIT(14) | BIT(15))),
		VFE_DBG_INFO_WITH_IDLE(24, "crop_rnd_clamp_ds4",
			0x000001D8, (BIT(16) | BIT(17) | BIT(18))),
		VFE_DBG_INFO_WITH_IDLE(28, "punch_ds16",
			0x000001D8, (BIT(19) | BIT(20) | BIT(21))),
	},
	{
		VFE_DBG_INFO_WITH_IDLE(0, "bayer_ds_4_ds16",
			0x000001D8, (BIT(22) | BIT(23) | BIT(24))),
		VFE_DBG_INFO_WITH_IDLE(4, "crop_rnd_clamp_ds16",
			0x000001D8, (BIT(25) | BIT(26) | BIT(27))),
		VFE_DBG_INFO_WITH_IDLE(8, "crop_rnd_clamp_ds2",
			0x000001D8, (BIT(28) | BIT(29) | BIT(30))),
		VFE_DBG_INFO_WITH_IDLE(12, "clc_haf",
			0x000001D8, BIT(31)),
		VFE_DBG_INFO_WITH_IDLE(16, "clc_rcs_ds2",
			0x000001DC, (BIT(0) | BIT(1) | BIT(2))),
		VFE_DBG_INFO_WITH_IDLE(20, "clc_crop_rnd_clamp_full_out",
			0x000001DC, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(24, "clc_compdecomp_bypass",
			0x000001DC, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(28, "clc_crop_rnd_clamp_bypass",
			0x000001DC, (BIT(9) | BIT(10) | BIT(11))),
	},
	{
		VFE_DBG_INFO_WITH_IDLE(0, "clc_rcs_full_out",
			0x000001DC, (BIT(12) | BIT(13) | BIT(14))),
		VFE_DBG_INFO_WITH_IDLE(4, "clc_haf",
			0x000001DC, BIT(15)),
		VFE_DBG_INFO_WITH_IDLE(8, "csid_tfe_ipp",
			0x000001DC, (BIT(16) | BIT(17) | BIT(18))),
		VFE_DBG_INFO_WITH_IDLE(12, "ppp_repeater",
			0x000001DC, BIT(19)),
		VFE_DBG_INFO_WITH_IDLE(16, "stats_awb_bg_tintless_throttle",
			0x000001DC, (BIT(20) | BIT(21) | BIT(22))),
		VFE_DBG_INFO_WITH_IDLE(20, "stats_awb_bg_ae_throttle",
			0x000001DC, (BIT(23) | BIT(24) | BIT(25))),
		VFE_DBG_INFO_WITH_IDLE(24, "stats_ae_bhist_throttle",
			0x000001DC, (BIT(26) | BIT(27) | BIT(28))),
		VFE_DBG_INFO_WITH_IDLE(28, "stats_bayer_rs_throttle",
			0x000001DC, (BIT(29) | BIT(30) | BIT(31))),
	},
	{
		VFE_DBG_INFO_WITH_IDLE(0, "stats_bayer_bfw_throttle",
			0x000001E0, (BIT(0) | BIT(1) | BIT(2))),
		VFE_DBG_INFO_WITH_IDLE(4, "stats_awb_bg_awb_throttle",
			0x000001E0, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(8, "stats_bhist_af_throttle",
			0x000001E0, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(12, "full_out_throttle",
			0x000001E0, (BIT(9) | BIT(10) | BIT(11))),
		VFE_DBG_INFO_WITH_IDLE(16, "ds4_out_y_throttle",
			0x000001E0, (BIT(12) | BIT(13) | BIT(14))),
		VFE_DBG_INFO_WITH_IDLE(20, "ds4_out_c_throttle",
			0x000001E0, (BIT(15) | BIT(16) | BIT(17))),
		VFE_DBG_INFO_WITH_IDLE(24, "ds16_out_y_throttle",
			0x000001E0, (BIT(18) | BIT(19) | BIT(20))),
		VFE_DBG_INFO_WITH_IDLE(28, "ds16_out_c_throttle",
			0x000001E0, (BIT(21) | BIT(22) | BIT(23))),
	},
	{
		VFE_DBG_INFO_WITH_IDLE(0, "ds2_out_y_throttle",
			0x000001E0, (BIT(24) | BIT(25) | BIT(26))),
		VFE_DBG_INFO_WITH_IDLE(4, "ds2_out_c_throttle",
			0x000001E0, (BIT(27) | BIT(28) | BIT(29))),
		VFE_DBG_INFO_WITH_IDLE(8, "tfe_w_ir_throttle",
			0x000001E4, (BIT(0) | BIT(1) | BIT(2))),
		VFE_DBG_INFO_WITH_IDLE(12, "fd_out_y_throttle",
			0x000001E4, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(16, "fd_out_c_throttle",
			0x000001E4, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(20, "haf_sad_stats_throttle",
			0x000001E0, BIT(30)),
		VFE_DBG_INFO_WITH_IDLE(24, "haf_caf_stats_throttle",
			0x000001E0, BIT(31)),
		VFE_DBG_INFO_WITH_IDLE(28, "haf_parsed_throttle",
			0x000001E4, BIT(9)),
	},
	{
		VFE_DBG_INFO_WITH_IDLE(0, "haf_pre_processed",
			0x000001E4, BIT(10)),
		VFE_DBG_INFO(4, "full_out"),
		VFE_DBG_INFO_CHECK_OVERFLOW(8, "full_ubwc_stats"),
		VFE_DBG_INFO(12, "ds4_out_y"),
		VFE_DBG_INFO(16, "ds4_out_c"),
		VFE_DBG_INFO(20, "ds16_out_y"),
		VFE_DBG_INFO(24, "ds16_out_c"),
		VFE_DBG_INFO(28, "ds2_out_y"),
	},
	{
		VFE_DBG_INFO_CHECK_OVERFLOW(0, "ds2_ubwc_stats"),
		VFE_DBG_INFO(4, "ds2_out_c"),
		VFE_DBG_INFO(8, "fd_out_y"),
		VFE_DBG_INFO(12, "fd_out_c"),
		VFE_DBG_INFO(16, "raw_out"),
		VFE_DBG_INFO(20, "stats_awb_bg_ae"),
		VFE_DBG_INFO(24, "stats_ae_bhist"),
		VFE_DBG_INFO(28, "stats_awb_bg_tintless"),
	},
	{
		VFE_DBG_INFO_WITH_IDLE(0, "stats_awb_bg_alsc",
			0x000001E4, (BIT(20) | BIT(21) | BIT(22))),
		VFE_DBG_INFO(4, "stats_throttle_to_bus_awb_bg_awb"),
		VFE_DBG_INFO(8, "stats_throttle_to_bus_bayer_bfw"),
		VFE_DBG_INFO(12, "stats_throttle_to_bus_bhist_af"),
		VFE_DBG_INFO(16, "stats_throttle_to_bus_awb_bg_alsc"),
		VFE_DBG_INFO(20, "stats_throttle_to_bus_bayer_rs"),
		VFE_DBG_INFO(24, "stats_throttle_to_bus_bhist_tmc"),
		VFE_DBG_INFO(28, "stats_throttle_to_bus_sad"),

	},
	VFE_DBG_INFO_ARRAY_4bit(
		"tfe_haf_processed_to_bus",
		"tfe_haf_parsed_to_bus",
		"tfe_stats_throttle_to_bus",
		"rdi0_splitter_to_bus_wr",
		"rdi1_splitter_to_bus_wr",
		"rdi2_splitter_to_bus_wr",
		"rdi3_splitter_to_bus_wr",
		"rdi4_splitter_to_bus_wr"
	),
	{
		VFE_DBG_INFO_WITH_IDLE(0, "stats_bhist_tmc_throttle",
			0x000001E4, (BIT(23) | BIT(24) | BIT(25))),
		VFE_DBG_INFO(4, "reserved"),
		VFE_DBG_INFO(8, "reserved"),
		VFE_DBG_INFO(12, "reserved"),
		VFE_DBG_INFO(16, "reserved"),
		VFE_DBG_INFO(20, "reserved"),
		VFE_DBG_INFO(24, "reserved"),
		VFE_DBG_INFO(28, "reserved"),
	},
	{
		/* needs to be parsed separately, doesn't conform to I, V, R */
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
	},
	{
		/* needs to be parsed separately, doesn't conform to I, V, R */
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
	},
	{
		/* needs to be parsed separately, doesn't conform to I, V, R */
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
	},
	{
		/* needs to be parsed separately, doesn't conform to I, V, R */
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
	},
};

/*
 * Bayer HM registers, Offsets w.r.t bayer_hm_base. If bayer_hm_base is 0,
 * make these offsets relative core start address.
 */
static uint32_t tfe980_bayer_debug_reg[] = {
	0x000001BC,
	0x000001C0,
	0x000001C4,
	0x000001C8,
	0x000001CC,
	0x000001D0,
	0x000001D4,
	0x000001D8,
	0x000001DC,
	0x000001E0,
};

static struct cam_vfe_top_ver4_debug_reg_info tfe980_bayer_dbg_reg_info[
	CAM_TFE_980_NUM_BAYER_DBG_REG][8] = {
	VFE_DBG_INFO_ARRAY_4bit("test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved"
	),
	{
		VFE_DBG_INFO_WITH_IDLE(0, "clc_demux_w0",
			0x000001E4, (BIT(0) | BIT(1) | BIT(2))),
		VFE_DBG_INFO_WITH_IDLE(4, "clc_bpc_pdpc_gic_w0",
			0x000001E4, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(8, "clc_pdpc_bpc_1d_w0",
			0x000001E4, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(12, "clc_abf_binc_w0",
			0x000001E4, (BIT(9) | BIT(10) | BIT(11))),
		VFE_DBG_INFO_WITH_IDLE(16, "clc_channel_gains_w0",
			0x000001E4, (BIT(12) | BIT(13) | BIT(14))),
		VFE_DBG_INFO_WITH_IDLE(20, "clc_lsc_w3",
			0x000001E4, (BIT(15) | BIT(16) | BIT(17))),
		VFE_DBG_INFO_WITH_IDLE(24, "clc_fcg_w2",
			0x000001E4, (BIT(18) | BIT(19) | BIT(20))),
		VFE_DBG_INFO_WITH_IDLE(28, "clc_wb_gain_w6",
			0x000001E4, (BIT(21) | BIT(22) | BIT(23))),
	},
	{
		VFE_DBG_INFO_WITH_IDLE(0, "clc_compdecomp_bayer_w0",
			0x000001E4, (BIT(24) | BIT(25) | BIT(26))),
		VFE_DBG_INFO_WITH_IDLE(4, "clc_crop_rnd_clamp_wirc_w10",
			0x000001E4, BIT(27)),
		VFE_DBG_INFO(8, "reserved"),
		VFE_DBG_INFO(12, "reserved"),
		VFE_DBG_INFO(16, "reserved"),
		VFE_DBG_INFO(20, "reserved"),
		VFE_DBG_INFO(24, "reserved"),
		VFE_DBG_INFO(28, "reserved"),
	},
	VFE_DBG_INFO_ARRAY_4bit(
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved"
	),
	VFE_DBG_INFO_ARRAY_4bit(
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved",
		"reserved"
	),
	{
		/* needs to be parsed separately, doesn't conform to I, V, R */
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
		VFE_DBG_INFO(32, "non_ccif_0"),
	},
	{
		/* needs to be parsed separately, doesn't conform to I, V, R */
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
		VFE_DBG_INFO(32, "non_ccif_1"),
	},
	{
		/* needs to be parsed separately, doesn't conform to I, V, R */
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
		VFE_DBG_INFO(32, "non_ccif_2"),
	},
	{
		/* needs to be parsed separately, doesn't conform to I, V, R */
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
		VFE_DBG_INFO(32, "non_ccif_3"),
	},
};

static struct cam_vfe_top_ver4_reg_offset_common tfe980_common_reg = {
	/*
	 * Top HM registers, Offsets w.r.t top_hm_base. If top_hm_base is 0,
	 * make these offsets relative core start address.
	 */
	.hw_version               = 0x00000000,
	.hw_capability            = 0x00000004,
	.main_feature             = 0x00000008,
	.bayer_feature            = 0x0000000C,
	.stats_feature            = 0x00000010,
	.fd_feature               = 0x00000014,
	.core_cgc_ovd_0           = 0x00000018,
	.ahb_cgc_ovd              = 0x00000020,
	.core_mux_cfg             = 0x00000024,
	.pdaf_input_cfg_0         = 0x00000028,
	.pdaf_input_cfg_1         = 0x0000002C,
	.stats_throttle_cfg_0     = 0x00000030,
	.stats_throttle_cfg_1     = 0x00000034,
	.stats_throttle_cfg_2     = 0x00000038,
	.core_cfg_4               = 0x0000003C,
	.pdaf_parsed_throttle_cfg = 0x00000040,
	.wirc_throttle_cfg        = 0x00000044,
	.fd_y_throttle_cfg        = 0x00000048,
	.fd_c_throttle_cfg        = 0x0000004C,
	.ds16_g_throttle_cfg      = 0x00000050,
	.ds16_br_throttle_cfg     = 0x00000054,
	.ds4_g_throttle_cfg       = 0x00000058,
	.ds4_br_throttle_cfg      = 0x0000005C,
	.ds2_g_throttle_cfg       = 0x00000060,
	.ds2_br_throttle_cfg      = 0x00000064,
	.full_out_throttle_cfg    = 0x00000068,
	.dsp_status               = 0x0000006C,
	.global_reset_cmd         = 0x0000007C,
	.diag_config              = 0x00000094,
	.diag_sensor_status       = {0x00000098, 0x0000009C},
	.diag_frm_cnt_status      = {0x000000A0, 0x000000A4, 0x000000A8},
	.ipp_violation_status     = 0x00000090,
	.num_perf_counters        = 4,
	.perf_count_reg = {
		{
			.perf_count_cfg    = 0x000000AC,
			.perf_count_cfg_mc = 0x000000B0,
			.perf_pix_count    = 0x000000B4,
			.perf_line_count   = 0x000000B8,
			.perf_stall_count  = 0x000000BC,
			.perf_always_count = 0x000000C0,
			.perf_count_status = 0x000000C4,
		},
		{
			.perf_count_cfg    = 0x000000C8,
			.perf_count_cfg_mc = 0x000000CC,
			.perf_pix_count    = 0x000000D0,
			.perf_line_count   = 0x000000D4,
			.perf_stall_count  = 0x000000D8,
			.perf_always_count = 0x000000DC,
			.perf_count_status = 0x000000E0,
		},
		{
			.perf_count_cfg    = 0x000000E4,
			.perf_count_cfg_mc = 0x000000E8,
			.perf_pix_count    = 0x000000EC,
			.perf_line_count   = 0x000000F0,
			.perf_stall_count  = 0x000000F4,
			.perf_always_count = 0x000000F8,
			.perf_count_status = 0x000000FC,
		},
		{
			.perf_count_cfg    = 0x00000100,
			.perf_count_cfg_mc = 0x00000104,
			.perf_pix_count    = 0x00000108,
			.perf_line_count   = 0x0000010C,
			.perf_stall_count  = 0x00000110,
			.perf_always_count = 0x00000114,
			.perf_count_status = 0x00000118,
		},
	},
	.top_debug_cfg            = 0x000001EC,
	.num_top_debug_reg        = CAM_TFE_980_NUM_TOP_DBG_REG,
	.top_debug = tfe980_top_debug_reg,
	/*
	 * HAF CLC registers, Offsets w.r.t haf_clc_base. If haf_clc_base is 0,
	 * make these offsets relative core start address.
	 */
	.pdaf_violation_status    = 0x00000004,
	/*
	 * Bus Wr registers, w.r.t bus_wr_base. If bus_wr_base is 0,
	 * make these offsets relative core start address.
	 */
	.bus_violation_status     = 0x00000064,
	.bus_overflow_status      = 0x00000068,
	/*
	 * Bayer HM registers, Offsets w.r.t bayer_hm_base. If bayer_hm_base is 0,
	 * make these offsets relative core start address.
	 */
	.bayer_violation_status   = 0x00000024,
	.bayer_debug_cfg          = 0x000001EC,
	.bayer_debug_cfg_en       = 0x3,
	.num_bayer_perf_counters       = 4,
	.bayer_perf_count_reg = {
		{
			.perf_count_cfg    = 0x00000028,
			.perf_count_cfg_mc = 0x0000002C,
			.perf_pix_count    = 0x00000030,
			.perf_line_count   = 0x00000034,
			.perf_stall_count  = 0x00000038,
			.perf_always_count = 0x0000003C,
			.perf_count_status = 0x00000040,
		},
		{
			.perf_count_cfg    = 0x00000044,
			.perf_count_cfg_mc = 0x00000048,
			.perf_pix_count    = 0x0000004C,
			.perf_line_count   = 0x00000050,
			.perf_stall_count  = 0x00000054,
			.perf_always_count = 0x00000058,
			.perf_count_status = 0x0000005C,
		},
		{
			.perf_count_cfg    = 0x00000060,
			.perf_count_cfg_mc = 0x00000064,
			.perf_pix_count    = 0x00000068,
			.perf_line_count   = 0x0000006C,
			.perf_stall_count  = 0x00000070,
			.perf_always_count = 0x00000074,
			.perf_count_status = 0x00000078,
		},
		{
			.perf_count_cfg    = 0x0000007C,
			.perf_count_cfg_mc = 0x00000080,
			.perf_pix_count    = 0x00000084,
			.perf_line_count   = 0x00000088,
			.perf_stall_count  = 0x0000008C,
			.perf_always_count = 0x00000090,
			.perf_count_status = 0x00000094,
		},
	},
	.num_bayer_debug_reg = CAM_TFE_980_NUM_BAYER_DBG_REG,
	.bayer_debug = tfe980_bayer_debug_reg,
	.frame_timing_irq_reg_idx = CAM_IFE_IRQ_CAMIF_REG_STATUS0,
	/* HW capabilities
	 */
	.capabilities = CAM_VFE_COMMON_CAP_SKIP_CORE_CFG |
			CAM_VFE_COMMON_CAP_CORE_MUX_CFG,
};

static struct cam_vfe_ver4_path_reg_data tfe980_ipp_common_reg_data = {
	.sof_irq_mask                    = 0x150,
	.eof_irq_mask                    = 0x2A0,
	.error_irq_mask                  = 0xF000004,
	.ipp_violation_mask              = 0x4000000,
	.bayer_violation_mask            = 0x4,
	.pdaf_violation_mask             = 0x2000000,
	.diag_violation_mask             = 0x8000000,
	.diag_sensor_sel_mask            = 0x6,
	.diag_frm_count_mask_0           = 0xF000,
	.enable_diagnostic_hw            = 0x1,
	.top_debug_cfg_en                = 3,
	.is_mc_path                      = true,
	/* SOF and EOF mask combined for each context */
	.frm_irq_hw_ctxt_mask = {
		0x30,
		0xC0,
		0x300,
	},
};

static struct cam_vfe_ver4_path_reg_data tfe980_pdlib_reg_data = {
	.sof_irq_mask                    = 0x400,
	.eof_irq_mask                    = 0x800,
	.diag_sensor_sel_mask            = 0x8,
	.diag_frm_count_mask_0           = 0x40,
	.enable_diagnostic_hw            = 0x40,
	.top_debug_cfg_en                = 3,
};

static struct cam_vfe_ver4_path_reg_data tfe980_vfe_full_rdi_reg_data[5] = {
	{
		.sof_irq_mask                    = 0x1000,
		.eof_irq_mask                    = 0x2000,
		.error_irq_mask                  = 0x0,
		.diag_sensor_sel_mask            = 0xA,
		.diag_frm_count_mask_0           = 0x80,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 3,
	},
	{
		.sof_irq_mask                    = 0x4000,
		.eof_irq_mask                    = 0x8000,
		.error_irq_mask                  = 0x0,
		.diag_sensor_sel_mask            = 0xC,
		.diag_frm_count_mask_0           = 0x100,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 3,
	},
	{
		.sof_irq_mask                    = 0x10000,
		.eof_irq_mask                    = 0x20000,
		.error_irq_mask                  = 0x0,
		.diag_sensor_sel_mask            = 0xE,
		.diag_frm_count_mask_0           = 0x200,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 3,
	},
	{
		.sof_irq_mask                    = 0x40000,
		.eof_irq_mask                    = 0x80000,
		.error_irq_mask                  = 0x0,
		.diag_sensor_sel_mask            = 0x10,
		.diag_frm_count_mask_0           = 0x400,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 3,
	},
	{
		.sof_irq_mask                    = 0x100000,
		.eof_irq_mask                    = 0x200000,
		.error_irq_mask                  = 0x0,
		.diag_sensor_sel_mask            = 0x12,
		.diag_frm_count_mask_0           = 0x800,
		.enable_diagnostic_hw            = 0x1,
		.top_debug_cfg_en                = 3,
	},
};

struct cam_vfe_ver4_path_hw_info
	tfe980_rdi_hw_info_arr[] = {
	{
		.common_reg     = &tfe980_common_reg,
		.reg_data       = &tfe980_vfe_full_rdi_reg_data[0],
	},
	{
		.common_reg     = &tfe980_common_reg,
		.reg_data       = &tfe980_vfe_full_rdi_reg_data[1],
	},
	{
		.common_reg     = &tfe980_common_reg,
		.reg_data       = &tfe980_vfe_full_rdi_reg_data[2],
	},
	{
		.common_reg     = &tfe980_common_reg,
		.reg_data       = &tfe980_vfe_full_rdi_reg_data[3],
	},
	{
		.common_reg     = &tfe980_common_reg,
		.reg_data       = &tfe980_vfe_full_rdi_reg_data[4],
	},
};

static struct cam_vfe_top_ver4_diag_reg_info tfe980_diag_reg_info[] = {
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
		.name    = "FRAME_CNT_PPP_PIPE",
	},
	{
		.bitmask = 0xFF00,
		.name    = "FRAME_CNT_RDI_0_PIPE",
	},
	{
		.bitmask = 0xFF0000,
		.name    = "FRAME_CNT_RDI_1_PIPE",
	},
	{
		.bitmask = 0xFF000000,
		.name    = "FRAME_CNT_RDI_2_PIPE",
	},
	{
		.bitmask = 0xFF,
		.name    = "FRAME_CNT_RDI_3_PIPE",
	},
	{
		.bitmask = 0xFF00,
		.name    = "FRAME_CNT_RDI_4_PIPE",
	},
	{
		.bitmask = 0xFF,
		.name    = "FRAME_CNT_IPP_CONTEXT0_PIPE",
	},
	{
		.bitmask = 0xFF00,
		.name    = "FRAME_CNT_IPP_CONTEXT1_PIPE",
	},
	{
		.bitmask = 0xFF0000,
		.name    = "FRAME_CNT_IPP_CONTEXT2_PIPE",
	},
	{
		.bitmask = 0xFF000000,
		.name    = "FRAME_CNT_IPP_ALL_CONTEXT_PIPE",
	},
};

static struct cam_vfe_top_ver4_diag_reg_fields tfe980_diag_sensor_field[] = {
	{
		.num_fields = 3,
		.field      = &tfe980_diag_reg_info[0],
	},
	{
		.num_fields = 1,
		.field      = &tfe980_diag_reg_info[3],
	},
};

static struct cam_vfe_top_ver4_diag_reg_fields tfe980_diag_frame_field[] = {
	{
		.num_fields = 4,
		.field      = &tfe980_diag_reg_info[4],
	},
	{
		.num_fields = 2,
		.field      = &tfe980_diag_reg_info[8],
	},
	{
		.num_fields = 4,
		.field      = &tfe980_diag_reg_info[10],
	},
};

/*
 * FCG CLC registers w.r.t fcg_clc_base. If the fcg_clc_base is 0,
 * offsets are relative to core start address.
 */
static struct cam_vfe_ver4_fcg_module_info tfe980_fcg_module_info = {
	.max_fcg_ch_ctx                      = 3,
	.max_fcg_predictions                 = 3,
	.fcg_index_shift                     = 16,
	.max_reg_val_pair_size               = 6,
	.fcg_type_size                       = 2,
	.fcg_phase_index_cfg_0               = 0x00000070,
	.fcg_phase_index_cfg_1               = 0x00000074,
	.fcg_reg_ctxt_shift                  = 0x0,
	.fcg_reg_ctxt_sel                    = 0x000001F4,
	.fcg_reg_ctxt_mask                   = 0x7,
};

static struct cam_vfe_top_ver4_hw_info tfe980_top_hw_info = {
	.common_reg = &tfe980_common_reg,
	.vfe_full_hw_info = {
		.common_reg     = &tfe980_common_reg,
		.reg_data       = &tfe980_ipp_common_reg_data,
	},
	.pdlib_hw_info = {
		.common_reg     = &tfe980_common_reg,
		.reg_data       = &tfe980_pdlib_reg_data,
	},
	.rdi_hw_info            = tfe980_rdi_hw_info_arr,
	.ipp_module_desc        = tfe980_ipp_mod_desc,
	.bayer_module_desc      = tfe980_bayer_mod_desc,
	.num_mux = 7,
	.mux_type = {
		CAM_VFE_CAMIF_VER_4_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_RDI_VER_1_0,
		CAM_VFE_PDLIB_VER_1_0,
	},
	.num_path_port_map = 3,
	.path_port_map = {
		{CAM_ISP_HW_VFE_IN_PDLIB, CAM_ISP_IFE_OUT_RES_2PD},
		{CAM_ISP_HW_VFE_IN_PDLIB, CAM_ISP_IFE_OUT_RES_PREPROCESS_2PD},
		{CAM_ISP_HW_VFE_IN_PDLIB, CAM_ISP_IFE_OUT_RES_PDAF_PARSED_DATA},
	},
	.num_rdi                         = ARRAY_SIZE(tfe980_rdi_hw_info_arr),
	.num_top_errors                  = ARRAY_SIZE(tfe980_top_irq_err_desc),
	.top_err_desc                    = tfe980_top_irq_err_desc,
	.num_pdaf_violation_errors       = ARRAY_SIZE(tfe980_haf_violation_desc),
	.pdaf_violation_desc             = tfe980_haf_violation_desc,
	.top_debug_reg_info              = &tfe980_top_dbg_reg_info,
	.bayer_debug_reg_info            = &tfe980_bayer_dbg_reg_info,
	.fcg_module_info                 = &tfe980_fcg_module_info,
	.fcg_mc_supported                = true,
	.diag_sensor_info                = tfe980_diag_sensor_field,
	.diag_frame_info                 = tfe980_diag_frame_field,
	.top_hm_base                     = 0x0,
	.bayer_hm_base                   = 0xC000,
	.fcg_clc_base                    = 0xDE00,
	.haf_clc_base                    = 0x9300,
	.bus_wr_base                     = 0x800,
	.bayer_hm_supported              = true,
};

/*
 * if bus_wr_base is defined in cam_vfe_bus_ver3_hw_info, offsets are w.r.t
 * bus start address, else these are defined w.r.t base of the core
 *
 */
static struct cam_irq_register_set tfe980_bus_irq_reg[2] = {
	{
		.mask_reg_offset   = 0x00000018,
		.clear_reg_offset  = 0x00000020,
		.status_reg_offset = 0x00000028,
		.set_reg_offset    = 0x00000050,
	},
	{
		.mask_reg_offset   = 0x0000001C,
		.clear_reg_offset  = 0x00000024,
		.status_reg_offset = 0x0000002C,
		.set_reg_offset    = 0x00000054,
	},
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	tfe980_ubwc_regs = {
	.meta_addr        = 0x00000040,
	.meta_cfg         = 0x00000044,
	.mode_cfg         = 0x00000048,
	.stats_ctrl       = 0x0000004C,
	.ctrl_2           = 0x00000050,
	.lossy_thresh0    = 0x00000054,
	.lossy_thresh1    = 0x00000058,
	.off_lossy_var    = 0x0000005C,
	.bw_limit         = 0x0000001C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_err_irq_desc tfe980_bus_irq_err_desc[][32] = {
	{
		{
			.bitmask = BIT(26),
			.err_name = "IPCC_FENCE_DATA_ERR",
			.desc = "IPCC or FENCE Data was not available in the Input Fifo",
		},
		{
			.bitmask = BIT(27),
			.err_name = "IPCC_FENCE_ADDR_ERR",
			.desc = "IPCC or FENCE address fifo was empty and read was attempted",
		},
		{
			.bitmask = BIT(28),
			.err_name = "CONS_VIOLATION",
			.desc = "Programming of software registers violated the constraints",
		},
		{
			.bitmask = BIT(30),
			.err_name = "VIOLATION",
			.desc = "Client has a violation in ccif protocol at input",
		},
		{
			.bitmask = BIT(31),
			.err_name = "IMAGE_SIZE_VIOLATION",
			.desc = "Programmed image size is not same as image size from the CCIF",
		},
	},
};

static struct cam_vfe_bus_ver3_hw_info tfe980_bus_hw_info = {
	.common_reg = {
	    /*
	     * if bus_wr_base is defined in cam_vfe_bus_ver3_hw_info, offsets are w.r.t
	     * bus start address, else these are defined w.r.t base of the core
	     *
	     */
		.hw_version                       = 0x00000000,
		.cgc_ovd                          = 0x00000008,
		.ctxt_sel                         = 0x00000124,
		.ubwc_static_ctrl                 = 0x00000058,
		.pwr_iso_cfg                      = 0x0000005C,
		.overflow_status_clear            = 0x00000060,
		.ccif_violation_status            = 0x00000064,
		.overflow_status                  = 0x00000068,
		.image_size_violation_status      = 0x00000070,
		.debug_status_top_cfg             = 0x000000F0,
		.debug_status_top                 = 0x000000F4,
		.test_bus_ctrl                    = 0x00000128,
		.mc_read_sel_shift                = 0x5,
		.mc_write_sel_shift               = 0x0,
		.mc_ctxt_mask                     = 0x7,
		.wm_mode_shift                    = 16,
		.wm_mode_val                      = { 0x0, 0x1, 0x2 },
		.wm_en_shift                      = 0,
		.frmheader_en_shift               = 2,
		.virtual_frm_en_shift             = 1,
		.irq_reg_info = {
			.num_registers            = 2,
			.irq_reg_set              = tfe980_bus_irq_reg,
			.global_irq_cmd_offset    = 0x00000030,
			.global_clear_bitmask     = 0x00000001,
		},
		.num_perf_counters                = 8,
		.perf_cnt_status                  = 0x000000B4,
		.perf_cnt_reg = {
			{
				.perf_cnt_cfg = 0x00000074,
				.perf_cnt_val = 0x00000094,
			},
			{
				.perf_cnt_cfg = 0x00000078,
				.perf_cnt_val = 0x00000098,
			},
			{
				.perf_cnt_cfg = 0x0000007C,
				.perf_cnt_val = 0x0000009C,
			},
			{
				.perf_cnt_cfg = 0x00000080,
				.perf_cnt_val = 0x000000A0,
			},
			{
				.perf_cnt_cfg = 0x00000084,
				.perf_cnt_val = 0x000000A4,
			},
			{
				.perf_cnt_cfg = 0x00000088,
				.perf_cnt_val = 0x000000A8,
			},
			{
				.perf_cnt_cfg = 0x0000008C,
				.perf_cnt_val = 0x000000AC,
			},
			{
				.perf_cnt_cfg = 0x00000090,
				.perf_cnt_val = 0x000000B0,
			},
		},
	},
	.bus_wr_base                              = 0x800,
	.support_dyn_offset                       = true,
	/*
	 * client_base is w.r.t bus_wr_base. If bus_wr_base is 0,
	 * make client_base relative core start address.
	 */
	.client_base                              = 0x500,
	.client_reg_size                          = 0x100,
	.ubwc_clients_mask                        = 0x10007F,
	.client_offsets = {
			.cfg                      = 0x00000000,
			.image_addr               = 0x00000004,
			.frame_incr               = 0x00000008,
			.image_cfg_0              = 0x0000000C,
			.image_cfg_1              = 0x00000010,
			.image_cfg_2              = 0x00000014,
			.packer_cfg               = 0x00000018,
			.frame_header_addr        = 0x00000020,
			.frame_header_incr        = 0x00000024,
			.frame_header_cfg         = 0x00000028,
			.irq_subsample_period     = 0x00000030,
			.irq_subsample_pattern    = 0x00000034,
			.framedrop_period         = 0x00000038,
			.framedrop_pattern        = 0x0000003C,
			.mmu_prefetch_cfg         = 0x00000060,
			.mmu_prefetch_max_offset  = 0x00000064,
			.system_cache_cfg         = 0x00000068,
			.addr_cfg                 = 0x00000070,
			.ctxt_cfg                 = 0x00000078,
			.addr_status_0            = 0x00000090,
			.addr_status_1            = 0x00000094,
			.addr_status_2            = 0x00000098,
			.addr_status_3            = 0x0000009C,
			.debug_status_cfg         = 0x0000007C,
			.debug_status_0           = 0x00000080,
			.debug_status_1           = 0x00000084,
			.bw_limiter_addr          = 0x0000001C,
			.ubwc_regs                = &tfe980_ubwc_regs,

	},
	.bus_client_reg = {
		/*
		 * For clients with Meta outputs, META MID is programmed in [31 : 16]
		 * Image MID is programmed in [16 : 0]
		 */
		/* BUS Client 0 FULL */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_TP_10) |
				BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_MIPI12) |
				BIT_ULL(PACKER_FMT_VER3_MIPI14) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.rcs_en_mask              = 0x200,
			.name                     = "FULL",
			.line_based               = 1,
			.mid                      = {(33 <<  16) | 32, (35 << 16) | 34,
							    (37 << 16) | 36},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_FULL,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(8) | BIT_ULL(9) | BIT_ULL(12),
		},
		/* BUS Client 1 DS_4 Y */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.rcs_en_mask              = 0x200,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_TP_10),
			.name                     = "DS4_Y",
			.line_based               = 1,
			.mid                      = {(33 <<  16) | 32, (35 << 16) | 34,
							    (37 << 16) | 36},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS4,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),

		},
		/* BUS Client 2 DS_4 C */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_TP_10),
			.name                     = "DS4_C",
			.line_based               = 1,
			.mid                      = {(39 <<  16) | 38, (41 << 16) | 40,
							    (43 << 16) | 42},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS4,
			.mc_based                 = true,
			.pid_mask                 = 0x6400,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),
		},
		/* BUS Client 3 DS_16 Y */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_TP_10),
			.name                     = "DS16_Y",
			.line_based               = 1,
			.mid                      = {(45 <<  16) | 44, (47 << 16) | 46,
							    (49 << 16) | 48},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS16,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),
		},
		/* BUS Client 4 DS_16 C */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_TP_10),
			.name                     = "DS16_C",
			.line_based               = 1,
			.mid                      = {(51 <<  16) | 50, (53 << 16) | 52,
							    (55 << 16) | 54},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS16,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),
		},
		/* BUS Client 5 DS_2 Y */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP) |
				BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_TP_10),
			.rcs_en_mask              = 0x200,
			.name                     = "DS2_Y",
			.line_based               = 1,
			.mid                      = {(39 <<  16) | 38, (41 << 16) | 40,
							    (43 << 16) | 42},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS2,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(8) | BIT_ULL(9) | BIT_ULL(12),
		},

		/* BUS Client 6 DS_2 C */
		{
			.comp_group               =  CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   =  BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP) |
				BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_TP_10),
			.rcs_en_mask              = 0x200,
			.name                     = "DS2_C",
			.line_based               = 1,
			.mid                      = {(45 <<  16) | 44, (47 << 16) | 46,
							    (49 << 16) | 48},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS2,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(8) | BIT_ULL(9) | BIT_ULL(12),
		},
		/* BUS Client 7 FD Y */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_1,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8_LSB_MSB_10_ODD_EVEN) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8_LSB_MSB_10),
			.name                     = "FD_Y",
			.line_based               = 1,
			.mid                      = {(9 << 16) | 8},
			.num_mid                  = 2,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_FD,
			.cntxt_cfg_except         = true,
			.pid_mask                 = BIT_ULL(16) | BIT_ULL(17) | BIT_ULL(18),
		},
		/* BUS Client 8 FD C */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_1,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8_LSB_MSB_10_ODD_EVEN) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8_LSB_MSB_10),
			.name                     = "FD_C",
			.line_based               = 1,
			.mid                      = {10},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_FD,
			.cntxt_cfg_except         = true,
			.pid_mask                 = BIT_ULL(16) | BIT_ULL(17) | BIT_ULL(18),
		},
		/* BUS Client 9 IR */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP) |
				BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_MIPI12) |
				BIT_ULL(PACKER_FMT_VER3_MIPI14),
			.name                     = "IR",
			.line_based               = 1,
			.mid                      = {(25 << 16) | 24},
			.num_mid                  = 2,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_IR,
			.cntxt_cfg_except         = true,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),
		},
		/* BUS Client 10 STATS_AEC_BE */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "STATS_AEC_BE",
			.mid                      = {32, 33, 34},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AEC_BE,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(4) | BIT_ULL(5) | BIT_ULL(6),
		},
		/* BUS Client 11 STATS_AEC_BHIST */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "STATS_BHIST",
			.mid                      = {35, 36, 37},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AEC_BHIST,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(4) | BIT_ULL(5) | BIT_ULL(6),
		},
		/* BUS Client 12 STATS_TINTLESS_BG */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "STATS_TL_BG",
			.mid                      = {38, 39, 40},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_TL_BG,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(4) | BIT_ULL(5) | BIT_ULL(6),
		},
		/* BUS Client 13 STATS_AWB_BG */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "STATS_AWB_BG",
			.mid                      = {41, 42, 43},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AWB_BG,
			.mc_based                 = true,
			.pid_mask                 = 0x70,
			.pid_mask                 = BIT_ULL(4) | BIT_ULL(5) | BIT_ULL(6),
		},
		/* BUS Client 14 STATS_AWB_BFW */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "AWB_BFW",
			.mid                      = {44, 45, 46},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_AWB_BFW,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(4) | BIT_ULL(5) | BIT_ULL(6),
		},
		/* BUS Client 15 STATS_AF_BHIST */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "AF_BHIST",
			.mid                      = {47, 48, 49},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AF_BHIST,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(4) | BIT_ULL(5) | BIT_ULL(6),
		},
		/* BUS Client 16 STATS_ALSC_BG */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "ALSC_BG",
			.mid                      = {50, 51, 52},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_ALSC,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(4) | BIT_ULL(5) | BIT_ULL(6),
		},
		/* BUS Client 17 STATS_FLICKER_BAYERS */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_32),
			.name                     = "STATS_RS",
			.mid                      = {53, 54, 55},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_BAYER_RS,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(4) | BIT_ULL(5) | BIT_ULL(6),
		},
		/* BUS Client 18 STATS_TMC_BHIST */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_2,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "STATS_TMC_BHIST",
			.mid                      = {56, 57, 58},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_TMC_BHIST,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(4) | BIT_ULL(5) | BIT_ULL(6),
		},
		/* BUS Client 19 PDAF_0 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "PDAF_0_2PD",
			.mid                      = {11},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_2PD,
			.pid_mask                 = BIT_ULL(16) | BIT_ULL(17) | BIT_ULL(18),
			.early_done_mask          = BIT(28),
		},
		/* BUS Client 20 PDAF_1 */
		{
			.comp_group                = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.supported_pack_formats    = BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                      = "PDAF_1_PREPROCESS_2PD",
			.line_based                = 1,
			.mid                       = {(51 << 16) | 50},
			.num_mid                   = 2,
			.out_type                  = CAM_VFE_BUS_VER3_VFE_OUT_PREPROCESS_2PD,
			.pid_mask                  = BIT_ULL(8) | BIT_ULL(9) | BIT_ULL(12),
		},
		/* BUS Client 21 PDAF_2 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "PDAF_2_PARSED_DATA",
			.line_based               = 1,
			.line_based               = 1,
			.mid                      = {12},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_PDAF_PARSED,
			.pid_mask                 = BIT_ULL(16) | BIT_ULL(17) | BIT_ULL(18),
		},
		/* BUS Client 22 PDAF_3 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_4,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "STATS_CAF",
			.mid                      = {13},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_CAF,
			.early_done_mask          = BIT(29),
			.pid_mask                 = BIT_ULL(16) | BIT_ULL(17) | BIT_ULL(18),
			.early_done_mask          = BIT(29),
		},
		/* BUS Client 23 RDI_0 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_5,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_MIPI12) | BIT_ULL(PACKER_FMT_VER3_MIPI14) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "RDI_0",
			.line_based               = 0,
			.mid                      = {58},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI0,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),
		},
		/* BUS Client 24 RDI_1 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_6,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_MIPI12) | BIT_ULL(PACKER_FMT_VER3_MIPI14) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "RDI_1",
			.line_based               = 1,
			.mid                      = {59},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI1,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),
		},
		/* BUS Client 25 RDI_2 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_7,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_MIPI12) |
				BIT_ULL(PACKER_FMT_VER3_MIPI14) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP),
			.name                     = "RDI_2",
			.line_based               = 1,
			.mid                      = {60},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI2,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),
		},
		/* BUS Client 26 RDI_3 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_8,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "RDI_3",
			.line_based               = 1,
			.mid                      = {61},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI3,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),
		},
		/* BUS Client 27 RDI_4 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_9,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "RDI_4",
			.line_based               = 1,
			.mid                      = {62},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI4,
			.pid_mask                 = BIT_ULL(10) | BIT_ULL(13) | BIT_ULL(14),
		},
	},
	.valid_wm_mask   = 0xFFFFFFF,
	.num_cons_err = 32,
	.constraint_error_list = {
		{
			.bitmask = BIT(0),
			.error_description = "PPC 1x1 input Not Supported"
		},
		{
			.bitmask = BIT(1),
			.error_description = "PPC 1x2 input Not Supported"
		},
		{
			.bitmask = BIT(2),
			.error_description = "PPC 2x1 input Not Supported"
		},
		{
			.bitmask = BIT(3),
			.error_description = "PPC 2x2 input Not Supported"
		},
		{
			.bitmask = BIT(4),
			.error_description = "Pack 8 BPP format Not Supported"
		},
		{
			.bitmask = BIT(5),
			.error_description = "Pack 16 BPP format Not Supported"
		},
		{
			.bitmask = BIT(6),
			.error_description = "Pack 24 BPP format Not Supported"
		},
		{
			.bitmask = BIT(7),
			.error_description = "Pack 32 BPP format Not Supported"
		},
		{
			.bitmask = BIT(8),
			.error_description = "Pack 64 BPP format Not Supported"
		},
		{
			.bitmask = BIT(9),
			.error_description = "Pack MIPI 20 format Not Supported"
		},
		{
			.bitmask = BIT(10),
			.error_description = "Pack MIPI 14 format Not Supported"
		},
		{
			.bitmask = BIT(11),
			.error_description = "Pack MIPI 12 format Not Supported"
		},
		{
			.bitmask = BIT(12),
			.error_description = "Pack MIPI 10 format Not Supported"
		},
		{
			.bitmask = BIT(13),
			.error_description = "Pack 128 BPP format Not Supported"
		},
		{
			.bitmask = BIT(14),
			.error_description = "UBWC P016 format Not Supported"
		},
		{
			.bitmask = BIT(15),
			.error_description = "UBWC P010 format Not Supported"
		},
		{
			.bitmask = BIT(16),
			.error_description = "UBWC NV12 format Not Supported"
		},
		{
			.bitmask = BIT(17),
			.error_description = "UBWC NV12 4R format Not Supported"
		},
		{
			.bitmask = BIT(18),
			.error_description = "UBWC TP10 format Not Supported"
		},
		{
			.bitmask = BIT(19),
			.error_description = "Frame based Mode Not Supported"
		},
		{
			.bitmask = BIT(20),
			.error_description = "Index based Mode Not Supported"
		},
		{
			.bitmask = BIT(21),
			.error_description = "FIFO image addr unalign"
		},
		{
			.bitmask = BIT(22),
			.error_description = "FIFO ubwc addr unalign"
		},
		{
			.bitmask = BIT(23),
			.error_description = "FIFO framehdr addr unalign"
		},
		{
			.bitmask = BIT(24),
			.error_description = "Image address unalign"
		},
		{
			.bitmask = BIT(25),
			.error_description = "UBWC address unalign"
		},
		{
			.bitmask = BIT(26),
			.error_description = "Frame Header address unalign"
		},
		{
			.bitmask = BIT(27),
			.error_description = "Stride unalign"
		},
		{
			.bitmask = BIT(28),
			.error_description = "X Initialization unalign"
		},
		{
			.bitmask = BIT(29),
			.error_description = "Image Width unalign",
		},
		{
			.bitmask = BIT(30),
			.error_description = "Image Height unalign",
		},
		{
			.bitmask = BIT(31),
			.error_description = "Meta Stride unalign",
		},
	},
	.num_bus_errors        = 1,
	.bus_err_desc          = &tfe980_bus_irq_err_desc,
	.num_comp_grp          = 10,
	.support_consumed_addr = true,
	.mc_comp_done_mask = {
		BIT(24), 0x0, BIT(25), 0x0, 0x0, 0x0,
		0x0, 0x0, 0x0, 0x0,
	},
	.comp_done_mask = {
		(BIT(0) | BIT(1) | BIT(2)), BIT(3), (BIT(4) | BIT(5) | BIT(6)), BIT(7),
		BIT(8), BIT(16), BIT(17), BIT(18), BIT(19), BIT(20),
	},
	.top_irq_shift         = 0,
	.max_out_res           = CAM_ISP_IFE_OUT_RES_BASE + 43,
	.pack_align_shift      = 5,
	.max_bw_counter_limit  = 0xFF,
	.skip_regdump          = true,
	.skip_regdump_start_offset = 0x800,
	.skip_regdump_stop_offset = 0x209C,
};

static struct cam_vfe_irq_hw_info tfe980_irq_hw_info = {
	.reset_mask    = 0,
	.supported_irq = CAM_VFE_HW_IRQ_CAP_EXT_CSID,
	.top_irq_reg   = &tfe980_top_irq_reg_info,
};

static struct cam_vfe_hw_info cam_tfe980_hw_info = {
	.irq_hw_info                  = &tfe980_irq_hw_info,

	.bus_version                   = CAM_VFE_BUS_VER_3_0,
	.bus_hw_info                   = &tfe980_bus_hw_info,

	.top_version                   = CAM_VFE_TOP_VER_4_0,
	.top_hw_info                   = &tfe980_top_hw_info,
};

#endif /* _CAM_TFE980_H_ */
