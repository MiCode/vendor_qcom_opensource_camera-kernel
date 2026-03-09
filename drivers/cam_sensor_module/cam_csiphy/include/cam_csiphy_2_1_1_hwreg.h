/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023, 2025, Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _CAM_CSIPHY_2_1_1_HWREG_H_
#define _CAM_CSIPHY_2_1_1_HWREG_H_

#include "../cam_csiphy_dev.h"

struct cam_cphy_dphy_status_reg_params_t status_regs_2_1_1 = {
	.csiphy_3ph_status0_offset = 0x0340,
	.csiphy_2ph_status0_offset = 0x00C0,
	.cphy_lane_status = {0x0358, 0x0758, 0x0B58},
	.csiphy_3ph_status_size = 24,
	.csiphy_2ph_status_size = 20,
};

struct csiphy_reg_t csiphy_lane_en_reg_2_1_1[] = {
	{0x1014, 0x00, 0x00, CSIPHY_LANE_ENABLE},
};

struct csiphy_reg_t csiphy_common_reg_2_1_1[] = {
	{0x1084, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x1018, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x101C, 0x7A, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t csiphy_reset_enter_reg_2_1_1[] = {
	{0x1000, 0x01, 0x01, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t csiphy_reset_exit_reg_2_1_1[] = {
	{0x1000, 0x02, 0x00, CSIPHY_2PH_REGS},
	{0x1000, 0x00, 0x00, CSIPHY_2PH_COMBO_REGS},
	{0x1000, 0x0E, 0xBE8, CSIPHY_3PH_REGS},
};

struct csiphy_reg_t csiphy_irq_reg_2_1_1[] = {
	{0x102c, 0xff, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x1030, 0xff, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x1034, 0xfb, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x1038, 0xff, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x103c, 0x7f, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x1040, 0xff, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x1044, 0xff, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x1048, 0xef, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x104c, 0xff, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x1050, 0xff, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x1054, 0xff, 0x64, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t csiphy_2ph_v2_1_1_reg[] = {
	{0x0094, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00A0, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x0f, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0098, 0x08, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0094, 0x07, 0x01, CSIPHY_DEFAULT_PARAMS},
	{0x0030, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0028, 0x0A, 0x00, CSIPHY_2PH_SEC_CLK_LN_SETTINGS},
	{0x0000, 0x8E, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0038, 0xFE, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x002C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0034, 0x0F, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x001C, 0x0A, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x60, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x003C, 0xB8, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0004, 0x0C, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0020, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0008, 0x10, 0x00, CSIPHY_SETTLE_CNT_LOWER_BYTE},
	{0x0010, 0x52, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0094, 0xD7, 0x00, CSIPHY_SKEW_CAL},
	{0x005C, 0x00, 0x00, CSIPHY_SKEW_CAL},
	{0x0060, 0xBD, 0x00, CSIPHY_SKEW_CAL},
	{0x0064, 0x7F, 0x00, CSIPHY_SKEW_CAL},
};

struct csiphy_reg_t csiphy_2ph_v2_1_1_clk_ln_reg[] = {
	{0x0028, 0x00, 0x00, CSIPHY_2PH_SEC_CLK_LN_SETTINGS},
	{0x0000, 0x80, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x000C, 0xFF, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0038, 0x1F, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t csiphy_3ph_v2_1_1_reg[] = {
	{0x0068, 0xF1, 0x64, CSIPHY_DEFAULT_PARAMS},
	{0x0094, 0x08, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00F4, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00F8, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00FC, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00F0, 0x00, 0x02, CSIPHY_DEFAULT_PARAMS},
	{0x00F0, 0xEF, 0x64, CSIPHY_DEFAULT_PARAMS},
	{0x0094, 0x09, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0004, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x000C, 0x10, 0x00, CSIPHY_SETTLE_CNT_LOWER_BYTE},
	{0x0008, 0x00, 0x00, CSIPHY_SETTLE_CNT_HIGHER_BYTE},
	{0x0010, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00E4, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00E8, 0x7F, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00EC, 0x7F, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0018, 0x3E, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x001C, 0x41, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0020, 0x41, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0024, 0x7F, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0028, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x002C, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0064, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0044, 0xB2, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0110, 0x35, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00BC, 0xD0, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0070, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0030, 0x94, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0034, 0x31, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0038, 0x60, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x003C, 0xA6, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0058, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0054, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x005C, 0x04, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0048, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x004C, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0040, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0060, 0xA8, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t bist_3ph_arr_2_1_1[] = {
	/* 3Phase BIST CONFIGURATION REG SET */
	{0x00D4, 0x64, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x00D8, 0x3E, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0050, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0044, 0xB1, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0040, 0x85, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t bist_status_arr_2_1_1[] = {
	{0x0344, 0x00, 0x00, CSIPHY_3PH_REGS},
	{0x0744, 0x00, 0x00, CSIPHY_3PH_REGS},
	{0x0B44, 0x00, 0x00, CSIPHY_3PH_REGS},
	{0x00C0, 0x00, 0x00, CSIPHY_2PH_REGS},
	{0x04C0, 0x00, 0x00, CSIPHY_2PH_REGS},
	{0x08C0, 0x00, 0x00, CSIPHY_2PH_REGS},
	{0x0CC0, 0x00, 0x00, CSIPHY_2PH_REGS},
};

struct bist_reg_settings_t bist_setting_2_1_1 = {
	.error_status_val_3ph = 0x10,
	.error_status_val_2ph = 0x10,
	.set_status_update_3ph_base_offset = 0x0240,
	.set_status_update_2ph_base_offset = 0x0050,
	.bist_status_3ph_base_offset = 0x0344,
	.bist_status_2ph_base_offset = 0x00C0,
	.bist_sensor_data_3ph_status_base_offset = 0x0340,
	.bist_counter_3ph_base_offset = 0x0348,
	.bist_counter_2ph_base_offset = 0x00C8,
	.number_of_counters = 2,
	.num_3ph_bist_settings = ARRAY_SIZE(bist_3ph_arr_2_1_1),
	.bist_3ph_settings_arry = bist_3ph_arr_2_1_1,
	.bist_2ph_settings_arry = NULL,
	.num_2ph_bist_settings = 0,
	.num_status_reg = ARRAY_SIZE(bist_status_arr_2_1_1),
	.bist_status_arr = bist_status_arr_2_1_1,
};

struct csiphy_reg_t datarate_211_1p2Gsps_cmn_ctrl[] = {
	{0x108C, 0x0C, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_1p5Gsps_5Gsps_cmn_ctrl[] = {
	{0x108C, 0x03, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_1p2Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x70, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x11, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x03, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_1p5Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x4D, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x03, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x09, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_1p7Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x43, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x03, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x06, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_2p1Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x32, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x03, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_2p35Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x2E, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x03, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_2p6Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x28, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x03, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_2p8Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x22, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x02, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_3p3Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x1D, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x02, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_3p5Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x15, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x02, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_4Gsps[] = {
	{0x0074, 0x07, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x12, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0x01, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x10, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x02, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_4p5Gsps[] = {
	{0x0074, 0x05, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x0C, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x04, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0xC1, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x18, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x02, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
};

struct csiphy_reg_t datarate_211_5Gsps[] = {
	{0x0074, 0x05, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0078, 0x0A, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0088, 0x04, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x008C, 0xC1, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x006C, 0x18, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0090, 0x02, 0x00, CSIPHY_DEFAULT_PARAMS},
	{0x0014, 0x00, 0x00, CSIPHY_DEFAULT_PARAMS},
};

static struct data_rate_reg_info_t data_rate_settings_2_1_1[] = {
	{
		/* ((1.2 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value*/
		.bandwidth = 2736000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_1p2Gsps),
		.data_rate_reg_array[0][0] = datarate_211_1p2Gsps,
		.data_rate_reg_array[1][0] = datarate_211_1p2Gsps,
		.data_rate_reg_array[2][0] = datarate_211_1p2Gsps,
		.data_rate_reg_array[3][0] = datarate_211_1p2Gsps,
		.data_rate_reg_array[4][0] = datarate_211_1p2Gsps,
		.data_rate_reg_array[5][0] = datarate_211_1p2Gsps,
		.data_rate_reg_array[6][0] = datarate_211_1p2Gsps,
		.data_rate_reg_array[7][0] = datarate_211_1p2Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p2Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p2Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p2Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p2Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p2Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p2Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p2Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p2Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p2Gsps_cmn_ctrl,
	},
	{
		/* ((1.5 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value*/
		.bandwidth = 3420000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps),
		.data_rate_reg_array[0][0] = datarate_211_1p5Gsps,
		.data_rate_reg_array[1][0] = datarate_211_1p5Gsps,
		.data_rate_reg_array[2][0] = datarate_211_1p5Gsps,
		.data_rate_reg_array[3][0] = datarate_211_1p5Gsps,
		.data_rate_reg_array[4][0] = datarate_211_1p5Gsps,
		.data_rate_reg_array[5][0] = datarate_211_1p5Gsps,
		.data_rate_reg_array[6][0] = datarate_211_1p5Gsps,
		.data_rate_reg_array[7][0] = datarate_211_1p5Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((1.7 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value*/
		.bandwidth = 3876000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_1p7Gsps),
		.data_rate_reg_array[0][0] = datarate_211_1p7Gsps,
		.data_rate_reg_array[1][0] = datarate_211_1p7Gsps,
		.data_rate_reg_array[2][0] = datarate_211_1p7Gsps,
		.data_rate_reg_array[3][0] = datarate_211_1p7Gsps,
		.data_rate_reg_array[4][0] = datarate_211_1p7Gsps,
		.data_rate_reg_array[5][0] = datarate_211_1p7Gsps,
		.data_rate_reg_array[6][0] = datarate_211_1p7Gsps,
		.data_rate_reg_array[7][0] = datarate_211_1p7Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((2.1 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value*/
		.bandwidth = 4788000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_2p1Gsps),
		.data_rate_reg_array[0][0] = datarate_211_2p1Gsps,
		.data_rate_reg_array[1][0] = datarate_211_2p1Gsps,
		.data_rate_reg_array[2][0] = datarate_211_2p1Gsps,
		.data_rate_reg_array[3][0] = datarate_211_2p1Gsps,
		.data_rate_reg_array[4][0] = datarate_211_2p1Gsps,
		.data_rate_reg_array[5][0] = datarate_211_2p1Gsps,
		.data_rate_reg_array[6][0] = datarate_211_2p1Gsps,
		.data_rate_reg_array[7][0] = datarate_211_2p1Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((2.35 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value*/
		.bandwidth = 5358000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_2p35Gsps),
		.data_rate_reg_array[0][0] = datarate_211_2p35Gsps,
		.data_rate_reg_array[1][0] = datarate_211_2p35Gsps,
		.data_rate_reg_array[2][0] = datarate_211_2p35Gsps,
		.data_rate_reg_array[3][0] = datarate_211_2p35Gsps,
		.data_rate_reg_array[4][0] = datarate_211_2p35Gsps,
		.data_rate_reg_array[5][0] = datarate_211_2p35Gsps,
		.data_rate_reg_array[6][0] = datarate_211_2p35Gsps,
		.data_rate_reg_array[7][0] = datarate_211_2p35Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((2.6 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value*/
		.bandwidth = 5928000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_2p6Gsps),
		.data_rate_reg_array[0][0] = datarate_211_2p6Gsps,
		.data_rate_reg_array[1][0] = datarate_211_2p6Gsps,
		.data_rate_reg_array[2][0] = datarate_211_2p6Gsps,
		.data_rate_reg_array[3][0] = datarate_211_2p6Gsps,
		.data_rate_reg_array[4][0] = datarate_211_2p6Gsps,
		.data_rate_reg_array[5][0] = datarate_211_2p6Gsps,
		.data_rate_reg_array[6][0] = datarate_211_2p6Gsps,
		.data_rate_reg_array[7][0] = datarate_211_2p6Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((2.8 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value */
		.bandwidth = 6384000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_2p8Gsps),
		.data_rate_reg_array[0][0] = datarate_211_2p8Gsps,
		.data_rate_reg_array[1][0] = datarate_211_2p8Gsps,
		.data_rate_reg_array[2][0] = datarate_211_2p8Gsps,
		.data_rate_reg_array[3][0] = datarate_211_2p8Gsps,
		.data_rate_reg_array[4][0] = datarate_211_2p8Gsps,
		.data_rate_reg_array[5][0] = datarate_211_2p8Gsps,
		.data_rate_reg_array[6][0] = datarate_211_2p8Gsps,
		.data_rate_reg_array[7][0] = datarate_211_2p8Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((3.3 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value */
		.bandwidth = 7524000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_3p3Gsps),
		.data_rate_reg_array[0][0] = datarate_211_3p3Gsps,
		.data_rate_reg_array[1][0] = datarate_211_3p3Gsps,
		.data_rate_reg_array[2][0] = datarate_211_3p3Gsps,
		.data_rate_reg_array[3][0] = datarate_211_3p3Gsps,
		.data_rate_reg_array[4][0] = datarate_211_3p3Gsps,
		.data_rate_reg_array[5][0] = datarate_211_3p3Gsps,
		.data_rate_reg_array[6][0] = datarate_211_3p3Gsps,
		.data_rate_reg_array[7][0] = datarate_211_3p3Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((3.5 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value */
		.bandwidth = 7980000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_3p5Gsps),
		.data_rate_reg_array[0][0] = datarate_211_3p5Gsps,
		.data_rate_reg_array[1][0] = datarate_211_3p5Gsps,
		.data_rate_reg_array[2][0] = datarate_211_3p5Gsps,
		.data_rate_reg_array[3][0] = datarate_211_3p5Gsps,
		.data_rate_reg_array[4][0] = datarate_211_3p5Gsps,
		.data_rate_reg_array[5][0] = datarate_211_3p5Gsps,
		.data_rate_reg_array[6][0] = datarate_211_3p5Gsps,
		.data_rate_reg_array[7][0] = datarate_211_3p5Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((4 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value */
		.bandwidth = 9120000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_4Gsps),
		.data_rate_reg_array[0][0] = datarate_211_4Gsps,
		.data_rate_reg_array[1][0] = datarate_211_4Gsps,
		.data_rate_reg_array[2][0] = datarate_211_4Gsps,
		.data_rate_reg_array[3][0] = datarate_211_4Gsps,
		.data_rate_reg_array[4][0] = datarate_211_4Gsps,
		.data_rate_reg_array[5][0] = datarate_211_4Gsps,
		.data_rate_reg_array[6][0] = datarate_211_4Gsps,
		.data_rate_reg_array[7][0] = datarate_211_4Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((4.5 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value */
		.bandwidth = 10260000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_4p5Gsps),
		.data_rate_reg_array[0][0] = datarate_211_4p5Gsps,
		.data_rate_reg_array[1][0] = datarate_211_4p5Gsps,
		.data_rate_reg_array[2][0] = datarate_211_4p5Gsps,
		.data_rate_reg_array[3][0] = datarate_211_4p5Gsps,
		.data_rate_reg_array[4][0] = datarate_211_4p5Gsps,
		.data_rate_reg_array[5][0] = datarate_211_4p5Gsps,
		.data_rate_reg_array[6][0] = datarate_211_4p5Gsps,
		.data_rate_reg_array[7][0] = datarate_211_4p5Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
	{
		/* ((5.0 GSpS) * (10^9) * (2.28 bits/symbol)) rounded value */
		.bandwidth = 11400000000,
		.data_rate_reg_array_size = ARRAY_SIZE(datarate_211_5Gsps),
		.data_rate_reg_array[0][0] = datarate_211_5Gsps,
		.data_rate_reg_array[1][0] = datarate_211_5Gsps,
		.data_rate_reg_array[2][0] = datarate_211_5Gsps,
		.data_rate_reg_array[3][0] = datarate_211_5Gsps,
		.data_rate_reg_array[4][0] = datarate_211_5Gsps,
		.data_rate_reg_array[5][0] = datarate_211_5Gsps,
		.data_rate_reg_array[6][0] = datarate_211_5Gsps,
		.data_rate_reg_array[7][0] = datarate_211_5Gsps,
		.common_ctrl_reg_array_size = ARRAY_SIZE(datarate_211_1p5Gsps_5Gsps_cmn_ctrl),
		.common_ctrl_reg_array[0][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[1][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[2][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[3][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[4][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[5][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[6][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
		.common_ctrl_reg_array[7][0] = datarate_211_1p5Gsps_5Gsps_cmn_ctrl,
	},
};

struct data_rate_settings_t data_rate_delta_table_2_1_1 = {
	.num_data_rate_settings = ARRAY_SIZE(data_rate_settings_2_1_1),
	.data_rate_settings = data_rate_settings_2_1_1,
};

struct csiphy_reg_parms_t csiphy_v2_1_1 = {
	.mipi_csiphy_interrupt_status0_addr = 0x10B0,
	.mipi_csiphy_interrupt_clear0_addr = 0x1058,
	.mipi_csiphy_glbl_irq_cmd_addr = 0x1028,
	.size_offset_betn_lanes = 0x400,
	.status_reg_params = &status_regs_2_1_1,
	.csiphy_common_reg_array_size = ARRAY_SIZE(csiphy_common_reg_2_1_1),
	.csiphy_reset_enter_array_size = ARRAY_SIZE(csiphy_reset_enter_reg_2_1_1),
	.csiphy_reset_exit_array_size = ARRAY_SIZE(csiphy_reset_exit_reg_2_1_1),
	.csiphy_2ph_config_array_size = ARRAY_SIZE(csiphy_2ph_v2_1_1_reg),
	.csiphy_2ph_clk_cfg_array_size = ARRAY_SIZE(csiphy_2ph_v2_1_1_clk_ln_reg),
	.csiphy_3ph_config_array_size = ARRAY_SIZE(csiphy_3ph_v2_1_1_reg),
	.csiphy_interrupt_status_size = ARRAY_SIZE(csiphy_irq_reg_2_1_1),
	.csiphy_num_common_status_regs = 20,
	.aon_sel_params = &aon_cam_select_params,
};

struct csiphy_ctrl_t ctrl_reg_2_1_1 = {
	.csiphy_common_reg = csiphy_common_reg_2_1_1,
	.csiphy_2ph_reg = csiphy_2ph_v2_1_1_reg,
	.csiphy_2ph_clk_ln_reg = csiphy_2ph_v2_1_1_clk_ln_reg,
	.csiphy_3ph_reg = csiphy_3ph_v2_1_1_reg,
	.csiphy_reg = &csiphy_v2_1_1,
	.csiphy_irq_reg = csiphy_irq_reg_2_1_1,
	.csiphy_reset_enter_regs = csiphy_reset_enter_reg_2_1_1,
	.csiphy_reset_exit_regs = csiphy_reset_exit_reg_2_1_1,
	.csiphy_lane_config_reg = csiphy_lane_en_reg_2_1_1,
	.csiphy_ln_offsets[DPHY_DATA_LANE_POS_0] = 0x0000,
	.csiphy_ln_offsets[DPHY_DATA_LANE_POS_1] = 0x0400,
	.csiphy_ln_offsets[DPHY_DATA_LANE_POS_2] = 0x0800,
	.csiphy_ln_offsets[DPHY_DATA_LANE_POS_3] = 0x0C00,
	.csiphy_ln_offsets[DPHY_CLOCK_LANE_POS] = 0x0E00,
	.csiphy_ln_offsets[CPHY_LANE_POS_0] = 0x0200,
	.csiphy_ln_offsets[CPHY_LANE_POS_1] = 0x0600,
	.csiphy_ln_offsets[CPHY_LANE_POS_2] = 0x0A00,
	.data_rates_settings_table = &data_rate_delta_table_2_1_1,
	.csiphy_bist_reg = &bist_setting_2_1_1,
	.getclockvoting = get_clk_voting_dynamic,
};

#endif /* _CAM_CSIPHY_2_1_1_HWREG_H_ */
