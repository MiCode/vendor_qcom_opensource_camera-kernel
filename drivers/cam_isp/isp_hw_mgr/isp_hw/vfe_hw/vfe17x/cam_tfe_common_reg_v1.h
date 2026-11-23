/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 */

#ifndef _CAM_COMMON_REG_V1_H_
#define _CAM_COMMON_REG_V1_H_
#include "cam_vfe_top_ver4.h"
#include "cam_vfe_core.h"
#include "cam_vfe_bus_ver3.h"
#include "cam_irq_controller.h"

/* Below IDs unique CLC IDs used to determine the violating CLCs.
 * Violation status register is updated with one of the below values
 * in case of any CLCs having violations.
 */
static struct cam_vfe_top_ver4_module_desc tfe_common_reg_v1_mod_desc[] = {
	{
		.id = 0,
		.desc = "UNUSED",
	},
	{
		.id  = 1,
		.desc = "BUS_WR",
	},
	{
		.id = 2,
		.desc = "CLC_HAF",
	},
	{
		.id = 3,
		.desc = "CLC_DEMUX",
	},
	{
		.id = 4,
		.desc = "CLC_UNIV_CHANNEL_GAINS",
	},
	{
		.id = 5,
		.desc = "CLC_QPD_PDPC_BPC",
	},
	{
		.id = 6,
		.desc = "CLC_QPDR_PDPC_COMBO",
	},
	{
		.id = 7,
		.desc = "CLC_PDPC_BPC_1D",
	},
	{
		.id = 8,
		.desc = "CLC_ABF_BINC",
	},
	{
		.id = 9,
		.desc = "CLC_CHANNEL_GAINS",
	},
	{
		.id = 10,
		.desc = "CLC_LSC",
	},
	{
		.id = 11,
		.desc = "CLC_FCG",
	},
	{
		.id = 12,
		.desc = "CLC_WB_GAIN",
	},
	{
		.id = 13,
		.desc = "CLC_COMPDECOMP_BAYER",
	},
	{
		.id = 14,
		.desc = "CLC_BDS",
	},
	{
		.id  = 15,
		.desc = "CLC_COMPDECOMP_BDS_MUX",
	},
	{
		.id = 16,
		.desc = "CLC_GBR_DS4_DS4",
	},
	{
		.id = 17,
		.desc = "CLC_COMPDECOMP_DS4",
	},
	{
		.id = 18,
		.desc = "CLC_CROP_RND_CLAMP_DS4",
	},
	{
		.id = 19,
		.desc = "CLC_GBR_DS4_DS16",
	},
	{
		.id = 20,
		.desc = "CLC_COMPDECOMP_DS16",
	},
	{
		.id = 21,
		.desc = "CLC_CROP_RND_CLAMP_DS16",
	},
	{
		.id = 22,
		.desc = "CLC_COMPDECOMP_BDS",
	},
	{
		.id = 23,
		.desc = "CLC_CROP_RND_CLAMP_BDS",
	},
	{
		.id = 24,
		.desc = "CLC_PUNCH_BDS",
	},
	{
		.id = 25,
		.desc = "CLC_COMPDECOMP_BYPASS",
	},
	{
		.id = 26,
		.desc = "CLC_CROP_RND_CLAMP_BYPASS",
	},
	{
		.id = 27,
		.desc = "CLC_RCS_FULL_OUT",
	},
	{
		.id = 28,
		.desc = "CLC_STATS_AWB_BG_TINTLESS",
	},
	{
		.id = 29,
		.desc = "CLC_STATS_AWB_BG_AE",
	},
	{
		.id = 30,
		.desc = "LC_STATS_BHIST_AEC",
	},
	{
		.id = 31,
		.desc = "CLC_STATS_RS",
	},
	{
		.id = 32,
		.desc = "CLC_STATS_BFW_AWB",
	},
	{
		.id = 33,
		.desc = "CLC_STATS_AWB_BG_AWB",
	},
	{
		.id = 34,
		.desc = "CLC_STATS_BHIST_AF",
	},
	{
		.id = 35,
		.desc = "CLC_STATS_AWB_BG_ALSC",
	},
	{
		.id = 36,
		.desc = "CLC_STATS_BHIST_TMC",
	},
	{
		.id = 37,
		.desc = "CLC_RB_DEMOSAIC",
	},
	{
		.id = 38,
		.desc = "CLC_COMPDECOMP_FD",
	},
	{
		.id = 39,
		.desc = "CLC_BLS",
	},
	{
		.id = 40,
		.desc = "CLC_GTM",
	},
	{
		.id = 41,
		.desc = "CLC_COLOR_CORRECT",
	},
	{
		.id = 42,
		.desc = "CLC_GLUT",
	},
	{
		.id = 43,
		.desc = "CLC_COLOR_XFORM",
	},
	{
		.id = 44,
		.desc = "CLC_DOWNSCALE_MN_Y",
	},
	{
		.id = 45,
		.desc = "CLC_DOWNSCALE_MN_C",
	},
	{
		.id = 46,
		.desc = "CLC_CROP_RND_CLAMP_FD_Y",
	},
	{
		.id = 47,
		.desc = "CCLC_CROP_RND_CLAMP_FD_C",
	},
	{
		.id = 48,
		.desc = "CLC_COMPDECOMP_RDI",
	},
	{
		.id = 49,
		.desc = "CLC_COMPDECOMP_PPP",
	},
	{
		.id = 50,
		.desc = "CLC_QPDR",
	},
	{
		.id = 51,
		.desc = "CLC_BPC_PDPC_GIC",
	},
	{
		.id = 52,
		.desc = "CLC_BDS2_DEMO",
	},
	{
		.id = 53,
		.desc = "CLC_PUNCH_BDS2",
	},
	{
		.id = 54,
		.desc = "CLC_PUNCH_DS4_MUX",
	},
	{
		.id = 55,
		.desc = "CLC_BAYER_DS_4_DS4",
	},
	{
		.id = 56,
		.desc = "CLC_PUNCH_DS16",
	},
	{
		.id = 57,
		.desc = "UCLC_BAYER_DS_4_DS16",
	},
	{
		.id = 58,
		.desc = "CLC_CROP_RND_CLAMP_DS2",
	},
	{
		.id = 59,
		.desc = "CLC_RCS_DS2",
	},
	{
		.id = 60,
		.desc = "CLC_CROP_RND_CLAMP_FULL_OUT",
	},
};

static struct cam_vfe_top_ver4_top_err_irq_desc tfe_common_reg_v1_top_irq_err_desc[] = {
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
	{
		.bitmask = BIT(28),
		.err_name = "STATIC SWI CHECK VIOLATION",
		.desc = "Core mux/timestamp frame ID updated outside of idle time",
	},

};

