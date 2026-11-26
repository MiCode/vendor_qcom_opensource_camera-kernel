// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2025 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <linux/firmware.h>
#include "cam_sensor_cmn_header.h"
#include "cam_ois_core.h"
#include "cam_ois_soc.h"
#include "cam_sensor_util.h"
#include "cam_debug_util.h"
#include "cam_res_mgr_api.h"
#include "cam_common_util.h"
#include "cam_packet_util.h"
#include "cam_mem_mgr_api.h"
#include "sem1218s_define.h"// xiaomi add
#include "cam_parklens_thread.h" //xiaomi add

/* xiaomi dev protection add*/
#include "xm_cam_dev_protection.h"
/* xiaomi dev protection add*/

#define CAM_OIS_FW_VERSION_CHECK_MASK 0x1
//xiaomi add begain
#define MAX_OIS_DEV                   5
#define CAM_OIS_FW_VERSION_MAX_LENGTH 2048
#define MAXOIS_TRANS_SIZE             256
#define MINOIS_TRANS_SIZE             64
static int flash_ois_fw_update_op = 0;
module_param(flash_ois_fw_update_op, int, 0644);

static struct ois_firmware_info ois_firmware[MAX_OIS_DEV] = {0};
//xiaomi add end

static inline uint64_t swap_high_byte_and_low_byte(uint8_t *src,
	uint8_t size_bytes)
{
	uint64_t ret_value = 0x00;
	uint8_t  cycle = 0;

	for (cycle = 0; cycle < size_bytes; cycle++)
		ret_value = ((ret_value<<8) | ((*(src+cycle))&0xff));

	return ret_value;
}

static inline uint64_t swap_high_word_and_low_word(uint16_t *src,
	uint8_t size_words)
{
	uint64_t ret_value = 0x00;
	uint8_t  cycle = 0;

	for (cycle = 0; cycle < size_words; cycle++)
		ret_value = ((ret_value<<16) | ((*(src+cycle))&0xffff));

	return ret_value;
}


int32_t cam_ois_construct_default_power_setting(
	struct cam_sensor_power_ctrl_t *power_info)
{
	int rc = 0;

	power_info->power_setting_size = 1;
	power_info->power_setting =
		CAM_MEM_ZALLOC(sizeof(struct cam_sensor_power_setting),
			GFP_KERNEL);
	if (!power_info->power_setting)
		return -ENOMEM;

	power_info->power_setting[0].seq_type = SENSOR_VAF;
	power_info->power_setting[0].seq_val = CAM_VAF;
	power_info->power_setting[0].config_val = 1;
	power_info->power_setting[0].delay = 2;

	power_info->power_down_setting_size = 1;
	power_info->power_down_setting =
		CAM_MEM_ZALLOC(sizeof(struct cam_sensor_power_setting),
			GFP_KERNEL);
	if (!power_info->power_down_setting) {
		rc = -ENOMEM;
		goto free_power_settings;
	}

	power_info->power_down_setting[0].seq_type = SENSOR_VAF;
	power_info->power_down_setting[0].seq_val = CAM_VAF;
	power_info->power_down_setting[0].config_val = 0;

	return rc;

free_power_settings:
	CAM_MEM_FREE(power_info->power_setting);
	power_info->power_setting = NULL;
	power_info->power_setting_size = 0;
	return rc;
}


/**
 * cam_ois_get_dev_handle - get device handle
 * @o_ctrl:     ctrl structure
 * @arg:        Camera control command argument
 *
 * Returns success or failure
 */
static int cam_ois_get_dev_handle(struct cam_ois_ctrl_t *o_ctrl,
	void *arg)
{
	struct cam_sensor_acquire_dev    ois_acq_dev;
	struct cam_create_dev_hdl        bridge_params;
	struct cam_control              *cmd = (struct cam_control *)arg;

	if (o_ctrl->bridge_intf.device_hdl != -1) {
		CAM_ERR(CAM_OIS, "Device is already acquired");
		return -EFAULT;
	}
	if (copy_from_user(&ois_acq_dev, u64_to_user_ptr(cmd->handle),
		sizeof(ois_acq_dev)))
		return -EFAULT;

	bridge_params.session_hdl = ois_acq_dev.session_handle;
	bridge_params.ops = &o_ctrl->bridge_intf.ops;
	bridge_params.v4l2_sub_dev_flag = 0;
	bridge_params.media_entity_flag = 0;
	bridge_params.priv = o_ctrl;
	bridge_params.dev_id = CAM_OIS;

	ois_acq_dev.device_handle =
		cam_create_device_hdl(&bridge_params);
	if (ois_acq_dev.device_handle <= 0) {
		CAM_ERR(CAM_OIS, "Can not create device handle");
		return -EFAULT;
	}
	o_ctrl->bridge_intf.device_hdl = ois_acq_dev.device_handle;
	o_ctrl->bridge_intf.session_hdl = ois_acq_dev.session_handle;

	CAM_DBG(CAM_OIS, "Device Handle: %d", ois_acq_dev.device_handle);
	if (copy_to_user(u64_to_user_ptr(cmd->handle), &ois_acq_dev,
		sizeof(struct cam_sensor_acquire_dev))) {
		CAM_ERR(CAM_OIS, "ACQUIRE_DEV: copy to user failed");
		return -EFAULT;
	}
	return 0;
}

static int cam_ois_power_up(struct cam_ois_ctrl_t *o_ctrl)
{
	int                                     rc = 0;
	struct cam_hw_soc_info                 *soc_info = &o_ctrl->soc_info;
	struct cam_ois_soc_private             *soc_private;
	struct cam_sensor_power_ctrl_t         *power_info;
	struct completion                      *i3c_probe_completion = NULL;
	struct timespec64                       ts1, ts2; // xiaomi add
	long                                    microsec = 0; // xiaomi add

	/* xiaomi add begin */
	CAM_GET_TIMESTAMP(ts1);
	CAM_DBG(MI_DEBUG, "%s start power_up", o_ctrl->ois_name);
	/* xiaomi add end */

	soc_private = (struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
	power_info = &soc_private->power_info;

	if ((power_info->power_setting == NULL) &&
		(power_info->power_down_setting == NULL)) {
		CAM_INFO(CAM_OIS,
			"Using default power settings");
		rc = cam_ois_construct_default_power_setting(power_info);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"Construct default ois power setting failed.");
			return rc;
		}
	}

	/* Parse and fill vreg params for power up settings */
	rc = msm_camera_fill_vreg_params(
		soc_info,
		power_info->power_setting,
		power_info->power_setting_size);
	if (rc) {
		CAM_ERR(CAM_OIS,
			"failed to fill vreg params for power up rc:%d", rc);
		return rc;
	}

	/* Parse and fill vreg params for power down settings*/
	rc = msm_camera_fill_vreg_params(
		soc_info,
		power_info->power_down_setting,
		power_info->power_down_setting_size);
	if (rc) {
		CAM_ERR(CAM_OIS,
			"failed to fill vreg params for power down rc:%d", rc);
		return rc;
	}

	power_info->dev = soc_info->dev;

	if (o_ctrl->io_master_info.master_type == I3C_MASTER)
		i3c_probe_completion = cam_ois_get_i3c_completion(o_ctrl->soc_info.index);

	rc = cam_sensor_core_power_up(power_info, soc_info, i3c_probe_completion);
	if (rc) {
		CAM_ERR(CAM_OIS, "failed in ois power up rc %d", rc);
		return rc;
	}

	rc = camera_io_init(&o_ctrl->io_master_info);
	if (rc) {
		CAM_ERR(CAM_OIS, "cci_init failed: rc: %d", rc);
		goto cci_failure;
	}

	/* xiaomi add begin */
	CAM_GET_TIMESTAMP(ts2);
	CAM_GET_TIMESTAMP_DIFF_IN_MICRO(ts1, ts2, microsec);
	CAM_DBG(MI_DEBUG, "%s end power_up, occupy time is: %ld ms",
		o_ctrl->ois_name, microsec/1000);
	/* xiaomi add end */

	return rc;
cci_failure:
	if (cam_sensor_util_power_down(power_info, soc_info))
		CAM_ERR(CAM_OIS, "Power Down failed");

	return rc;
}

/**
 * cam_ois_power_down - power down OIS device
 * @o_ctrl:     ctrl structure
 *
 * Returns success or failure
 */
static int cam_ois_power_down(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t                         rc = 0;
	struct cam_sensor_power_ctrl_t  *power_info;
	struct cam_hw_soc_info          *soc_info =
		&o_ctrl->soc_info;
	struct cam_ois_soc_private *soc_private;
	struct timespec64               ts1, ts2; // xiaomi add
	long                            microsec = 0; // xiaomi add

	/* xiaomi add begin */
	CAM_GET_TIMESTAMP(ts1);
	CAM_DBG(MI_DEBUG, "%s start power_down", o_ctrl->ois_name);
	/* xiaomi add end */

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "failed: o_ctrl %pK", o_ctrl);
		return -EINVAL;
	}

	soc_private =
		(struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
	power_info = &soc_private->power_info;
	soc_info = &o_ctrl->soc_info;

	if (!power_info) {
		CAM_ERR(CAM_OIS, "failed: power_info %pK", power_info);
		return -EINVAL;
	}

	rc = cam_sensor_util_power_down(power_info, soc_info);
	if (rc) {
		CAM_ERR(CAM_OIS, "power down the core is failed:%d", rc);
		return rc;
	}

	camera_io_release(&o_ctrl->io_master_info);
	o_ctrl->cam_ois_state = CAM_OIS_ACQUIRE;
	/* xiaomi add begin */
	CAM_GET_TIMESTAMP(ts2);
	CAM_GET_TIMESTAMP_DIFF_IN_MICRO(ts1, ts2, microsec);
	CAM_DBG(MI_DEBUG, "%s end power_down, occupy time is: %ld ms",
		o_ctrl->ois_name, microsec/1000);
	/* xiaomi add end */

	return rc;
}

static int cam_ois_update_time(struct i2c_settings_array *i2c_set,
	enum cam_endianness_type endianness)
{
	struct i2c_settings_list *i2c_list;
	int32_t rc = 0;
	uint32_t size = 0;
	uint32_t i = 0;
	uint64_t qtime_ns = 0;

	if (i2c_set == NULL) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	rc = cam_sensor_util_get_current_qtimer_ns(&qtime_ns);
	if (rc < 0) {
		CAM_ERR(CAM_OIS,
			"Failed to get current qtimer value: %d",
			rc);
		return rc;
	}

	if (endianness == CAM_ENDIANNESS_BIG)
		qtime_ns = swap_high_word_and_low_word((uint16_t *)(&qtime_ns),
					sizeof(qtime_ns) / sizeof(uint16_t));

	list_for_each_entry(i2c_list,
		&(i2c_set->list_head), list) {
		if (i2c_list->op_code ==  CAM_SENSOR_I2C_WRITE_SEQ) {
			size = i2c_list->i2c_settings.size;
			/* qtimer is 8 bytes so validate here*/
			if (size * (uint32_t)(i2c_list->i2c_settings.data_type) != 8) {
				CAM_ERR(CAM_OIS, "Invalid write time settings");
				return -EINVAL;
			}
			switch (i2c_list->i2c_settings.data_type) {
			case CAMERA_SENSOR_I2C_TYPE_BYTE:
				for (i = 0; i < size; i++) {
					CAM_DBG(CAM_OIS, "time: reg_data[%d]: 0x%x",
						i, (qtime_ns & 0xFF));
					i2c_list->i2c_settings.reg_setting[i].reg_data =
						(qtime_ns & 0xFF);
					qtime_ns >>= 8;
				}

				break;
			case CAMERA_SENSOR_I2C_TYPE_WORD:
				for (i = 0; i < size; i++) {
					uint16_t  data = (qtime_ns & 0xFFFF);
					i2c_list->i2c_settings.reg_setting[i].reg_data =
						data;

					qtime_ns >>= 16;

					CAM_DBG(CAM_OIS, "time: reg_data[%d]: 0x%x",
							i, data);
				}

				break;
			default:
				CAM_ERR(CAM_OIS, "Unsupported reg data type");
				return -EINVAL;
			}
		}
	}

	return rc;
}

/* xiaomi dev protection modified*/
static int cam_ois_apply_settings(struct cam_ois_ctrl_t *o_ctrl,
	struct i2c_settings_array *i2c_set,
	enum xm_cam_dev_i2c_cmd_type i2c_cmd_type)
/* xiaomi dev protection modified*/
{
	struct i2c_settings_list *i2c_list;
	int32_t rc = 0;
	uint32_t i, size;
	int32_t j = 0; // xiaomi add

	if (o_ctrl == NULL || i2c_set == NULL) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	if (i2c_set->is_settings_valid != 1) {
		CAM_ERR(CAM_OIS, " Invalid settings");
		return -EINVAL;
	}

	/* xiaomi dev protection add*/
	if (XM_CAM_DEV_XXSKIP_SENSOR_OP_CODE == xm_cam_dev_protection_enable2(XM_CAM_DEV_TYPE_OIS, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		if (XM_CAM_DEV_XXSKIP_SENSOR_OP_CODE ==
			xm_cam_dev_need_skip_i2c_operation3(get_ois_xm_cam_dev_info(o_ctrl), i2c_cmd_type)) {
			CAM_INFO(CAM_OIS, "skip i2c operation");
			return 0;
		}
	}
	/* xiaomi dev protection add*/

	list_for_each_entry(i2c_list,
		&(i2c_set->list_head), list) {
		/* xiaomi add I2C trace begin */
		switch (i2c_list->op_code) {
		case CAM_SENSOR_I2C_WRITE_RANDOM:
		case CAM_SENSOR_I2C_WRITE_BURST:
		case CAM_SENSOR_I2C_WRITE_SEQ: {
			for (j = 0;j < i2c_list->i2c_settings.size;j++) {
				CAM_DBG(CAM_OIS,"[OISSETTINGS Write] ois name %s, request id %d, j=[%d], reg_addr 0x%x, 0x%x", o_ctrl->ois_name,
						i2c_set->request_id, j, i2c_list->i2c_settings.reg_setting[j].reg_addr,
						i2c_list->i2c_settings.reg_setting[j].reg_data);
				trace_cam_i2c_write_log_event("[OISSETTINGS]", o_ctrl->ois_name,
					i2c_set->request_id, j, "WRITE", i2c_list->i2c_settings.reg_setting[j].reg_addr,
					i2c_list->i2c_settings.reg_setting[j].reg_data);
			}
			break;
		}
		case CAM_SENSOR_I2C_READ_RANDOM:
		case CAM_SENSOR_I2C_READ_SEQ: {
			for (j = 0;j < i2c_list->i2c_settings.size;j++) {
				CAM_DBG(CAM_OIS, "[OISSETTINGS Read] ois name  %s, request id %d, j=[%d], reg_addr 0x%x, 0x%x",o_ctrl->ois_name,
						i2c_set->request_id, j,i2c_list->i2c_settings.reg_setting[j].reg_addr,
						i2c_list->i2c_settings.reg_setting[j].reg_data);
				trace_cam_i2c_write_log_event("[OISSETTINGS]", o_ctrl->ois_name,
					i2c_set->request_id, j, "READ", i2c_list->i2c_settings.reg_setting[j].reg_addr,
					i2c_list->i2c_settings.reg_setting[j].reg_data);
			}
			break;
		}
		case CAM_SENSOR_I2C_POLL: {
			for (j = 0;j < i2c_list->i2c_settings.size;j++) {
				trace_cam_i2c_write_log_event("[OISSETTINGS]", o_ctrl->ois_name,
					i2c_set->request_id, j, "POLL", i2c_list->i2c_settings.reg_setting[j].reg_addr,
					i2c_list->i2c_settings.reg_setting[j].reg_data);
			}
			break;
		}
		default:
			break;
		} /* xiaomi add I2C trace end */
		if (i2c_list->op_code ==  CAM_SENSOR_I2C_WRITE_RANDOM) {
			rc = camera_io_dev_write(&(o_ctrl->io_master_info),
				&(i2c_list->i2c_settings));
			/* xiaomi dev protection add begin*/
			if (XM_CAM_DEV_XXSKIP_SENSOR_OP_CODE == xm_cam_dev_protection_enable2(XM_CAM_DEV_TYPE_OIS, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
				if (XM_CMD_DEV_I2C_OIS_CMD_TYPE_INIT == i2c_cmd_type &&
					true == xm_cam_dev_is_i2c_write_cmd(i2c_list)) {
					xm_cam_dev_set_init_result(get_ois_xm_cam_dev_info(o_ctrl), XM_CAM_DEV_INIT_STATUS_SUCCESS);
					if (rc < 0) {
						xm_cam_dev_set_init_result(get_ois_xm_cam_dev_info(o_ctrl), XM_CAM_DEV_INIT_STATUS_FAILURE);
						xm_cam_dev_set_status_info(get_ois_xm_cam_dev_info(o_ctrl),
												   XM_CAM_DEV_OPERATION_INIT,
												   XM_CAM_DEV_STATUS_CODE_I2C_WRITE_ERROR);
					} else {
						xm_cam_dev_set_status_info(get_ois_xm_cam_dev_info(o_ctrl),
												   XM_CAM_DEV_OPERATION_INIT,
												   XM_CAM_DEV_STATUS_CODE_I2C_WRITE_SUCCESS);
					}
				}
				if (rc < 0) {
					if (true == xm_cam_dev_need_change_i2c_rc(get_ois_xm_cam_dev_info(o_ctrl), i2c_cmd_type)) {
						rc = 0;
					}
				}
			}
			/* xiaomi dev protection add end*/
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
					"Failed in Applying i2c wrt settings");
				return rc;
			}
		} else if (i2c_list->op_code == CAM_SENSOR_I2C_WRITE_SEQ) {
			rc = camera_io_dev_write_continuous(
					&(o_ctrl->io_master_info),
					&(i2c_list->i2c_settings),
					CAM_SENSOR_I2C_WRITE_SEQ);
			/* xiaomi dev protection add begin*/
			if (XM_CAM_DEV_XXSKIP_SENSOR_OP_CODE == xm_cam_dev_protection_enable2(XM_CAM_DEV_TYPE_OIS, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
				if (XM_CMD_DEV_I2C_OIS_CMD_TYPE_INIT == i2c_cmd_type &&
					true == xm_cam_dev_is_i2c_write_cmd(i2c_list)) {
					xm_cam_dev_set_init_result(get_ois_xm_cam_dev_info(o_ctrl), XM_CAM_DEV_INIT_STATUS_SUCCESS);
					if (rc < 0) {
						xm_cam_dev_set_init_result(get_ois_xm_cam_dev_info(o_ctrl), XM_CAM_DEV_INIT_STATUS_FAILURE);
						xm_cam_dev_set_status_info(get_ois_xm_cam_dev_info(o_ctrl),
												   XM_CAM_DEV_OPERATION_INIT,
												   XM_CAM_DEV_STATUS_CODE_I2C_WRITE_ERROR);
					} else {
						xm_cam_dev_set_status_info(get_ois_xm_cam_dev_info(o_ctrl),
												   XM_CAM_DEV_OPERATION_INIT,
												   XM_CAM_DEV_STATUS_CODE_I2C_WRITE_SUCCESS);
					}
				}
				if (rc < 0) {
					if (true == xm_cam_dev_need_change_i2c_rc(get_ois_xm_cam_dev_info(o_ctrl), i2c_cmd_type)) {
						rc = 0;
					}
				}
			}
			/* xiaomi dev protection add end*/
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
					"Failed to seq write I2C settings: %d",
					rc);
				return rc;
			}
		} else if (i2c_list->op_code == CAM_SENSOR_I2C_POLL) {
			size = i2c_list->i2c_settings.size;
			for (i = 0; i < size; i++) {
				rc = camera_io_dev_poll(
				&(o_ctrl->io_master_info),
				i2c_list->i2c_settings.reg_setting[i].reg_addr,
				i2c_list->i2c_settings.reg_setting[i].reg_data,
				i2c_list->i2c_settings.reg_setting[i].data_mask,
				i2c_list->i2c_settings.addr_type,
				i2c_list->i2c_settings.data_type,
				i2c_list->i2c_settings.reg_setting[i].delay);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
						"i2c poll apply setting Fail");
					return rc;
				} else if (rc ==  I2C_COMPARE_MISMATCH) {
					CAM_ERR(CAM_OIS, "i2c poll mismatch");
					//xiaomi begin change
					if (((o_ctrl->fw_info.fw_count != 0) && (0 != o_ctrl->opcode.customized_ois_flag)) ||
						(XM_CMD_DEV_I2C_OIS_CMD_TYPE_INIT == i2c_cmd_type)) {
						return rc;//download v2 should return to do check ois fw version.
					} else {
						rc = 0;
					}
					//xiaomi end change
				} else if (rc == I2C_COMPARE_MATCH) {//xiaomi add
					if (o_ctrl->opcode.fw_version_addr ==
						i2c_list->i2c_settings.reg_setting[i].reg_addr)
						ois_fw_version_set(
							i2c_list->i2c_settings.reg_setting[i].reg_data,
							o_ctrl->ois_name);
				}//xiaomi add
			}
		}
	}

	return rc;
}

