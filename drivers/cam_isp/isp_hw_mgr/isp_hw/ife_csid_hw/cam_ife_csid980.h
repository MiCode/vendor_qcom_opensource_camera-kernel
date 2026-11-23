/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2023-2024, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_IFE_CSID_980_H_
#define _CAM_IFE_CSID_980_H_

#include <linux/module.h>
#include "cam_ife_csid_dev.h"
#include "camera_main.h"
#include "cam_ife_csid_common.h"
#include "cam_ife_csid_hw_ver2.h"
#include "cam_irq_controller.h"
#include "cam_isp_hw_mgr_intf.h"

#define CAM_CSID_VERSION_V980                 0x90080000

static struct cam_ife_csid_irq_desc cam_ife_csid_980_rx_irq_desc[][32] = {
	{
		{
			.bitmask = BIT(0),
			.desc = "DL0_EOT",
		},
		{
			.bitmask = BIT(1),
			.desc = "DL1_EOT",
		},
		{
			.bitmask = BIT(2),
			.desc = "DL2_EOT",
		},
		{
			.bitmask = BIT(3),
			.desc = "DL3_EOT",
		},
		{
			.bitmask = BIT(4),
			.desc = "DL0_SOT",
		},
		{
			.bitmask = BIT(5),
			.desc = "DL1_SOT",
		},
		{
			.bitmask = BIT(6),
			.desc = "DL2_SOT",
		},
		{
			.bitmask = BIT(7),
			.desc = "DL3_SOT",
		},
		{
			.bitmask = BIT(8),
			.desc = "DPHY_PH_ECC_SEC",
		},
		{
			.bitmask = BIT(9),
			.desc = "SENSOR_MODE_ID_CHANGE",
		},
		{0},
		{0},
		{
			.bitmask = BIT(12),
			.desc =
				"DL0_EOT_LOST, Sensor: Issue is with the timing signals received in the cphy packet on lane 0 - Check phy/sensor config",
		},
		{
			.bitmask = BIT(13),
			.desc =
				"DL1_EOT_LOST, Sensor: Issue is with the timing signals received in the cphy packet on lane 1 - Check phy/sensor config",
		},
		{
			.bitmask = BIT(14),
			.desc =
				"DL2_EOT_LOST, Sensor: Issue is with the timing signals received in the cphy packet on lane 2 - Check phy/sensor config",
		},
		{
			.bitmask = BIT(15),
			.desc =
				"DL3_EOT_LOST, Sensor: Issue is with the timing signals received in the cphy packet on lane 3 - Check phy/sensor config",
		},
		{
			.bitmask = BIT(16),
			.desc =
				"DL0_SOT_LOST, Sensor: Timing signals are missed received in the cphy packet on lane 0 - Check phy/sensor config",
		},
		{
			.bitmask = BIT(17),
			.desc =
				"DL1_SOT_LOST, Sensor: Timing signals are missed received in the cphy packet on lane 1 - Check phy/sensor config",
		},
		{
			.bitmask = BIT(18),
			.desc =
				"DL2_SOT_LOST, Sensor: Timing signals are missed received in the cphy packet on lane 2 - Check phy/sensor config",
		},
		{
			.bitmask = BIT(19),
			.desc =
				"DL3_SOT_LOST, Sensor: Timing signals are missed received in the cphy packet on lane 3 - Check phy/sensor config",
		},
		{
			.bitmask = BIT(20),
			.desc =
				"DL0_FIFO_OVERFLOW, System: Data has been lost when transferring from PHY to CSID on Lane 0 - Check PHY clock, CSID clock and possible skew among the data lanes",
		},
		{
			.bitmask = BIT(21),
			.desc =
				"DL1_FIFO_OVERFLOW, System: Data has been lost when transferring from PHY to CSID on Lane 1 - Check PHY clock, CSID clock and possible skew among the data lanes",
		},
		{
			.bitmask = BIT(22),
			.desc =
				"DL2_FIFO_OVERFLOW, System: Data has been lost when transferring from PHY to CSID on Lane 2 - Check PHY clock, CSID clock and possible skew among the data lanes",
		},
		{
			.bitmask = BIT(23),
			.desc =
				"DL3_FIFO_OVERFLOW, System: Data has been lost when transferring from PHY to CSID on Lane 3 - Check PHY clock, CSID clock and possible skew among the data lanes",
		},
		{
			.bitmask = BIT(24),
			.desc =
				"CPHY_PH_CRC, Sensor: All CPHY packet headers received are corrupted - Check phy/sensor config",
		},
		{
			.bitmask = BIT(25),
			.desc =
				"PAYLOAD_CRC, Sensor: The calculated CRC of a long packet does not match the transmitted (expected) CRC, possible corruption - Check phy/sensor config",
		},
		{
			.bitmask = BIT(26),
			.desc =
				"DPHY_PH_ECC_DED, Sensor: A short or long packet is corrupted and cannot be recovered - Check phy/sensor config",
		},
		{
			.bitmask = BIT(27),
			.desc =
				"MMAPPED_VC_DT, SW: A long packet has a VC_DT combination that is configured for more than one IPP or RDIs",
		},
		{
			.bitmask = BIT(28),
			.desc =
				"UNMAPPED_VC_DT, Sensor: A long packet has a VC_DT combination that is not configured for IPP or RDIs",
		},
		{
			.bitmask = BIT(29),
			.desc =
				"STREAM_UNDERFLOW, Sensor: Long packet payload size is less than payload header size, resulting a corrupted frame - Perform PHY Tuning/Check sensor limitations",
		},
		{0},
		{
			.bitmask = BIT(31),
			.desc = "CSI2_RX_IRQ_STATUS_2",
		},
	},
	{
		{
			.bitmask = BIT(0),
			.desc =
				"LONG_PKT, Debug: The header of the first long pkt matching the configured vc-dt has been captured",
		},
		{
			.bitmask = BIT(1),
			.desc =
				"SHORT_PKT, Debug: The header of the first short pkt matching the configured vc-dt has been captured",
		},
		{
			.bitmask = BIT(2),
			.desc =
				"CPHY_PKT_HDR, Debug: The header of the first cphy pkt matching the configured vc-dt has been captured",
		},
		{
			.bitmask = BIT(3),
			.desc = "Illegal programming for next frame ID config",
		},
	},
};

static uint32_t cam_ife_csid_980_num_rx_irq_desc[] = {
	ARRAY_SIZE(cam_ife_csid_980_rx_irq_desc[0]),
	ARRAY_SIZE(cam_ife_csid_980_rx_irq_desc[1]),
};