static struct cam_vfe_top_ver4_pdaf_violation_desc tfe_common_reg_v1_haf_violation_desc[] = {
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
	{
		.bitmask = BIT(6),
		.desc = "RDI_PD protocol violation",
	},
	{
		.bitmask = BIT(7),
		.desc = "Sim Monitor 5 violation",
	},
};

static struct cam_vfe_top_ver4_module_desc tfe_common_reg_v1_core_violation_desc[] = {
	{
		.id = 0,
		.desc = "CORE_MUX_CFG__MUX_FD_EXPOSURE_SEL",
	},
	{
		.id = 1,
		.desc = "CORE_MUX_CFG__MUX_RAW_REMO_SEL",
	},
	{
		.id = 2,
		.desc = "CORE_MUX_CFG__MUX_AE_STATS_SEL",
	},
	{
		.id = 3,
		.desc = "CORE_MUX_CFG__MUX_AF_STATS_SEL",
	},
	{
		.id = 4,
		.desc = "CORE_MUX_CFG__MUX_DS4_SEL",
	},
	{
		.id = 5,
		.desc = "CORE_MUX_CFG__MUX_FULL_OUT_SEL",
	},
	{
		.id = 6,
		.desc = "TIMESTAMP_FRAME_ID_CFG__CONTEXT2_INPUT_SEL",
	},
	{
		.id = 7,
		.desc = "TIMESTAMP_FRAME_ID_CFG__CONTEXT1_INPUT_SEL",
	},
	{
		.id = 8,
		.desc = "TIMESTAMP_FRAME_ID_CFG__CONTEXT0_INPUT_SEL",
	},
};

/*
 * Top HM registers, Offsets w.r.t top_hm_base. If top_hm_base is 0,
 * make these offsets relative core start address.
 */
static struct cam_irq_register_set tfe_common_reg_v1_top_irq_reg_set = {
	.mask_reg_offset   = 0x1D8,
	.clear_reg_offset  = 0x1DC,
	.status_reg_offset = 0x1E0,
	.set_reg_offset    = 0x1E4,
	.test_set_val      = BIT(0),
	.test_sub_val      = BIT(0),
};

static struct cam_irq_controller_reg_info tfe_common_reg_v1_top_irq_reg_info = {
	.num_registers = 1,
	.irq_reg_set = &tfe_common_reg_v1_top_irq_reg_set,
	.global_irq_cmd_offset = 0x1D4,
	.global_clear_bitmask  = 0x1,
	.global_set_bitmask    = 0x10,
	.clear_all_bitmask     = 0xFFFFFFFF,
};

static uint32_t tfe_common_reg_v1_top_debug_reg[] = {
	0x4EC,
	0x4F0,
	0x4F4,
	0x4F8,
	0x4FC,
	0x500,
	0x504,
	0x508,
	0x50C,
	0x510,
	0x514,
	0x518,
	0x51C,
	0x520,
	0x524,
	0x528,
	0x52C,
};