static int cam_ois_slaveInfo_pkt_parser(struct cam_ois_ctrl_t *o_ctrl,
	uint32_t *cmd_buf, size_t len)
{
	int32_t rc = 0;
	struct cam_cmd_ois_info *ois_info;
	int32_t i = 0; //xiaomi add

	if (!o_ctrl || !cmd_buf || len < sizeof(struct cam_cmd_ois_info)) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	ois_info = (struct cam_cmd_ois_info *)cmd_buf;
	if (o_ctrl->io_master_info.master_type == CCI_MASTER) {
		o_ctrl->io_master_info.cci_client->i2c_freq_mode =
			ois_info->i2c_freq_mode;
		o_ctrl->io_master_info.cci_client->sid =
			ois_info->slave_addr >> 1;
		o_ctrl->ois_fw_flag = ois_info->ois_fw_flag;
		o_ctrl->is_ois_calib = ois_info->is_ois_calib;
		memcpy(o_ctrl->ois_name, ois_info->ois_name, OIS_NAME_LEN);
		o_ctrl->ois_name[OIS_NAME_LEN - 1] = '\0';
		o_ctrl->io_master_info.cci_client->retries = 3;
		o_ctrl->io_master_info.cci_client->id_map = 0;
		memcpy(&(o_ctrl->opcode), &(ois_info->opcode),
			sizeof(struct cam_ois_opcode));
		CAM_DBG(CAM_OIS, "Slave addr: 0x%x Freq Mode: %d",
			ois_info->slave_addr, ois_info->i2c_freq_mode);
	} else if (o_ctrl->io_master_info.master_type == I2C_MASTER) {
		if (!o_ctrl->io_master_info.qup_client) {
			CAM_ERR(CAM_OIS, "io_master_info.qup_client is NULL");
			return -EINVAL;
		}
		o_ctrl->io_master_info.qup_client->i2c_client->addr =
			ois_info->slave_addr;
		CAM_DBG(CAM_OIS, "Slave addr: 0x%x", ois_info->slave_addr);
	} else {
		CAM_ERR(CAM_OIS, "Invalid Master type : %d",
			o_ctrl->io_master_info.master_type);
		rc = -EINVAL;
	}
	//xiaomi add begain
	if ((true == o_ctrl->opcode.fw_mem_store) && (0 == (o_ctrl->fw_store.fw_count))) {
		for(i = 0 ; i < MAX_FW_COUNT ; i++) {
			o_ctrl->fw_store.fw_name[i] = kzalloc(sizeof(char)*OIS_NAME_LEN, GFP_KERNEL);
			if (!o_ctrl->fw_store.fw_name[i])
				return -ENOMEM;
		}
		CAM_INFO(CAM_OIS, "o_ctrl->ois_name %s kzalloc name", o_ctrl->ois_name);
	}
	//xiaomi add end
	return rc;
}

static const struct firmware *cam_fw_mem_find(struct cam_ois_ctrl_t *o_ctrl, const char *fw_name)
{
	int32_t rc = 0;
	uint32_t index;
	const char **string = NULL;

	if (o_ctrl->fw_store.fw_count) {
		string = (const char **)(o_ctrl->fw_store.fw_name);
		rc = cam_common_util_get_string_index(string,
			o_ctrl->fw_store.fw_count, fw_name, &index);

		if ((rc == 0) && (index < MAX_FW_COUNT)) {
			CAM_DBG(CAM_OIS, "find fw %s %p",fw_name,o_ctrl->fw_store.fw[index]);
			return o_ctrl->fw_store.fw[index];
		}
	}

	return NULL;
}

static int cam_fw_mem_store(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t         rc = 0, j = 0, fw_num=0;
	const char      *fw_name                                  = NULL;
	const char      fw_name_exten[MAX_FW_COUNT][OIS_NAME_LEN] ={{"%s.coeff"},{"%s.prog"},{"%s.mem"},{"%s.pher"},{"%s.afcoeff"}};
	struct device   *dev                                      = &(o_ctrl->pdev->dev);

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}
	//store ois fw name in cam
	if (!(o_ctrl->fw_store.fw_count)) {
		CAM_INFO(CAM_OIS, "begin store fw_index, MAX_FW_COUNT %d , so store it", MAX_FW_COUNT);

		for(fw_num = 0, j = 0;fw_num < MAX_FW_COUNT;fw_num++,j++) {
			snprintf(o_ctrl->fw_store.fw_name[j], OIS_NAME_LEN, fw_name_exten[fw_num], o_ctrl->ois_name);
			fw_name = o_ctrl->fw_store.fw_name[j];
			/* Load FW */
			rc = request_firmware((const struct firmware**)&(o_ctrl->fw_store.fw[j]), fw_name, dev);
			if (rc) {
				CAM_WARN(CAM_OIS, "Failed to locate %s", fw_name_exten[fw_num]);
				o_ctrl->fw_store.fw[j] = NULL;
				j--;
			} else {
				CAM_INFO(CAM_OIS, "o_ctrl->ois_name %s fw_name %s store", o_ctrl->ois_name, fw_name);
			}
		}
		o_ctrl->fw_store.fw_count = j;
	} else {
		for(fw_num = 0;fw_num < o_ctrl->fw_store.fw_count;fw_num++) {
			if (NULL != (o_ctrl->fw_store.fw[fw_num]))
				CAM_DBG(CAM_OIS, "fw_num %d fw_name %s size %d", fw_num, o_ctrl->fw_store.fw_name[fw_num], (o_ctrl->fw_store.fw[fw_num])->size);
			else
				CAM_DBG(CAM_OIS, "fw_num %d fw_name %s  is NULL", fw_num, o_ctrl->fw_store.fw_name[fw_num]);
		}
	}
	return rc;
}
//xiaomi add end

static int cam_ois_parse_fw_setting(uint8_t *cmd_buf, uint32_t size,
	struct i2c_settings_array *reg_settings)
{
	int32_t                 rc = 0;
	uint32_t                byte_cnt = 0;
	struct common_header   *cmm_hdr;
	uint16_t                op_code;
	uint32_t                j = 0;
	struct list_head       *list = NULL;
	uint32_t                payload_count = 0;
	size_t                  tot_size = 0;

	while (byte_cnt < size) {
		if ((size - byte_cnt) < sizeof(struct common_header)) {
			CAM_ERR(CAM_OIS, "Not enough buffer");
			rc = -EINVAL;
			goto end;
		}
		cmm_hdr = (struct common_header *)cmd_buf;
		op_code = cmm_hdr->fifth_byte;
		CAM_DBG(CAM_OIS, "Command Type:%d, Op code:%d",
				 cmm_hdr->cmd_type, op_code);

		switch (cmm_hdr->cmd_type) {
		case CAMERA_SENSOR_CMD_TYPE_I2C_RNDM_WR: {
			uint32_t cmd_length_in_bytes = 0;
			struct cam_cmd_i2c_random_wr
			*cam_cmd_i2c_random_wr =
			(struct cam_cmd_i2c_random_wr *)cmd_buf;
			payload_count = cam_cmd_i2c_random_wr->header.count;

			if ((size - byte_cnt) < sizeof(struct cam_cmd_i2c_random_wr)) {
				CAM_ERR(CAM_OIS,
					"Not enough buffer provided,size %d,byte_cnt %d",
					size, byte_cnt);
				rc = -EINVAL;
				goto end;
			}
			tot_size = sizeof(struct i2c_rdwr_header) +
			(sizeof(struct i2c_random_wr_payload) *
			payload_count);

			if (tot_size > (size - byte_cnt)) {
				CAM_ERR(CAM_SENSOR_UTIL,
				"Not enough buffer provided %d, %d, %d",
				tot_size, size, byte_cnt);
				rc = -EINVAL;
				goto end;
			}

			rc = cam_sensor_handle_random_write(
				cam_cmd_i2c_random_wr,
				reg_settings,
				&cmd_length_in_bytes, &j, &list, payload_count);
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
				"Failed in random write %d", rc);
				goto end;
			}

			byte_cnt += sizeof(struct cam_cmd_i2c_random_wr);
			cmd_buf += sizeof(struct cam_cmd_i2c_random_wr);

			break;
		}
		case CAMERA_SENSOR_CMD_TYPE_I2C_CONT_WR: {
			uint32_t cmd_length_in_bytes = 0;
			struct cam_cmd_i2c_continuous_wr
			*cam_cmd_i2c_continuous_wr =
			(struct cam_cmd_i2c_continuous_wr *)cmd_buf;
			payload_count = cam_cmd_i2c_continuous_wr->header.count;
			if ((size - byte_cnt) < sizeof(struct cam_cmd_i2c_continuous_wr)) {
				CAM_ERR(CAM_OIS,
					"Not enough buffer provided,size %d,byte_cnt %d",
					size, byte_cnt);
				rc = -EINVAL;
				goto end;
			}
			tot_size = sizeof(struct i2c_rdwr_header) +
			sizeof(cam_cmd_i2c_continuous_wr->reg_addr) +
			(sizeof(struct cam_cmd_read) *
			payload_count);

			if (tot_size > (size - byte_cnt)) {
				CAM_ERR(CAM_SENSOR_UTIL,
				"Not enough buffer provided %d, %d, %d",
				tot_size, size, byte_cnt);
				rc = -EINVAL;
				goto end;
			}
			rc = cam_sensor_handle_continuous_write(
				cam_cmd_i2c_continuous_wr,
				reg_settings,
				&cmd_length_in_bytes, &j, &list, payload_count);
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
				"Failed in continuous write %d", rc);
				goto end;
			}

			byte_cnt += sizeof(struct cam_cmd_i2c_continuous_wr);
			cmd_buf += sizeof(struct cam_cmd_i2c_continuous_wr);

			break;
		}
		case CAMERA_SENSOR_CMD_TYPE_WAIT: {
			if (op_code == CAMERA_SENSOR_WAIT_OP_HW_UCND ||
				op_code == CAMERA_SENSOR_WAIT_OP_SW_UCND) {
				if ((size - byte_cnt) <
					sizeof(struct cam_cmd_unconditional_wait)) {
					CAM_ERR(CAM_OIS,
						"Not enough buffer provided,size %d,byte_cnt %d",
						size, byte_cnt);
					rc = -EINVAL;
					goto end;
				}

				rc = cam_sensor_handle_delay(
					(uint32_t **)(&cmd_buf), op_code,
					reg_settings, j, &byte_cnt,
					list);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
						"delay hdl failed: %d",
						rc);
					goto end;
				}
			} else if (op_code == CAMERA_SENSOR_WAIT_OP_COND) {
				if ((size - byte_cnt) <
					sizeof(struct cam_cmd_conditional_wait)) {
					CAM_ERR(CAM_OIS,
						"Not enough buffer provided,size %d,byte_cnt %d",
						size, byte_cnt);
					rc = -EINVAL;
					goto end;
				}
				rc = cam_sensor_handle_poll(
					(uint32_t **)(&cmd_buf), reg_settings,
					&byte_cnt, &j, &list);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
						"parsing POLL fail: %d",
						rc);
					goto end;
				}
			} else {
				CAM_ERR(CAM_OIS,
					"Wrong Wait Command: %d",
					op_code);
				rc = -EINVAL;
				goto end;
			}
			break;
		}
		case CAMERA_SENSOR_CMD_TYPE_I2C_RNDM_RD: {
			uint16_t cmd_length_in_bytes = 0;
			struct cam_cmd_i2c_random_rd *i2c_random_rd =
			(struct cam_cmd_i2c_random_rd *)cmd_buf;
			payload_count = i2c_random_rd->header.count;
			if ((size - byte_cnt) < sizeof(struct cam_cmd_i2c_random_rd)) {
				CAM_ERR(CAM_OIS,
					"Not enough buffer provided,size %d,byte_cnt %d",
					size, byte_cnt);
				rc = -EINVAL;
				goto end;
			}

			tot_size = sizeof(struct i2c_rdwr_header) +
			(sizeof(struct cam_cmd_read) *
			payload_count);
			if (tot_size > (size - byte_cnt)) {
				CAM_ERR(CAM_SENSOR_UTIL,
				"Not enough buffer provided %d, %d, %d",
				tot_size, size, byte_cnt);
				rc = -EINVAL;
				goto end;
			}
			rc = cam_sensor_handle_random_read(
				i2c_random_rd,
				reg_settings,
				&cmd_length_in_bytes, &j, &list,
				NULL, payload_count);
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
				"Failed in random read %d", rc);
				goto end;
			}

			byte_cnt += sizeof(struct cam_cmd_i2c_random_rd);
			cmd_buf += sizeof(struct cam_cmd_i2c_random_rd);

			break;
		}
		case CAMERA_SENSOR_CMD_TYPE_I2C_CONT_RD: {
			uint16_t cmd_length_in_bytes = 0;
			struct cam_cmd_i2c_continuous_rd
			*i2c_continuous_rd =
			(struct cam_cmd_i2c_continuous_rd *)cmd_buf;

			if ((size - byte_cnt) < sizeof(struct cam_cmd_i2c_continuous_rd)) {
				CAM_ERR(CAM_OIS,
					"Not enough buffer provided,size %d,byte_cnt %d",
					size, byte_cnt);
				rc = -EINVAL;
				goto end;
			}

			rc = cam_sensor_handle_continuous_read(
				i2c_continuous_rd,
				reg_settings,
				&cmd_length_in_bytes, &j, &list,
				NULL);
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
				"Failed in continuous read %d", rc);
				goto end;
			}

			byte_cnt += sizeof(struct cam_cmd_i2c_continuous_rd);
			cmd_buf += sizeof(struct cam_cmd_i2c_continuous_rd);

			break;
		}
		default:
			CAM_ERR(CAM_OIS, "Invalid Command Type:%d",
				 cmm_hdr->cmd_type);
			rc = -EINVAL;
			goto end;
		}
	}