static struct cam_ife_csid_irq_desc cam_ife_csid_980_path_irq_desc[] = {
	{
		.bitmask = BIT(0),
		.err_type = CAM_ISP_HW_ERROR_CSID_FATAL,
		.irq_name = "ILLEGAL_PROGRAMMING",
		.desc = "SW: Illegal programming sequence",
		.debug = "Check the following possiblities:",
		.err_handler = cam_ife_csid_ver2_print_illegal_programming_irq_status,
	},
	{0},
	{
		.bitmask = BIT(2),
		.irq_name = "INFO_DATA_FIFO_FULL",
	},
	{
		.bitmask = BIT(3),
		.irq_name = "CAMIF_EOF",
	},
	{
		.bitmask = BIT(4),
		.irq_name = "CAMIF_SOF",
	},
	{0},
	{0},
	{0},
	{0},
	{
		.bitmask = BIT(9),
		.irq_name = "INFO_INPUT_EOF",
	},
	{
		.bitmask = BIT(10),
		.irq_name = "INFO_INPUT_EOL",
	},
	{
		.bitmask = BIT(11),
		.irq_name = "INFO_INPUT_SOL",
	},
	{
		.bitmask = BIT(12),
		.irq_name = "INFO_INPUT_SOF",
	},
	{
		.bitmask = BIT(13),
		.err_type = CAM_ISP_HW_ERROR_CSID_FRAME_SIZE,
		.irq_name = "ERROR_PIX_COUNT",
		.desc = "SW: Mismatch in expected versus received number of pixels per line",
		.debug = "Check SW config/sensor stream",
		.err_handler = cam_ife_csid_ver2_print_format_measure_info,
	},
	{
		.bitmask = BIT(14),
		.err_type = CAM_ISP_HW_ERROR_CSID_FRAME_SIZE,
		.irq_name = "ERROR_LINE_COUNT",
		.desc = "SW: Mismatch in expected versus received number of lines",
		.debug = "Check SW config/sensor stream",
		.err_handler = cam_ife_csid_ver2_print_format_measure_info,
	},
	{
		.bitmask = BIT(15),
		.irq_name = "VCDT_GRP1_SEL",
	},
	{
		.bitmask = BIT(16),
		.irq_name = "VCDT_GRP0_SEL",
	},
	{
		.bitmask = BIT(17),
		.irq_name = "VCDT_GRP_CHANGE",
	},
	{
		.bitmask = BIT(18),
		.err_type = CAM_ISP_HW_ERROR_CSID_CAMIF_FRAME_DROP,
		.irq_name = "CAMIF_FRAME_DROP",
		.desc =
			"Sensor: The pre CAMIF frame drop would drop a frame in case the new frame starts prior to the end of the previous frame",
		.debug = "Slower downstream processing or faster frame generation from sensor",
	},
	{
		.bitmask = BIT(19),
		.err_type = CAM_ISP_HW_ERROR_RECOVERY_OVERFLOW,
		.irq_name = "OVERFLOW_RECOVERY",
		.desc = "Detected by the overflow recovery block",
		.debug = "Backpressure downstream",
	},
	{
		.bitmask = BIT(20),
		.irq_name = "ERROR_REC_CCIF_VIOLATION",
		.desc = "Output CCIF has a violation with respect to frame timing",
	},
	{
		.bitmask = BIT(21),
		.irq_name = "CAMIF_EPOCH0",
	},
	{
		.bitmask = BIT(22),
		.irq_name = "CAMIF_EPOCH1",
	},
	{
		.bitmask = BIT(23),
		.irq_name = "RUP_DONE",
	},
	{
		.bitmask = BIT(24),
		.irq_name = "ILLEGAL_BATCH_ID",
		.desc = "SW: Decoded frame ID does not match with any of the programmed batch IDs",
		.debug = "Check frame ID and all available batch IDs",
	},
	{
		.bitmask = BIT(25),
		.irq_name = "BATCH_END_MISSING_VIOLATION",
		.desc = "SW: Input number of frames is not a multiple of the batch size",
		.debug = "Check the configured pattern/period for batching",
	},
	{
		.bitmask = BIT(26),
		.err_type = CAM_ISP_HW_ERROR_CSID_UNBOUNDED_FRAME,
		.irq_name = "UNBOUNDED_FRAME",
		.desc = "Sensor: Frame end or frame start is missing",
		.debug = "Check the settle count in sensor driver XML",
	},
	{0},
	{
		.bitmask = BIT(28),
		.irq_name = "SENSOR_SWITCH_OUT_OF_SYNC_FRAME_DROP",
		.desc =
			"Sensor/SW: The programmed MUP is out of sync with the VC of the incoming frame",
		.err_handler = cam_ife_csid_hw_ver2_mup_mismatch_handler,
	},
	{
		.bitmask = BIT(29),
		.irq_name = "CCIF_VIOLATION",
		.desc =
			"The output CCIF from the serializer has a violation with respect to frame timing",
	},
};

static struct cam_ife_csid_top_irq_desc cam_ife_csid_980_top_irq_desc[][32] = {
	{
		{
			.bitmask  = BIT(1),
			.err_type = CAM_ISP_HW_ERROR_CSID_SENSOR_SWITCH_ERROR,
			.err_name = "FATAL_SENSOR_SWITCHING_IRQ",
			.desc =
				"Sensor/SW: Minimum VBI period between dynamically switching between two sensor modes was either violated or the downstream pipe was not active when the switch was made",
		},
	},
	{
		{
			.bitmask  = BIT(2),
			.err_name = "ERROR_NO_VOTE_DN",
			.desc =
				"DRV: vote_down is never generated for the same frame and resource is never relinquished",
			.debug = "Check vote up generated time",
		},
		{
			.bitmask  = BIT(3),
			.err_type = CAM_ISP_HW_ERROR_DRV_VOTEUP_LATE,
			.err_name = "ERROR_VOTE_UP_LATE",
			.desc = "DRV: vote_up is generated after SOF",
			.debug = "Check the vote up timer value",
			.err_handler = cam_ife_csid_hw_ver2_drv_err_handler,
		},
		{
			.bitmask  = BIT(4),
			.err_type = CAM_ISP_HW_ERROR_CSID_OUTPUT_FIFO_OVERFLOW,
			.err_name = "ERROR_RDI_LINE_BUFFER_CONFLICT",
			.desc =
				"System/SW: Multiple RDIs configured to access the same shared line buffer, more of a SW issue that led to this programming",
			.err_handler = cam_ife_csid_hw_ver2_rdi_line_buffer_conflict_handler,
		},
		{
			.bitmask = BIT(5),
			.err_name = "ERROR_SENSOR_HBI",
			.desc = "Sensor: Sensor HBI is less than expected HBI",
			.debug = "Check sensor configuration",
		},
	},
};

static uint32_t cam_ife_csid_980_num_top_irq_desc[] = {
	ARRAY_SIZE(cam_ife_csid_980_top_irq_desc[0]),
	ARRAY_SIZE(cam_ife_csid_980_top_irq_desc[1]),
};

static struct cam_irq_register_set cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_MAX] = {
	/* Top_1 */
	{
		.mask_reg_offset   = 0x00000088,
		.clear_reg_offset  = 0x0000008C,
		.status_reg_offset = 0x00000084,
		.set_reg_offset    = 0x00000090,
		.test_set_val      = BIT(0),
		.test_sub_val      = BIT(0),
		.force_rd_mask     = BIT(31), /* force read due to hw errata */
	},
	/* RX_1 */
	{
		.mask_reg_offset   = 0x000000B8,
		.clear_reg_offset  = 0x000000BC,
		.status_reg_offset = 0x000000B4,
	},
	/* RDI0 */
	{
		.mask_reg_offset   = 0x00000118,
		.clear_reg_offset  = 0x0000011C,
		.status_reg_offset = 0x00000114,
	},
	/* RDI1 */
	{
		.mask_reg_offset   = 0x00000128,
		.clear_reg_offset  = 0x0000012C,
		.status_reg_offset = 0x00000124,
	},
	/* RDI2 */
	{
		.mask_reg_offset   = 0x00000138,
		.clear_reg_offset  = 0x0000013C,
		.status_reg_offset = 0x00000134,
	},
	/* RDI3 */
	{
		.mask_reg_offset   = 0x00000148,
		.clear_reg_offset  = 0x0000014C,
		.status_reg_offset = 0x00000144,
	},
	/* RDI4 */
	{
		.mask_reg_offset   = 0x00000158,
		.clear_reg_offset  = 0x0000015C,
		.status_reg_offset = 0x00000154,
	},
	/* IPP_0 */
	{
		.mask_reg_offset   = 0x000000D8,
		.clear_reg_offset  = 0x000000DC,
		.status_reg_offset = 0x000000D4,
	},
	/* PPP */
	{
		.mask_reg_offset   = 0x00000108,
		.clear_reg_offset  = 0x0000010C,
		.status_reg_offset = 0x00000104,
	},
	/* UDI_0 */
	{0},
	/* UDI_1 */
	{0},
	/* UDI_2 */
	{0},