static struct cam_vfe_top_ver4_debug_reg_info tfe_common_reg_v1_top_dbg_reg_info[][8] = {
	/* debug_0 */
		VFE_DBG_INFO_ARRAY_4bit("test_bus_reserved",
			"test_bus_reserved",
			"test_bus_reserved",
			"test_bus_reserved",
			"test_bus_reserved",
			"test_bus_reserved",
			"test_bus_reserved",
			"test_bus_reserved"
	),
	/* debug_1 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "STATS_AWB_BG_TINTLESS",
			0x530, (BIT(0) | BIT(1) | BIT(2))),
		VFE_DBG_INFO_WITH_IDLE(4, "STATS_AWB_BG_AE",
			0x530, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(8, "STATS_BHIST_AEC",
			0x530, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(12, "STATS_RS",
			0x530, (BIT(9) | BIT(10) | BIT(11))),
		VFE_DBG_INFO_WITH_IDLE(16, "STATS_BFW_AWB",
			0x0530, (BIT(12) | BIT(13) | BIT(14))),
		VFE_DBG_INFO_WITH_IDLE(20, "STATS_AWB_BG_AWB",
			0x530, (BIT(15) | BIT(16) | BIT(17))),
		VFE_DBG_INFO_WITH_IDLE(24, "STATS_BHIST_AF",
			0x530, (BIT(18) | BIT(19) | BIT(20))),
		VFE_DBG_INFO_WITH_IDLE(28, "STATS_AWB_BG_ALSC",
			0x530, (BIT(21) | BIT(22) | BIT(23))),
	},
	/* debug_2 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "STATS_BHIST_TMC",
			0x530, (BIT(24) | BIT(25) | BIT(26))),
		VFE_DBG_INFO_WITH_IDLE(4, "demosaic",
			0x530, BIT(27)),
		VFE_DBG_INFO_WITH_IDLE(8, "compdecomp_fd",
			0x530, BIT(28)),
		VFE_DBG_INFO_WITH_IDLE(12, "bls",
			0x530, BIT(29)),
		VFE_DBG_INFO_WITH_IDLE(16, "gtm",
			0x0530, BIT(30)),
		VFE_DBG_INFO_WITH_IDLE(20, "color_correct",
			0x530, BIT(31)),
		VFE_DBG_INFO_WITH_IDLE(24, "glut",
			0x534, BIT(0)),
		VFE_DBG_INFO_WITH_IDLE(28, "color_xform",
			0x534, BIT(1)),
	},
	/* debug_3 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "downscale_mn_y",
			0x534, BIT(2)),
		VFE_DBG_INFO_WITH_IDLE(4, "downscale_mn_c",
			0x534, BIT(3)),
		VFE_DBG_INFO_WITH_IDLE(8, "crop_rnd_clamp_fd_y",
			0x534, (BIT(4))),
		VFE_DBG_INFO_WITH_IDLE(12, "crop_rnd_clamp_fd_c",
			0x534, (BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(16, "compdecomp_rdi",
			0x534, (BIT(6))),
		VFE_DBG_INFO_WITH_IDLE(20, "compdecomp_ppp",
			0x534, (BIT(7))),
		VFE_DBG_INFO_WITH_IDLE(24, "haf_idle0",
			0x534, (BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(28, "haf_idle1",
			0x534, (BIT(9))),
	},
	/* debug_4 */
	{
		VFE_DBG_INFO(0, "bayer_bds_main_fd"),
		VFE_DBG_INFO(4, "rb_demo_msb_fd"),
		VFE_DBG_INFO(8, "gtm_round_fd"),
		VFE_DBG_INFO(12, "csid_tfe_ppp"),
		VFE_DBG_INFO(16, "ppp_front_end"),
		VFE_DBG_INFO_WITH_IDLE(20, "stats_awb_bg_tintless_throttle",
			0x534, (BIT(11) | BIT(12) | BIT(13))),
		VFE_DBG_INFO_WITH_IDLE(24, "stats_awb_bg_ae_throttle",
			0x534, (BIT(14) | BIT(15) | BIT(16))),
		VFE_DBG_INFO_WITH_IDLE(28, "stats_ae_bhist_throttle",
			0x534, (BIT(17) | BIT(18) | BIT(19))),
	},
	/* debug_5 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "stats_awb_bg_alsc_throttle",
			0x53C, (BIT(9) | BIT(10) | BIT(11))),
		VFE_DBG_INFO_WITH_IDLE(4, "stats_bayer_rs_throttle",
			0x534, BIT(20) | BIT(21) | BIT(22)),
		VFE_DBG_INFO_WITH_IDLE(8, "stats_bayer_bfw_throttle",
			0x534, (BIT(23) | BIT(24) | BIT(25))),
		VFE_DBG_INFO_WITH_IDLE(12, "stats_awb_bg_awb_throttle",
			0x534, (BIT(26) | BIT(27) | BIT(28))),
		VFE_DBG_INFO_WITH_IDLE(16, "stats_bhist_af_throttle",
			0x534, (BIT(29) | BIT(30) | BIT(31))),
		VFE_DBG_INFO_WITH_IDLE(20, "full_out_y_throttle",
			0x538, (BIT(12) | BIT(13) | BIT(14))),
		VFE_DBG_INFO_WITH_IDLE(24, "full_out_uv_throttle",
			0x538, (BIT(15) | BIT(16) | BIT(17))),
		VFE_DBG_INFO_WITH_IDLE(28, "dc4_out_y_throttle",
			0x538, (BIT(0) | BIT(1) | BIT(2))),
	},
	/* debug_6 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "dc4_out_c_throttle",
			0x538, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(4, "dc16_out_y_throttle",
			0x538, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(8, "dc16_out_c_throttle",
			0x538, (BIT(9) | BIT(10) | BIT(11))),
		VFE_DBG_INFO_WITH_IDLE(12, "raw_out_throttle",
			0x538, (BIT(20) | BIT(21) | BIT(22))),
		VFE_DBG_INFO_WITH_IDLE(16, "fd_out_y_throttle",
			0x538, (BIT(23) | BIT(24) | BIT(25))),
		VFE_DBG_INFO_WITH_IDLE(20, "fd_out_c_throttle",
			0x538, (BIT(26) | BIT(27) | BIT(28))),
		VFE_DBG_INFO_WITH_IDLE(24, "pdaf_sad_throttle",
			0x538, BIT(18)),
		VFE_DBG_INFO_WITH_IDLE(28, "stats_caf_throttle",
			0x538, BIT(19)),
	},
	/* debug_7 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "haf_parsed_to_bus",
			0x538, BIT(29)),
		VFE_DBG_INFO_WITH_IDLE(4, "haf_pre_processed_to_bus",
			0x538, BIT(30)),
		VFE_DBG_INFO_WITH_IDLE(8, "haf_pre_processed2_to_bus",
			0x538, BIT(31)),
		VFE_DBG_INFO(12, "full_out_y_to_bus"),
		VFE_DBG_INFO(16, "full_out_uv_to_bus"),
		VFE_DBG_INFO(20, "ds4_out_y_to_bus"),
		VFE_DBG_INFO(24, "ds4_out_uv_to_bus"),
		VFE_DBG_INFO(28, "ds16_out_y_to_bus"),
	},
	/* debug_8 */
	{
		VFE_DBG_INFO(0, "ds16_out_uv_to_bus"),
		VFE_DBG_INFO(4, "fd_out_y_to_bus"),
		VFE_DBG_INFO(8, "fd_out_uv_to_bus"),
		VFE_DBG_INFO(12, "raw_out_to_bus"),
		VFE_DBG_INFO(16, "stats_awb_bg_ae_to_bus"),
		VFE_DBG_INFO(20, "stats_ae_bhist_to_bus"),
		VFE_DBG_INFO(24, "stats_awb_bg_tintless_to_bus"),
		VFE_DBG_INFO(28, "stats_awb_bg_awb_to_bus"),
	},
	{
	/* debug_9 */
		VFE_DBG_INFO(0, "stats_bayer_bfw_to_bus"),
		VFE_DBG_INFO(4, "stats_bhist_af_to_bus"),
		VFE_DBG_INFO(8, "stats_awb_bg_alsc_to_bus"),
		VFE_DBG_INFO(12, "stats_bayer_rs_to_bus"),
		VFE_DBG_INFO(16, "stats_bhist_tmc_to_bus"),
		VFE_DBG_INFO(20, "stats_sad_to_bus"),
		VFE_DBG_INFO(24, "stats_haf_processed_to_bus"),
		VFE_DBG_INFO(28, "stats_haf_processed2_to_bus"),
	},
	/* debug_10 */
	{
		VFE_DBG_INFO(0, "stats_haf_parsed_to_bus"),
		VFE_DBG_INFO(4, "stats_caf_to_bus"),
		VFE_DBG_INFO(8, "rdi_0_splitter_to_bus"),
		VFE_DBG_INFO(12, "rdi_1_splitter_to_bus"),
		VFE_DBG_INFO(16, "rdi_2_splitter_to_bus"),
		VFE_DBG_INFO(20, "rdi_3_splitter_to_bus"),
		VFE_DBG_INFO(24, "rdi_4_splitter_to_bus"),
		VFE_DBG_INFO_WITH_IDLE(28, "stats_bhist_tmc_to_bus",
			0x53C, (BIT(12) | BIT(13) | BIT(14))),
	},
	/* debug_11 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "csid_tfe_ipp",
			0x53C, (BIT(22) | BIT(23) | BIT(24))),
		VFE_DBG_INFO(8, "reserved"),
		VFE_DBG_INFO(8, "reserved"),
		VFE_DBG_INFO(12, "reserved"),
		VFE_DBG_INFO(16, "reserved"),
		VFE_DBG_INFO(20, "reserved"),
		VFE_DBG_INFO(24, "reserved"),
		VFE_DBG_INFO(28, "reserved"),
	},
	/* debug_12 */
	{
		VFE_DBG_INFO(0, "reserved"),
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
static uint32_t tfe_common_reg_v1_bayer_debug_reg[] = {
	0x4EC,
	0x4F0,
	0x4F4,
	0x4F8,
	0x4FC,
	0x500,
	0x504,
	0x508,
	0x50C,
	0x510,
	0x514,
	0x518,
};

static struct cam_vfe_top_ver4_debug_reg_info tfe_common_reg_v1_bayer_dbg_reg_info[][8] = {
	/* debug_0 */
	VFE_DBG_INFO_ARRAY_4bit("test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved",
		"test_bus_reserved"
	),
	/* debug_1 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "clc_demux",
			0x51C, (BIT(0) | BIT(1) | BIT(2))),
		VFE_DBG_INFO_WITH_IDLE(4, "clc_univ_channel_gains",
			0x51C, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(8, "clc_qpd_pdpc_bpc",
			0x51C, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(12, "clc_qpdr_pdpc_combo",
			0x51C, (BIT(9) | BIT(10) | BIT(11))),
		VFE_DBG_INFO_WITH_IDLE(16, "clc_pdpc_bpc_1d",
			0x51C, (BIT(12) | BIT(13) | BIT(14))),
		VFE_DBG_INFO_WITH_IDLE(20, "clc_abf_binc",
			0x51C, (BIT(15) | BIT(16) | BIT(17))),
		VFE_DBG_INFO_WITH_IDLE(24, "clc_channel_gains",
			0x51C, (BIT(18) | BIT(19) | BIT(20))),
		VFE_DBG_INFO_WITH_IDLE(28, "clc_lsc",
			0x51C, (BIT(21) | BIT(22) | BIT(23))),
	},
	/* debug_2 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "clc_fcg",
			0x51C, (BIT(24) | BIT(25) | BIT(26))),
		VFE_DBG_INFO_WITH_IDLE(4, "clc_wb_gain",
			0x51C, (BIT(27) | BIT(28) | BIT(29))),
		VFE_DBG_INFO_WITH_IDLE(8, "clc_compdecomp_bayer",
			0x520, (BIT(0) | BIT(1) | BIT(2))),
		VFE_DBG_INFO_WITH_IDLE(12, "clc_bds",
			0x520, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(16, "clc_compdecomp_bds_mux",
			0x520, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(20, "clc_gbr_ds4_ds4",
			0x520, (BIT(9) | BIT(10) | BIT(11))),
		VFE_DBG_INFO_WITH_IDLE(24, "clc_compdecomp_ds4",
			0x520, (BIT(12) | BIT(13) | BIT(14))),
		VFE_DBG_INFO_WITH_IDLE(28, "clc_crop_rnd_clamp_ds4",
			0x520, (BIT(15) | BIT(16) | BIT(17))),
	},
	/* debug_3 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "clc_gbr_ds4_ds16",
			0x520, (BIT(18) | BIT(19) | BIT(20))),
		VFE_DBG_INFO_WITH_IDLE(4, "clc_compdecomp_ds16",
			0x520, (BIT(21) | BIT(22) | BIT(23))),
		VFE_DBG_INFO_WITH_IDLE(8, "clc_crop_rnd_clamp_ds16",
			0x520, (BIT(24) | BIT(25) | BIT(26))),
		VFE_DBG_INFO_WITH_IDLE(12, "clc_compdecomp_bds",
			0x520, (BIT(27) | BIT(28) | BIT(29))),
		VFE_DBG_INFO_WITH_IDLE(16, "clc_crop_rnd_clamp_bds",
			0x524, (BIT(0) | BIT(1) | BIT(2))),
		VFE_DBG_INFO_WITH_IDLE(20, "clc_clc_punch_bds",
			0x524, (BIT(3) | BIT(4) | BIT(5))),
		VFE_DBG_INFO_WITH_IDLE(24, "clc_compdecomp_bypass",
			0x524, (BIT(6) | BIT(7) | BIT(8))),
		VFE_DBG_INFO_WITH_IDLE(28, "clc_crop_rnd_clamp_bypass",
			0x524, (BIT(9) | BIT(10) | BIT(11))),
	},
	/* debug_4 */
	{
		VFE_DBG_INFO_WITH_IDLE(0, "clc_rcs_full_out",
			0x524, (BIT(12) | BIT(13) | BIT(14))),
		VFE_DBG_INFO(16, "reserved"),
		VFE_DBG_INFO(16, "reserved"),
		VFE_DBG_INFO(16, "reserved"),
		VFE_DBG_INFO(16, "reserved"),
		VFE_DBG_INFO(16, "reserved"),
		VFE_DBG_INFO(16, "reserved"),
		VFE_DBG_INFO(16, "reserved"),
	},
	/* debug_5 */
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
	/* debug_6 */
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
	/* debug_7 */
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

static struct cam_vfe_top_ver4_reg_offset_common tfe_common_reg_v1_common_reg = {
	/*
	 * Top HM registers, Offsets w.r.t top_hm_base. If top_hm_base is 0,
	 * make these offsets relative core start address.
	 */
	.hw_version               = 0x0,
	.core_cgc_ovd_0           = 0x100,
	.ahb_cgc_ovd              = 0x108,
	.core_mux_cfg             = 0x110,
	.pdaf_input_cfg_0         = 0x118,
	.pdaf_input_cfg_1         = 0x11C,
	.diag_config              = 0x39C,
	.diag_sensor_status       = {0x3A0, 0x3A4, 0x3A8, 0x3AC},
	.diag_frm_cnt_status      = {0x3B0, 0x3B4, 0x3B8},
	.ipp_violation_status     = 0x248,
	.core_violation_status    = 0x24C,
	.dsp_status               = 0x0,
	.num_perf_counters        = 4,
	.perf_count_reg = {
		{
			.perf_count_cfg    = 0x3DC,
			.perf_count_cfg_mc = 0x3E0,
			.perf_pix_count    = 0x3E4,
			.perf_line_count   = 0x3E8,
			.perf_stall_count  = 0x3EC,
			.perf_always_count = 0x3F0,
			.perf_count_status = 0x3F4,
		},
		{
			.perf_count_cfg    = 0x3F8,
			.perf_count_cfg_mc = 0x3FC,
			.perf_pix_count    = 0x400,
			.perf_line_count   = 0x404,
			.perf_stall_count  = 0x408,
			.perf_always_count = 0x40C,
			.perf_count_status = 0x410,
		},
		{
			.perf_count_cfg    = 0x414,
			.perf_count_cfg_mc = 0x418,
			.perf_pix_count    = 0x41C,
			.perf_line_count   = 0x420,
			.perf_stall_count  = 0x424,
			.perf_always_count = 0x428,
			.perf_count_status = 0x42C,
		},
		{
			.perf_count_cfg    = 0x430,
			.perf_count_cfg_mc = 0x434,
			.perf_pix_count    = 0x438,
			.perf_line_count   = 0x43C,
			.perf_stall_count  = 0x440,
			.perf_always_count = 0x444,
			.perf_count_status = 0x448,
		},
	},
	.top_debug_cfg            = 0x558,
	.top_debug_err_vec_irq    = {0x4DC, 0x4E0},
	.top_debug_err_vec_ts_lb  = 0x4D4,
	.top_debug_err_vec_ts_mb  = 0x4D8,
	.num_top_debug_reg        = 17,
	.top_debug                = tfe_common_reg_v1_top_debug_reg,
	/*
	 * HAF CLC registers, Offsets w.r.t haf_clc_base. If haf_clc_base is 0,
	 * make these offsets relative core start address.
	 */
	.pdaf_violation_status    = 0x4,
	/*
	 * Bus Wr registers, w.r.t bus_wr_base. If bus_wr_base is 0,
	 * make these offsets relative core start address.
	 */
	.bus_violation_status     = 0xB0,
	.bus_overflow_status      = 0xB8,
	/*
	 * Bayer HM registers, Offsets w.r.t bayer_hm_base. If bayer_hm_base is 0,
	 *make these offsets relative core start address.
	 */
	.bayer_debug_cfg          = 0x544,
	.bayer_violation_status   = 0x248,
	.num_bayer_perf_counters  = 4,
	.bayer_debug_cfg_en       = 0x3,
	.bayer_perf_count_reg = {
		/*  Bayer perf count regs from here onwards */
		{
			.perf_count_cfg    = 0x3DC,
			.perf_count_cfg_mc = 0x3E0,
			.perf_pix_count    = 0x3E4,
			.perf_line_count   = 0x3E8,
			.perf_stall_count  = 0x3EC,
			.perf_always_count = 0x3F0,
			.perf_count_status = 0x3F4,
		},
		{
			.perf_count_cfg    = 0x3F8,
			.perf_count_cfg_mc = 0x3FC,
			.perf_pix_count    = 0x400,
			.perf_line_count   = 0x404,
			.perf_stall_count  = 0x408,
			.perf_always_count = 0x40C,
			.perf_count_status = 0x410,
		},
		{
			.perf_count_cfg    = 0x414,
			.perf_count_cfg_mc = 0x418,
			.perf_pix_count    = 0x41C,
			.perf_line_count   = 0x420,
			.perf_stall_count  = 0x424,
			.perf_always_count = 0x428,
			.perf_count_status = 0x42C,
		},
		{
			.perf_count_cfg    = 0x430,
			.perf_count_cfg_mc = 0x434,
			.perf_pix_count    = 0x438,
			.perf_line_count   = 0x43C,
			.perf_stall_count  = 0x440,
			.perf_always_count = 0x444,
			.perf_count_status = 0x448,
		},
	},
	.num_bayer_debug_reg = 12,
	.bayer_debug = tfe_common_reg_v1_bayer_debug_reg,
	.frame_timing_irq_reg_idx = CAM_IFE_IRQ_CAMIF_REG_STATUS0,
	/* HW capabilities
	 */
	.capabilities = CAM_VFE_COMMON_CAP_SKIP_CORE_CFG |
			CAM_VFE_COMMON_CAP_CORE_MUX_CFG |
			CAM_VFE_COMMON_CAP_DEBUG_ERR_VEC,
};

static struct cam_vfe_ver4_path_reg_data tfe_common_reg_v1_ipp_common_reg_data = {
	.sof_irq_mask                    = 0x150,
	.eof_irq_mask                    = 0x2A0,
	.error_irq_mask                  = 0x1F000004,
	.ipp_violation_mask              = 0x4000000,
	.bayer_violation_mask            = 0x4,
	.pdaf_violation_mask             = 0x2000000,
	.diag_violation_mask             = 0x8000000,
	.core_violation_mask             = 0x10000000,
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

static struct cam_vfe_ver4_path_reg_data tfe_common_reg_v1_pdlib_reg_data = {
	.sof_irq_mask                    = 0x400,
	.eof_irq_mask                    = 0x800,
	.diag_sensor_sel_mask            = 0x8,
	.diag_frm_count_mask_0           = 0x40,
	.enable_diagnostic_hw            = 0x1,
	.top_debug_cfg_en                = 3,
};

static struct cam_vfe_ver4_path_reg_data tfe_common_reg_v1_vfe_full_rdi_reg_data[5] = {
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
		.enable_diagnostic_hw            = 0x1,
		.diag_sensor_sel_mask            = 0xE,
		.diag_frm_count_mask_0           = 0x200,
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
	tfe_common_reg_v1_rdi_hw_info_arr[] = {
	{
		.common_reg     = &tfe_common_reg_v1_common_reg,
		.reg_data       = &tfe_common_reg_v1_vfe_full_rdi_reg_data[0],
	},
	{
		.common_reg     = &tfe_common_reg_v1_common_reg,
		.reg_data       = &tfe_common_reg_v1_vfe_full_rdi_reg_data[1],
	},
	{
		.common_reg     = &tfe_common_reg_v1_common_reg,
		.reg_data       = &tfe_common_reg_v1_vfe_full_rdi_reg_data[2],
	},
	{
		.common_reg     = &tfe_common_reg_v1_common_reg,
		.reg_data       = &tfe_common_reg_v1_vfe_full_rdi_reg_data[3],
	},
	{
		.common_reg     = &tfe_common_reg_v1_common_reg,
		.reg_data       = &tfe_common_reg_v1_vfe_full_rdi_reg_data[4],
	},
};

static struct cam_vfe_top_ver4_diag_reg_info tfe_common_reg_v1_diag_reg_info[] = {
	{
		.bitmask = 0x3FFF,
		.name = "SENSOR_HBI",
	},
	{
		.bitmask = 0x4000,
		.name = "SENSOR_NEQ_HBI",
	},
	{
		.bitmask = 0x8000,
		.name = "SENSOR_HBI_MIN_ERROR",
	},
	{
		.bitmask = 0xFFFFFF,
		.name = "SENSOR_VBI",
	},
	{
		.bitmask = 0xFFFF,
		.name = "SENSOR_VBI_IPP_CONTEXT_0",
	},
	{
		.bitmask = 0x10000000,
		.name = "SENSOR_VBI_IPP_MIN_ERROR_CONTEXT_0",
	},
	{
		.bitmask = 0x20000000,
		.name = "SENSOR_VBI_IPP_MIN_ERROR_CONTEXT_1",
	},
	{
		.bitmask = 0x40000000,
		.name = "SENSOR_VBI_IPP_MIN_ERROR_CONTEXT_2",
	},
	{
		.bitmask = 0xFFFF,
		.name = "SENSOR_VBI_IPP_CONTEXT_1",
	},
	{
		.bitmask = 0xFFFF0000,
		.name = "SENSOR_VBI_IPP_CONTEXT_2",
	},
	{
		.bitmask = 0xFF,
		.name = "FRAME_CNT_PPP_PIPE",
	},
	{
		.bitmask = 0xFF00,
		.name = "FRAME_CNT_RDI_0_PIPE",
	},
	{
		.bitmask = 0xFF0000,
		.name = "FRAME_CNT_RDI_1_PIPE",
	},
	{
		.bitmask = 0xFF000000,
		.name = "FRAME_CNT_RDI_2_PIPE",
	},
	{
		.bitmask = 0xFF,
		.name = "FRAME_CNT_RDI_3_PIPE",
	},
	{
		.bitmask = 0xFF00,
		.name = "FRAME_CNT_RDI_4_PIPE",
	},
	{
		.bitmask = 0xFF,
		.name = "FRAME_CNT_IPP_CONTEXT0_PIPE",
	},
	{
		.bitmask = 0xFF00,
		.name = "FRAME_CNT_IPP_CONTEXT1_PIPE",
	},
	{
		.bitmask = 0xFF0000,
		.name = "FRAME_CNT_IPP_CONTEXT2_PIPE",
	},
	{
		.bitmask = 0xFF000000,
		.name = "FRAME_CNT_IPP_ALL_CONTEXT_PIPE",
	},
};

static struct cam_vfe_top_ver4_diag_reg_fields tfe_common_reg_v1_diag_sensor_field[] = {
	{
		.num_fields = 3,
		.field = &tfe_common_reg_v1_diag_reg_info[0],
	},
	{
		.num_fields = 1,
		.field = &tfe_common_reg_v1_diag_reg_info[3],
	},
	{
		.num_fields = 4,
		.field = &tfe_common_reg_v1_diag_reg_info[4],
	},
	{
		.num_fields = 2,
		.field = &tfe_common_reg_v1_diag_reg_info[8],
	},
};

static struct cam_vfe_top_ver4_diag_reg_fields tfe_common_reg_v1_diag_frame_field[] = {
	{
		.num_fields = 4,
		.field = &tfe_common_reg_v1_diag_reg_info[10],
	},
	{
		.num_fields = 2,
		.field = &tfe_common_reg_v1_diag_reg_info[14],
	},
	{
		.num_fields = 4,
		.field = &tfe_common_reg_v1_diag_reg_info[16],
	},
};

/*
 * FCG CLC registers w.r.t fcg_clc_base. If the fcg_clc_base is 0,
 * offsets are relative to core start address.
 */
static struct cam_vfe_ver4_fcg_module_info tfe_common_reg_v1_fcg_module_info = {
	.max_fcg_ch_ctx                      = 3,
	.max_fcg_predictions                 = 3,
	.fcg_index_shift                     = 16,
	.max_reg_val_pair_size               = 6,
	.fcg_type_size                       = 2,
	.fcg_phase_index_cfg_0               = 0x70,
	.fcg_phase_index_cfg_1               = 0x74,
	.fcg_reg_ctxt_shift                  = 0x0,
	.fcg_reg_ctxt_sel                    = 0x1F4,
	.fcg_reg_ctxt_mask                   = 0x7,
};

struct cam_vfe_top_ver4_query_dmi_reg_info tfe_common_reg_v1_top_query_reg = {
		.dmi_cfg                     = 0x14,
		.dmi_lut_cfg                 = 0x18,
		.dmi_data                    = 0x1C,
		.num_entries_per_hm          = 3,
		.hm_start_id                 = 10,
		.num_entries_per_clc         = 15,
		.clc_start_id                = 34,
		.reg_base_mask               = 0xFFFFF,
		.valid_mask                  = 0x1,
		.query_sel_val               = 1,
		.hm_id_top                   = 0,
		.hm_id_bayer                 = 1,
		.clc_id_bus_wr               = 1,
		.clc_id_haf                  = 2,
		.clc_id_fcg                  = 11,
};

static struct cam_vfe_top_ver4_hw_info tfe_common_reg_v1_top_hw_info = {
	.common_reg = &tfe_common_reg_v1_common_reg,
	.vfe_full_hw_info = {
		.common_reg     = &tfe_common_reg_v1_common_reg,
		.reg_data       = &tfe_common_reg_v1_ipp_common_reg_data,
	},
	.pdlib_hw_info = {
		.common_reg     = &tfe_common_reg_v1_common_reg,
		.reg_data       = &tfe_common_reg_v1_pdlib_reg_data,
	},
	.rdi_hw_info            = tfe_common_reg_v1_rdi_hw_info_arr,
	.ipp_module_desc        = tfe_common_reg_v1_mod_desc,
	.bayer_module_desc      = tfe_common_reg_v1_mod_desc,
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
	.num_rdi                         = ARRAY_SIZE(tfe_common_reg_v1_rdi_hw_info_arr),
	.num_top_errors                  = ARRAY_SIZE(tfe_common_reg_v1_top_irq_err_desc),
	.top_err_desc                    = tfe_common_reg_v1_top_irq_err_desc,
	.num_pdaf_violation_errors       = ARRAY_SIZE(tfe_common_reg_v1_haf_violation_desc),
	.pdaf_violation_desc             = tfe_common_reg_v1_haf_violation_desc,
	.core_violation_desc             = tfe_common_reg_v1_core_violation_desc,
	.top_debug_reg_info              = &tfe_common_reg_v1_top_dbg_reg_info,
	.bayer_debug_reg_info            = &tfe_common_reg_v1_bayer_dbg_reg_info,
	.fcg_module_info                 = &tfe_common_reg_v1_fcg_module_info,
	.fcg_mc_supported                = true,
	.diag_sensor_info                = tfe_common_reg_v1_diag_sensor_field,
	.diag_frame_info                 = tfe_common_reg_v1_diag_frame_field,
	.query_reg                       = &tfe_common_reg_v1_top_query_reg,
	.top_hm_base                     = 0x0,
	.bayer_hm_base                   = 0x12800,
	.fcg_clc_base                    = 0x14C80,
	.haf_clc_base                    = 0xD100,
	.bus_wr_base                     = 0x800,
	.bayer_hm_supported              = true,
};

/*
 * if bus_wr_base is defined in cam_vfe_bus_ver3_hw_info, offsets are w.r.t
 * bus start address, else these are defined w.r.t base of the core
 *
 */
static struct cam_irq_register_set tfe_common_reg_v1_bus_irq_reg[2] = {
	{
		.mask_reg_offset   = 0x10,
		.clear_reg_offset  = 0x24,
		.status_reg_offset = 0x38,
		.set_reg_offset    = 0x94,
	},
	{
		.mask_reg_offset   = 0x1C,
		.clear_reg_offset  = 0x30,
		.status_reg_offset = 0x44,
		.set_reg_offset    = 0xA0,
	},
};

static struct cam_vfe_bus_ver3_reg_offset_ubwc_client
	tfe_common_reg_v1_ubwc_regs = {
	.meta_addr        = 0x40,
	.meta_cfg         = 0x44,
	.mode_cfg         = 0x48,
	.stats_ctrl       = 0x4C,
	.ctrl_2           = 0x50,
	.lossy_thresh0    = 0x54,
	.lossy_thresh1    = 0x58,
	.off_lossy_var    = 0x60,
	.bw_limit         = 0x1C,
	.ubwc_comp_en_bit = BIT(1),
};

static struct cam_vfe_bus_ver3_err_irq_desc tfe_common_reg_v1_bus_irq_err_desc[][32] = {
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

static struct cam_vfe_bus_ver3_hw_info tfe_common_reg_v1_bus_hw_info = {
	.common_reg = {
	    /*
	     * if bus_wr_base is defined in cam_vfe_bus_ver3_hw_info, offsets are w.r.t
	     * bus start address, else these are defined w.r.t base of the core
	     *
	     */
		.hw_version                       = 0x0,
		.cgc_ovd                          = 0x4,
		.ubwc_static_ctrl                 = 0xA4,
		.pwr_iso_cfg                      = 0xA8,
		.overflow_status_clear            = 0xAC,
		.ccif_violation_status            = 0xB0,
		.overflow_status                  = 0xB8,
		.image_size_violation_status      = 0xC0,
		.debug_status_top_cfg             = 0x12C,
		.debug_status_top                 = 0x130,
		.ctxt_sel                         = 0x13C,
		.rd_ctxt_sel                      = 0x140,
		.test_bus_ctrl                    = 0x144,
		.mc_read_sel_shift                = 0x0,
		.mc_write_sel_shift               = 0x0,
		.mc_ctxt_mask                     = 0x7,
		.wm_mode_shift                    = 16,
		.wm_mode_val                      = { 0x0, 0x1, 0x2 },
		.wm_en_shift                      = 0,
		.frmheader_en_shift               = 2,
		.virtual_frm_en_shift             = 1,
		.irq_reg_info = {
		    .num_registers            = 2,
		    .irq_reg_set              = tfe_common_reg_v1_bus_irq_reg,
		    .global_irq_cmd_offset    = 0x48,
		    .global_clear_bitmask     = 0x1,
		},
		.num_perf_counters                = 8,
		.perf_cnt_status                  = 0x108,
		.perf_cnt_reg = {
			{
				.perf_cnt_cfg = 0xC8,
				.perf_cnt_val = 0xE8,
			},
			{
				.perf_cnt_cfg = 0xCC,
				.perf_cnt_val = 0xEC,
			},
			{
				.perf_cnt_cfg = 0xD0,
				.perf_cnt_val = 0xF0,
			},
			{
				.perf_cnt_cfg = 0xD4,
				.perf_cnt_val = 0xF4,
			},
			{
				.perf_cnt_cfg = 0xD8,
				.perf_cnt_val = 0xF8,
			},
			{
				.perf_cnt_cfg = 0xDC,
				.perf_cnt_val = 0xFC,
			},
			{
				.perf_cnt_cfg = 0xE0,
				.perf_cnt_val = 0x100,
			},
			{
				.perf_cnt_cfg = 0xE4,
				.perf_cnt_val = 0x104,
			},
		},
		/* HW capabilities*/
		.capabilities =
			CAM_VFE_COMMON_CAP_SPLIT_CTXT_RD_WR_SEL,
	},
	.bus_wr_base                              = 0x800,
	.support_dyn_offset                       = true,
	/*
	 * client_base is w.r.t bus_wr_base. If bus_wr_base is 0,
	 * make client_base relative core start address.
	 */
	.client_base                              = 0x1000,
	.client_reg_size                          = 0x200,
	.ubwc_clients_mask                        = 0x1F,
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
			.irq_subsample_period     = 0x30,
			.irq_subsample_pattern    = 0x34,
			.framedrop_period         = 0x38,
			.framedrop_pattern        = 0x3C,
			.mmu_prefetch_cfg         = 0x64,
			.mmu_prefetch_max_offset  = 0x68,
			.system_cache_cfg         = 0x6C,
			.addr_cfg                 = 0x70,
			.ctxt_cfg                 = 0x74,
			.debug_status_cfg         = 0x78,
			.debug_status_0           = 0x7C,
			.debug_status_1           = 0x80,
			.addr_status_0            = 0x8C,
			.addr_status_1            = 0x90,
			.addr_status_2            = 0x94,
			.addr_status_3            = 0x98,
			.bw_limiter_addr          = 0x1C,
			.ubwc_regs                = &tfe_common_reg_v1_ubwc_regs,

	},
	.bus_client_reg = {
		/* BUS Client 0 FULL_Y */
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
			.name                     = "FULL_Y",
			.line_based               = 1,
			.mid                      = {(33 <<  16) | 32, (35 << 16) | 36,
							    (37 << 16) | 38},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_FULL,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(5) | BIT_ULL(6) | BIT_ULL(7),
		},
		/* BUS Client 1 FULL_C */
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
			.name                     = "FULL_C",
			.line_based               = 1,
			.mid                      = {(39 <<  16) | 38, (41 << 16) | 40,
							    (43 << 16) | 42},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_FULL,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(5) | BIT_ULL(6) | BIT_ULL(7),
		},
		/* BUS Client 2 DS4_Y */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_TP_10) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "DS4_Y",
			.line_based               = 1,
			.mid                      = {(33 <<  16) | 32, (35 << 16) | 36,
							    (37 << 16) | 38},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS4,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(8) | BIT_ULL(9) | BIT_ULL(10),
		},
		/* BUS Client 3 DS4_C */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_TP_10) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "DS4_C",
			.line_based               = 1,
			.mid                      = {(39 <<  16) | 38, (41 << 16) | 40,
							    (43 << 16) | 42},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS4,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(8) | BIT_ULL(9) | BIT_ULL(10),
		},
		/* BUS Client 4 DS16_Y */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_TP_10) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "DS16_Y",
			.line_based               = 1,
			.mid                      = {(45 <<  16) | 44, (47 << 16) | 46,
							    (49 << 16) | 48},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS16,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(8) | BIT_ULL(9) | BIT_ULL(10),
		},
		/* BUS Client 5 DS16_C */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_TP_10) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
						    BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "DS16_C",
			.line_based               = 1,
			.mid                      = {(51 <<  16) | 50, (53 << 16) | 52,
							    (55 << 16) | 54},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_DS16,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(8) | BIT_ULL(9) | BIT_ULL(10),
		},
		/* BUS Client 6 FD_Y */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_8),
			.name                     = "FD_Y",
			.line_based               = 1,
			.mid                      = {(33 <<  16) | 32},
			.num_mid                  = 2,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_FD,
			.cntxt_cfg_except         = true,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 7 FD_C */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_8),
			.name                     = "FD_C",
			.line_based               = 1,
			.mid                      = {34},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_FD,
			.cntxt_cfg_except         = true,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 8 PIXEL RAW */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP) |
				BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_MIPI12) |
				BIT_ULL(PACKER_FMT_VER3_MIPI14),
			.name                     = "PIXEL_RAW",
			.line_based               = 1,
			.mid                      = {(44 <<  16) | 43, (46 << 16) | 45,
							    (46 << 16) | 47},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RAW_DUMP,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 9 STATS_AEC_BE */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "STATS_AEC_BE",
			.mid                      = {32, 33, 34},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AEC_BE,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 10 STATS_AEC_BHIST */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "STATS_BHIST",
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AEC_BHIST,
			.mid                      = {35, 36, 37},
			.num_mid                  = 3,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 11 STATS_TINTLESS_BG */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "STATS_TL_BG",
			.mid                      = {38, 39, 40},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_TL_BG,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 12 STATS_AWB_BG */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "STATS_AWB_BG",
			.mid                      = {41, 42, 43},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AWB_BG,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 13 STATS_AWB_BFW */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "AWB_BFW",
			.mid                      = {44, 45, 46},
			.num_mid                  = 3,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_AWB_BFW,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 14 STATS_AF_BHIST */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "AF_BHIST",
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_AF_BHIST,
			.mid                      = {47, 48, 49},
			.num_mid                  = 3,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 15 STATS_ALSC_BG */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "ALSC_BG",
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_ALSC,
			.mid                      = {50, 51, 52},
			.num_mid                  = 3,
			.mc_based                 =  true,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 16 STATS_FLICKER_BAYERS */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_32),
			.name                     = "STATS_RS",
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_BAYER_RS,
			.mid                      = {53, 54, 55},
			.num_mid                  = 3,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 17 STATS_TMC_BHIST */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "STATS_TMC_BHIST",
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_TMC_BHIST,
			.mid                      = {56, 57, 58},
			.num_mid                  = 3,
			.mc_based                 = true,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 18 PDAF_0 */ /* Note: PDAF_SAD == 2PD*/
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_64),
			.name                     = "PDAF_0_2PD",
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_2PD,
			.mid                      = {44},
			.num_mid                  = 1,
			.pid_mask                 = BIT_ULL(5) | BIT_ULL(6) | BIT_ULL(7),
			.early_done_mask          = BIT(18),
		},
		/* BUS Client 19 PDAF_1 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "PDAF_1_PREPROCESS_2PD",
			.line_based               = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_PREPROCESS_2PD,
			.mid                      = {40},
			.num_mid                  = 1,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 20 PDAF_2 */
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
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_PDAF_PARSED,
			.mid                      = {41},
			.num_mid                  = 1,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 21 PDAF_3 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_4,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "STATS_CAF",
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_STATS_CAF,
			.mid                      = {45},
			.num_mid                  = 1,
			.pid_mask                 = BIT_ULL(5) | BIT_ULL(6) | BIT_ULL(7),
			.early_done_mask          = BIT(21),
		},
		/* BUS Client 22 PDAF_4 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_3,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "STATS_PDAF_4",
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_PDAF_PREPROCESSED2,
			.mid                      = {42},
			.num_mid                  = 1,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
			.early_done_mask          = BIT(22),
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
			.line_based               = 1,
			.mid                      = {35},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI0,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 24 RDI_1 */
		{
			.comp_group               =  CAM_VFE_BUS_VER3_COMP_GRP_6,
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
			.mid                      = {36},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI1,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 25 RDI_2 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_7,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_MIPI10) |
				BIT_ULL(PACKER_FMT_VER3_MIPI12) | BIT_ULL(PACKER_FMT_VER3_MIPI14) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_128) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_8) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_10BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_12BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_14BPP) |
				BIT_ULL(PACKER_FMT_VER3_PLAIN_16_16BPP),
			.name                     = "RDI_2",
			.line_based               = 1,
			.mid                      = {37},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI2,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 26 RDI_3 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_8,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "RDI_3",
			.line_based               = 1,
			.mid                      = {38},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI3,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 27 RDI_4 */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_9,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_128),
			.name                     = "RDI_4",
			.line_based               = 1,
			.mid                      = {39},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_RDI4,
			.pid_mask                 = BIT_ULL(11) | BIT_ULL(12) | BIT_ULL(13),
		},
		/* BUS Client 28 FD_SECURE_Y */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_8),
			.name                     = "CPA_Y",
			.line_based               = 1,
			.mid                      = {59, 60},
			.num_mid                  = 2,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_FD_SECURE,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
		/* BUS Client 29 FD_SECURE_C */
		{
			.comp_group               = CAM_VFE_BUS_VER3_COMP_GRP_0,
			.supported_pack_formats   = BIT_ULL(PACKER_FMT_VER3_PLAIN_8),
			.name                     = "CPA_C",
			.line_based               = 1,
			.mid                      = {61},
			.num_mid                  = 1,
			.out_type                 = CAM_VFE_BUS_VER3_VFE_OUT_FD_SECURE,
			.pid_mask                 = BIT_ULL(14) | BIT_ULL(15) | BIT_ULL(16),
		},
	},
	.valid_wm_mask   = 0x3FFFFFFF,
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
	.bus_err_desc          = &tfe_common_reg_v1_bus_irq_err_desc,
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
};

static struct cam_vfe_irq_hw_info tfe_common_reg_v1_irq_hw_info = {
	.reset_mask    = 0,
	.supported_irq = CAM_VFE_HW_IRQ_CAP_EXT_CSID,
	.top_irq_reg   = &tfe_common_reg_v1_top_irq_reg_info,
};

static struct cam_vfe_hw_info cam_tfe_common_reg_v1_hw_info = {
	.irq_hw_info                  = &tfe_common_reg_v1_irq_hw_info,

	.bus_version                   = CAM_VFE_BUS_VER_3_0,
	.bus_hw_info                   = &tfe_common_reg_v1_bus_hw_info,

	.top_version                   = CAM_VFE_TOP_VER_4_0,
	.top_hw_info                   = &tfe_common_reg_v1_top_hw_info,
};
#endif /* _CAM_COMMON_REG_V1_H_ */