end:
	return rc;
}

static int cam_ois_fw_info_pkt_parser(struct cam_ois_ctrl_t *o_ctrl,
	uint32_t *cmd_buf, size_t len)
{
	int32_t                         rc = 0;
	struct cam_cmd_ois_fw_info     *ois_fw_info;
	uint8_t                        *pSettingData = NULL;
	uint32_t                        size = 0;
	uint32_t                        version_size = 0;
	struct i2c_settings_array      *reg_settings = NULL;
	uint8_t                         count = 0;
	uint32_t                        idx;
	uint8_t                         ois_fw_count = 0;

	if (!o_ctrl || !cmd_buf || len < sizeof(struct cam_cmd_ois_fw_info)) {
		CAM_ERR(CAM_OIS, "Invalid Args,o_ctrl %p,cmd_buf %p,len %d",
			o_ctrl, cmd_buf, len);
		return -EINVAL;
	}

	ois_fw_info = (struct cam_cmd_ois_fw_info *)cmd_buf;
	ois_fw_count = ois_fw_info->fw_count;
	CAM_DBG(CAM_OIS, "endianness %d, fw_count %d",
		ois_fw_info->endianness, ois_fw_count);

	if (ois_fw_count <= MAX_OIS_FW_COUNT) {
		o_ctrl->fw_info.version     = ois_fw_info->version;
		o_ctrl->fw_info.cmd_type    = ois_fw_info->cmd_type;
		o_ctrl->fw_info.fw_count    = ois_fw_info->fw_count;
		o_ctrl->fw_info.endianness  = ois_fw_info->endianness;
		o_ctrl->fw_info.fw_param    = CAM_MEM_ZALLOC(sizeof(struct cam_cmd_ois_fw_param) * ois_fw_info->fw_count, GFP_KERNEL);
		if (!o_ctrl->fw_info.fw_param) {
			return -ENOMEM;
		}

		memcpy(o_ctrl->fw_info.fw_param, &ois_fw_info->fw_param[0], sizeof(struct cam_cmd_ois_fw_param) * ois_fw_info->fw_count);

		pSettingData = (uint8_t *)cmd_buf + sizeof(struct cam_cmd_ois_fw_info);

		if ((ois_fw_info->param_mask & CAM_OIS_FW_VERSION_CHECK_MASK) == 0x1) {
			version_size = ois_fw_info->params[0];
			CAM_DBG(CAM_OIS, "versionSize: %d", version_size);
		}

		if ((version_size != 0) && (o_ctrl->i2c_fw_version_data.is_settings_valid == 0)) {
			reg_settings = &o_ctrl->i2c_fw_version_data;
			reg_settings->is_settings_valid = 1;
			rc = cam_ois_parse_fw_setting(pSettingData, version_size, reg_settings);
			if (rc) {
				CAM_ERR(CAM_OIS, "Failed to parse fw version settings");
				goto release;
			}

			pSettingData += version_size;
		}

		o_ctrl->i2c_fw_init_data     = CAM_MEM_ZALLOC(sizeof(struct i2c_settings_array) * ois_fw_info->fw_count, GFP_KERNEL);
		o_ctrl->i2c_fw_finalize_data = CAM_MEM_ZALLOC(sizeof(struct i2c_settings_array) * ois_fw_info->fw_count, GFP_KERNEL);
		if (!o_ctrl->i2c_fw_init_data || !o_ctrl->i2c_fw_finalize_data){
			rc= -ENOMEM;
			goto release;
		}

		for (count = 0; count < o_ctrl->fw_info.fw_count; count++){
			INIT_LIST_HEAD(&(o_ctrl->i2c_fw_init_data[count].list_head));
			INIT_LIST_HEAD(&(o_ctrl->i2c_fw_finalize_data[count].list_head));
		}

		for (count = 0; count < o_ctrl->fw_info.fw_count*2; count++) {
			idx = count / 2;
			/* init settings */
			if ((count & 0x1) == 0) {
				size =  o_ctrl->fw_info.fw_param[idx].fw_init_size;
				reg_settings = &o_ctrl->i2c_fw_init_data[idx];
				CAM_DBG(CAM_OIS, "init size %d", size);
			/* finalize settings */
			} else if ((count & 0x1) == 1) {
				size = o_ctrl->fw_info.fw_param[idx].fw_finalize_size;
				reg_settings = &o_ctrl->i2c_fw_finalize_data[idx];
				CAM_DBG(CAM_OIS, "finalize size %d", size);
			} else {
				size = 0;
				CAM_DBG(CAM_OIS, "Unsupported case");
				rc= -EINVAL;
				goto release;
			}

			if (size != 0) {
				reg_settings->is_settings_valid = 1;
				rc = cam_ois_parse_fw_setting(pSettingData, size, reg_settings);
			}

			if (rc) {
				CAM_ERR(CAM_OIS, "Failed to parse fw setting");
				goto release;
			}

			pSettingData += size;
		}
	} else {
		CAM_ERR(CAM_OIS, "Exceed max fw count");
		return -EINVAL;
	}

	return rc;

release:
	if (o_ctrl->fw_info.fw_param){
		CAM_MEM_FREE(o_ctrl->fw_info.fw_param);
		o_ctrl->fw_info.fw_param = NULL;
	}
	if (o_ctrl->i2c_fw_init_data){
		CAM_MEM_FREE(o_ctrl->i2c_fw_init_data);
		o_ctrl->i2c_fw_init_data = NULL;
	}
	if (o_ctrl->i2c_fw_finalize_data){
		CAM_MEM_FREE(o_ctrl->i2c_fw_finalize_data);
		o_ctrl->i2c_fw_finalize_data = NULL;
	}
	return rc;
}


int cam_ois_fw_info_input_check(uint32_t *cmd_buf, size_t len)
{
	int32_t     rc           = 0;
	uint32_t    version_size = 0;
	size_t      cmdsize      = 0;
	uint8_t     count        = 0;
	struct cam_cmd_ois_fw_info_v2  *ois_fw_info;

	ois_fw_info  = (struct cam_cmd_ois_fw_info_v2 *)cmd_buf;
	if ((ois_fw_info->param_mask & CAM_OIS_FW_VERSION_CHECK_MASK) == 0x1) {
		version_size = ois_fw_info->params[0];
		CAM_DBG(CAM_OIS, "versionSize: %d", version_size);
	}

	cmdsize += sizeof(struct cam_cmd_ois_fw_info_v2);
	cmdsize += sizeof(struct cam_cmd_ois_fw_param) * ois_fw_info->fw_count;
	cmdsize += version_size;
	for (count = 0; count < ois_fw_info->fw_count; count++){
		if (ois_fw_info->fw_param[count].fw_init_size > 0){
			cmdsize += ois_fw_info->fw_param[count].fw_init_size;
			CAM_DBG(CAM_OIS, "count:%d, fw init size:%d", count, ois_fw_info->fw_param[count].fw_init_size);
		}
		if (ois_fw_info->fw_param[count].fw_finalize_size > 0){
			cmdsize += ois_fw_info->fw_param[count].fw_finalize_size;
			CAM_DBG(CAM_OIS, "count:%d, fw finalize size:%d", count, ois_fw_info->fw_param[count].fw_finalize_size);
		}
	}
	if (cmdsize != len){
		CAM_ERR(CAM_OIS, "cmd size validate error! cmdsize:%d, expect:%d", cmdsize, len);
		rc = -EINVAL;
	}

	return rc;
}

static int cam_ois_fw_info_pkt_parser_v2(struct cam_ois_ctrl_t *o_ctrl,
	uint32_t *cmd_buf, size_t len)
{
	int32_t                         rc = 0;
	struct cam_cmd_ois_fw_info_v2  *ois_fw_info;
	uint8_t                        *pSettingData = NULL;
	uint32_t                        size = 0;
	uint32_t                        version_size = 0;
	struct i2c_settings_array      *reg_settings = NULL;
	uint8_t                         count = 0;
	uint8_t                        *ois_fw_param = NULL;

	if (!o_ctrl || !cmd_buf || len < sizeof(struct cam_cmd_ois_fw_info_v2)) {
		CAM_ERR(CAM_OIS, "Invalid Args,o_ctrl %p,cmd_buf %p,len %d",
			o_ctrl, cmd_buf, len);
		return -EINVAL;
	}

	if(cam_ois_fw_info_input_check(cmd_buf, len) < 0) {
		return -EINVAL;
	}

	ois_fw_info  = (struct cam_cmd_ois_fw_info_v2 *)cmd_buf;
	ois_fw_param = (uint8_t*)ois_fw_info->fw_param;

	CAM_DBG(CAM_OIS, "endianness %d, fw_count %d",
		ois_fw_info->endianness, ois_fw_info->fw_count);

	if (ois_fw_info->fw_count > 0) {
		o_ctrl->fw_info.version     = ois_fw_info->version;
		o_ctrl->fw_info.cmd_type    = ois_fw_info->cmd_type;
		o_ctrl->fw_info.fw_count    = ois_fw_info->fw_count;
		o_ctrl->fw_info.endianness  = ois_fw_info->endianness;
		o_ctrl->fw_info.fw_param    = CAM_MEM_ZALLOC(sizeof(struct cam_cmd_ois_fw_param) * ois_fw_info->fw_count, GFP_KERNEL);
		if (!o_ctrl->fw_info.fw_param) {
			return -ENOMEM;
		}
		memcpy(o_ctrl->fw_info.fw_param, ois_fw_param, sizeof(struct cam_cmd_ois_fw_param) * ois_fw_info->fw_count);

		pSettingData = (uint8_t *)cmd_buf + sizeof(struct cam_cmd_ois_fw_info_v2) + sizeof(struct cam_cmd_ois_fw_param)*ois_fw_info->fw_count;

		if ((ois_fw_info->param_mask & CAM_OIS_FW_VERSION_CHECK_MASK) == 0x1) {
			version_size = ois_fw_info->params[0];
			CAM_DBG(CAM_OIS, "versionSize: %d", version_size);
		}
		if ((version_size != 0) && (o_ctrl->i2c_fw_version_data.is_settings_valid == 0)) {
			reg_settings = &o_ctrl->i2c_fw_version_data;
			reg_settings->is_settings_valid = 1;
			rc = cam_ois_parse_fw_setting(pSettingData, version_size, reg_settings);
			if (rc) {
				CAM_ERR(CAM_OIS, "Failed to parse fw version settings");
				goto release;
			}
			pSettingData += version_size;
		}

		o_ctrl->i2c_fw_init_data     = CAM_MEM_ZALLOC(sizeof(struct i2c_settings_array) * ois_fw_info->fw_count, GFP_KERNEL);
		o_ctrl->i2c_fw_finalize_data = CAM_MEM_ZALLOC(sizeof(struct i2c_settings_array) * ois_fw_info->fw_count, GFP_KERNEL);
		if (!o_ctrl->i2c_fw_init_data || !o_ctrl->i2c_fw_finalize_data) {
			goto release;
		}

		for (count = 0; count < ois_fw_info->fw_count; count++) {
			INIT_LIST_HEAD(&(o_ctrl->i2c_fw_init_data[count].list_head));
			INIT_LIST_HEAD(&(o_ctrl->i2c_fw_finalize_data[count].list_head));

			if (o_ctrl->fw_info.fw_param[count].fw_init_size > 0)
			{
				reg_settings = &o_ctrl->i2c_fw_init_data[count];
				reg_settings->is_settings_valid = 1;
				size = o_ctrl->fw_info.fw_param[count].fw_init_size;
				rc = cam_ois_parse_fw_setting(pSettingData, size, reg_settings);
				if (rc) {
					CAM_ERR(CAM_OIS, "Failed to parse fw setting");
					goto release;
				}
				pSettingData += size;
				CAM_DBG(CAM_OIS, "count:%d, init size %d", count, size);
			}

			if (o_ctrl->fw_info.fw_param[count].fw_finalize_size > 0)
			{
				reg_settings = &o_ctrl->i2c_fw_finalize_data[count];
				reg_settings->is_settings_valid = 1;
				size = o_ctrl->fw_info.fw_param[count].fw_finalize_size;
				rc = cam_ois_parse_fw_setting(pSettingData, size, reg_settings);
				if (rc) {
					CAM_ERR(CAM_OIS, "Failed to parse fw setting");
					goto release;
				}
				pSettingData += size;
				CAM_DBG(CAM_OIS, "count:%d, finalize size %d", count, size);
			}
		}
	}

	return rc;

release:
	if (o_ctrl->fw_info.fw_param){
		CAM_MEM_FREE(o_ctrl->fw_info.fw_param);
		o_ctrl->fw_info.fw_param = NULL;
	}
	if (o_ctrl->i2c_fw_init_data){
		CAM_MEM_FREE(o_ctrl->i2c_fw_init_data);
		o_ctrl->i2c_fw_init_data = NULL;
	}
	if (o_ctrl->i2c_fw_finalize_data){
		CAM_MEM_FREE(o_ctrl->i2c_fw_finalize_data);
		o_ctrl->i2c_fw_finalize_data = NULL;
	}
	return rc;
}
//xiaomi add begin
static int cam_ois_fw_download_chip(
	struct cam_ois_ctrl_t *o_ctrl,
	const char* fw_name,
	uint32_t fw_addr,
	int32_t bytes_for_perdownload)
{
	uint16_t                           total_bytes = 0;
	uint8_t                           *ptr = NULL;
	uint32_t                           fw_size;
	const struct firmware             *fw = NULL;
	int32_t                            rc = 0, cnt, i, j;
	struct device                     *dev = &(o_ctrl->pdev->dev);
	struct cam_sensor_i2c_reg_setting  i2c_reg_setting;
	void                              *vaddr = NULL;
	bool                               is_need_reload = false;

	if (true == o_ctrl->opcode.fw_mem_store) {
		fw = cam_fw_mem_find(o_ctrl, fw_name);
		if (NULL == fw) {
  			CAM_DBG(CAM_OIS, "Failed to find %s",fw_name);
			/* Load FW */
			rc = request_firmware(&fw, fw_name, dev);
			if (rc) {
				CAM_INFO(CAM_OIS, "Failed to locate %s, reset rc %d to 0", fw_name, rc);
				rc = 0;
				return rc;
			}
			is_need_reload = true;
		}
	}
	else {
		/* Load FW */
		rc = request_firmware(&fw, fw_name, dev);
		if (rc) {
			CAM_INFO(CAM_OIS, "Failed to locate %s, reset rc %d to 0", fw_name, rc);
			rc = 0;
			return rc;
		}
	}

	total_bytes = fw->size;
	i2c_reg_setting.addr_type = o_ctrl->opcode.fw_addr_type;
	i2c_reg_setting.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	i2c_reg_setting.size = total_bytes;
	i2c_reg_setting.delay = 0;
	fw_size = (sizeof(struct cam_sensor_i2c_reg_array) * total_bytes);
	vaddr = vmalloc(fw_size);
	if (!vaddr) {
		CAM_ERR(CAM_OIS,
			"Failed in allocating i2c_array: fw_size: %u", fw_size);
		if ((false == is_need_reload) && (true == o_ctrl->opcode.fw_mem_store)) {
			CAM_DBG(CAM_OIS, "FW no need release");
		} else {
			release_firmware(fw);
			fw = NULL;
		}
		return -ENOMEM;
	}

	CAM_INFO(CAM_OIS, "FW %s start address 0x%x size:%d bytes_for_perdownload %d.", fw_name, fw_addr, total_bytes, bytes_for_perdownload);

	i2c_reg_setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (
		vaddr);

	for (i = 0, ptr = (uint8_t *)fw->data, j = 0; i < total_bytes;) {
		for (cnt = 0; cnt < bytes_for_perdownload && i < total_bytes;
			cnt++, ptr++, i++) {
				i2c_reg_setting.reg_setting[cnt].reg_addr =
					fw_addr + j * bytes_for_perdownload;
				i2c_reg_setting.reg_setting[cnt].reg_data = *ptr;
				i2c_reg_setting.reg_setting[cnt].delay = 0;
				i2c_reg_setting.reg_setting[cnt].data_mask = 0;
		}
		i2c_reg_setting.size = cnt;

		if (o_ctrl->opcode.is_addr_increase) {
			j++;
		}

		rc = camera_io_dev_write_continuous(&(o_ctrl->io_master_info),
			&i2c_reg_setting, CAM_SENSOR_I2C_WRITE_BURST);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "OIS FW size(%d) download failed. %d",
				total_bytes, rc);
		}
	}

	vfree(vaddr);
	vaddr = NULL;
	if ((false == is_need_reload) && (true == o_ctrl->opcode.fw_mem_store)) {
		CAM_DBG(CAM_OIS, "FW is_need_reload is false not release");
	} else {
		release_firmware(fw);
		fw = NULL;
	}

	return rc;
}
//xiaomi add end
static int cam_ois_fw_download(struct cam_ois_ctrl_t *o_ctrl, int32_t bytes_for_perdownload)//xiaomi add
//static int cam_ois_fw_download(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t			rc = 0, i;
	struct device	*dev;
	const char		*fw_name[MAX_FW_COUNT] = {NULL};//xiaomi add
	char			name_prog[OIS_NAME_LEN] = {0};//xiaomi add
	char			name_coeff[OIS_NAME_LEN] = {0};//xiaomi add
	char			name_mem[OIS_NAME_LEN] = {0};//xiaomi add
	char			name_pher[OIS_NAME_LEN] = {0};//xiaomi add
	char			name_afcoeff[OIS_NAME_LEN] = {0};
	uint32_t		write_addr[MAX_FW_COUNT] = {0};//xiaomi add

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	dev = &(o_ctrl->pdev->dev);
	//xiaomi modify begin
	snprintf(name_coeff, OIS_NAME_LEN, "%s.coeff", o_ctrl->ois_name);
	snprintf(name_prog, OIS_NAME_LEN, "%s.prog", o_ctrl->ois_name);
	snprintf(name_mem, OIS_NAME_LEN, "%s.mem", o_ctrl->ois_name);
	snprintf(name_pher, OIS_NAME_LEN, "%s.pher", o_ctrl->ois_name);
	snprintf(name_afcoeff, OIS_NAME_LEN, "%s.afcoeff", o_ctrl->ois_name);

	/* cast pointer as const pointer*/
	fw_name[0] = name_prog;
	fw_name[1] = name_coeff;
	fw_name[2] = name_mem;
	fw_name[3] = name_pher;
	fw_name[4] = name_afcoeff;

	write_addr[0] = o_ctrl->opcode.prog;
	write_addr[1] = o_ctrl->opcode.coeff;
	write_addr[2] = o_ctrl->opcode.memory;
	write_addr[3] = o_ctrl->opcode.pheripheral;
	write_addr[4] = o_ctrl->opcode.afcoeff;

	CAM_INFO(CAM_OIS,"%s FW down Bebin",o_ctrl->ois_name);
	for (i=0; i<MAX_FW_COUNT; i++)
	{
		if (INVALID_FW_ADDR == write_addr[i])
		{
			CAM_DBG(CAM_OIS, "OIS FW(%s) addr:0x%x ,skip it",
				 fw_name[i], write_addr[i]);
			continue;
		}

		rc = cam_ois_fw_download_chip(o_ctrl, fw_name[i], write_addr[i], bytes_for_perdownload);
		if (rc < 0)
		{
			CAM_ERR(CAM_OIS, "OIS FW(%s) download failed rc: %d",
				 fw_name[i], rc);
			break;
		}
	}

	CAM_INFO(CAM_OIS,"%s FW down End,rc=%d",o_ctrl->ois_name, rc);