	/* Top_2 */
	{
		.mask_reg_offset   = 0x00000098,
		.clear_reg_offset  = 0x0000009C,
		.status_reg_offset = 0x00000094,
		.set_reg_offset    = 0x00000090,
	},
	/* RX_2 */
	{
		.mask_reg_offset   = 0x000000C8,
		.clear_reg_offset  = 0x000000CC,
		.status_reg_offset = 0x000000C4,
	},
	/* IPP_1 */
	{
		.mask_reg_offset   = 0x000000E8,
		.clear_reg_offset  = 0x000000EC,
		.status_reg_offset = 0x000000E4,
	},
	/* IPP_2 */
	{
		.mask_reg_offset   = 0x000000F8,
		.clear_reg_offset  = 0x000000FC,
		.status_reg_offset = 0x000000F4,
	},
};

static struct cam_irq_controller_reg_info cam_ife_csid_980_top_irq_reg_info[] = {
	{
	.num_registers = 1,
	.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_TOP],
	.global_irq_cmd_offset = 0x00000014,
	.global_clear_bitmask  = 0x00000001,
	.global_set_bitmask    = 0x00000010,
	.clear_all_bitmask     = 0xFFFFFFFF,
	},
	{
	.num_registers = 1,
	.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_TOP_2],
	.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},

};

static struct cam_irq_controller_reg_info cam_ife_csid_980_rx_irq_reg_info[] = {
	{
	.num_registers = 1,
	.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_RX],
	.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},
	{
	.num_registers = 1,
	.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_RX_2],
	.global_irq_cmd_offset = 0, /* intentionally set to zero */

	},
};

static struct cam_irq_controller_reg_info
	cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_MAX] = {
	{
		.num_registers = 1,
		.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_RDI_0],
		.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},
	{
		.num_registers = 1,
		.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_RDI_1],
		.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},
	{
		.num_registers = 1,
		.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_RDI_2],
		.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},
	{
		.num_registers = 1,
		.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_RDI_3],
		.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},
	{
		.num_registers = 1,
		.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_RDI_4],
		.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},
	{
		.num_registers = 1,
		.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_IPP],
		.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},
	{
		.num_registers = 1,
		.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_PPP],
		.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},
	/* UDI_0 */
	{0},
	/* UDI_1 */
	{0},
	/* UDI_2 */
	{0},
	{
		.num_registers = 1,
		.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_IPP_1],
		.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},
	{
		.num_registers = 1,
		.irq_reg_set = &cam_ife_csid_980_irq_reg_set[CAM_IFE_CSID_IRQ_REG_IPP_2],
		.global_irq_cmd_offset = 0, /* intentionally set to zero */
	},

};

static struct cam_irq_register_set cam_ife_csid_980_buf_done_irq_reg_set[1] = {
	{
		.mask_reg_offset   = 0x000000A8,
		.clear_reg_offset  = 0x000000AC,
		.status_reg_offset = 0x000000A4,
	},
};