//xiaomi modify end
	return rc;
}

static int write_ois_fw(uint8_t *fw_data, enum cam_endianness_type endianness,
	struct cam_cmd_ois_fw_param *fw_param, struct camera_io_master io_master_info,
	uint8_t i2c_operation)
{
	int32_t                             rc = 0;
	struct cam_sensor_i2c_reg_setting   setting;
	uint8_t                            *ptr = fw_data;
	int32_t                             cnt = 0, wcnt = 0;
	void                               *vaddr = NULL;
	uint16_t                            data_type = fw_param->fw_data_type;
	uint16_t                            len_per_write = fw_param->fw_len_per_write /
								fw_param->fw_data_type;

	vaddr = vmalloc((sizeof(struct cam_sensor_i2c_reg_array) * len_per_write));
	if (!vaddr) {
		CAM_ERR(CAM_OIS,
			"Failed in allocating i2c_array: size: %u",
			(sizeof(struct cam_sensor_i2c_reg_array) * len_per_write));
		return -ENOMEM;
	}

	setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (vaddr);
	setting.addr_type   = fw_param->fw_addr_type;
	setting.data_type   = fw_param->fw_data_type;
	setting.size        = len_per_write;
	setting.delay       = fw_param->fw_delayUs;

	for (wcnt = 0; wcnt < (fw_param->fw_size/data_type); wcnt += len_per_write) {
		for (cnt = 0; cnt < len_per_write; cnt++, ptr += data_type) {
			setting.reg_setting[cnt].reg_addr =
				fw_param->fw_reg_addr + wcnt + cnt;
			/* Big */
			if (endianness == CAM_ENDIANNESS_BIG) {
				setting.reg_setting[cnt].reg_data =
					(uint32_t)swap_high_byte_and_low_byte(ptr, data_type);
			/* Little */
			} else if (endianness == CAM_ENDIANNESS_LITTLE) {
				switch (data_type) {
				case CAMERA_SENSOR_I2C_TYPE_BYTE:
					setting.reg_setting[cnt].reg_data = *((uint8_t *)ptr);
					break;
				case CAMERA_SENSOR_I2C_TYPE_WORD:
					setting.reg_setting[cnt].reg_data = *((uint16_t *)ptr);
					break;
				default:
					CAM_ERR(CAM_OIS,
						"Unsupported data type");
					rc = -EINVAL;
					goto End;
				}
			}

			setting.reg_setting[cnt].delay = fw_param->fw_delayUs;
			setting.reg_setting[cnt].data_mask = 0;
		}

		if (i2c_operation == CAM_SENSOR_I2C_WRITE_RANDOM) {
			rc = camera_io_dev_write(&(io_master_info),
				&setting);
		} else if (i2c_operation == CAM_SENSOR_I2C_WRITE_BURST ||
			i2c_operation == CAM_SENSOR_I2C_WRITE_SEQ) {
			rc = camera_io_dev_write_continuous(&io_master_info,
				&setting, i2c_operation);
		}

		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"Failed in Applying i2c wrt settings");
			break;
		}
	}

End:
	vfree(vaddr);
	vaddr = NULL;

	return rc;
}
/*xiaomi modify start*/
static int32_t i2c_write_data_seq(struct camera_io_master *io_master_info, uint32_t addr, uint32_t length, uint8_t* data, uint32_t delay)
{
	struct cam_sensor_i2c_reg_array w_data[WRITE_BUFFER_MAXSIZE] = { {0} };
	struct cam_sensor_i2c_reg_setting write_setting;
	uint32_t i = 0;
	int32_t rc = 0;

	if (!data || (length < 1) || WRITE_BUFFER_MAXSIZE < length) {
		CAM_ERR(CAM_OIS, "[SEM1218S] Invalid Args");
		rc = -EINVAL;
		return rc;
	}

	for (i = 0; i < length; i++) {
		w_data[i].reg_addr = addr + i;
		w_data[i].reg_data = data[i];
		w_data[i].delay = 0;
		w_data[i].data_mask = 0;
	}

	write_setting.size = length;
	write_setting.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
	write_setting.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	write_setting.delay = delay;
	write_setting.reg_setting = w_data;

	rc = camera_io_dev_write_continuous(io_master_info,
		&write_setting, CAM_SENSOR_I2C_WRITE_SEQ);

	if (rc < 0) {
		CAM_ERR(CAM_OIS, "[SEM1218S] OIS i2c_write_data write failed, rc: %d", rc);
	}

	for (i = 0; i < length; i++) {
		CAM_INFO(CAM_OIS, "[SEM1218S] write i %d is add 0x%04x data 0x%x delay %d",
			i, w_data[i].reg_addr, w_data[i].reg_data, delay);
	}

	return rc;
}

static int32_t i2c_read_data_seq(struct camera_io_master *io_master_info, uint32_t addr, uint32_t length, uint8_t *data)
{
	int32_t rc = 0;
	int32_t i = 0;

	rc = camera_io_dev_read_seq(io_master_info,
		addr, data,
		CAMERA_SENSOR_I2C_TYPE_WORD,
		CAMERA_SENSOR_I2C_TYPE_BYTE,
		length);

	for (i = 0; i < length; i++) {
		CAM_INFO(CAM_OIS, "[SEM1218S] read addr 0x%04x[%d] = 0x%02x", addr, i, data[i]);
	}

	if (rc) {
		CAM_ERR(CAM_OIS, "[SEM1218S] Failed to read, rc: %d", rc);
	}

	return rc;
}

static int write_ois_fw_sem1218s(uint8_t *fw_data, enum cam_endianness_type endianness,
	struct cam_cmd_ois_fw_param *fw_param, struct camera_io_master io_master_info,
	uint8_t i2c_operation)
{
	int32_t                             rc = 0;
	struct cam_sensor_i2c_reg_setting   setting;
	uint8_t                            *ptr = fw_data;
	int32_t                             cnt = 0, wcnt = 0;
	void                               *vaddr = NULL;
	uint16_t                            len_per_write = fw_param->fw_len_per_write;
	uint32_t            check_sum = 0;
	uint8_t             readbuffer[4] = {0}, lookup_idx;
	uint8_t             writebuffer[4] = {0};
	uint32_t            updated_ver = 0, new_fw_ver = 0;
	uint32_t            crc32_table[256] = {0};

	CAM_INFO(CAM_OIS, "[SEM1218S] len_per_write %d fw_delayUs %d, endianness %d i2c_operation %d",
		len_per_write, fw_param->fw_delayUs, endianness, i2c_operation);

	msleep(200);

	new_fw_ver = *(uint32_t *)&fw_data[APP_FW_SIZE - 12];  /* 0x7FF4 ~ 0x7FF7 */

	vaddr = vmalloc((sizeof(struct cam_sensor_i2c_reg_array) * len_per_write));
	if (!vaddr) {
		CAM_ERR(CAM_OIS,
			"[SEM1218S] Failed in allocating i2c_array: size: %u",
			(sizeof(struct cam_sensor_i2c_reg_array) * len_per_write));
		return -ENOMEM;
	}

	setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (vaddr);
	setting.addr_type   = CAMERA_SENSOR_I2C_TYPE_WORD;
	setting.data_type   = CAMERA_SENSOR_I2C_TYPE_BYTE;
	setting.size        = len_per_write;
	setting.delay       = 0;//delay 0 ms

	// init crc table
	for (uint32_t i = 0; i < 256; i++) {
		uint32_t crc = i << 24;
		// each bit
		for (int j = 0; j < 8; j++) {
			if (crc & 0x80000000ul)
				crc = (crc << 1) ^ POLYNOMIAL;
			else
				crc = crc << 1;
		}
		crc32_table[i] = crc;
	}

	for (wcnt = 0; wcnt < fw_param->fw_size; wcnt += len_per_write) {
		CAM_INFO(CAM_OIS, "[SEM1218S] Write REG_DATA_BUF wcnt = %d", wcnt);
		for (cnt = 0; cnt < len_per_write; cnt++, ptr++) {
			setting.reg_setting[cnt].reg_addr = fw_param->fw_reg_addr + cnt;
			setting.reg_setting[cnt].reg_data = *((uint8_t *)ptr);
			setting.reg_setting[cnt].delay = 0;
			setting.reg_setting[cnt].data_mask = 0;
			CAM_DBG(CAM_OIS, "[SEM1218S] count %d address is: 0x%04x data is 0x%x",
				cnt, setting.reg_setting[cnt].reg_addr, setting.reg_setting[cnt].reg_data);
			//CRC32 calculation
			lookup_idx = ((uint32_t)(check_sum >> 24) ^ (*((uint8_t *)ptr))) & 0xFF;
			check_sum = (check_sum << 8) ^ crc32_table[lookup_idx];
		}
		rc = camera_io_dev_write_continuous(&io_master_info,
				&setting, CAM_SENSOR_I2C_WRITE_SEQ);

		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"[SEM1218S] Failed in Applying i2c wrt settings");
			break;
		}
		msleep(20);
	}

	writebuffer[0] = (uint8_t)(check_sum & 0xFF);
	writebuffer[1] = (uint8_t)(check_sum >> 8 & 0xFF);
	writebuffer[2] = (uint8_t)(check_sum >> 16 & 0xFF);
	writebuffer[3] = (uint8_t)(check_sum >> 24 & 0xFF);
	CAM_INFO(CAM_OIS, "[SEM1218S] check_sum is 0x%04x, writebuffer is 0x%02x 0x%02x 0x%02x 0x%02x",
		check_sum, writebuffer[0], writebuffer[1], writebuffer[2], writebuffer[3]);

	rc = i2c_write_data_seq(&io_master_info, REG_FWUP_CRC, 4, writebuffer, 0);
	if (rc) {
		CAM_ERR(CAM_OIS, "[SEM1218S] Failed to write REG_FWUP_CRC");
	}

	msleep(200);

	rc = i2c_read_data_seq(&io_master_info, REG_FWUP_ERR, 1, readbuffer);
	if (rc) {
		CAM_ERR(CAM_OIS, "[SEM1218S] Failed to read REG_FWUP_ERR");
	}
	if (readbuffer[0] != NO_ERROR) {
		CAM_ERR(CAM_OIS, "[SEM1218S] Failed to update firmware");
	}

	writebuffer[0] = RESET_REQ;
	rc = i2c_write_data_seq(&io_master_info, REG_FWUP_CTRL, 1, writebuffer, 0);
	if (rc) {
		CAM_ERR(CAM_OIS, "[SEM1218S] Failed to write REG_FWUP_CTRL");
	}
	msleep(200);

	rc = i2c_read_data_seq(&io_master_info, REG_APP_VER, 4, readbuffer);
	if (rc) {
		CAM_ERR(CAM_OIS, "[SEM1218S] Failed to read REG_APP_VER");
	}

	updated_ver = *(uint32_t *)readbuffer;
	CAM_INFO(CAM_OIS, "[SEM1218S] firmware version = %d, new firmware version = %d", updated_ver, new_fw_ver);
	goto memory_free;

memory_free:
	vfree(vaddr);
	vaddr = NULL;

	return rc;
}
/*xiaomi modify end*/
static int cam_ois_fw_download_v2(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t                             rc = 0;
	struct cam_cmd_ois_fw_param        *fw_param = NULL;
	uint32_t                            fw_size;
	uint16_t                            len_per_write = 0;
	uint8_t                            *ptr = NULL;
	const struct firmware              *fw = NULL;
	struct device                      *dev;
	uint8_t                             count = 0;
	uint8_t                             cont_wr_flag = 0;
	uint8_t                             fwcnt = 0;
	uint8_t                             endianness = 0;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	if (o_ctrl->i2c_fw_version_data.is_settings_valid == 1) {
		CAM_DBG(CAM_OIS, "check version to decide FW download");
		rc = cam_ois_apply_settings(o_ctrl, &o_ctrl->i2c_fw_version_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD);
		if ((rc == -EAGAIN) &&
			(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
			CAM_WARN(CAM_OIS,
			"CCI HW is resetting: Reapplying FW init settings");
			usleep_range(1000, 1010);
			rc = cam_ois_apply_settings(o_ctrl,
				&o_ctrl->i2c_fw_version_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD);
		}

		if (delete_request(&o_ctrl->i2c_fw_version_data) < 0)
			CAM_WARN(CAM_OIS,
				"Fail deleting i2c_fw_version_data: rc: %d", rc);

		if (rc == I2C_COMPARE_MATCH) {
			/*xiaomi modify begin*/
			if ((FORCE_UPDATE_FIRMWARE == flash_ois_fw_update_op) &&
				(true == o_ctrl->opcode.is_ois_fw_update)) {
				CAM_INFO(CAM_OIS,
					"OIS FW version matched, but force FW download");
			} else {
				CAM_INFO(CAM_OIS,
					"OIS FW version matched, skipping FW download %d %d",
						o_ctrl->opcode.is_ois_fw_update, flash_ois_fw_update_op);
				return rc;
			}
		} else if (rc == I2C_COMPARE_MISMATCH) {
			if ((false == o_ctrl->opcode.is_ois_fw_update) ||
				(SKIP_UPDATE_FIRMWARE == flash_ois_fw_update_op)) {
				CAM_INFO(CAM_OIS, "OIS FW version not matched, but skip update %d %d",
					o_ctrl->opcode.is_ois_fw_update, flash_ois_fw_update_op);
				rc = I2C_COMPARE_MATCH;
				return rc;
			} else {
				CAM_INFO(CAM_OIS, "OIS FW version not matched, load FW");
			}
			/*xiaomi modify end*/
		} else {
			CAM_WARN(CAM_OIS, "OIS FW version check failed,rc=%d", rc);
		}
	}

	if (o_ctrl->fw_info.fw_count > 0) {
		fwcnt      = o_ctrl->fw_info.fw_count;
		fw_param   = &o_ctrl->fw_info.fw_param[0];
		endianness = o_ctrl->fw_info.endianness;
	}

	for (count = 0; count < fwcnt; count++, fw_param++) {
		fw_size       = fw_param->fw_size;
		len_per_write = fw_param->fw_len_per_write / fw_param->fw_data_type;

		CAM_DBG(CAM_OIS, "count: %d, fw_size: %d, data_type: %d, len_per_write: %d",
			count, fw_size, fw_param->fw_data_type, len_per_write);

		/* Load FW */
		dev = &(o_ctrl->pdev->dev);
		//xiaomi add begin
		//customized_ois_flag is 1 mean semco
		if ((1 == o_ctrl->opcode.customized_ois_flag) && (INVALID_FW_ADDR !=o_ctrl->opcode.prog)) {
			snprintf(fw_param->fw_name, OIS_NAME_LEN, "%s.prog", o_ctrl->ois_name);
			CAM_INFO(CAM_OIS, "[SEM1218S] fw_name change to %s", fw_param->fw_name);
		}
		//xiaomi add end
		rc = request_firmware(&fw, fw_param->fw_name, dev);
		if (rc) {
			CAM_ERR(CAM_OIS, "Failed to locate %s", fw_param->fw_name);
			return rc;
		}

		if (0 == rc && NULL != fw &&
			(fw_size <= fw->size - fw_param->fw_start_pos)) {

			/* fw init */
			CAM_DBG(CAM_OIS, "fw init");
			if (o_ctrl->i2c_fw_init_data[count].is_settings_valid == 1) {
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_fw_init_data[count], XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD);
				if ((rc == -EAGAIN) &&
					(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
					CAM_WARN(CAM_OIS,
					"CCI HW is resetting: Reapplying FW init settings");
					usleep_range(1000, 1010);
					rc = cam_ois_apply_settings(o_ctrl,
						&o_ctrl->i2c_fw_init_data[count], XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD);
				}
				if (rc) {
					CAM_ERR(CAM_OIS,
						"Cannot apply FW init settings %d",
						rc);
					goto release_firmware;
				} else {
					CAM_INFO(CAM_OIS, "OIS FW init settings success");//xiaomi modify
				}
			}

			/* send fw */
			CAM_DBG(CAM_OIS, "send fw, operation %d", fw_param->fw_operation);

			ptr = (uint8_t *)(fw->data + fw_param->fw_start_pos);
			if (fw_param->fw_operation == CAMERA_SENSOR_I2C_OP_RNDM_WR)
				cont_wr_flag = CAM_SENSOR_I2C_WRITE_RANDOM;
			else if (fw_param->fw_operation == CAMERA_SENSOR_I2C_OP_CONT_WR_BRST)
				cont_wr_flag = CAM_SENSOR_I2C_WRITE_BURST;
			else if (fw_param->fw_operation == CAMERA_SENSOR_I2C_OP_CONT_WR_SEQN)
				cont_wr_flag = CAM_SENSOR_I2C_WRITE_SEQ;

			//xiaomi add begin
			if (1 == o_ctrl->opcode.customized_ois_flag) {
				write_ois_fw_sem1218s(ptr, (o_ctrl->fw_info.endianness & OIS_ENDIANNESS_MASK_FW),
					fw_param, o_ctrl->io_master_info, cont_wr_flag);
			} else {
				write_ois_fw(ptr, (o_ctrl->fw_info.endianness & OIS_ENDIANNESS_MASK_FW),
					fw_param, o_ctrl->io_master_info, cont_wr_flag);
			}
			//xiaomi add end

			/* fw finalize */
			CAM_DBG(CAM_OIS, "fw finalize");
			if (o_ctrl->i2c_fw_finalize_data[count].is_settings_valid == 1) {
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_fw_finalize_data[count], XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD);
				if ((rc == -EAGAIN) &&
					(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
					CAM_WARN(CAM_OIS,
					"CCI HW is resetting: Reapplying FW finalize settings");
					usleep_range(1000, 1010);
					rc = cam_ois_apply_settings(o_ctrl,
						&o_ctrl->i2c_fw_finalize_data[count], XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD);
				}
				if (rc) {
					CAM_ERR(CAM_OIS,
						"Cannot apply FW finalize settings %d",
						rc);
					goto release_firmware;
				} else {
					CAM_DBG(CAM_OIS, "OIS FW finalize settings success");
				}
			}
		}

		if (fw != NULL) {
			release_firmware(fw);
			fw = NULL;
		}
	}

release_firmware:
	if (fw != NULL) {
		release_firmware(fw);
		fw = NULL;
	}

	return rc;
}

/**
 * cam_ois_pkt_parse - Parse csl packet
 * @o_ctrl:     ctrl structure
 * @arg:        Camera control command argument
 *
 * Returns success or failure
 */
static int cam_ois_pkt_parse(struct cam_ois_ctrl_t *o_ctrl, void *arg)
{
	int32_t                         rc = 0;
	int32_t                         i = 0;
	uint32_t                        total_cmd_buf_in_bytes = 0;
	struct common_header           *cmm_hdr = NULL;
	uintptr_t                       generic_ptr;
	struct cam_control             *ioctl_ctrl = NULL;
	struct cam_config_dev_cmd       dev_config;
	struct i2c_settings_array      *i2c_reg_settings = NULL;
	struct cam_cmd_buf_desc        *cmd_desc = NULL;
	uintptr_t                       generic_pkt_addr;
	size_t                          pkt_len;
	size_t                          remain_len = 0;
	struct cam_packet              *csl_packet = NULL;
	struct cam_packet              *csl_packet_u = NULL;
	size_t                          len_of_buff = 0;
	uint32_t                       *offset = NULL, *cmd_buf;
	struct cam_ois_soc_private     *soc_private =
		(struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t  *power_info = &soc_private->power_info;
	uint32_t                        fwversion = 0;
	struct timespec64                ts1, ts2; // xiaomi add
	long                             microsec = 0; // xiaomi add

	bool    parklens_power_down = true; //xiaomi add
	int32_t parklens_state      = 0;    //xiaomi add

	ioctl_ctrl = (struct cam_control *)arg;
	if (copy_from_user(&dev_config,
		u64_to_user_ptr(ioctl_ctrl->handle),
		sizeof(dev_config)))
		return -EFAULT;
	rc = cam_mem_get_cpu_buf(dev_config.packet_handle,
		&generic_pkt_addr, &pkt_len);
	if (rc) {
		CAM_ERR(CAM_OIS,
			"error in converting command Handle Error: %d", rc);
		return rc;
	}

	remain_len = pkt_len;
	if ((sizeof(struct cam_packet) > pkt_len) ||
		((size_t)dev_config.offset >= pkt_len -
		sizeof(struct cam_packet))) {
		CAM_ERR(CAM_OIS,
			"Inval cam_packet strut size: %zu, len_of_buff: %zu",
			 sizeof(struct cam_packet), pkt_len);
		rc = -EINVAL;
		goto put_ref;
	}

	remain_len -= (size_t)dev_config.offset;
	csl_packet_u = (struct cam_packet *)
		(generic_pkt_addr + (uint32_t)dev_config.offset);
	rc = cam_packet_util_copy_pkt_to_kmd(csl_packet_u, &csl_packet, remain_len);
	if (rc) {
		CAM_ERR(CAM_OIS, "Copying packet to KMD failed");
		goto put_ref;
	}

	switch (csl_packet->header.op_code & 0xFFFFFF) {
	case CAM_OIS_PACKET_OPCODE_INIT:
		CAM_DBG(CAM_OIS, "CAM_OIS_PACKET_OPCODE_INIT,num_cmd_buf %d",
			csl_packet->num_cmd_buf);

		offset = (uint32_t *)&csl_packet->payload_flex;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);

		/* Loop through multiple command buffers */
		for (i = 0; i < csl_packet->num_cmd_buf; i++) {
			rc = cam_packet_util_validate_cmd_desc(&cmd_desc[i]);
			if (rc) {
				CAM_ERR(CAM_OIS, "Invalid cmd desc");
				goto end;
			}

			total_cmd_buf_in_bytes = cmd_desc[i].length;
			if (!total_cmd_buf_in_bytes)
				continue;

			rc = cam_mem_get_cpu_buf(cmd_desc[i].mem_handle,
				&generic_ptr, &len_of_buff);
			if (rc < 0) {
				CAM_ERR(CAM_OIS, "Failed to get cpu buf : 0x%x",
					cmd_desc[i].mem_handle);
				goto end;
			}
			cmd_buf = (uint32_t *)generic_ptr;
			if (!cmd_buf) {
				CAM_ERR(CAM_OIS, "invalid cmd buf");
				rc = -EINVAL;
				cam_mem_put_cpu_buf(cmd_desc[i].mem_handle);
				goto end;
			}

			if ((len_of_buff < sizeof(struct common_header)) ||
				(cmd_desc[i].offset > (len_of_buff -
				sizeof(struct common_header)))) {
				CAM_ERR(CAM_OIS,
					"Invalid length for sensor cmd");
				rc = -EINVAL;
				cam_mem_put_cpu_buf(cmd_desc[i].mem_handle);
				goto end;
			}
			remain_len = len_of_buff - cmd_desc[i].offset;
			cmd_buf += cmd_desc[i].offset / sizeof(uint32_t);
			cmm_hdr = (struct common_header *)cmd_buf;

			CAM_DBG(CAM_OIS,
					"cmm_hdr->cmd_type: %d", cmm_hdr->cmd_type);
			switch (cmm_hdr->cmd_type) {
			case CAMERA_SENSOR_CMD_TYPE_I2C_INFO:
				rc = cam_ois_slaveInfo_pkt_parser(
					o_ctrl, cmd_buf, remain_len);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
					"Failed in parsing slave info");
					break;
				}
				break;
			case CAMERA_SENSOR_CMD_TYPE_PWR_UP:
			case CAMERA_SENSOR_CMD_TYPE_PWR_DOWN:
				CAM_INFO(CAM_OIS,
					"Received power settings buffer");// xiaomi modify
				// xiaomi modify begin
				if (PARKLENS_INVALID != parklens_atomic_read(&(o_ctrl->parklens_ctrl.parklens_state))) {
					ois_parklens_thread_stop(o_ctrl, EXIT_PARKLENS_WITHOUT_POWERDOWN);
					parklens_power_down = ois_parklens_power_down(o_ctrl);

					// Set the last few Settings, Ensure that the ois state is normal
					if(parklens_power_down == false){
						CAM_INFO(MI_PARKLENS, "[OISParklensLog] May be need reset OIS");
					}
					ois_deinit_parklens_info(o_ctrl);
					CAM_DBG(MI_PARKLENS, "[OISParklensLog] stop parklens thread, powerdown:%d",
						parklens_power_down);
				} else {
					CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread is invalid, powerdown:%d",
						parklens_power_down);
				}

				if(parklens_power_down == true) {
				 	CAM_INFO(MI_PARKLENS, "[OISParklensLog] need power up again");
					rc = cam_sensor_update_power_settings(
							cmd_buf,
							total_cmd_buf_in_bytes,
							power_info, remain_len);
					CAM_DBG(MI_PARKLENS, "[OISParklensLog] power up again successed");
					if (rc) {
						CAM_ERR(MI_PARKLENS, "[OISParklensLog] Failed:parse power settings: %d",
							rc);
						return rc;
					}
				} else {
					CAM_INFO(MI_PARKLENS, "[OISParklensLog] no need repower up again");
				}
				// xiaomi modify end
				break;
			case CAMERA_SENSOR_OIS_CMD_TYPE_FW_INFO:
				fwversion = *cmd_buf;
				CAM_DBG(CAM_OIS,
					"Received fwInfo buffer,total_cmd_buf_in_bytes: %d, fwversion: %d",
					total_cmd_buf_in_bytes, fwversion);
				if (fwversion == CAM_OIS_FWINFO_VERSION_2) {
					rc = cam_ois_fw_info_pkt_parser_v2(
						o_ctrl, cmd_buf, total_cmd_buf_in_bytes);
					if (rc) {
						CAM_ERR(CAM_OIS,
						"Failed: parse fw info2 settings");
						break;
					}
				}
				else {
					rc = cam_ois_fw_info_pkt_parser(
						o_ctrl, cmd_buf, total_cmd_buf_in_bytes);
					if (rc) {
						CAM_ERR(CAM_OIS,
						"Failed: parse fw info settings");
						break;
					}
				}
				break;
			default:
			if (o_ctrl->i2c_init_data.is_settings_valid == 0) {
				CAM_DBG(CAM_OIS,
				"Received init/config settings");
				i2c_reg_settings =
					&(o_ctrl->i2c_init_data);
				i2c_reg_settings->is_settings_valid = 1;
				i2c_reg_settings->request_id = 0;
				rc = cam_sensor_i2c_command_parser(
					&o_ctrl->io_master_info,
					i2c_reg_settings,
					&cmd_desc[i], 1, NULL);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
					"init parsing failed: %d", rc);
					break;
				}
			} else if ((o_ctrl->is_ois_calib != 0) &&
				(o_ctrl->i2c_calib_data.is_settings_valid ==
				0)) {
				CAM_DBG(CAM_OIS,
					"Received calib settings");
				i2c_reg_settings = &(o_ctrl->i2c_calib_data);
				i2c_reg_settings->is_settings_valid = 1;
				i2c_reg_settings->request_id = 0;
				rc = cam_sensor_i2c_command_parser(
					&o_ctrl->io_master_info,
					i2c_reg_settings,
					&cmd_desc[i], 1, NULL);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
						"Calib parsing failed: %d", rc);
					break;
				}
			} else if (o_ctrl->i2c_fwinit_data.is_settings_valid == 0) {
				CAM_DBG(CAM_OIS, "received fwinit settings");
				i2c_reg_settings =
					&(o_ctrl->i2c_fwinit_data);
				i2c_reg_settings->is_settings_valid = 1;
				i2c_reg_settings->request_id = 0;
				rc = cam_sensor_i2c_command_parser(
					&o_ctrl->io_master_info,
					i2c_reg_settings,
					&cmd_desc[i], 1, NULL);
				if (rc < 0) {
					CAM_DBG(CAM_OIS,
					"fw init parsing failed: %d", rc);
					break;
				}
			} else if (o_ctrl->i2c_postinit_data.is_settings_valid == 0) {
                          	// xiaomi add begin
				CAM_DBG(CAM_OIS, "received postinit settings");
				i2c_reg_settings =
					&(o_ctrl->i2c_postinit_data);
				i2c_reg_settings->is_settings_valid = 1;
				i2c_reg_settings->request_id = 0;
				rc = cam_sensor_i2c_command_parser(
					&o_ctrl->io_master_info,
					i2c_reg_settings,
					&cmd_desc[i], 1, NULL);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
					"post init parsing failed: %d", rc);
					return rc;
				}
			}else if (o_ctrl->i2c_motion_data.is_settings_valid == 0) {
				CAM_DBG(CAM_OIS, "received motion settings");
				i2c_reg_settings =
					&(o_ctrl->i2c_motion_data);
				i2c_reg_settings->is_settings_valid = 1;
				i2c_reg_settings->request_id = 0;
				rc = cam_sensor_i2c_command_parser(
					&o_ctrl->io_master_info,
					i2c_reg_settings,
					&cmd_desc[i], 1, NULL);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
					"post init parsing failed: %d", rc);
					return rc;
				}
			}// xiaomi add end
			break;
			}
			cam_mem_put_cpu_buf(cmd_desc[i].mem_handle);

			if (rc < 0)
				goto end;
		}

		if (o_ctrl->cam_ois_state != CAM_OIS_CONFIG) {
			// xiaomi modify begin
			if (parklens_power_down == true) {
				CAM_DBG(MI_PARKLENS, "[OISParklensLog] OIS Power up start");
				rc = cam_ois_power_up(o_ctrl);
				if (rc) {
					CAM_ERR(CAM_OIS, " OIS Power up failed");
					goto end;
				}
			}
			// xiaomi modify end
		}

		CAM_DBG(CAM_OIS, "ois_fw_flag: %d", o_ctrl->ois_fw_flag);
		CAM_INFO(CAM_OIS, "customized_ois_flag %d, fw_mem_store %d, is_ois_fw_update %d",
			o_ctrl->opcode.customized_ois_flag, o_ctrl->opcode.fw_mem_store, o_ctrl->opcode.is_ois_fw_update);//xiaomi add
		if (o_ctrl->ois_fw_flag) {
			CAM_DBG(CAM_OIS, "fw_count: %d", o_ctrl->fw_info.fw_count);
			//if (o_ctrl->fw_info.fw_count != 0) { //xiaomi modify
			if ((o_ctrl->fw_info.fw_count != 0) && (0 != o_ctrl->opcode.customized_ois_flag)) { //xiaomi modify
				/* xiaomi add begin */
				CAM_GET_TIMESTAMP(ts1);
				CAM_DBG(MI_PERF, "%s start firmware download", o_ctrl->ois_name);
				/* xiaomi add end */
				rc = cam_ois_fw_download_v2(o_ctrl);
				if (rc) {
					CAM_ERR(CAM_OIS, "Failed OIS FW Download v2");
					goto pwr_dwn;
				}
				/* xiaomi add begin */
				CAM_GET_TIMESTAMP(ts2);
				CAM_GET_TIMESTAMP_DIFF_IN_MICRO(ts1, ts2, microsec);
				CAM_DBG(MI_PERF, "%s end firmware download, occupy time is: %ld ms",
					o_ctrl->ois_name, microsec/1000);
				/* xiaomi add end */
			} else {
				if (o_ctrl->i2c_fwinit_data.is_settings_valid == 1) {
					/* xiaomi dev protection modified*/
					rc = cam_ois_apply_settings(o_ctrl,
						&o_ctrl->i2c_fwinit_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD);
					/* xiaomi dev protection modified*/
					if ((rc == -EAGAIN) &&
						(o_ctrl->io_master_info.master_type ==
							CCI_MASTER)) {
						CAM_WARN(CAM_OIS,
							"Reapplying fwinit settings");
						usleep_range(1000, 1010);
						/* xiaomi dev protection modified*/
						rc = cam_ois_apply_settings(o_ctrl,
							&o_ctrl->i2c_fwinit_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD);
						/* xiaomi dev protection modified*/
					}
					if (rc) {
						CAM_ERR(CAM_OIS,
							"Cannot apply fwinit data %d",
							rc);
						goto pwr_dwn;
					} else {
						CAM_DBG(CAM_OIS, "OIS fwinit settings success");
					}
					/* xiaomi add begin */
					CAM_GET_TIMESTAMP(ts1);
					CAM_DBG(MI_PERF, "%s start firmware download", o_ctrl->ois_name);
					/* xiaomi add end */
					rc = cam_ois_fw_download(o_ctrl, MAXOIS_TRANS_SIZE);//xiaomi add
					//rc = cam_ois_fw_download(o_ctrl);
					if (rc) {
						CAM_ERR(CAM_OIS, "Failed OIS FW Download");
						goto pwr_dwn;
					}
					/* xiaomi add begin */
					CAM_GET_TIMESTAMP(ts2);
					CAM_GET_TIMESTAMP_DIFF_IN_MICRO(ts1, ts2, microsec);
					CAM_DBG(MI_PERF, "%s end firmware download, occupy time is: %ld ms",
						o_ctrl->ois_name, microsec/1000);
					/* xiaomi add end */
				}
			}
		}
		if (o_ctrl->i2c_init_data.is_settings_valid == 1)
		{
			/* xiaomi dev protection modified*/
			rc = cam_ois_apply_settings(o_ctrl, &o_ctrl->i2c_init_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_INIT);
			/* xiaomi dev protection modified*/
			if ((rc == -EAGAIN) &&
				(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
				CAM_WARN(CAM_OIS,
					"CCI HW is restting: Reapplying INIT settings");
				usleep_range(1000, 1010);
				/* xiaomi dev protection modified*/
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_init_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_INIT);
				/* xiaomi dev protection modified*/
			//xiaomi add begin
			} else if (I2C_COMPARE_MISMATCH == rc) {
				if (o_ctrl->i2c_fwinit_data.is_settings_valid == 1) {
					rc = cam_ois_apply_settings(o_ctrl,
						&o_ctrl->i2c_fwinit_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_FW_DOWNLOAD);
					if (rc) {
						CAM_ERR(CAM_OIS,
							"Cannot apply fwinit data %d",
							rc);
						goto pwr_dwn;
					} else {
						CAM_DBG(CAM_OIS, "OIS fwinit settings success");
					}
					rc = cam_ois_fw_download(o_ctrl, MINOIS_TRANS_SIZE);
					if (rc) {
						CAM_ERR(CAM_OIS, "Failed OIS FW Download");
						goto pwr_dwn;
					}
				}
				rc = cam_ois_apply_settings(o_ctrl, &o_ctrl->i2c_init_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_INIT);
			}
			//xiaomi add end

			if (rc < 0) {
				CAM_ERR(CAM_OIS,
					"Cannot apply Init settings: rc = %d",
					rc);
				goto pwr_dwn;
			} else {
				CAM_DBG(CAM_OIS, "apply Init settings success");
			}
		}

		if (o_ctrl->is_ois_calib) {
			/* xiaomi dev protection modified*/
			rc = cam_ois_apply_settings(o_ctrl,
				&o_ctrl->i2c_calib_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_CALIBRATION);
			/* xiaomi dev protection modified*/
			if ((rc == -EAGAIN) &&
				(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
				CAM_WARN(CAM_OIS,
					"CCI HW is restting: Reapplying calib settings");
				usleep_range(1000, 1010);
				/* xiaomi dev protection modified*/
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_calib_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_CALIBRATION);
				/* xiaomi dev protection modified*/
			}
			if (rc) {
				CAM_ERR(CAM_OIS, "Cannot apply calib data");
				goto pwr_dwn;
			} else {
				CAM_DBG(CAM_OIS, "apply calib data settings success");
			}
		}

		o_ctrl->cam_ois_state = CAM_OIS_CONFIG;

		rc = delete_request(&o_ctrl->i2c_fwinit_data);
		if (rc < 0)
			CAM_WARN(CAM_OIS,
				"Fail deleting fwinit data: rc: %d", rc);

		for (i = 0; i < o_ctrl->fw_info.fw_count; i++) {
			if (o_ctrl->i2c_fw_init_data &&
				(o_ctrl->i2c_fw_init_data[i].is_settings_valid == 1)) {
				rc = delete_request(&o_ctrl->i2c_fw_init_data[i]);
				if (rc < 0)
					CAM_WARN(CAM_OIS,
						"Fail deleting i2c_fw_init_data: rc: %d", rc);
			}
			if (o_ctrl->i2c_fw_finalize_data &&
				(o_ctrl->i2c_fw_finalize_data[i].is_settings_valid == 1)) {
				rc = delete_request(&o_ctrl->i2c_fw_finalize_data[i]);
				if (rc < 0)
					CAM_WARN(CAM_OIS,
						"Fail deleting i2c_fw_finalize_data: rc: %d", rc);
			}
		}
		if (o_ctrl->i2c_fw_init_data){
			CAM_MEM_FREE(o_ctrl->i2c_fw_init_data);
			o_ctrl->i2c_fw_init_data = NULL;
		}
		if (o_ctrl->i2c_fw_finalize_data){
			CAM_MEM_FREE(o_ctrl->i2c_fw_finalize_data);
			o_ctrl->i2c_fw_finalize_data = NULL;
		}
		if (o_ctrl->fw_info.fw_param) {
			CAM_MEM_FREE(o_ctrl->fw_info.fw_param);
			o_ctrl->fw_info.fw_param = NULL;
		}

		// xiaomi add begin
		if (o_ctrl->i2c_postinit_data.is_settings_valid == 1) {
			/* xiaomi dev protection modified*/
			rc = cam_ois_apply_settings(o_ctrl,
				&o_ctrl->i2c_postinit_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_POSTINIT);
			/* xiaomi dev protection modified*/
			if ((rc == -EAGAIN) &&
				(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
				CAM_WARN(CAM_OIS,
					"CCI HW is restting: Reapplying postinit settings");
				usleep_range(1000, 1010);
				/* xiaomi dev protection modified*/
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_postinit_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_POSTINIT);
				/* xiaomi dev protection modified*/
			}
			if (rc) {
				CAM_ERR(CAM_OIS,
					"Cannot apply postinit data %d",
					rc);
				goto pwr_dwn;
			} else {
				CAM_DBG(CAM_OIS, "OIS postinit settings success");
			}
		}

		rc = delete_request(&o_ctrl->i2c_postinit_data);
		if (rc < 0) {
			CAM_WARN(CAM_OIS,
				"Fail deleting postinit data: rc: %d", rc);
			rc = 0;
		}

		if (o_ctrl->i2c_motion_data.is_settings_valid == 1) {
			/* xiaomi dev protection modified*/
			rc = cam_ois_apply_settings(o_ctrl,
				&o_ctrl->i2c_motion_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_MOTION);
			/* xiaomi dev protection modified*/
			if ((rc == -EAGAIN) &&
				(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
				CAM_WARN(CAM_OIS,
					"CCI HW is restting: Reapplying postinit settings");
				usleep_range(1000, 1010);
				/* xiaomi dev protection modified*/
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_motion_data, XM_CMD_DEV_I2C_OIS_CMD_TYPE_MOTION);
				/* xiaomi dev protection modified*/
			}
			if (rc) {
				CAM_ERR(CAM_OIS,
					"Cannot apply motion data %d",
					rc);
				goto pwr_dwn;
			} else {
				CAM_DBG(CAM_OIS, "OIS motion settings success");
			}
		}

		rc = delete_request(&o_ctrl->i2c_motion_data);
		if (rc < 0) {
			CAM_WARN(CAM_OIS,
				"Fail deleting motion data: rc: %d", rc);
			rc = 0;
		}
		// xiaomi add end

		rc = delete_request(&o_ctrl->i2c_init_data);
		if (rc < 0)
			CAM_WARN(CAM_OIS,
				"Fail deleting Init data: rc: %d", rc);

		rc = delete_request(&o_ctrl->i2c_calib_data);
		if (rc < 0) {
			CAM_WARN(CAM_OIS,
				"Fail deleting Calibration data: rc: %d", rc);
			rc = 0;
		}
		break;
	case CAM_OIS_PACKET_OPCODE_OIS_CONTROL:
		CAM_DBG(CAM_OIS, "CAM_OIS_PACKET_OPCODE_OIS_CONTROL");
		if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
				"Not in right state to control OIS: %d",
				o_ctrl->cam_ois_state);
			goto end;
		}
		offset = (uint32_t *)&csl_packet->payload_flex;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);
		i2c_reg_settings = &(o_ctrl->i2c_mode_data);
		i2c_reg_settings->is_settings_valid = 1;
		i2c_reg_settings->request_id = 0;
		rc = cam_sensor_i2c_command_parser(&o_ctrl->io_master_info,
			i2c_reg_settings,
			cmd_desc, 1, NULL);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "OIS pkt parsing failed: %d", rc);
			goto end;
		}

		/* xiaomi dev protection modified*/
		rc = cam_ois_apply_settings(o_ctrl, i2c_reg_settings, XM_CMD_DEV_I2C_OIS_CMD_TYPE_OIS_CONTROL);
		/* xiaomi dev protection modified*/
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "Cannot apply mode settings");
			goto end;
		}

		rc = delete_request(i2c_reg_settings);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"Fail deleting Mode data: rc: %d", rc);
			goto end;
		}
		break;
	case CAM_OIS_PACKET_OPCODE_READ: {
		uint64_t qtime_ns;
		struct cam_buf_io_cfg *io_cfg;
		struct i2c_settings_array i2c_read_settings;

		CAM_DBG(CAM_OIS, "CAM_OIS_PACKET_OPCODE_READ");

		if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
				"Not in right state to read OIS: %d",
				o_ctrl->cam_ois_state);
			goto end;
		}
		CAM_DBG(CAM_OIS, "number of I/O configs: %d:",
			csl_packet->num_io_configs);
		if (csl_packet->num_io_configs == 0) {
			CAM_ERR(CAM_OIS, "No I/O configs to process");
			rc = -EINVAL;
			goto end;
		}

		INIT_LIST_HEAD(&(i2c_read_settings.list_head));

		io_cfg = (struct cam_buf_io_cfg *) ((uint8_t *)
			&csl_packet->payload_flex +
			csl_packet->io_configs_offset);

		/* validate read data io config */
		if (io_cfg == NULL) {
			CAM_ERR(CAM_OIS, "I/O config is invalid(NULL)");
			rc = -EINVAL;
			goto end;
		}

		offset = (uint32_t *)&csl_packet->payload_flex;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);
		i2c_read_settings.is_settings_valid = 1;
		i2c_read_settings.request_id = 0;
		rc = cam_sensor_i2c_command_parser(&o_ctrl->io_master_info,
			&i2c_read_settings,
			cmd_desc, 1, &io_cfg[0]);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "OIS read pkt parsing failed: %d", rc);
			goto end;
		}

		mutex_lock(&(o_ctrl->read_buf_lock));
		rc = cam_sensor_util_add_read_buf_to_list(&(o_ctrl->read_buf_list),
			io_cfg->mem_handle[0]);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "Add read buf to list failed rc:%d", rc);
			mutex_unlock(&(o_ctrl->read_buf_lock));
			goto end;
		}
		mutex_unlock(&(o_ctrl->read_buf_lock));

		rc = cam_sensor_util_get_current_qtimer_ns(&qtime_ns);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "failed to get qtimer rc:%d");
			goto end;
		}

		// xiaomi change begin
		/*rc = cam_sensor_i2c_read_data(
			&i2c_read_settings,
			&o_ctrl->io_master_info);*/
			rc = cam_sensor_i2c_read_and_write_data(
				&i2c_read_settings,
				&o_ctrl->io_master_info);
		// xiaomi change end

		if (rc < 0) {
			CAM_ERR(CAM_OIS, "cannot read data rc: %d", rc);
			delete_request(&i2c_read_settings);
			goto end;
		}

		if (csl_packet->num_io_configs > 1) {
			rc = cam_sensor_util_write_qtimer_to_io_buffer(
				qtime_ns, &io_cfg[1]);
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
					"write qtimer failed rc: %d", rc);
				delete_request(&i2c_read_settings);
				goto end;
			}
		}

		rc = delete_request(&i2c_read_settings);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"Failed in deleting the read settings");
			goto end;
		}
		break;
	}
	case CAM_OIS_PACKET_OPCODE_WRITE_TIME: {
		CAM_DBG(CAM_OIS,
				"CAM_OIS_PACKET_OPCODE_WRITE_TIME");
		if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
			rc = -EINVAL;
			CAM_ERR(CAM_OIS,
				"Not in right state to write time to OIS: %d",
				o_ctrl->cam_ois_state);
			goto end;
		}
		offset = (uint32_t *)&csl_packet->payload_flex;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);
		i2c_reg_settings = &(o_ctrl->i2c_time_data);
		i2c_reg_settings->is_settings_valid = 1;
		i2c_reg_settings->request_id = 0;
		rc = cam_sensor_i2c_command_parser(&o_ctrl->io_master_info,
			i2c_reg_settings,
			cmd_desc, 1, NULL);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "OIS pkt parsing failed: %d", rc);
			goto end;
		}

		if ((o_ctrl->fw_info.fw_count > 0) && (0 != o_ctrl->opcode.customized_ois_flag)) {//xiaomi modify
			uint8_t ois_endianness =
				(o_ctrl->fw_info.endianness & OIS_ENDIANNESS_MASK_INPUTPARAM) >> 4;
			rc = cam_ois_update_time(i2c_reg_settings, ois_endianness);
		} else
			rc = cam_ois_update_time(i2c_reg_settings, CAM_ENDIANNESS_LITTLE);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "Cannot update time");
			goto end;
		}

		/* xiaomi dev protection modified*/
		rc = cam_ois_apply_settings(o_ctrl, i2c_reg_settings, XM_CMD_DEV_I2C_OIS_CMD_TYPE_WRITE_TIME);
		/* xiaomi dev protection modified*/
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "Cannot apply mode settings");
			goto end;
		}

		rc = delete_request(i2c_reg_settings);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"Fail deleting Mode data: rc: %d", rc);
			goto end;
		}
		break;
	}
	// xiaomi modify begin
	case CAM_OIS_PACKET_OPCODE_OIS_PARKLENS:{
		int32_t exit_result = parklens_atomic_read(&(o_ctrl->parklens_ctrl.exit_result));

		parklens_state = parklens_atomic_read(
							&(o_ctrl->parklens_ctrl.parklens_state));
		CAM_INFO(MI_PARKLENS, "[OISParklensLog] Received parklens buffer parklens_state %d",
			parklens_state);
		if ((parklens_state != PARKLENS_INVALID) ||
			(o_ctrl->cam_ois_state < CAM_OIS_CONFIG)) {
			rc = -EINVAL;
			CAM_WARN(MI_PARKLENS,
				"[OISParklensLog] Not in right state to do parklens: %d/%d, exit result %d release power control right",
				o_ctrl->cam_ois_state,
				parklens_state, exit_result);
			ois_parklens_thread_stop(o_ctrl, EXIT_PARKLENS_WITH_POWERDOWN);
			ois_deinit_parklens_info(o_ctrl);
			return rc;
		}

		i2c_reg_settings = &(o_ctrl->i2c_parklens_data);

		i2c_reg_settings->request_id = 0;
		i2c_reg_settings->is_settings_valid = 1;
		offset   = (uint32_t *)&csl_packet->payload;
		offset  += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);

		rc = cam_sensor_i2c_command_parser(
				&o_ctrl->io_master_info,
				i2c_reg_settings,
				cmd_desc, 1, NULL);

		if (rc < 0) {
			CAM_ERR(MI_PARKLENS, "[OISParklensLog] Failed:parse parklens settings: %d",
				rc);
			delete_request(i2c_reg_settings);
			return rc;
		}

		ois_parklens_thread_trigger(o_ctrl);
		break;
	}
	// xiaomi modify end
	default:
		CAM_ERR(CAM_OIS, "Invalid Opcode: %d",
			(csl_packet->header.op_code & 0xFFFFFF));
		rc = -EINVAL;
		goto end;
	}

	if (!rc)
		goto end;