static struct cam_irq_controller_reg_info
	cam_ife_csid_980_buf_done_irq_reg_info = {
	.num_registers = 1,
	.irq_reg_set = cam_ife_csid_980_buf_done_irq_reg_set,
	.global_irq_cmd_offset  = 0, /* intentionally set to zero */
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_ipp_cmn_reg = {
		.irq_status_addr                  = 0x00D4,
		.irq_mask_addr                    = 0x00D8,
		.irq_clear_addr                   = 0x00DC,
		.irq_set_addr                     = 0x00E0,
		.cfg0_addr                        = 0x0000,
		.ctrl_addr                        = 0x0004,
		.debug_clr_cmd_addr               = 0x0008,
		.multi_vcdt_cfg0_addr             = 0x000C,
		.cfg1_addr                        = 0,
		.sparse_pd_extractor_cfg_addr     = 0,
		.err_recovery_cfg0_addr           = 0,
		.err_recovery_cfg1_addr           = 0,
		.err_recovery_cfg2_addr           = 0,
		.camif_frame_cfg_addr             = 0x0038,
		.epoch_irq_cfg_addr               = 0,
		.epoch0_subsample_ptrn_addr       = 0,
		.epoch1_subsample_ptrn_addr       = 0,
		.debug_rup_aup_status             = 0x0048,
		.debug_camif_1_addr               = 0x004C,
		.debug_camif_0_addr               = 0x0050,
		.frm_drop_pattern_addr            = 0x0080,
		.frm_drop_period_addr             = 0x0084,
		.irq_subsample_pattern_addr       = 0x0088,
		.irq_subsample_period_addr        = 0x008C,
		.hcrop_addr                       = 0,
		.vcrop_addr                       = 0,
		.pix_drop_pattern_addr            = 0,
		.pix_drop_period_addr             = 0,
		.line_drop_pattern_addr           = 0,
		.line_drop_period_addr            = 0,
		.debug_halt_status_addr           = 0x0054,
		.debug_misr_val0_addr             = 0x0058,
		.debug_misr_val1_addr             = 0x005C,
		.debug_misr_val2_addr             = 0x0060,
		.debug_misr_val3_addr             = 0x0064,
		.format_measure_cfg0_addr         = 0x0090,
		.format_measure_cfg1_addr         = 0x0094,
		.format_measure_cfg2_addr         = 0x0098,
		.format_measure0_addr             = 0x009C,
		.format_measure1_addr             = 0x00A0,
		.format_measure2_addr             = 0x00A4,
		.timestamp_curr0_sof_addr         = 0x00A8,
		.timestamp_curr1_sof_addr         = 0x00AC,
		.timestamp_perv0_sof_addr         = 0x00B0,
		.timestamp_perv1_sof_addr         = 0x00B4,
		.timestamp_curr0_eof_addr         = 0x00B8,
		.timestamp_curr1_eof_addr         = 0x00BC,
		.timestamp_perv0_eof_addr         = 0x00C0,
		.timestamp_perv1_eof_addr         = 0x00C4,
		.lut_bank_cfg_addr                = 0,
		.secure_mask_cfg0                 = 0,
		.path_batch_status                = 0x010C,
		.path_frame_id                    = 0x0110,
		.cfg2_addr                        = 0x0114,

		/* configurations */
		.resume_frame_boundary            = 1,
		.start_mode_internal              = 0x0,
		.start_mode_global                = 0x1,
		.start_mode_master                = 0x2,
		.start_mode_slave                 = 0x3,
		.sof_retiming_dis_shift           = 5,
		.start_mode_shift                 = 2,
		.start_master_sel_val             = 0,
		.start_master_sel_shift           = 4,
		.crop_v_en_shift_val              = 13,
		.crop_h_en_shift_val              = 12,
		.drop_v_en_shift_val              = 11,
		.drop_h_en_shift_val              = 10,
		.pix_store_en_shift_val           = 0,
		.early_eof_en_shift_val           = 16,
		.bin_h_en_shift_val               = 20,
		.bin_v_en_shift_val               = 21,
		.bin_en_shift_val                 = 18,
		.bin_qcfa_en_shift_val            = 19,
		.format_measure_en_shift_val      = 4,
		.timestamp_en_shift_val           = 6,
		.overflow_ctrl_en                 = 0,
		.overflow_ctrl_mode_val           = 0x8,
		.min_hbi_shift_val                = 4,
		.start_master_sel_shift_val       = 4,
		.fatal_err_mask                   = 0x241c6001,
		.non_fatal_err_mask               = 0x12000000,
		.sof_irq_mask                     = 0x10,
		.rup_irq_mask                     = 0x800000,
		.epoch0_irq_mask                  = 0x200000,
		.epoch1_irq_mask                  = 0x400000,
		.eof_irq_mask                     = 0x8,
		.epoch0_shift_val                 = 16,
		.epoch1_shift_val                 = 0,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_ipp_reg_info = {
		.irq_status_addr                  = 0x00D4,
		.irq_mask_addr                    = 0x00D8,
		.irq_clear_addr                   = 0x00DC,
		.irq_set_addr                     = 0x00E0,

		/* configurations */
		.binning_supported                = 0x7,
		.capabilities                     = CAM_IFE_CSID_CAP_SOF_RETIME_DIS |
							CAM_IFE_CSID_CAP_MULTI_CTXT,
		.rup_mask                         = 0x1,
		.aup_mask                         = 0x1,
		.rup_aup_set_mask                 = 0x1,
		.top_irq_mask                     = {0x100,},
		.disable_sof_retime_default       = true,
		.base                             = 0x600,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_ipp_1_reg_info = {
		.irq_status_addr                  = 0x00E4,
		.irq_mask_addr                    = 0x00E8,
		.irq_clear_addr                   = 0x00EC,
		.irq_set_addr                     = 0x00F0,

		/* configurations */
		.binning_supported                = 0x7,
		.capabilities                     = CAM_IFE_CSID_CAP_SOF_RETIME_DIS |
							CAM_IFE_CSID_CAP_MULTI_CTXT,
		.rup_mask                         = 0x2,
		.aup_mask                         = 0x2,
		.rup_aup_set_mask                 = 0x1,
		.top_irq_mask                     = {0x200,},
		.disable_sof_retime_default       = true,
		.base                             = 0x800,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_ipp_2_reg_info = {
		.irq_status_addr                  = 0x00F4,
		.irq_mask_addr                    = 0x00F8,
		.irq_clear_addr                   = 0x00FC,
		.irq_set_addr                     = 0x0100,

		/* configurations */
		.binning_supported                = 0x7,
		.capabilities                     = CAM_IFE_CSID_CAP_SOF_RETIME_DIS |
							CAM_IFE_CSID_CAP_MULTI_CTXT,
		.rup_mask                         = 0x4,
		.aup_mask                         = 0x4,
		.rup_aup_set_mask                 = 0x1,
		.top_irq_mask                     = {0x400,},
		.disable_sof_retime_default       = true,
		.base                             = 0xA00,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_ppp_reg_info = {
		.irq_status_addr                  = 0x0104,
		.irq_mask_addr                    = 0x0108,
		.irq_clear_addr                   = 0x010C,
		.irq_set_addr                     = 0x0110,
		.cfg0_addr                        = 0x0C00,
		.ctrl_addr                        = 0x0C04,
		.debug_clr_cmd_addr               = 0x0C08,
		.multi_vcdt_cfg0_addr             = 0x0C0C,
		.cfg1_addr                        = 0x0C10,
		.bin_cfg0_addr                    = 0x0C14,
		.pix_store_cfg0_addr              = 0x0C18,
		.sparse_pd_extractor_cfg_addr     = 0x0C1C,
		.err_recovery_cfg0_addr           = 0,
		.err_recovery_cfg1_addr           = 0,
		.err_recovery_cfg2_addr           = 0,
		.bin_pd_detect_cfg0_addr          = 0,
		.bin_pd_detect_cfg1_addr          = 0,
		.bin_pd_detect_cfg2_addr	      = 0,
		.camif_frame_cfg_addr             = 0x0C38,
		.epoch_irq_cfg_addr               = 0x0C3C,
		.epoch0_subsample_ptrn_addr       = 0x0C40,
		.epoch1_subsample_ptrn_addr       = 0x0C44,
		.debug_rup_aup_status             = 0x0C48,
		.debug_camif_1_addr               = 0x0C4C,
		.debug_camif_0_addr               = 0x0C50,
		.frm_drop_pattern_addr            = 0x0C80,
		.frm_drop_period_addr             = 0x0C84,
		.irq_subsample_pattern_addr       = 0x0C88,
		.irq_subsample_period_addr        = 0x0C8C,
		.hcrop_addr                       = 0x0C68,
		.vcrop_addr                       = 0x0C6C,
		.pix_drop_pattern_addr            = 0x0C70,
		.pix_drop_period_addr             = 0x0C74,
		.line_drop_pattern_addr           = 0x0C78,
		.line_drop_period_addr            = 0x0C7C,
		.debug_halt_status_addr           = 0x0C54,
		.debug_misr_val0_addr             = 0x0C58,
		.debug_misr_val1_addr             = 0x0C5C,
		.debug_misr_val2_addr             = 0x0C60,
		.debug_misr_val3_addr             = 0x0C64,
		.format_measure_cfg0_addr         = 0x0C90,
		.format_measure_cfg1_addr         = 0x0C94,
		.format_measure_cfg2_addr         = 0x0C98,
		.format_measure0_addr             = 0x0C9C,
		.format_measure1_addr             = 0x0CA0,
		.format_measure2_addr             = 0x0CA4,
		.timestamp_curr0_sof_addr         = 0x0CA8,
		.timestamp_curr1_sof_addr         = 0x0CAC,
		.timestamp_perv0_sof_addr         = 0x0CB0,
		.timestamp_perv1_sof_addr         = 0x0CB4,
		.timestamp_curr0_eof_addr         = 0x0CB8,
		.timestamp_curr1_eof_addr         = 0x0CBC,
		.timestamp_perv0_eof_addr         = 0x0CC0,
		.timestamp_perv1_eof_addr         = 0x0CC4,
		.lut_bank_cfg_addr                = 0x0CC8,
		.batch_id_cfg0_addr               = 0x0CCC,
		.batch_id_cfg1_addr               = 0x0CD0,
		.batch_period_cfg_addr            = 0x0CD4,
		.batch_stream_id_cfg_addr         = 0x0CD8,
		.epoch0_cfg_batch_id0_addr        = 0x0CDC,
		.epoch1_cfg_batch_id0_addr        = 0x0CE0,
		.epoch0_cfg_batch_id1_addr        = 0x0CE4,
		.epoch1_cfg_batch_id1_addr        = 0x0CE8,
		.epoch0_cfg_batch_id2_addr        = 0x0CEC,
		.epoch1_cfg_batch_id2_addr        = 0x0CF0,
		.epoch0_cfg_batch_id3_addr        = 0x0CF4,
		.epoch1_cfg_batch_id3_addr        = 0x0CF8,
		.epoch0_cfg_batch_id4_addr        = 0x0CFC,
		.epoch1_cfg_batch_id4_addr        = 0x0D00,
		.epoch0_cfg_batch_id5_addr        = 0x0D04,
		.epoch1_cfg_batch_id5_addr        = 0x0D08,
		.secure_mask_cfg0                 = 0,
		.path_batch_status                = 0x0D0C,
		.path_frame_id                    = 0x0D10,
		.debug_sim_monitor                = 0x0D14,

		/* configurations */
		.capabilities                     = CAM_IFE_CSID_CAP_SOF_RETIME_DIS,
		.resume_frame_boundary            = 1,
		.start_mode_shift                 = 2,
		.start_mode_internal              = 0x0,
		.start_mode_global                = 0x1,
		.start_mode_master                = 0x2,
		.start_mode_slave                 = 0x3,
		.start_master_sel_val             = 3,
		.start_master_sel_shift           = 4,
		.binning_supported                = 0x1,
		.bin_h_en_shift_val               = 18,
		.bin_en_shift_val                 = 18,
		.early_eof_en_shift_val           = 16,
		.pix_store_en_shift_val           = 0,
		.crop_v_en_shift_val              = 13,
		.crop_h_en_shift_val              = 12,
		.drop_v_en_shift_val              = 11,
		.drop_h_en_shift_val              = 10,
		.format_measure_en_shift_val      = 4,
		.timestamp_en_shift_val           = 6,
		.overflow_ctrl_en                 = 0,
		.overflow_ctrl_mode_val           = 0x8,
		.min_hbi_shift_val                = 1,
		.start_master_sel_shift_val       = 4,
		.lut_bank_0_sel_val               = 0,
		.lut_bank_1_sel_val               = 1,
		.fatal_err_mask                   = 0x241c6001,
		.non_fatal_err_mask               = 0x12000000,
		.rup_mask                         = 0x10000,
		.aup_mask                         = 0x10000,
		.rup_aup_set_mask                 = 0x1,
		.top_irq_mask                     = {0x10,},
		.epoch0_shift_val                 = 16,
		.epoch1_shift_val                 = 0,
		.sof_retiming_dis_shift           = 5,
		.disable_sof_retime_default       = true,
		.use_master_slave_default         = true,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_rdi_cmn_reg_info = {
		.irq_status_addr                  = 0x0114,
		.irq_mask_addr                    = 0x0118,
		.irq_clear_addr                   = 0x011C,
		.irq_set_addr                     = 0x0120,
		.cfg0_addr                        = 0x0000,
		.ctrl_addr                        = 0x0004,
		.debug_clr_cmd_addr               = 0x0008,
		.multi_vcdt_cfg0_addr             = 0x000C,
		.cfg1_addr                        = 0x0010,
		.pix_store_cfg0_addr              = 0x0014,
		.err_recovery_cfg0_addr           = 0,
		.err_recovery_cfg1_addr           = 0,
		.err_recovery_cfg2_addr           = 0,
		.debug_byte_cntr_ping_addr        = 0x0024,
		.debug_byte_cntr_pong_addr        = 0x0028,
		.camif_frame_cfg_addr             = 0x002C,
		.epoch_irq_cfg_addr               = 0x0030,
		.epoch0_subsample_ptrn_addr       = 0x0034,
		.epoch1_subsample_ptrn_addr       = 0x0038,
		.debug_rup_aup_status             = 0x003C,
		.debug_camif_1_addr               = 0x0040,
		.debug_camif_0_addr               = 0x0044,
		.frm_drop_pattern_addr            = 0x0048,
		.frm_drop_period_addr             = 0x004C,
		.irq_subsample_pattern_addr       = 0x0050,
		.irq_subsample_period_addr        = 0x0054,
		.hcrop_addr                       = 0x0058,
		.vcrop_addr                       = 0x005C,
		.pix_drop_pattern_addr            = 0x0060,
		.pix_drop_period_addr             = 0x0064,
		.line_drop_pattern_addr           = 0x0068,
		.line_drop_period_addr            = 0x006C,
		.debug_halt_status_addr           = 0x0074,
		.debug_misr_val0_addr             = 0x0078,
		.debug_misr_val1_addr             = 0x007C,
		.debug_misr_val2_addr             = 0x0080,
		.debug_misr_val3_addr             = 0x0084,
		.format_measure_cfg0_addr         = 0x0088,
		.format_measure_cfg1_addr         = 0x008C,
		.format_measure_cfg2_addr         = 0x0090,
		.format_measure0_addr             = 0x0094,
		.format_measure1_addr             = 0x0098,
		.format_measure2_addr             = 0x009C,
		.timestamp_curr0_sof_addr         = 0x00A0,
		.timestamp_curr1_sof_addr         = 0x00A4,
		.timestamp_perv0_sof_addr         = 0x00A8,
		.timestamp_perv1_sof_addr         = 0x00AC,
		.timestamp_curr0_eof_addr         = 0x00B0,
		.timestamp_curr1_eof_addr         = 0x00B4,
		.timestamp_perv0_eof_addr         = 0x00B8,
		.timestamp_perv1_eof_addr         = 0x00BC,
		.secure_mask_cfg0                 = 0,
		.path_batch_status                = 0x0100,
		.path_frame_id                    = 0x0104,
		.cfg2_addr                        = 0x0108,

		/* configurations */
		.resume_frame_boundary            = 1,
		.start_mode_internal              = 0x0,
		.start_mode_global                = 0x1,
		.start_mode_shift                 = 2,
		.overflow_ctrl_mode_val           = 0x8,
		.packing_fmt_shift_val            = 15,
		.plain_alignment_shift_val        = 11,
		.plain_fmt_shift_val              = 12,
		.crop_v_en_shift_val              = 8,
		.crop_h_en_shift_val              = 7,
		.drop_v_en_shift_val              = 6,
		.drop_h_en_shift_val              = 5,
		.early_eof_en_shift_val           = 14,
		.format_measure_en_shift_val      = 4,
		.timestamp_en_shift_val           = 6,
		.debug_byte_cntr_rst_shift_val    = 7,
		.offline_mode_en_shift_val        = 2,
		.ccif_violation_en                = 1,
		.fatal_err_mask                   = 0x241c6001,
		.non_fatal_err_mask               = 0x12000000,
		.sof_irq_mask                     = 0x10,
		.rup_irq_mask                     = 0x800000,
		.epoch0_irq_mask                  = 0x200000,
		.epoch1_irq_mask                  = 0x400000,
		.eof_irq_mask                     = 0x8,
		.epoch0_shift_val                 = 16,
		.epoch1_shift_val                 = 0,
		.pix_store_en_shift_val           = 0,
		.sof_retiming_dis_shift           = 5,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_rdi_0_reg_info = {
		.irq_status_addr                  = 0x0114,
		.irq_mask_addr                    = 0x0118,
		.irq_clear_addr                   = 0x011C,
		.irq_set_addr                     = 0x0120,
		.base                             = 0x0E00,

		/* configurations */
		.capabilities                     = CAM_IFE_CSID_CAP_INPUT_LCR |
							CAM_IFE_CSID_CAP_RDI_UNPACK_MSB |
							CAM_IFE_CSID_CAP_LINE_SMOOTHING_IN_RDI |
							CAM_IFE_CSID_CAP_SOF_RETIME_DIS,
		.rup_mask                         = 0x100,
		.aup_mask                         = 0x100,
		.rup_aup_set_mask                 = 0x1,
		.top_irq_mask                     = {0x10000,},
		.default_out_format               = CAM_FORMAT_PLAIN16_16,
		.disable_sof_retime_default       = true,
		.mipi_pack_supported              = 1,
		.offline_mode_supported           = 1,
		.use_master_slave_default         = true,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_rdi_1_reg_info = {
		.irq_status_addr                  = 0x0124,
		.irq_mask_addr                    = 0x0128,
		.irq_clear_addr                   = 0x012C,
		.irq_set_addr                     = 0x0130,
		.base                             = 0x1000,

		/* configurations */
		.capabilities                     = CAM_IFE_CSID_CAP_INPUT_LCR |
							CAM_IFE_CSID_CAP_RDI_UNPACK_MSB |
							CAM_IFE_CSID_CAP_LINE_SMOOTHING_IN_RDI |
							CAM_IFE_CSID_CAP_SOF_RETIME_DIS,
		.rup_mask                         = 0x200,
		.aup_mask                         = 0x200,
		.rup_aup_set_mask                 = 0x1,
		.top_irq_mask                     = {0x20000,},
		.mipi_pack_supported              = 1,
		.offline_mode_supported           = 1,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_rdi_2_reg_info = {
		.irq_status_addr                  = 0x0134,
		.irq_mask_addr                    = 0x0138,
		.irq_clear_addr                   = 0x013C,
		.irq_set_addr                     = 0x0140,
		.base                             = 0x1200,
		/* configurations */
		.resume_frame_boundary            = 1,
		.overflow_ctrl_en                 = 0,
		.capabilities                     = CAM_IFE_CSID_CAP_INPUT_LCR |
							CAM_IFE_CSID_CAP_RDI_UNPACK_MSB |
							CAM_IFE_CSID_CAP_LINE_SMOOTHING_IN_RDI |
							CAM_IFE_CSID_CAP_SOF_RETIME_DIS,
		.rup_mask                         = 0x400,
		.aup_mask                         = 0x400,
		.rup_aup_set_mask                 = 0x1,
		.top_irq_mask                     = {0x40000,},
		.mipi_pack_supported              = 1,
		.offline_mode_supported           = 1,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_rdi_3_reg_info = {
		.irq_status_addr                  = 0x0144,
		.irq_mask_addr                    = 0x0148,
		.irq_clear_addr                   = 0x014C,
		.irq_set_addr                     = 0x0150,
		.base                             = 0x1400,
		/* configurations */
		.capabilities                     = CAM_IFE_CSID_CAP_INPUT_LCR |
							CAM_IFE_CSID_CAP_RDI_UNPACK_MSB |
							CAM_IFE_CSID_CAP_LINE_SMOOTHING_IN_RDI |
							CAM_IFE_CSID_CAP_SOF_RETIME_DIS,
		.rup_mask                         = 0x800,
		.aup_mask                         = 0x800,
		.rup_aup_set_mask                 = 0x1,
		.top_irq_mask                     = {0x80000,},
		.mipi_pack_supported              = 1,
		.offline_mode_supported           = 1,
};

static struct cam_ife_csid_ver2_path_reg_info
	cam_ife_csid_980_rdi_4_reg_info = {
		.irq_status_addr                  = 0x0154,
		.irq_mask_addr                    = 0x0158,
		.irq_clear_addr                   = 0x015C,
		.irq_set_addr                     = 0x0160,
		.base                             = 0x1600,
		/* configurations */
		.capabilities                     = CAM_IFE_CSID_CAP_INPUT_LCR |
							CAM_IFE_CSID_CAP_RDI_UNPACK_MSB |
							CAM_IFE_CSID_CAP_LINE_SMOOTHING_IN_RDI |
							CAM_IFE_CSID_CAP_SOF_RETIME_DIS,
		.rup_mask                         = 0x1000,
		.aup_mask                         = 0x1000,
		.rup_aup_set_mask                 = 0x1,
		.top_irq_mask                     = {0x100000,},
		.mipi_pack_supported              = 1,
		.offline_mode_supported           = 1,
};

static struct cam_ife_csid_rx_debug_mask cam_ife_csid_980_rx_debug_mask = {

	.evt_bitmap = {
		BIT_ULL(CAM_IFE_CSID_RX_DL0_EOT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_DL1_EOT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_DL2_EOT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_DL3_EOT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_DL0_SOT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_DL1_SOT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_DL2_SOT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_DL3_SOT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_WARNING_ECC) |
			BIT_ULL(CAM_IFE_CSID_RX_ERROR_CPHY_PH_CRC) |
			BIT_ULL(CAM_IFE_CSID_RX_LANE0_FIFO_OVERFLOW) |
			BIT_ULL(CAM_IFE_CSID_RX_LANE1_FIFO_OVERFLOW) |
			BIT_ULL(CAM_IFE_CSID_RX_LANE2_FIFO_OVERFLOW) |
			BIT_ULL(CAM_IFE_CSID_RX_LANE3_FIFO_OVERFLOW) |
			BIT_ULL(CAM_IFE_CSID_RX_ERROR_CRC) |
			BIT_ULL(CAM_IFE_CSID_RX_ERROR_ECC) |
			BIT_ULL(CAM_IFE_CSID_RX_MMAPPED_VC_DT) |
			BIT_ULL(CAM_IFE_CSID_RX_UNMAPPED_VC_DT) |
			BIT_ULL(CAM_IFE_CSID_RX_STREAM_UNDERFLOW) |
			BIT_ULL(CAM_IFE_CSID_RX_RX2_IRQ) |
			BIT_ULL(CAM_IFE_CSID_RX_DL0_EOT_LOST) |
			BIT_ULL(CAM_IFE_CSID_RX_DL1_EOT_LOST) |
			BIT_ULL(CAM_IFE_CSID_RX_DL2_EOT_LOST) |
			BIT_ULL(CAM_IFE_CSID_RX_DL3_EOT_LOST) |
			BIT_ULL(CAM_IFE_CSID_RX_DL0_SOT_LOST) |
			BIT_ULL(CAM_IFE_CSID_RX_DL1_SOT_LOST) |
			BIT_ULL(CAM_IFE_CSID_RX_DL2_SOT_LOST) |
			BIT_ULL(CAM_IFE_CSID_RX_DL3_SOT_LOST),

		BIT_ULL(CAM_IFE_CSID_RX_LONG_PKT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_SHORT_PKT_CAPTURED) |
			BIT_ULL(CAM_IFE_CSID_RX_CPHY_PKT_HDR_CAPTURED),
	},


	.bit_pos[CAM_IFE_CSID_RX_DL0_EOT_CAPTURED] = 0,
	.bit_pos[CAM_IFE_CSID_RX_DL1_EOT_CAPTURED] = 1,
	.bit_pos[CAM_IFE_CSID_RX_DL2_EOT_CAPTURED] = 2,
	.bit_pos[CAM_IFE_CSID_RX_DL3_EOT_CAPTURED] = 3,
	.bit_pos[CAM_IFE_CSID_RX_DL0_SOT_CAPTURED] = 4,
	.bit_pos[CAM_IFE_CSID_RX_DL1_SOT_CAPTURED] = 5,
	.bit_pos[CAM_IFE_CSID_RX_DL2_SOT_CAPTURED] = 6,
	.bit_pos[CAM_IFE_CSID_RX_DL3_SOT_CAPTURED] = 7,
	.bit_pos[CAM_IFE_CSID_RX_WARNING_ECC] = 8,
	.bit_pos[CAM_IFE_CSID_RX_LONG_PKT_CAPTURED] = 0,
	.bit_pos[CAM_IFE_CSID_RX_SHORT_PKT_CAPTURED] = 1,
	.bit_pos[CAM_IFE_CSID_RX_CPHY_PKT_HDR_CAPTURED] = 2,
	.bit_pos[CAM_IFE_CSID_RX_ERROR_CPHY_PH_CRC] = 24,
	.bit_pos[CAM_IFE_CSID_RX_LANE0_FIFO_OVERFLOW] = 20,
	.bit_pos[CAM_IFE_CSID_RX_LANE1_FIFO_OVERFLOW] = 21,
	.bit_pos[CAM_IFE_CSID_RX_LANE2_FIFO_OVERFLOW] = 22,
	.bit_pos[CAM_IFE_CSID_RX_LANE3_FIFO_OVERFLOW] = 23,
	.bit_pos[CAM_IFE_CSID_RX_ERROR_CRC] = 25,
	.bit_pos[CAM_IFE_CSID_RX_ERROR_ECC] = 26,
	.bit_pos[CAM_IFE_CSID_RX_MMAPPED_VC_DT] = 27,
	.bit_pos[CAM_IFE_CSID_RX_UNMAPPED_VC_DT] = 28,
	.bit_pos[CAM_IFE_CSID_RX_STREAM_UNDERFLOW] = 29,
	.bit_pos[CAM_IFE_CSID_RX_RX2_IRQ] = 31,
	.bit_pos[CAM_IFE_CSID_RX_DL0_EOT_LOST] = 12,
	.bit_pos[CAM_IFE_CSID_RX_DL1_EOT_LOST] = 13,
	.bit_pos[CAM_IFE_CSID_RX_DL2_EOT_LOST] = 14,
	.bit_pos[CAM_IFE_CSID_RX_DL3_EOT_LOST] = 15,
	.bit_pos[CAM_IFE_CSID_RX_DL0_SOT_LOST] = 16,
	.bit_pos[CAM_IFE_CSID_RX_DL1_SOT_LOST] = 17,
	.bit_pos[CAM_IFE_CSID_RX_DL2_SOT_LOST] = 18,
	.bit_pos[CAM_IFE_CSID_RX_DL3_SOT_LOST] = 19,
};

static struct cam_ife_csid_top_debug_mask cam_ife_csid_980_top_debug_mask = {

	.evt_bitmap = {
		0ULL,

		BIT_ULL(CAM_IFE_CSID_TOP_INFO_VOTE_UP) |
			BIT_ULL(CAM_IFE_CSID_TOP_INFO_VOTE_DN) |
			BIT_ULL(CAM_IFE_CSID_TOP_ERR_NO_VOTE_DN),
	},

	.bit_pos[CAM_IFE_CSID_TOP_INFO_VOTE_UP] = 0,
	.bit_pos[CAM_IFE_CSID_TOP_INFO_VOTE_DN] = 1,
	.bit_pos[CAM_IFE_CSID_TOP_ERR_NO_VOTE_DN] = 2,
};


static struct cam_ife_csid_ver2_csi2_rx_reg_info
	cam_ife_csid_980_csi2_reg_info = {
		.irq_status_addr                 = {0x00B4, 0x00C4},
		.irq_mask_addr                   = {0x00B8, 0x00C8},
		.irq_clear_addr                  = {0x00BC, 0x00CC},
		.irq_set_addr                    = {0x00C0, 0x00D0},
		/*CSI2 rx control */
		.cfg0_addr                       = 0x0400,
		.cfg1_addr                       = 0x0404,
		.capture_ctrl_addr               = 0x0408,
		.rst_strobes_addr                = 0x040C,
		.cap_unmap_long_pkt_hdr_0_addr   = 0x0410,
		.cap_unmap_long_pkt_hdr_1_addr   = 0x0414,
		.captured_short_pkt_0_addr       = 0x0418,
		.captured_short_pkt_1_addr       = 0x041c,
		.captured_long_pkt_0_addr        = 0x0420,
		.captured_long_pkt_1_addr        = 0x0424,
		.captured_long_pkt_ftr_addr      = 0x0428,
		.captured_cphy_pkt_hdr_addr      = 0x042c,
		.lane0_misr_addr                 = 0x0430,
		.lane1_misr_addr                 = 0x0434,
		.lane2_misr_addr                 = 0x0438,
		.lane3_misr_addr                 = 0x043c,
		.total_pkts_rcvd_addr            = 0x0440,
		.stats_ecc_addr                  = 0x0444,
		.total_crc_err_addr              = 0x0448,
		.de_scramble_type3_cfg0_addr     = 0x044C,
		.de_scramble_type3_cfg1_addr     = 0x0450,
		.de_scramble_type2_cfg0_addr     = 0x0454,
		.de_scramble_type2_cfg1_addr     = 0x0458,
		.de_scramble_type1_cfg0_addr     = 0x045C,
		.de_scramble_type1_cfg1_addr     = 0x0460,
		.de_scramble_type0_cfg0_addr     = 0x0464,
		.de_scramble_type0_cfg1_addr     = 0x0468,

		.rst_done_shift_val              = 27,
		.irq_mask_all                    = 0xFFFFFFF,
		.misr_enable_shift_val           = 6,
		.vc_mode_shift_val               = 2,
		.capture_long_pkt_en_shift       = 0,
		.capture_short_pkt_en_shift      = 1,
		.capture_cphy_pkt_en_shift       = 2,
		.capture_long_pkt_dt_shift       = 4,
		.capture_long_pkt_vc_shift       = 10,
		.capture_short_pkt_vc_shift      = 15,
		.capture_cphy_pkt_dt_shift       = 20,
		.capture_cphy_pkt_vc_shift       = 26,
		.phy_num_mask                    = 0xf,
		.vc_mask                         = 0x7C00000,
		.dt_mask                         = 0x3f0000,
		.wc_mask                         = 0xffff,
		.vc_shift                        = 0x16,
		.dt_shift                        = 0x10,
		.wc_shift                        = 0,
		.calc_crc_mask                   = 0xffff0000,
		.expected_crc_mask               = 0xffff,
		.calc_crc_shift                  = 0x10,
		.ecc_correction_shift_en         = 0,
		.lane_num_shift                  = 0,
		.lane_cfg_shift                  = 4,
		.phy_type_shift                  = 24,
		.phy_num_shift                   = 20,
		.tpg_mux_en_shift                = 27,
		.tpg_num_sel_shift               = 28,
		.phy_bist_shift_en               = 7,
		.epd_mode_shift_en               = 8,
		.eotp_shift_en                   = 9,
		.dyn_sensor_switch_shift_en      = 10,
		.rup_aup_latch_shift             = 13,
		.rup_aup_latch_supported         = true,
		.long_pkt_strobe_rst_shift       = 0,
		.short_pkt_strobe_rst_shift      = 1,
		.cphy_pkt_strobe_rst_shift       = 2,
		.unmapped_pkt_strobe_rst_shift   = 3,
		.fatal_err_mask                  = {0x25fff000, 0x0},
		.part_fatal_err_mask             = {0x02000000, 0x0},
		.non_fatal_err_mask              = {0x08000000, 0x0},
		.top_irq_mask                    = {0x4, 0x0},
		.rx_rx2_irq_mask                 = 0x80000000,
};

static struct cam_ife_csid_ver2_common_reg_info
			cam_ife_csid_980_cmn_reg_info = {
	.hw_version_addr                         = 0x0000,
	.cfg0_addr                               = 0x0004,
	.global_cmd_addr                         = 0x0008,
	.reset_cfg_addr                          = 0x000C,
	.reset_cmd_addr                          = 0x0010,
	.irq_cmd_addr                            = 0x0014,
	.rup_cmd_addr                            = 0x0018,
	.aup_cmd_addr                            = 0x001C,
	.rup_aup_cmd_addr                        = 0x0020,
	.offline_cmd_addr                        = 0x0024,
	.shdr_master_slave_cfg_addr              = 0x0028,
	.multi_sensor_mode_addr                  = 0x002C,
	.top_irq_status_addr                     = {0x0084, 0x0094},
	.top_irq_mask_addr                       = {0x0088, 0x0098},
	.top_irq_clear_addr                      = {0x008C, 0x009C},
	.top_irq_set_addr                        = {0x0090, 0x00A0},
	.buf_done_irq_status_addr                = 0x00A4,
	.buf_done_irq_mask_addr                  = 0x00A8,
	.buf_done_irq_clear_addr                 = 0x00AC,
	.buf_done_irq_set_addr                   = 0x00B0,
	.test_bus_ctrl                           = 0x03F4,
	.test_bus_debug                          = 0x03F8,
	.drv_cfg0_addr                           = 0x0164,
	.drv_cfg1_addr                           = 0x0168,
	.drv_cfg2_addr                           = 0x016C,
	.debug_drv_0_addr                        = 0x0170,
	.debug_drv_1_addr                        = 0x0174,
	.debug_sensor_hbi_irq_vcdt_addr          = 0x0180,
	.debug_violation_addr                    = 0x03D4,
	.debug_cfg_addr                          = 0x03E0,
	.rx_mode_id_cfg1_addr                    = 0x0470,
	.perf_cnt_reg = {
		{
			.perf_cnt_cfg0           = 0x0184,
			.perf_cnt_cfg1           = 0x0188,
			.perf_cnt_val            = 0x019C,
			.perf_cnt_status         = 0x01A0,
		},
		{
			.perf_cnt_cfg0           = 0x018C,
			.perf_cnt_cfg1           = 0x0190,
			.perf_cnt_val            = 0x01A4,
			.perf_cnt_status         = 0x01A8,
		},
		{
			.perf_cnt_cfg0           = 0x0194,
			.perf_cnt_cfg1           = 0x0198,
			.perf_cnt_val            = 0x01AC,
			.perf_cnt_status         = 0x01B0,
		},
	},

	/*configurations */
	.major_version                           = 6,
	.minor_version                           = 8,
	.version_incr                            = 0,
	.num_rdis                                = 5,
	.num_pix                                 = 3,
	.num_ppp                                 = 1,
	.rst_done_shift_val                      = 1,
	.path_en_shift_val                       = 31,
	.dt_id_shift_val                         = 27,
	.vc_shift_val                            = 22,
	.vc_mask                                 = 0x1F,
	.dt_shift_val                            = 16,
	.dt_mask                                 = 0x3F,
	.crop_shift_val                          = 16,
	.decode_format_shift_val                 = 12,
	.decode_format1_shift_val                = 16,
	.decode_format1_supported                = true,
	.decode_format_mask                      = 0xF,
	.frame_id_decode_en_shift_val            = 1,
	.multi_vcdt_vc1_shift_val                = 2,
	.multi_vcdt_dt1_shift_val                = 7,
	.multi_vcdt_ts_combo_en_shift_val        = 13,
	.multi_vcdt_en_shift_val                 = 0,
	.timestamp_stb_sel_shift_val             = 8,
	.vfr_en_shift_val                        = 0,
	.mup_shift_val                           = 4,
	.shdr_slave_ppp_shift                    = 16,
	.shdr_slave_rdi2_shift                   = 18,
	.shdr_slave_rdi1_shift                   = 17,
	.shdr_master_rdi0_shift                  = 4,
	.shdr_master_slave_en_shift              = 0,
	.drv_en_shift                            = 0,
	.drv_rup_en_shift                        = 0,
	.early_eof_supported                     = 1,
	.vfr_supported                           = 1,
	.multi_vcdt_supported                    = 1,
	.ts_comb_vcdt_en                         = true,
	.ts_comb_vcdt_mask                       = 3,
	.frame_id_dec_supported                  = 1,
	.measure_en_hbi_vbi_cnt_mask             = 0xc,
	.measure_pixel_line_en_mask              = 0x3,
	.crop_pix_start_mask                     = 0x3fff,
	.crop_pix_end_mask                       = 0xffff,
	.crop_line_start_mask                    = 0x3fff,
	.crop_line_end_mask                      = 0xffff,
	.drop_supported                          = 1,
	.ipp_irq_mask_all                        = 0x7FFF,
	.rdi_irq_mask_all                        = 0x7FFF,
	.ppp_irq_mask_all                        = 0xFFFF,
	.top_err_irq_mask                        = {0x00000002, 0x18},
	.rst_loc_path_only_val                   = 0x0,
	.rst_loc_complete_csid_val               = 0x1,
	.rst_mode_frame_boundary_val             = 0x0,
	.rst_mode_immediate_val                  = 0x1,
	.rst_cmd_hw_reset_complete_val           = 0x1,
	.rst_cmd_sw_reset_complete_val           = 0x2,
	.rst_cmd_irq_ctrl_only_val               = 0x4,
	.timestamp_strobe_val                    = 0x2,
	.top_reset_irq_mask                      = {0x1,},
	.rst_location_shift_val                  = 4,
	.rst_mode_shift_val                      = 0,
	.epoch_factor                            = 50,
	.global_reset                            = 1,
	.aup_rup_supported                       = 1,
	.only_master_rup                         = 1,
	.format_measure_height_mask_val          = 0xFFFF,
	.format_measure_height_shift_val         = 0x10,
	.format_measure_width_mask_val           = 0xFFFF,
	.format_measure_width_shift_val          = 0x0,
	.format_measure_max_hbi_shift            = 16,
	.format_measure_min_hbi_mask             = 0xFFF,
	.top_buf_done_irq_mask                   = 0x8,
	.decode_format_payload_only              = 0xF,
	.timestamp_enabled_in_cfg0               = true,
	.camif_irq_support                       = true,
	.capabilities                            = CAM_IFE_CSID_CAP_SPLIT_RUP_AUP |
							CAM_IFE_CSID_CAP_SKIP_PATH_CFG1 |
							CAM_IFE_CSID_CAP_SKIP_EPOCH_CFG|
							CAM_IFE_CSID_CAP_MULTI_CTXT,
	.top_top2_irq_mask                       = 0x80000000,
	.drv_rup_en_val_map = {
		2, /*RDI0 */
		3, /*RDI1 */
		4, /*RDI2 */
		5, /*RDI3 */
		6, /*RDI4 */
		0, /*IPP */
		1, /*PPP */
		0, /*UDI0 */
		0, /*UDI1 */
		0, /*UDI2 */
	},
	.drv_path_idle_en_val_map = {
		BIT(4), /*CAM_ISP_PXL_PATH */
		BIT(5), /*CAM_ISP_PPP_PATH */
		0,      /* LCR not applicable */
		BIT(6), /*CAM_ISP_RDI0_PATH */
		BIT(7), /*CAM_ISP_RDI1_PATH */
		BIT(8), /*CAM_ISP_RDI2_PATH */
		BIT(9), /*CAM_ISP_RDI3_PATH */
		BIT(10), /*CAM_ISP_RDI4_PATH */
	},
	.path_domain_id_cfg0                     = 0x0,
	.path_domain_id_cfg1                     = 0x4,
	.path_domain_id_cfg2                     = 0x8,
	.phy_sel_base_idx                        = 1,
	.num_perf_cntrs                          = 3,
};

struct cam_ife_csid_ver2_mc_reg_info
	cam_ife_csid_980_ipp_mc_reg_info = {
	.irq_comp_cfg0_addr                = 0x0178,
	.ipp_src_ctxt_mask_shift           = 4,
	.ipp_dst_ctxt_mask_shift           = 0,
	.comp_rup_mask                     = 0x4000000,
	.comp_epoch0_mask                  = 0x8000000,
	.comp_eof_mask                     = 0x20000000,
	.comp_sof_mask                     = 0x40000000,
	.comp_subgrp0_mask                 = 0x1000000,
	.comp_subgrp2_mask                 = 0x2000000,
};

static struct cam_ife_csid_ver2_reg_info cam_ife_csid_980_reg_info = {
	.top_irq_reg_info      = cam_ife_csid_980_top_irq_reg_info,
	.rx_irq_reg_info       = cam_ife_csid_980_rx_irq_reg_info,
	.path_irq_reg_info     = {
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_0],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_1],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_2],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_3],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_RDI_4],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_IPP],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_PPP],
		NULL,
		NULL,
		NULL,
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_IPP_1],
		&cam_ife_csid_980_path_irq_reg_info[CAM_IFE_PIX_PATH_RES_IPP_2],
	},
	.buf_done_irq_reg_info = &cam_ife_csid_980_buf_done_irq_reg_info,
	.cmn_reg                              = &cam_ife_csid_980_cmn_reg_info,
	.csi2_reg                             = &cam_ife_csid_980_csi2_reg_info,
	.ipp_reg_info                         = &cam_ife_csid_980_ipp_cmn_reg,
	.rdi_reg_info                         = &cam_ife_csid_980_rdi_cmn_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_IPP]   = &cam_ife_csid_980_ipp_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_IPP_1] = &cam_ife_csid_980_ipp_1_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_IPP_2] = &cam_ife_csid_980_ipp_2_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_PPP]   = &cam_ife_csid_980_ppp_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_0] = &cam_ife_csid_980_rdi_0_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_1] = &cam_ife_csid_980_rdi_1_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_2] = &cam_ife_csid_980_rdi_2_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_3] = &cam_ife_csid_980_rdi_3_reg_info,
	.path_reg[CAM_IFE_PIX_PATH_RES_RDI_4] = &cam_ife_csid_980_rdi_4_reg_info,
	.ipp_mc_reg                           = &cam_ife_csid_980_ipp_mc_reg_info,
	.need_top_cfg = 0x0,
	.dynamic_drv_supported = true,
	.support_dyn_offset  = true,
	.top_irq_desc        = &cam_ife_csid_980_top_irq_desc,
	.rx_irq_desc         = &cam_ife_csid_980_rx_irq_desc,
	.path_irq_desc       = cam_ife_csid_980_path_irq_desc,
	.num_top_err_irqs    = cam_ife_csid_980_num_top_irq_desc,
	.num_rx_err_irqs     = cam_ife_csid_980_num_rx_irq_desc,
	.num_path_err_irqs   = ARRAY_SIZE(cam_ife_csid_980_path_irq_desc),
	.top_debug_mask      = &cam_ife_csid_980_top_debug_mask,
	.rx_debug_mask       = &cam_ife_csid_980_rx_debug_mask,
	.num_top_regs        = 2,
	.num_rx_regs         = 2,
};
#endif /*_CAM_IFE_CSID_980_H_ */