pwr_dwn:
	cam_ois_power_down(o_ctrl);
end:
	cam_common_mem_free(csl_packet);
put_ref:
	cam_mem_put_cpu_buf(dev_config.packet_handle);
	return rc;
}

void cam_ois_shutdown(struct cam_ois_ctrl_t *o_ctrl)
{
	int rc = 0, i = 0;
	struct cam_ois_soc_private *soc_private =
		(struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t *power_info = &soc_private->power_info;

	/* xiaomi add begin */
	if (PARKLENS_INVALID !=
		parklens_atomic_read(&(o_ctrl->parklens_ctrl.parklens_state))) {
		ois_parklens_thread_stop(o_ctrl, EXIT_PARKLENS_WITH_POWERDOWN);
		CAM_INFO(MI_PARKLENS,
			"[OISParklensLog] exit parklens with powerdown in shutdown");
	} else {
		CAM_INFO(MI_PARKLENS,
			"[OISParklensLog] parklens is invalid in shutdown");
	}
	/* xiaomi add end */

	if (o_ctrl->cam_ois_state == CAM_OIS_INIT)
		return;

	/* xiaomi add begin */
	if (o_ctrl->cam_ois_state >= CAM_OIS_CONFIG) {
		if(false == ois_parklens_power_down(o_ctrl)) {
			rc = cam_ois_power_down(o_ctrl);
			if (rc < 0)
				CAM_ERR(CAM_OIS, "OIS Power down failed");
			CAM_INFO(MI_PARKLENS, "[OISParklensLog] parklens isn't powerdown");
		} else {
			CAM_INFO(MI_PARKLENS, "[OISParklensLog] parklens has been powerdown");
		}
	} else {
		CAM_INFO(MI_PARKLENS,
			"[OISParklensLog] shut down but not in CONFIG, parklens powerdown: %d",
			ois_parklens_power_down(o_ctrl));
	}
	/* xiaomi add end*/

	if (o_ctrl->cam_ois_state >= CAM_OIS_ACQUIRE) {
		rc = cam_destroy_device_hdl(o_ctrl->bridge_intf.device_hdl);
		if (rc < 0)
			CAM_ERR(CAM_OIS, "destroying the device hdl");
		// o_ctrl->bridge_intf.device_hdl = -1;
		// o_ctrl->bridge_intf.link_hdl = -1;
		// o_ctrl->bridge_intf.session_hdl = -1;
	}
	/* xiaomi add begin */
	o_ctrl->bridge_intf.device_hdl  = -1;
	o_ctrl->bridge_intf.link_hdl    = -1;
	o_ctrl->bridge_intf.session_hdl = -1;
	/* xiaomi add end*/
	if (o_ctrl->i2c_fwinit_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_fwinit_data);

	for (i = 0; i < o_ctrl->fw_info.fw_count; i++) {
		if (o_ctrl->i2c_fw_init_data &&
			(o_ctrl->i2c_fw_init_data[i].is_settings_valid == 1)) {
			rc = delete_request(&o_ctrl->i2c_fw_init_data[i]);
			if (rc < 0)
				CAM_WARN(CAM_OIS,
					"Fail deleting i2c_fw_init_data: rc: %d", rc);
		}
		if (o_ctrl->i2c_fw_finalize_data &&
			(o_ctrl->i2c_fw_finalize_data[i].is_settings_valid == 1)) {
			rc = delete_request(&o_ctrl->i2c_fw_finalize_data[i]);
			if (rc < 0)
				CAM_WARN(CAM_OIS,
					"Fail deleting i2c_fw_finalize_data: rc: %d", rc);
		}
	}

	if (o_ctrl->i2c_fw_version_data.is_settings_valid == 1) {
		rc = delete_request(&o_ctrl->i2c_fw_version_data);
		if (rc < 0)
			CAM_WARN(CAM_OIS,
				"Fail deleting i2c_fw_version_data: rc: %d", rc);
	}

	if (o_ctrl->i2c_mode_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_mode_data);

	if (o_ctrl->i2c_calib_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_calib_data);

	if (o_ctrl->i2c_init_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_init_data);

	// xiaomi add begin
	if (o_ctrl->i2c_postinit_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_postinit_data);

	if (o_ctrl->i2c_motion_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_motion_data);

	if (PARKLENS_INVALID != parklens_atomic_read(&(o_ctrl->parklens_ctrl.parklens_state))) {
		ois_deinit_parklens_info(o_ctrl);
		CAM_INFO(MI_PARKLENS,
			"[OISParklensLog] parklens is not valid, deinit parklens info");
	}
	// xiaomi add end

	if (o_ctrl->i2c_fw_init_data){
		CAM_MEM_FREE(o_ctrl->i2c_fw_init_data);
		o_ctrl->i2c_fw_init_data = NULL;
	}
	if (o_ctrl->i2c_fw_finalize_data){
		CAM_MEM_FREE(o_ctrl->i2c_fw_finalize_data);
		o_ctrl->i2c_fw_finalize_data = NULL;
	}
	if (o_ctrl->fw_info.fw_param) {
		CAM_MEM_FREE(o_ctrl->fw_info.fw_param);
		o_ctrl->fw_info.fw_param = NULL;
	}
	CAM_MEM_FREE(power_info->power_setting);
	CAM_MEM_FREE(power_info->power_down_setting);
	power_info->power_setting = NULL;
	power_info->power_down_setting = NULL;
	power_info->power_down_setting_size = 0;
	power_info->power_setting_size = 0;

	o_ctrl->cam_ois_state = CAM_OIS_INIT;
}

/**
 * cam_ois_driver_cmd - Handle ois cmds
 * @e_ctrl:     ctrl structure
 * @arg:        Camera control command argument
 *
 * Returns success or failure
 */
int cam_ois_driver_cmd(struct cam_ois_ctrl_t *o_ctrl, void *arg)
{
	int                              rc = 0, i = 0;
	struct cam_ois_query_cap_t       ois_cap = {0};
	struct cam_control              *cmd = (struct cam_control *)arg;
	struct cam_ois_soc_private      *soc_private = NULL;
	struct cam_sensor_power_ctrl_t  *power_info = NULL;
	bool                            parklens_power_down = true;   //xiaomi add

	if (!o_ctrl || !cmd) {
		CAM_ERR(CAM_OIS, "Invalid arguments");
		return -EINVAL;
	}

	if (cmd->handle_type != CAM_HANDLE_USER_POINTER) {
		CAM_ERR(CAM_OIS, "Invalid handle type: %d",
			cmd->handle_type);
		return -EINVAL;
	}

	soc_private =
		(struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
	power_info = &soc_private->power_info;

	mutex_lock(&(o_ctrl->ois_mutex));
	switch (cmd->op_code) {
	case CAM_QUERY_CAP:
		ois_cap.slot_info = o_ctrl->soc_info.index;

		if (copy_to_user(u64_to_user_ptr(cmd->handle),
			&ois_cap,
			sizeof(struct cam_ois_query_cap_t))) {
			CAM_ERR(CAM_OIS, "Failed Copy to User");
			rc = -EFAULT;
			goto release_mutex;
		}
		CAM_DBG(CAM_OIS, "ois_cap: ID: %d", ois_cap.slot_info);
		break;
	case CAM_ACQUIRE_DEV:
		rc = cam_ois_get_dev_handle(o_ctrl, arg);
		if (rc) {
			CAM_ERR(CAM_OIS, "Failed to acquire dev");
			goto release_mutex;
		}

		o_ctrl->cam_ois_state = CAM_OIS_ACQUIRE;
		/* xiaomi dev protection add*/
		xm_cam_dev_set_init_result(get_ois_xm_cam_dev_info(o_ctrl), XM_CAM_DEV_INIT_STATUS_SUCCESS);
		/* xiaomi dev protection add*/
		break;
	case CAM_START_DEV:
		if (o_ctrl->cam_ois_state != CAM_OIS_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
			"Not in right state for start : %d",
			o_ctrl->cam_ois_state);
			goto release_mutex;
		}
		o_ctrl->cam_ois_state = CAM_OIS_START;
		break;
	case CAM_CONFIG_DEV:
		rc = cam_ois_pkt_parse(o_ctrl, arg);
		if (rc) {
			CAM_ERR(CAM_OIS, "Failed in ois pkt Parsing");
			goto release_mutex;
		}
		break;
	case CAM_RELEASE_DEV:
		/* xiaomi dev protection add*/
		xm_cam_dev_set_init_result(get_ois_xm_cam_dev_info(o_ctrl), XM_CAM_DEV_INIT_STATUS_SUCCESS);
		/* xiaomi dev protection add*/
		if (o_ctrl->cam_ois_state == CAM_OIS_START) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
				"Cant release ois: in start state");
			goto release_mutex;
		}

		//xiaomi modify begin
		parklens_power_down = ois_parklens_power_down(o_ctrl);
		CAM_INFO(MI_PARKLENS, "[OISParklensLog] CAM_RELEASE_DEV is_parklens_power_down status %d/%d",
			parklens_power_down,
			o_ctrl->cam_ois_state);
		if (o_ctrl->cam_ois_state == CAM_OIS_CONFIG) {
			if(parklens_power_down == false){
				CAM_DBG(CAM_OIS, "cam_ois_power_down");
				rc = cam_ois_power_down(o_ctrl);
				if (rc < 0) {
					CAM_ERR(CAM_OIS, "OIS Power down failed");
					goto release_mutex;
				}
			}
		}
		//xiaomi modify end

		if (o_ctrl->bridge_intf.device_hdl == -1) {
			CAM_ERR(CAM_OIS, "link hdl: %d device hdl: %d",
				o_ctrl->bridge_intf.device_hdl,
				o_ctrl->bridge_intf.link_hdl);
			rc = -EINVAL;
			goto release_mutex;
		}
		rc = cam_destroy_device_hdl(o_ctrl->bridge_intf.device_hdl);
		if (rc < 0)
			CAM_ERR(CAM_OIS, "destroying the device hdl");
		o_ctrl->bridge_intf.device_hdl = -1;
		o_ctrl->bridge_intf.link_hdl = -1;
		o_ctrl->bridge_intf.session_hdl = -1;
		o_ctrl->cam_ois_state = CAM_OIS_INIT;

		//xiaomi modify begin
		if (parklens_power_down == false) {
			CAM_MEM_FREE(power_info->power_setting);
			CAM_MEM_FREE(power_info->power_down_setting);
			power_info->power_setting = NULL;
			power_info->power_down_setting = NULL;
			power_info->power_down_setting_size = 0;
			power_info->power_setting_size = 0;
		}
		//xiaomi modify end

		if (o_ctrl->i2c_mode_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_mode_data);

		if (o_ctrl->i2c_calib_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_calib_data);

		if (o_ctrl->i2c_init_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_init_data);

		if (o_ctrl->i2c_fwinit_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_fwinit_data);

		// xiaomi add begin
		if (o_ctrl->i2c_postinit_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_postinit_data);

		if (o_ctrl->i2c_motion_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_motion_data);
		// xiaomi add end

		for (i = 0; i < o_ctrl->fw_info.fw_count; i++) {
			if (o_ctrl->i2c_fw_init_data &&
				(o_ctrl->i2c_fw_init_data[i].is_settings_valid == 1)) {
				rc = delete_request(&o_ctrl->i2c_fw_init_data[i]);
				if (rc < 0) {
					CAM_WARN(CAM_OIS,
						"Fail deleting i2c_fw_init_data: rc: %d", rc);
					rc = 0;
				}
			}
			if (o_ctrl->i2c_fw_finalize_data &&
				(o_ctrl->i2c_fw_finalize_data[i].is_settings_valid == 1)) {
				rc = delete_request(&o_ctrl->i2c_fw_finalize_data[i]);
				if (rc < 0) {
					CAM_WARN(CAM_OIS,
						"Fail deleting i2c_fw_finalize_data: rc: %d", rc);
					rc = 0;
				}
			}
		}

		//xiaomi add begain
		if (true == o_ctrl->opcode.fw_mem_store) {
			cam_fw_mem_store(o_ctrl);
		}
		//xiaomi add end

		if (o_ctrl->i2c_fw_init_data){
			CAM_MEM_FREE(o_ctrl->i2c_fw_init_data);
			o_ctrl->i2c_fw_init_data = NULL;
		}
		if (o_ctrl->i2c_fw_finalize_data){
			CAM_MEM_FREE(o_ctrl->i2c_fw_finalize_data);
			o_ctrl->i2c_fw_finalize_data = NULL;
		}
		if (o_ctrl->fw_info.fw_param) {
			CAM_MEM_FREE(o_ctrl->fw_info.fw_param);
			o_ctrl->fw_info.fw_param = NULL;
		}
		break;
	case CAM_STOP_DEV:
		if (o_ctrl->cam_ois_state != CAM_OIS_START) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
				"Not in right state for stop : %d",
				o_ctrl->cam_ois_state);
			goto release_mutex;
		}
		o_ctrl->cam_ois_state = CAM_OIS_CONFIG;
		break;
	case CAM_FLUSH_REQ:
		CAM_DBG(CAM_OIS, "Flush recveived");
		break;
	default:
		CAM_ERR(CAM_OIS, "invalid opcode: %d", cmd->op_code);
		goto release_mutex;
	}
release_mutex:
	mutex_lock(&(o_ctrl->read_buf_lock));
	cam_sensor_util_release_read_buf(&(o_ctrl->read_buf_list));
	mutex_unlock(&(o_ctrl->read_buf_lock));
	mutex_unlock(&(o_ctrl->ois_mutex));
	return rc;
}

//xiaomi add begain
void ois_fw_version_set(uint64_t fw_version, char* name)
{
	bool rc = false;
	int i;

	for (i = 0; i < MAX_OIS_DEV; i++)
	{
		if (false == ois_firmware[i].is_valid ||
			0 == strcmp(ois_firmware[i].name, name))
		{
			strcpy(ois_firmware[i].name, name);
			ois_firmware[i].fw_version = fw_version;
			ois_firmware[i].is_valid = true;
			rc = true;
			break;
		}
	}

	CAM_INFO(MI_DEBUG, "index:%d cach version:%s, name:%s version:0x%llx",
		i, (rc == true)?"Y":"N", name, fw_version);
}

static int cam_ois_fw_set(const char *kmessage,
	const struct kernel_param *kp)
{
	return 0;
}

static int cam_ois_fw_get(char *buffer,
	const struct kernel_param *kp)
{
	uint16_t buff_max_size = CAM_OIS_FW_VERSION_MAX_LENGTH;
	uint16_t ret = 0, i;

	for (i=0; (ois_firmware[i].is_valid == true && i < MAX_OIS_DEV); i++)
	{
		ret += scnprintf(buffer+ret, buff_max_size, "%s : 0x%llx\n",
					ois_firmware[i].name,
					ois_firmware[i].fw_version);
	}
	return ret;
}

static const struct kernel_param_ops cam_ois_fw_ops = {
	.set = cam_ois_fw_set,
	.get = cam_ois_fw_get
};

module_param_cb(cam_ois_fw, &cam_ois_fw_ops, NULL, 0644);

static int32_t cam_ois_i2c_modes_util(
		struct camera_io_master *io_master_info,
		struct i2c_settings_list *i2c_list)
{
	int32_t rc = 0;
	uint32_t i, size;

	if (i2c_list->op_code ==  CAM_SENSOR_I2C_WRITE_RANDOM) {
		rc = camera_io_dev_write((io_master_info),
			&(i2c_list->i2c_settings));

		CAM_DBG(MI_PARKLENS, "[OISParklensLog] write addr: %x  data: %x delay %d",
		i2c_list->i2c_settings.reg_setting->reg_addr,
		i2c_list->i2c_settings.reg_setting->reg_data,
		i2c_list->i2c_settings.delay);

		if (rc < 0) {
			CAM_ERR(MI_PARKLENS, "[OISParklensLog] Failed in Applying i2c wrt settings");
			return rc;
		}
	} else if (i2c_list->op_code == CAM_SENSOR_I2C_WRITE_SEQ) {
		rc = camera_io_dev_write_continuous(
			(io_master_info),
			&(i2c_list->i2c_settings),
			CAM_SENSOR_I2C_WRITE_SEQ);

		CAM_DBG(MI_PARKLENS, "[OISParklensLog] write continuous addr: %x  data: %x delay %d",
		i2c_list->i2c_settings.reg_setting->reg_addr,
		i2c_list->i2c_settings.reg_setting->reg_data,
		i2c_list->i2c_settings.delay);

		if (rc < 0) {
			CAM_ERR(MI_PARKLENS, "[OISParklensLog] Failed to seq write I2C settings: %d",
				rc);
			return rc;
		}
	} else if (i2c_list->op_code == CAM_SENSOR_I2C_POLL) {
		size = i2c_list->i2c_settings.size;
		for (i = 0; i < size; i++) {
			rc = camera_io_dev_poll(
			(io_master_info),
			i2c_list->i2c_settings.reg_setting[i].reg_addr,
			i2c_list->i2c_settings.reg_setting[i].reg_data,
			i2c_list->i2c_settings.reg_setting[i].data_mask,
			i2c_list->i2c_settings.addr_type,
			i2c_list->i2c_settings.data_type,
			i2c_list->i2c_settings.reg_setting[i].delay);

			CAM_DBG(MI_PARKLENS, "[OISParklensLog] poll  addr: %x  data: %x delay %d",
			i2c_list->i2c_settings.reg_setting[i].reg_addr,
			i2c_list->i2c_settings.reg_setting[i].reg_data,
			i2c_list->i2c_settings.reg_setting[i].delay);

			if (rc < 0) {
				CAM_ERR(MI_PARKLENS, "[OISParklensLog] i2c poll apply setting Fail");
				return rc;
			}
		}
	}
	return rc;
}

static int32_t ois_parklens_thread_func(void *arg)
{
    struct cam_ois_parklens_ctrl_t *parklens_ctrl = NULL;
	struct cam_ois_soc_private  *soc_private = NULL;
	struct cam_sensor_power_ctrl_t *power_info = NULL;
	struct cam_ois_ctrl_t *o_ctrl = NULL;
	struct i2c_settings_array *i2c_set  = NULL;
	struct i2c_settings_list *i2c_list = NULL;
	struct i2c_settings_list *i2c_list_last = NULL;
	int    ois_parklens_time_with_ms = 100, delaytimes = 0, i=0;
	int32_t parklens_opcode = 0, rc = 0;

	if (!arg) {
		rc = -EINVAL;
		parklens_atomic_set(&(parklens_ctrl->exit_result),
			PARKLENS_EXIT_CREATE_WAKELOCK_FAILED);
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] ois_parklens_thread_func arg is NULL");
	} else {
		o_ctrl = (struct cam_ois_ctrl_t *)arg;
		parklens_ctrl = &(o_ctrl->parklens_ctrl);
		soc_private = (struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
		i2c_set     = (struct i2c_settings_array *)(&(o_ctrl->i2c_parklens_data));
		power_info = &soc_private->power_info;
	}

	parklens_event_set(&(parklens_ctrl->start_event));
	if (rc < 0)
		goto exit_without_powerdown;

	CAM_INFO(MI_PARKLENS, "[OISParklensLog] parklens thread start up");

	while (ois_parklens_time_with_ms > 0) {
		parklens_opcode = parklens_atomic_read(&(parklens_ctrl->parklens_opcode));
		if (parklens_opcode >= EXIT_PARKLENS_WITH_POWERDOWN) {
			if (parklens_opcode == EXIT_PARKLENS_WITHOUT_POWERDOWN) {
				CAM_DBG(MI_PARKLENS, "[OISParklensLog] exit parklens thread no need power off");
				parklens_atomic_set(
					&(parklens_ctrl->exit_result),
					PARKLENS_EXIT_WITHOUT_POWERDOWN);
				goto exit_without_powerdown;
			} else if (parklens_opcode == EXIT_PARKLENS_WITH_POWERDOWN) {
				CAM_DBG(MI_PARKLENS, "[OISParklensLog] exit parklens thread no need power off");
				parklens_atomic_set(
					&(parklens_ctrl->exit_result),
					PARKLENS_EXIT_WITH_POWERDOWN);
				goto exit_with_powerdown;
			} else {
				CAM_WARN(MI_PARKLENS, "[OISParklensLog] Invalid opcode for parklens %d", parklens_opcode);
				continue;
			}
		}

		if (i2c_set == NULL) {
			CAM_ERR(MI_PARKLENS, "[OISParklensLog] parklens Invalid parklens settings");
			parklens_atomic_set(
				&(parklens_ctrl->exit_result),
				PARKLENS_EXIT_WITHOUT_PARKLENS);
			goto exit_with_powerdown;
		} else if (i2c_set->is_settings_valid != 1) {
			CAM_ERR(MI_PARKLENS, "[OISParklensLog] parklens settings is not valid");
			parklens_atomic_set(
				&(parklens_ctrl->exit_result),
				PARKLENS_EXIT_WITHOUT_PARKLENS);
			goto exit_with_powerdown;
		} else {
			if(!delaytimes){
				i2c_list_last = i2c_list;

				if (i2c_list == NULL)
					i2c_list = list_first_entry(&(i2c_set->list_head), typeof(*i2c_list), list);
				else
					i2c_list = list_next_entry(i2c_list, list);

				if ((i2c_list_last != NULL) && list_entry_is_head(i2c_list, &(i2c_set->list_head), list)) {
					CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens settings execute done");
					parklens_atomic_set(
						&(parklens_ctrl->exit_result),
						PARKLENS_EXIT_WITH_POWERDOWN);
					goto exit_with_powerdown;
				} else {
					delaytimes = i2c_list->i2c_settings.delay;
					i2c_list->i2c_settings.delay = 0;

					for (i = 0; i < (i2c_list->i2c_settings.size); i++) {
						CAM_DBG(MI_PARKLENS, "[OISParklensLog] Address 0x%x data 0x%x delay time %d ms",
							i2c_list->i2c_settings.reg_setting[i].reg_addr,
							i2c_list->i2c_settings.reg_setting[i].reg_data,
							delaytimes);
					}

					rc = cam_ois_i2c_modes_util(
						&(o_ctrl->io_master_info),
						i2c_list);
					if (rc < 0) {
						CAM_ERR(MI_PARKLENS, "[OISParklensLog] parklens step failed, %d", rc);
						parklens_atomic_set(
							&(parklens_ctrl->exit_result),
							PARKLENS_EXIT_CCI_ERROR);
						goto exit_with_powerdown;
					} else {
						CAM_DBG(MI_PARKLENS, "[OISParklensLog] ois_parklens_thread_func arg time(ms)/state %d/%d",
							delaytimes,
							parklens_ctrl->parklens_state);
					}
				}
			}
			else{
				--delaytimes;
				usleep_range(990,1010);
				CAM_DBG(MI_PARKLENS, "[OISParklensLog] delay time %d ms",
						delaytimes);
			}
		}
		ois_parklens_time_with_ms--;
		if (0 == ois_parklens_time_with_ms){
			CAM_INFO(MI_PARKLENS, "[OISParklensLog] ois parklens time exceed! power down");
				parklens_atomic_set(
					&(parklens_ctrl->exit_result),
					PARKLENS_EXIT_WITH_POWERDOWN);
		}
	}

exit_with_powerdown:
	CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread exit with power down");
	CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread remaining time(ms)/result %d/%d",
		ois_parklens_time_with_ms,
		parklens_atomic_read(&(parklens_ctrl->exit_result)));

	if(o_ctrl->io_master_info.master_type == CCI_MASTER)
	{
		lock_power_sync_mutex(o_ctrl->io_master_info.cci_client, o_ctrl->io_master_info.cci_client->cci_i2c_master);
		rc = cam_ois_power_down(o_ctrl);
		unlock_power_sync_mutex(o_ctrl->io_master_info.cci_client, o_ctrl->io_master_info.cci_client->cci_i2c_master);

	}
	else
	{
		lock_power_sync_mutex(o_ctrl->io_master_info.cci_client, MASTER_0);
		rc = cam_ois_power_down(o_ctrl);
		unlock_power_sync_mutex(o_ctrl->io_master_info.cci_client, MASTER_0);
	}

	if (rc < 0) {
		CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens power down failed rc: %d", rc);
		parklens_atomic_set(
			&(parklens_ctrl->exit_result),
			PARKLENS_EXIT_WITH_POWERDOWN_FAILED);
	}
	else
		CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread  power down success");

	kfree(power_info->power_setting);
	kfree(power_info->power_down_setting);
	power_info->power_setting = NULL;
	power_info->power_down_setting = NULL;
	power_info->power_setting_size = 0;
	power_info->power_down_setting_size = 0;

exit_without_powerdown:
	if (parklens_atomic_read(&(parklens_ctrl->exit_result)) <=
		PARKLENS_EXIT_WITHOUT_POWERDOWN) {
		CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread exit without power down");
		CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread exit time(ms)/result %d/%d",
			ois_parklens_time_with_ms,
			parklens_atomic_read(&(parklens_ctrl->exit_result)));
	}

	CAM_INFO(MI_PARKLENS, "[OISParklensLog] parklens thread delete request");
	rc = delete_request(&(o_ctrl->i2c_parklens_data));
	if (rc < 0)
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] delete parklens request failed rc: %d", rc);

	parklens_event_set(&(parklens_ctrl->shutdown_event));
	parklens_exit_thread(true);

	return 0;
}
//xiaomi add end

// xiaomi add begin
int32_t init_ois_parklens_info(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t rc = 0;
	struct cam_ois_parklens_ctrl_t *parklens_ctrl;

	if(!o_ctrl) {
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] failed: o_ctrl %pK", o_ctrl);
		return -EINVAL;
	}

	parklens_ctrl = &(o_ctrl->parklens_ctrl);

	CAM_DBG(MI_PARKLENS, "[OISParklensLog] init parklens: %s %s", o_ctrl->device_name,o_ctrl->ois_name);

	parklens_atomic_set(&(parklens_ctrl->parklens_opcode),
		ENTER_PARKLENS_WITH_POWERDOWN);
	parklens_atomic_set(&(parklens_ctrl->exit_result),
		PARKLENS_ENTER);
	parklens_atomic_set(&(parklens_ctrl->parklens_state),
		PARKLENS_INVALID);

	rc = parklens_event_create(&(parklens_ctrl->start_event));
	if (rc < 0) {
		CAM_ERR(MI_PARKLENS,
			"[OISParklensLog] failed to create start event for parklens");
		return rc;
	}


	rc = parklens_event_create(&(parklens_ctrl->shutdown_event));
	if (rc < 0) {
		CAM_ERR(MI_PARKLENS,
			"[OISParklensLog] failed to create shutdown event for parklens");
		return rc;
	}

	parklens_ctrl->parklens_thread = NULL;
	parklens_atomic_set(&(parklens_ctrl->parklens_state), PARKLENS_STOP);

	return 0;
}

int32_t ois_deinit_parklens_info(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t rc = 0;
	struct cam_ois_parklens_ctrl_t *parklens_ctrl;

	if(!o_ctrl) {
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] failed: o_ctrl %pK", o_ctrl);
		return -EINVAL;
	}

	CAM_DBG(MI_PARKLENS, "[OISParklensLog] deinit parklens: %s", o_ctrl->device_name);

	parklens_ctrl = &(o_ctrl->parklens_ctrl);

	if (PARKLENS_STOP !=
		parklens_atomic_read(&(parklens_ctrl->parklens_state))) {
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] deinit parklens in wrong state");
		return -EINVAL;
	}

	rc = parklens_event_destroy(&(parklens_ctrl->start_event));
	if (rc < 0)
		CAM_ERR(MI_PARKLENS,
			"[OISParklensLog] failed to destroy start event for parklens");

	rc = parklens_event_destroy(&(parklens_ctrl->shutdown_event));
	if (rc < 0)
		CAM_ERR(MI_PARKLENS,
			"[OISParklensLog] failed to destroy shutdown event for parklens");

	parklens_atomic_set(&(parklens_ctrl->parklens_opcode),
		ENTER_PARKLENS_WITH_POWERDOWN);
	parklens_atomic_set(&(parklens_ctrl->exit_result),
		PARKLENS_ENTER);
	parklens_atomic_set(&(parklens_ctrl->parklens_state),
		PARKLENS_INVALID);

	return 0;
}

bool ois_parklens_power_down(struct cam_ois_ctrl_t *o_ctrl)
{
	bool    is_power_down = false;
	int32_t parklens_state = 0;
	struct  cam_ois_parklens_ctrl_t *parklens_ctrl;

	if(!o_ctrl) {
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] failed: o_ctrl %pK", o_ctrl);
		return false;
	}

	parklens_ctrl = &(o_ctrl->parklens_ctrl);
	parklens_state = parklens_atomic_read(&(parklens_ctrl->parklens_state));

	switch (parklens_state) {
	case PARKLENS_RUNNING: {
		CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens is running, no need powerdown");
		is_power_down = true;
	}
		break;

	case PARKLENS_STOP: {
		if (parklens_atomic_read(&(parklens_ctrl->exit_result)) <= PARKLENS_EXIT_WITHOUT_POWERDOWN) {
			CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens stop without power down");
			is_power_down = false;
		} else {
			is_power_down = true;
		}
	}
		break;

	case PARKLENS_INVALID: {
		CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens is not created, power down");
		is_power_down = false;
	}
		break;

	default:
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] Invalid parklens_state %d", parklens_state);
	}

	return is_power_down;
}

int32_t ois_parklens_thread_trigger(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t rc = 0;
	struct cam_ois_parklens_ctrl_t *parklens_ctrl;

	if(!o_ctrl) {
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] failed: o_ctrl %pK", o_ctrl);
		return -EINVAL;;
	}

	rc = init_ois_parklens_info(o_ctrl);
	if(rc < 0){
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] failed to init parklens info %d", rc);
		return rc;
	}
	parklens_ctrl = &(o_ctrl->parklens_ctrl);

	/* intialize parameters of parklens*/
	parklens_atomic_set(&(parklens_ctrl->parklens_opcode),
		ENTER_PARKLENS_WITH_POWERDOWN);
	parklens_atomic_set(&(parklens_ctrl->exit_result),
		PARKLENS_ENTER);
	parklens_atomic_set(&(parklens_ctrl->parklens_state),
		PARKLENS_STOP);

	parklens_ctrl->parklens_thread =
					parklens_thread_run(ois_parklens_thread_func,
										o_ctrl,
										"parklens-thread");

	if (!(parklens_ctrl->parklens_thread)) {
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] parklens_thread create failed");
		rc = -ENOMEM;
		goto deinit_parklens;
	} else {
		CAM_DBG(MI_PARKLENS, "[OISParklensLog] %s trigger parklens thread",o_ctrl->ois_name);
		parklens_wait_single_event(&(parklens_ctrl->start_event),0);

		rc = parklens_atomic_read(&(parklens_ctrl->exit_result));
		if (rc > PARKLENS_ENTER) {
			CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread execute failed %d", rc);
			goto clear_thread;
		} else
			parklens_atomic_set(&(parklens_ctrl->parklens_state),
								PARKLENS_RUNNING);
	}

	return rc;

clear_thread:
	parklens_wait_single_event(&(parklens_ctrl->shutdown_event),0);
	rc = parklens_atomic_read(&(parklens_ctrl->exit_result));
	CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread exit: %d", rc);

	parklens_atomic_set(&(parklens_ctrl->parklens_state),PARKLENS_STOP);
	parklens_ctrl->parklens_thread = NULL;
	return rc;

deinit_parklens:
	ois_deinit_parklens_info(o_ctrl);
	return rc;

}


int32_t ois_parklens_thread_stop(
	struct cam_ois_ctrl_t *o_ctrl,
	enum parklens_opcodes opcode)
{
	int32_t exit_result = 0;
	int32_t parklens_state = 0;
	struct cam_ois_parklens_ctrl_t *parklens_ctrl;

	if(!o_ctrl) {
		CAM_ERR(MI_PARKLENS, "[OISParklensLog] failed: o_ctrl %pK", o_ctrl);
		return -EINVAL;
	}

	parklens_ctrl = &(o_ctrl->parklens_ctrl);

	parklens_state = parklens_atomic_read(&(parklens_ctrl->parklens_state)) ;
	exit_result = parklens_atomic_read(&(parklens_ctrl->exit_result));

	if (parklens_state != PARKLENS_RUNNING) {
		CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread is in other state: %d", parklens_state);
		return -EINVAL;
	} else {
		if(exit_result != PARKLENS_ENTER) {
			CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread is already end: %d", exit_result);
		} else {
			if ((opcode == EXIT_PARKLENS_WITH_POWERDOWN) ||
				(opcode == EXIT_PARKLENS_WITHOUT_POWERDOWN)) {
				parklens_atomic_set(&(parklens_ctrl->parklens_opcode), opcode);
			} else {
				CAM_DBG(MI_PARKLENS, "[OISParklensLog] Invalid stop opcode, stop parklens with power down");
				parklens_atomic_set(&(parklens_ctrl->parklens_opcode),
									EXIT_PARKLENS_WITH_POWERDOWN);
			}
			CAM_DBG(MI_PARKLENS, "[OISParklensLog] wake up parklens thread to stop it");
		}

		parklens_wait_single_event(&(parklens_ctrl->shutdown_event), 0);

		parklens_atomic_set(&(parklens_ctrl->parklens_state), PARKLENS_STOP);
		parklens_ctrl->parklens_thread = NULL;

		exit_result = parklens_atomic_read(&(parklens_ctrl->exit_result));
		if(exit_result) {
			CAM_INFO(MI_PARKLENS, "[OISParklensLog] parklens thread exit status: %d", exit_result);
		} else
			CAM_DBG(MI_PARKLENS, "[OISParklensLog] parklens thread exit failed ! ! !");
	}

	return exit_result;
}


//xiaomi add end
