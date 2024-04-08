// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "xiaomi_flash_ois.h"
#include "sem1217s.h"
#include "cam_parklens_thread.h" //xiaomi add

static int semco_ois_fw_update_op = 1;
module_param(semco_ois_fw_update_op, int, 0644);

int32_t cam_ois_construct_default_power_setting(
	struct cam_sensor_power_ctrl_t *power_info)
{
	int rc = 0;

	power_info->power_setting_size = 1;
	power_info->power_setting =
		kzalloc(sizeof(struct cam_sensor_power_setting),
			GFP_KERNEL);
	if (!power_info->power_setting)
		return -ENOMEM;

	power_info->power_setting[0].seq_type = SENSOR_VAF;
	power_info->power_setting[0].seq_val = CAM_VAF;
	power_info->power_setting[0].config_val = 1;
	power_info->power_setting[0].delay = 2;

	power_info->power_down_setting_size = 1;
	power_info->power_down_setting =
		kzalloc(sizeof(struct cam_sensor_power_setting),
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
	kfree(power_info->power_setting);
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
	CAM_DBG(CAM_OIS,"%s cam_ois_power_up end",o_ctrl->ois_name);
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
	/* xiaomi add begin */
	CAM_GET_TIMESTAMP(ts2);
	CAM_GET_TIMESTAMP_DIFF_IN_MICRO(ts1, ts2, microsec);
	CAM_DBG(MI_DEBUG, "%s end power_down, occupy time is: %ld ms",
		o_ctrl->ois_name, microsec/1000);
	CAM_DBG(CAM_OIS,"%s cam_ois_power_down end",o_ctrl->ois_name);
	/* xiaomi add end */

	return rc;
}

static int cam_ois_update_time(struct i2c_settings_array *i2c_set)
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

	list_for_each_entry(i2c_list,
		&(i2c_set->list_head), list) {
		if (i2c_list->op_code ==  CAM_SENSOR_I2C_WRITE_SEQ) {
			size = i2c_list->i2c_settings.size;
			/* qtimer is 8 bytes so validate here*/
			if (size < 8) {
				CAM_ERR(CAM_OIS, "Invalid write time settings");
				return -EINVAL;
			}
			for (i = 0; i < size; i++) {
				CAM_DBG(CAM_OIS, "time: reg_data[%d]: 0x%x",
					i, (qtime_ns & 0xFF));
				i2c_list->i2c_settings.reg_setting[i].reg_data =
					(qtime_ns & 0xFF);
				qtime_ns >>= 8;
			}
		}
	}

	return rc;
}

static int cam_ois_apply_settings(struct cam_ois_ctrl_t *o_ctrl,
	struct i2c_settings_array *i2c_set)
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

	list_for_each_entry(i2c_list,
		&(i2c_set->list_head), list) {
		/* xiaomi add I2C trace begin */
		switch (i2c_list->op_code) {
		case CAM_SENSOR_I2C_WRITE_RANDOM:
		case CAM_SENSOR_I2C_WRITE_BURST:
		case CAM_SENSOR_I2C_WRITE_SEQ: {
			for (j = 0;j < i2c_list->i2c_settings.size;j++) {
				trace_cam_i2c_write_log_event("[OISSETTINGS]", o_ctrl->ois_name,
					i2c_set->request_id, j, "WRITE", i2c_list->i2c_settings.reg_setting[j].reg_addr,
					i2c_list->i2c_settings.reg_setting[j].reg_data);
			}
			break;
		}
		case CAM_SENSOR_I2C_READ_RANDOM:
		case CAM_SENSOR_I2C_READ_SEQ: {
			for (j = 0;j < i2c_list->i2c_settings.size;j++) {
				trace_cam_i2c_write_log_event("[OISSETTINGS]", o_ctrl->ois_name,
					i2c_set->request_id, j, "READ", i2c_list->i2c_settings.reg_setting[j].reg_addr,
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
				}
			}
		}
	}

	return rc;
}

#define OIS_TRANS_SIZE 256

static int cam_ois_slaveInfo_pkt_parser(struct cam_ois_ctrl_t *o_ctrl,
	uint32_t *cmd_buf, size_t len)
{
	int32_t rc = 0;
	struct cam_cmd_ois_info *ois_info;

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
		o_ctrl->io_master_info.client->addr = ois_info->slave_addr;
		CAM_DBG(CAM_OIS, "Slave addr: 0x%x", ois_info->slave_addr);
	} else {
		CAM_ERR(CAM_OIS, "Invalid Master type : %d",
			o_ctrl->io_master_info.master_type);
		rc = -EINVAL;
	}

	return rc;
}

static int cam_default_ois_fw_download(struct cam_ois_ctrl_t *o_ctrl)
{
	uint16_t                           total_bytes = 0;
	uint8_t                           *ptr = NULL;
	int32_t                            rc = 0, cnt, i, j;
	uint32_t                           fw_size;
	const struct firmware             *fw = NULL;
	const char                        *fw_name_prog = NULL;
	const char                        *fw_name_coeff = NULL;
	const char                        *fw_name_mem = NULL;
	char                               name_prog[64] = {0};
	char                               name_coeff[64] = {0};
	char                               name_mem[64] = {0};
	struct device                     *dev = &(o_ctrl->pdev->dev);
	struct cam_sensor_i2c_reg_setting  i2c_reg_setting;
	void                              *vaddr = NULL;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return -EINVAL;
	}

	snprintf(name_coeff, 64, "%s.coeff", o_ctrl->ois_name);

	snprintf(name_prog, 64, "%s.prog", o_ctrl->ois_name);

	snprintf(name_mem, 64, "%s.mem", o_ctrl->ois_name);

	/* cast pointer as const pointer*/
	fw_name_prog = name_prog;
	fw_name_coeff = name_coeff;
	fw_name_mem = name_mem;

	/* Load FW */
	rc = request_firmware(&fw, fw_name_prog, dev);
	if (rc) {
		CAM_ERR(CAM_OIS, "Failed to locate %s", fw_name_prog);
		return rc;
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
		release_firmware(fw);
		return -ENOMEM;
	}

	CAM_DBG(CAM_OIS, "FW prog size:%d.", total_bytes);

	i2c_reg_setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (
		vaddr);

	for (i = 0, ptr = (uint8_t *)fw->data, j = 0; i < total_bytes;) {
		for (cnt = 0; cnt < OIS_TRANS_SIZE && i < total_bytes;
			cnt++, ptr++, i++) {
				i2c_reg_setting.reg_setting[cnt].reg_addr =
					o_ctrl->opcode.prog + j * OIS_TRANS_SIZE;
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
			CAM_ERR(CAM_OIS, "OIS FW(prog) size(%d) download failed. %d",
				total_bytes, rc);
			goto release_firmware;
		}
	}
	vfree(vaddr);
	vaddr = NULL;
	fw_size = 0;
	release_firmware(fw);

	rc = request_firmware(&fw, fw_name_coeff, dev);
	if (rc) {
		CAM_ERR(CAM_OIS, "Failed to locate %s", fw_name_coeff);
		return rc;
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
		release_firmware(fw);
		return -ENOMEM;
	}

	CAM_DBG(CAM_OIS, "FW coeff size:%d", total_bytes);

	i2c_reg_setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (
		vaddr);

	for (i = 0, ptr = (uint8_t *)fw->data, j = 0; i < total_bytes;) {
		for (cnt = 0; cnt < OIS_TRANS_SIZE && i < total_bytes;
			cnt++, ptr++, i++) {
				i2c_reg_setting.reg_setting[cnt].reg_addr =
					o_ctrl->opcode.coeff + j * OIS_TRANS_SIZE;
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
			CAM_ERR(CAM_OIS, "OIS FW(coeff) size(%d) download failed rc: %d",
				total_bytes, rc);
			goto release_firmware;
		}
	}
	vfree(vaddr);
	vaddr = NULL;
	fw_size = 0;
	release_firmware(fw);

	rc = request_firmware(&fw, fw_name_mem, dev);
	if (rc) {
		CAM_ERR(CAM_OIS, "Failed to locate %s", fw_name_mem);
		return rc;
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
		release_firmware(fw);
		return -ENOMEM;
	}

	CAM_DBG(CAM_OIS, "FW mem size:%d", total_bytes);

	i2c_reg_setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (
		vaddr);

	for (i = 0, ptr = (uint8_t *)fw->data, j = 0; i < total_bytes;) {
		for (cnt = 0; cnt < OIS_TRANS_SIZE && i < total_bytes;
			cnt++, ptr++, i++) {
				i2c_reg_setting.reg_setting[cnt].reg_addr =
					o_ctrl->opcode.memory + j * OIS_TRANS_SIZE;
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

		if (rc < 0)
			CAM_ERR(CAM_OIS, "OIS FW(mem) size(%d) download failed rc: %d",
				total_bytes, rc);
	}

release_firmware:
	vfree(vaddr);
	vaddr = NULL;
	fw_size = 0;
	release_firmware(fw);
	return rc;
}

static int cam_sem1217s_ois_fw_download(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t             txdata[TX_BUFFER_SIZE] = {0};
	uint8_t             rxdata[RX_BUFFER_SIZE] = {0};
	uint16_t            tx_buff_size = 0;
	uint16_t            i = 0;
	uint16_t            chk_index = 0;
	uint16_t            idx = 0;
	uint16_t            check_sum = 0;
	uint32_t            updated_ver = 0;
	uint32_t            new_fw_ver = 0;
	uint32_t            current_fw_ver = 0;
	char                *fw_name_prog = NULL;
	char                name_prog[64] = {0};
	uint8_t             *chk_buffer = NULL;
	uint8_t             *fw_data = NULL;
	int32_t             rc = 0;
	uint32_t            current_fw_ver_temp = 0;

	chk_buffer = vmalloc(APP_FW_SIZE);
	fw_data = vmalloc(APP_FW_SIZE);

	if (NULL == chk_buffer || NULL == fw_data) {
		rc = -ENOMEM;
		goto memory_free;
	}

	/* Get FW Ver from Binary File */
	snprintf(name_prog, 64, "%s.prog", o_ctrl->ois_name);
	fw_name_prog = name_prog;

	rc = load_fw_buff(o_ctrl, fw_name_prog, fw_data, APP_FW_SIZE);

	if (rc) {
		CAM_ERR(CAM_OIS, "[SEM1217S] Failed to load firmware: %s", fw_name_prog);
		goto memory_free;
	}

	new_fw_ver = *(uint32_t *)&fw_data[APP_FW_SIZE - 12];  /* 0x7FF4 ~ 0x7FF7 */

	rc = i2c_read_data(o_ctrl, REG_APP_VER, 4, rxdata);

	if (rc) {

		CAM_ERR(CAM_OIS, "[SEM1217S] Failed to read REG_APP_VER:0x%x", REG_APP_VER);
		rc = -EIO;
		goto memory_free;

	}

	current_fw_ver = *(uint32_t *)rxdata;
	CAM_INFO(CAM_OIS,
		"[SEM1217S] Current firmware version = %d, new firmware version = %d",
		current_fw_ver, new_fw_ver);

	if( ((current_fw_ver < new_fw_ver) && (0 == semco_ois_fw_update_op)) ||
		((current_fw_ver != new_fw_ver) && (FIRMWARE_UPDATE_FORCED == semco_ois_fw_update_op)) ||
		(FIRMWARE_UPDATE_EVERY_TIMES == semco_ois_fw_update_op)) {

		/* If there is firmware that needs to be updated, turn off OIS and AF */
		if (0 != current_fw_ver) {

			rc = i2c_read_data(o_ctrl, REG_OIS_STS, 1, rxdata); /* Read REG_OIS_STS */

			if (rc) {
				CAM_ERR(CAM_OIS,
					"[SEM1217S] Failed to read REG_OIS_STS:0x%x",
					REG_OIS_STS);
				rc = -EIO;
				goto memory_free;
			}

			if (rxdata[0] != STATE_READY) {
				txdata[0] = OIS_OFF; /* Set OIS_OFF */
				/* Write REG_OIS_CTRL information */
				rc = i2c_write_data(o_ctrl, REG_OIS_CTRL, 1, txdata, 0);
				if (rc) {
					CAM_ERR(CAM_OIS,
						"[SEM1217S] Failed to set REG_OIS_CTRL:0x%x,0x%x",
						REG_OIS_CTRL, txdata[0]);
					rc = -EIO;
					goto memory_free;
				}
			}

			rc = i2c_read_data(o_ctrl, REG_AF_STS, 1, rxdata); /* Read REG_AF_STS */

			if (rc) {
				CAM_ERR(CAM_OIS,
					"[SEM1217S] Failed to read REG_AF_STS:0x%x",
					REG_AF_STS);
				rc = -EIO;
				goto memory_free;
			}

			if (rxdata[0] != STATE_READY) {
				txdata[0] = AF_OFF; /* Set AF_OFF */
 				/* Write REG_AF_CTRL information */
				rc = i2c_write_data(o_ctrl, REG_AF_CTRL, 1, txdata, 0);
				if (rc) {
					CAM_ERR(CAM_OIS,
						"[SEM1217S] Failed to set REG_AF_CTRL:0x%x,0x%x",
						REG_AF_CTRL, txdata[0]);
					rc = -EIO;
					goto memory_free;
				}
			}
		}

		/* PAYLOAD_LEN = Packet size, FW_UPEN = TRUE */
		tx_buff_size = TX_SIZE_256_BYTE;
		switch (tx_buff_size) {
			case TX_SIZE_32_BYTE:
				txdata[0] = FWUP_CTRL_32_SET;
				break;
			case TX_SIZE_64_BYTE:
				txdata[0] = FWUP_CTRL_64_SET;
				break;
			case TX_SIZE_128_BYTE:
				txdata[0] = FWUP_CTRL_128_SET;
				break;
			case TX_SIZE_256_BYTE:
				txdata[0] = FWUP_CTRL_256_SET;
				break;
			default:
				/* Tx data size is not set, warning message */
				break;
		}

		/* Set the firmware version update control register */
		rc = i2c_write_data(o_ctrl, REG_FWUP_CTRL, 1, txdata, 0);
		if (rc) {
			CAM_ERR(CAM_OIS,
                                "[SEM1217S] Failed to set REG_FWUP_CTRL:0x%x,0x%x",
                                REG_AF_CTRL, txdata[0]);
			rc = -EIO;
			goto memory_free;
		}

		msleep(60);

		rc = i2c_read_data(o_ctrl, REG_OIS_STS, 1, rxdata);
		if (rc) {
			CAM_ERR(CAM_OIS, "[SEM1217S] Failed to read REG_OIS_STS:0x%x", REG_OIS_STS);
			rc = -EIO;
			goto memory_free;
		}

		if (STATE_FW_UPDATE != rxdata[0]) {
			CAM_INFO(CAM_OIS, "[SEM1217S] OIS firmware upgrade status check failed");
			rc = -EINVAL;
			goto memory_free;
		}

		rc = i2c_read_data(o_ctrl, REG_APP_VER, 4, rxdata);
		if (rc) {
			CAM_ERR(CAM_OIS, "[SEM1217S] Failed to read REG_APP_VER:0x%x", REG_APP_VER);
			rc = -EIO;
			goto memory_free;
		}

		current_fw_ver_temp = *(uint32_t *)rxdata;

		if (0x00 != current_fw_ver_temp) {
			CAM_ERR(CAM_OIS, "[SEM1217S] OIS firmware version check failed");
			goto memory_free;
		}

		for (i = 0; i < (APP_FW_SIZE / tx_buff_size); i++) {

			CAM_INFO(CAM_OIS, "[SEM1217S] Write REG_DATA_BUF i = %d",i);
			memcpy(&chk_buffer[tx_buff_size * i], &fw_data[idx], tx_buff_size);

			for (chk_index = 0; chk_index < tx_buff_size; chk_index += 2) {
				check_sum += ((chk_buffer[chk_index + 1 + (tx_buff_size * i)] << 8) |
				chk_buffer[chk_index + (tx_buff_size * i)]);
			}

			memcpy(txdata, &fw_data[idx], tx_buff_size);

			rc = i2c_write_data(o_ctrl, REG_DATA_BUF, tx_buff_size, txdata, 0);
			if (rc) {
				CAM_ERR(CAM_OIS,
					"[SEM1217S] Failed to write REG_DATA_BUF:0x%x, index = %d",
					REG_DATA_BUF, i);
				rc = -EIO;
				goto memory_free;
			}
			idx += tx_buff_size;
			msleep(20);

		}

		*(uint16_t *)txdata = check_sum;

		rc = i2c_write_data(o_ctrl, REG_FWUP_CHKSUM, 2, txdata, 0);
		if (rc) {
			CAM_ERR(CAM_OIS,
				"[SEM1217S] Failed to set REG_FWUP_CTRL:0x%x,0x%x",
				REG_AF_CTRL, txdata[0]);
			rc = -EIO;
			goto memory_free;
		}

		msleep(200);

		rc = i2c_read_data(o_ctrl, REG_FWUP_CHKSUM, 2, rxdata);
		if (rc) {
			CAM_ERR(CAM_OIS,
				"[SEM1217S] Failed to read REG_FWUP_CHKSUM:0x%x",
				REG_FWUP_CHKSUM);
			rc = -EIO;
			goto memory_free;
		}

		CAM_INFO(CAM_OIS, "[SEM1217S] REG_FWUP_CHKSUM = 0x%x, 0x%x", rxdata[0], rxdata[1]);

		rc = i2c_read_data(o_ctrl, REG_FWUP_ERR, 1, rxdata);
		if (rc) {
			CAM_ERR(CAM_OIS, "[SEM1217S] Failed to read REG_FWUP_ERR:0x%x", REG_FWUP_ERR);
			rc = -EIO;
			goto memory_free;
		}

		CAM_INFO(CAM_OIS, "[SEM1217S] REG_FWUP_ERR = 0x%x", rxdata[0]);

		if (rxdata[0] != NO_ERROR) {
			CAM_ERR(CAM_OIS, "[SEM1217S] Failed to update firmware");
			rc = -EINVAL;
			goto memory_free;
		}

		txdata[0] = RESET_REQ;
		rc = i2c_write_data(o_ctrl, REG_FWUP_CTRL, 1, txdata, 0);
		if (rc) {
			CAM_ERR(CAM_OIS,
				"[SEM1217S] Failed to set REG_FWUP_CTRL:0x%x,0x%x",
				REG_FWUP_CTRL, txdata[0]);
			rc = -EIO;
			goto memory_free;
		}

		msleep(200);

		rc = i2c_read_data(o_ctrl, REG_APP_VER, 4, rxdata);
		if (rc) {
			CAM_ERR(CAM_OIS, "[SEM1217S] Failed to read REG_APP_VER:0x%x", REG_APP_VER);
			rc = -EIO;
			goto memory_free;
		}

		updated_ver = *(uint32_t *)rxdata;
		CAM_INFO(CAM_OIS,
			"[SEM1217S] firmware version = %d, new firmware version = %d",
			updated_ver, new_fw_ver);

		if (updated_ver != new_fw_ver) {
			CAM_ERR(CAM_OIS, "[SEM1217S] updated_ver != new_fw_ver");
			rc = -EINVAL;
			goto memory_free;
		}

		CAM_INFO(CAM_OIS, "[SEM1217S] Firmware update success");
	}

memory_free:

	if (NULL != chk_buffer) {
		vfree(chk_buffer);
	}

	if (NULL != fw_data) {
		vfree(fw_data);
	}

	return rc;
}

static int cam_ois_fw_download(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t   rc  = 0;

	switch (o_ctrl->opcode.fw_download_type){
		case 1217:
			CAM_DBG(CAM_OIS, "Apply sem1217s OIS firmware update function");
			rc = cam_sem1217s_ois_fw_download(o_ctrl);
			break;

		default:
			CAM_DBG(CAM_OIS, "Apply default firmware update function");
			rc = cam_default_ois_fw_download(o_ctrl);
			break;
	}

	return rc;
}

// xiaomi add
static int32_t cam_ois_i2c_modes_util(
		struct camera_io_master *io_master_info,
		struct i2c_settings_list *i2c_list)
{
	int32_t rc = 0;
	uint32_t i, size;

	if (i2c_list->op_code ==  CAM_SENSOR_I2C_WRITE_RANDOM) {
		rc = camera_io_dev_write((io_master_info),
			&(i2c_list->i2c_settings));

		CAM_DBG(CAM_OIS,"write addr: %x  data: %x delay %d",
		i2c_list->i2c_settings.reg_setting->reg_addr,
		i2c_list->i2c_settings.reg_setting->reg_data,
		i2c_list->i2c_settings.delay);

		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"Failed in Applying i2c wrt settings");
			return rc;
		}
	} else if (i2c_list->op_code == CAM_SENSOR_I2C_WRITE_SEQ) {
		rc = camera_io_dev_write_continuous(
			(io_master_info),
			&(i2c_list->i2c_settings),
			CAM_SENSOR_I2C_WRITE_SEQ);

		CAM_DBG(CAM_OIS,"write continuous addr: %x  data: %x delay %d",
		i2c_list->i2c_settings.reg_setting->reg_addr,
		i2c_list->i2c_settings.reg_setting->reg_data,
		i2c_list->i2c_settings.delay);

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
			(io_master_info),
			i2c_list->i2c_settings.reg_setting[i].reg_addr,
			i2c_list->i2c_settings.reg_setting[i].reg_data,
			i2c_list->i2c_settings.reg_setting[i].data_mask,
			i2c_list->i2c_settings.addr_type,
			i2c_list->i2c_settings.data_type,
			i2c_list->i2c_settings.reg_setting[i].delay);

			CAM_DBG(CAM_OIS,"poll  addr: %x  data: %x delay %d",
			i2c_list->i2c_settings.reg_setting[i].reg_addr,
			i2c_list->i2c_settings.reg_setting[i].reg_data,
			i2c_list->i2c_settings.reg_setting[i].delay);

			if (rc < 0) {
				CAM_ERR(CAM_OIS,
					"i2c poll apply setting Fail");
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
	struct i2c_settings_list *i2c_save_list_last = NULL;

	unsigned short delaytimes = 0;
	int parklens_step = 0;
	int i=0;
	int32_t rc = 0;
	int32_t parklens_opcode = 0;

	if (!arg) {
		rc = -EINVAL;
		parklens_atomic_set(&(parklens_ctrl->exit_result),
			PARKLENS_EXIT_CREATE_WAKELOCK_FAILED);
		CAM_ERR(CAM_OIS, "ois_parklens_thread_func arg is NULL");
	} else {
		o_ctrl = (struct cam_ois_ctrl_t *)arg;
		parklens_ctrl = &(o_ctrl->parklens_ctrl);
		soc_private = (struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
		i2c_set     = (struct i2c_settings_array *)(&(o_ctrl->i2c_data.parklens_settings));
		power_info = &soc_private->power_info;
	}

	parklens_event_set(&(parklens_ctrl->start_event));
	if (rc < 0)
		goto exit_without_powerdown;

	for (i=0; i < PARKLENS_LAST_SAVE_SETTING; i++){
		if(i == 0){
			i2c_list_last = list_last_entry(&(i2c_set->list_head), typeof(*i2c_list_last), list);
			INIT_LIST_HEAD(&parklens_ctrl->last_setting_array.list_head);
			parklens_ctrl->last_setting_array.is_settings_valid = 1;
			parklens_ctrl->last_setting_array.request_id        = 0;
		}
		else
			i2c_list_last = list_prev_entry(i2c_list_last, list);

		i2c_save_list_last = kmalloc(sizeof(struct i2c_settings_list),GFP_KERNEL);
		i2c_save_list_last->i2c_settings = i2c_list_last->i2c_settings;
		i2c_save_list_last->op_code      = i2c_list_last->op_code;
		i2c_save_list_last->i2c_settings.reg_setting = kmalloc(sizeof(struct cam_sensor_i2c_reg_array),GFP_KERNEL);
		memcpy(i2c_save_list_last->i2c_settings.reg_setting,i2c_list_last->i2c_settings.reg_setting,sizeof(struct cam_sensor_i2c_reg_array));
		list_add(&i2c_save_list_last->list,&parklens_ctrl->last_setting_array.list_head);
	}
	CAM_DBG(CAM_OIS,"parklens thread start up");

	for (;;) {
		parklens_opcode = parklens_atomic_read(&(parklens_ctrl->parklens_opcode));
		if (parklens_opcode >= EXIT_PARKLENS_WITH_POWERDOWN) {
			if (parklens_opcode == EXIT_PARKLENS_WITHOUT_POWERDOWN) {
				CAM_DBG(CAM_OIS,
					"exit parklens thread no need power off");
				parklens_atomic_set(
					&(parklens_ctrl->exit_result),
					PARKLENS_EXIT_WITHOUT_POWERDOWN);
				goto exit_without_powerdown;
			} else if (parklens_opcode == EXIT_PARKLENS_WITH_POWERDOWN) {
				CAM_DBG(CAM_OIS,
					"exit parklens thread no need power off");
				parklens_atomic_set(
					&(parklens_ctrl->exit_result),
					PARKLENS_EXIT_WITH_POWEDOWN);
				goto exit_with_powerdown;
			} else {
				CAM_WARN(CAM_OIS, "Invalid opcode for parklens");
				continue;
			}
		}

		if (i2c_set == NULL) {
			CAM_ERR(CAM_OIS,
				"parklens Invalid parklens settings");
			parklens_atomic_set(
				&(parklens_ctrl->exit_result),
				PARKLENS_EXIT_WITHOUT_PARKLENS);
			goto exit_with_powerdown;
		} else if (i2c_set->is_settings_valid != 1) {
			CAM_ERR(CAM_OIS,
				"parklens settings is not valid");
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
					CAM_DBG(CAM_OIS,
						"parklens settings execute done");
					parklens_atomic_set(
						&(parklens_ctrl->exit_result),
						PARKLENS_EXIT_WITH_POWEDOWN);
					goto exit_with_powerdown;
				} else {
					delaytimes = i2c_list->i2c_settings.delay;
					i2c_list->i2c_settings.delay = 0;

					CAM_DBG(CAM_OIS, "Parklens total delay time %dms", delaytimes);

					rc = cam_ois_i2c_modes_util(
						&(o_ctrl->io_master_info),
						i2c_list);
					if (rc < 0) {
						CAM_ERR(CAM_OIS,
							"parklens step failed, %d", rc);
						parklens_atomic_set(
							&(parklens_ctrl->exit_result),
							PARKLENS_EXIT_CCI_ERROR);
						goto exit_with_powerdown;
					} else
						CAM_DBG(CAM_OIS,
							"ois_parklens_thread_func arg step/state %d/%d",
							++parklens_step,
							parklens_ctrl->parklens_state);
				}
			}
			else{
				--delaytimes;
				usleep_range(990,1010);
			}
		}
	}

exit_with_powerdown:
	CAM_DBG(CAM_OIS,
		"parklens thread exit with power down");
	CAM_DBG(CAM_OIS,
		"parklens thread exit step/result %d/%d",
		parklens_step,
		parklens_atomic_read(&(parklens_ctrl->exit_result)));

	rc = cam_ois_power_down(o_ctrl);
	if (rc < 0) {
		CAM_DBG(CAM_OIS,
			"parklens power down failed rc: %d", rc);
		parklens_atomic_set(
			&(parklens_ctrl->exit_result),
			PARKLENS_EXIT_WITH_POWEDOWN_FAILED);
	}
	else
		CAM_DBG(CAM_OIS,"parklens thread  power down success");

	kfree(power_info->power_setting);
	kfree(power_info->power_down_setting);
	power_info->power_setting = NULL;
	power_info->power_down_setting = NULL;
	power_info->power_setting_size = 0;
	power_info->power_down_setting_size = 0;

exit_without_powerdown:
	if (parklens_atomic_read(&(parklens_ctrl->exit_result)) <=
		PARKLENS_EXIT_WITHOUT_POWERDOWN) {
		CAM_DBG(CAM_OIS,
			"parklens thread exit without power down");
		CAM_DBG(CAM_OIS,
			"parklens thread exit step/result %d/%d",
			parklens_step,
			parklens_atomic_read(&(parklens_ctrl->exit_result)));
	}

	CAM_DBG(CAM_OIS,"parklens thread delete request");
	rc = delete_request(&(o_ctrl->i2c_data.parklens_settings));
	if (rc < 0)
		CAM_ERR(CAM_SENSOR,
			"delete parklens request failed rc: %d", rc);

	parklens_event_set(&(parklens_ctrl->shutdown_event));
	parklens_exit_thread(true);

	return 0;
}

int32_t init_ois_parklens_info(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t rc = 0;
	struct cam_ois_parklens_ctrl_t *parklens_ctrl;

	if(!o_ctrl) {
		CAM_ERR(CAM_OIS, "failed: o_ctrl %pK", o_ctrl);
		return -EINVAL;
	}

	parklens_ctrl = &(o_ctrl->parklens_ctrl);

	CAM_DBG(CAM_OIS, "init parklens: %s %s", o_ctrl->device_name,o_ctrl->ois_name);

	parklens_atomic_set(&(parklens_ctrl->parklens_opcode),
		ENTER_PARKLENS_WITH_POWERDOWN);
	parklens_atomic_set(&(parklens_ctrl->exit_result),
		PARKLENS_ENTER);
	parklens_atomic_set(&(parklens_ctrl->parklens_state),
		PARKLENS_INVALID);

	rc = parklens_event_create(&(parklens_ctrl->start_event));
	if (rc < 0) {
		CAM_ERR(CAM_OIS,
			"failed to create start event for parklens");
		return rc;
	}


	rc = parklens_event_create(&(parklens_ctrl->shutdown_event));
	if (rc < 0) {
		CAM_ERR(CAM_OIS,
			"failed to create shutdown event for parklens");
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
	struct i2c_settings_list *i2c_list_safe = NULL;
	struct i2c_settings_list *i2c_list_last = NULL;

	if(!o_ctrl) {
		CAM_ERR(CAM_OIS, "failed: o_ctrl %pK", o_ctrl);
		return -EINVAL;
	}

	CAM_DBG(CAM_OIS, "deinit parklens: %s", o_ctrl->device_name);

	parklens_ctrl = &(o_ctrl->parklens_ctrl);

	if (PARKLENS_STOP !=
		parklens_atomic_read(&(parklens_ctrl->parklens_state))) {
		CAM_ERR(CAM_OIS, "deinit parklens in wrong state");
		return -EINVAL;
	}

	list_for_each_entry_safe(i2c_list_last, i2c_list_safe,&(parklens_ctrl->last_setting_array.list_head), list) {
		list_del(&i2c_list_last->list);
		kfree(i2c_list_last->i2c_settings.reg_setting);
		kfree(i2c_list_last);
	}

	rc = parklens_event_destroy(&(parklens_ctrl->start_event));
	if (rc < 0)
		CAM_ERR(CAM_OIS,
			"failed to destroy start event for parklens");

	rc = parklens_event_destroy(&(parklens_ctrl->shutdown_event));
	if (rc < 0)
		CAM_ERR(CAM_OIS,
			"failed to destroy shutdown event for parklens");

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
		CAM_ERR(CAM_OIS, "failed: o_ctrl %pK", o_ctrl);
		return false;
	}

	parklens_ctrl = &(o_ctrl->parklens_ctrl);
	parklens_state = parklens_atomic_read(&(parklens_ctrl->parklens_state));

	switch (parklens_state) {
	case PARKLENS_RUNNING: {
		CAM_DBG(CAM_OIS, "parklens is running, no need powerdown");
		is_power_down = true;
	}
		break;

	case PARKLENS_STOP: {
		if (parklens_atomic_read(&(parklens_ctrl->exit_result)) <= PARKLENS_EXIT_WITHOUT_POWERDOWN) {
			CAM_DBG(CAM_OIS,"parklens stop without power down");
			is_power_down = false;
		} else {
			is_power_down = true;
		}
	}
		break;

	case PARKLENS_INVALID: {
		CAM_DBG(CAM_OIS,
		"parklens is not created, power down");
		is_power_down = false;
	}
		break;

	default:
		CAM_ERR(CAM_OIS, "Invalid parklens_state %d", parklens_state);
	}

	return is_power_down;
}

int32_t ois_parklens_thread_trigger(struct cam_ois_ctrl_t *o_ctrl)
{
	int32_t rc = 0;
	struct cam_ois_parklens_ctrl_t *parklens_ctrl;

	if(!o_ctrl) {
		CAM_ERR(CAM_OIS, "failed: o_ctrl %pK", o_ctrl);
		return -EINVAL;;
	}

	rc = init_ois_parklens_info(o_ctrl);
	if(rc < 0){
		CAM_ERR(CAM_OIS, "failed to init parklens info %d", rc);
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
		CAM_ERR(CAM_OIS, "parklens_thread create failed");
		rc = -ENOMEM;
		goto deinit_parklens;
	} else {
		CAM_DBG(CAM_OIS, "%s trigger parklens thread",o_ctrl->ois_name);
		parklens_wait_single_event(&(parklens_ctrl->start_event),0);

		rc = parklens_atomic_read(&(parklens_ctrl->exit_result));
		if (rc > PARKLENS_ENTER) {
			CAM_DBG(CAM_OIS, "parklens thread execute failed %d", rc);
			goto clear_thread;
		} else
			parklens_atomic_set(&(parklens_ctrl->parklens_state),
								PARKLENS_RUNNING);
	}

	return rc;

clear_thread:
	parklens_wait_single_event(&(parklens_ctrl->shutdown_event),0);
	rc = parklens_atomic_read(&(parklens_ctrl->exit_result));
	CAM_DBG(CAM_OIS, "parklens thread exit: %d", rc);

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
		CAM_ERR(CAM_OIS, "failed: o_ctrl %pK", o_ctrl);
		return -EINVAL;
	}

	parklens_ctrl = &(o_ctrl->parklens_ctrl);

	parklens_state = parklens_atomic_read(&(parklens_ctrl->parklens_state)) ;
	exit_result = parklens_atomic_read(&(parklens_ctrl->exit_result));

	if (parklens_state != PARKLENS_RUNNING) {
		CAM_DBG(CAM_OIS,
			"parklens thread is in other state: %d", parklens_state);
		return -EINVAL;
	} else {
		if(exit_result != PARKLENS_ENTER) {
			CAM_DBG(CAM_OIS,
				"parklens thread is already end: %d", exit_result);
		} else {
			if ((opcode == EXIT_PARKLENS_WITH_POWERDOWN) ||
				(opcode == EXIT_PARKLENS_WITHOUT_POWERDOWN)) {
				parklens_atomic_set(&(parklens_ctrl->parklens_opcode), opcode);
			} else {
				CAM_DBG(CAM_OIS,
					"Invalid stop opcode, stop parklens with power down");
				parklens_atomic_set(&(parklens_ctrl->parklens_opcode),
									EXIT_PARKLENS_WITH_POWERDOWN);
			}
			CAM_DBG(CAM_OIS, "wake up parklens thread to stop it");
		}

		parklens_wait_single_event(&(parklens_ctrl->shutdown_event), 0);

		parklens_atomic_set(&(parklens_ctrl->parklens_state), PARKLENS_STOP);
		parklens_ctrl->parklens_thread = NULL;

		exit_result = parklens_atomic_read(&(parklens_ctrl->exit_result));
		if(exit_result) {
			CAM_INFO(CAM_OIS, "parklens thread exit status: %d", exit_result);
		} else
			CAM_DBG(CAM_OIS, "parklens thread exit failed ! ! !");
	}

	return exit_result;
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
	int32_t                         i = 0, j =0;
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
	size_t                          len_of_buff = 0;
	uint32_t                       *offset = NULL, *cmd_buf;
	struct cam_ois_soc_private     *soc_private =
		(struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t  *power_info = &soc_private->power_info;
	const struct flash_ois_function *ps = pflash_ois;
	uint8_t                          config_flag = 0;
	struct timespec64                ts1, ts2; // xiaomi add
	long                             microsec = 0; // xiaomi add
	struct i2c_data_settings        *i2c_data = NULL; // xiaomi add
	struct skip_frame                *skip_frame_msg = NULL; // xiaomi add
	bool    parklens_power_down = true; //xiaomi add
	int32_t parklens_state      = 0;    //xiaomi add
	struct i2c_settings_list *i2c_list_last = NULL;

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
		return -EINVAL;
	}

	remain_len -= (size_t)dev_config.offset;
	csl_packet = (struct cam_packet *)
		(generic_pkt_addr + (uint32_t)dev_config.offset);

	if (cam_packet_util_validate_packet(csl_packet,
		remain_len)) {
		CAM_ERR(CAM_OIS, "Invalid packet params");
		return -EINVAL;
	}

	// xiaomi add
	o_ctrl->is_second_init=false;
	CAM_DBG(CAM_OIS, "[CRM-OIS] op_code: %d, cmd buf: %d",
		(csl_packet->header.op_code & 0xFFFFFF),
		csl_packet->num_cmd_buf);

	if ((csl_packet->header.op_code & 0xFFFFFF) !=
		CAM_OIS_PACKET_OPCODE_INIT &&
		csl_packet->header.request_id <= o_ctrl->last_flush_req
		&& o_ctrl->last_flush_req != 0) {
		CAM_DBG(CAM_OIS,
			"reject request %llu, last request to flush %llu",
			csl_packet->header.request_id, o_ctrl->last_flush_req);
		rc = -EBADR;
		goto pwr_dwn;
	}

	if (csl_packet->header.request_id > o_ctrl->last_flush_req)
		o_ctrl->last_flush_req = 0;
	// xiaomi add

	switch (csl_packet->header.op_code & 0xFFFFFF) {
	case CAM_OIS_PACKET_OPCODE_INIT:
		offset = (uint32_t *)&csl_packet->payload;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);

		CAM_DBG(CAM_OIS, "num_cmd_buf %d",
			csl_packet->num_cmd_buf);

		// xiaomi add
		memset(o_ctrl->skip_frame_queue,
			0,
			sizeof(struct skip_frame) * MAX_PER_FRAME_ARRAY);
		// xiaomi add

		/* Loop through multiple command buffers */
		for (i = 0; i < csl_packet->num_cmd_buf; i++) {
			total_cmd_buf_in_bytes = cmd_desc[i].length;
			if (!total_cmd_buf_in_bytes)
				continue;

			rc = cam_mem_get_cpu_buf(cmd_desc[i].mem_handle,
				&generic_ptr, &len_of_buff);
			if (rc < 0) {
				CAM_ERR(CAM_OIS, "Failed to get cpu buf : 0x%x",
					cmd_desc[i].mem_handle);
				return rc;
			}
			cmd_buf = (uint32_t *)generic_ptr;
			if (!cmd_buf) {
				CAM_ERR(CAM_OIS, "invalid cmd buf");
				return -EINVAL;
			}

			if ((len_of_buff < sizeof(struct common_header)) ||
				(cmd_desc[i].offset > (len_of_buff -
				sizeof(struct common_header)))) {
				CAM_ERR(CAM_OIS,
					"Invalid length for sensor cmd");
				return -EINVAL;
			}
			remain_len = len_of_buff - cmd_desc[i].offset;
			cmd_buf += cmd_desc[i].offset / sizeof(uint32_t);
			cmm_hdr = (struct common_header *)cmd_buf;

			switch (cmm_hdr->cmd_type) {
			case CAMERA_SENSOR_CMD_TYPE_I2C_INFO:
				rc = cam_ois_slaveInfo_pkt_parser(
					o_ctrl, cmd_buf, remain_len);
				if (rc < 0) {
					CAM_ERR(CAM_OIS,
					"Failed in parsing slave info");
					return rc;
				}
				break;
			case CAMERA_SENSOR_CMD_TYPE_PWR_UP:
			case CAMERA_SENSOR_CMD_TYPE_PWR_DOWN:
				CAM_DBG(CAM_OIS,"Received power settings buffer:%d",(cmm_hdr->cmd_type));
				if (PARKLENS_INVALID != parklens_atomic_read(&(o_ctrl->parklens_ctrl.parklens_state))) {
					ois_parklens_thread_stop(o_ctrl, EXIT_PARKLENS_WITHOUT_POWERDOWN);
					parklens_power_down = ois_parklens_power_down(o_ctrl);

					// Set the last few Settings, Ensure that the ois state is normal
					if(parklens_power_down == false){
						CAM_DBG(CAM_OIS,"apply last settings");
						list_for_each_entry(i2c_list_last,&(o_ctrl->parklens_ctrl.last_setting_array.list_head), list)
							cam_ois_i2c_modes_util(&(o_ctrl->io_master_info),i2c_list_last);
					}
					ois_deinit_parklens_info(o_ctrl);
					CAM_DBG(CAM_OIS,
						"stop parklens thread, powerdown:%d",
						parklens_power_down);
				} else {
					CAM_DBG(CAM_OIS,
						"parklens thread is invalid, powerdown:%d",
						parklens_power_down);
				}

				if(parklens_power_down == true) {
				 	CAM_DBG(CAM_OIS,
				 		"need power up again");
					rc = cam_sensor_update_power_settings(
							cmd_buf,
							total_cmd_buf_in_bytes,
							power_info, remain_len);
					CAM_DBG(CAM_OIS,"power up again successed");
					if (rc) {
						CAM_ERR(CAM_OIS,
							"Failed:parse power settings: %d",
							rc);
						return rc;
					}
				} else {
					CAM_DBG(CAM_OIS,
						"no need repower up again");
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
					return rc;
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
					return rc;
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
					CAM_ERR(CAM_OIS,
					"fw init parsing failed: %d", rc);
					return rc;
				}
			} else if (o_ctrl->i2c_postinit_data.is_settings_valid == 0) {
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
			}
			break;
			}
		}

		if (o_ctrl->cam_ois_state != CAM_OIS_CONFIG) {
			if (parklens_power_down == true) {
				CAM_DBG(CAM_OIS, "OIS Power up start");
				rc = cam_ois_power_up(o_ctrl);
				if (rc) {
					CAM_ERR(CAM_OIS, " OIS Power up failed");
					return rc;
				}
			}
			o_ctrl->cam_ois_state = CAM_OIS_CONFIG;
		}

		//interface for different ois add by xiaomi
		if ( o_ctrl->opcode.customized_ois_flag ) {
			for(j = 0; j < sizeof(pflash_ois) / sizeof(struct flash_ois_function) ; j++)
			{
				if(ps[j].flag == o_ctrl->opcode.customized_ois_flag){
					config_flag++;
					rc = ps[j].mi_ois_pkt_download(o_ctrl);
					if (rc) {
						CAM_ERR(CAM_OIS, "Failed OIS Customer Pkt Download");
						goto pwr_dwn;
					}
				}
			}
			if ( config_flag != 1 ) {
				CAM_ERR(CAM_OIS, "ERROR! need  pkt function or repeat flag , flag  %d", config_flag);
			}
		} else {
			if (o_ctrl->i2c_fwinit_data.is_settings_valid == 1) {
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_fwinit_data);
				if ((rc == -EAGAIN) &&
					(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
					CAM_WARN(CAM_OIS,
						"CCI HW is restting: Reapplying fwinit settings");
					usleep_range(1000, 1010);
					rc = cam_ois_apply_settings(o_ctrl,
						&o_ctrl->i2c_fwinit_data);
				}
				if (rc) {
					CAM_ERR(CAM_OIS,
						"Cannot apply fwinit data %d",
						rc);
					goto pwr_dwn;
				} else {
					CAM_DBG(CAM_OIS, "OIS fwinit settings success");
				}
			}

			if (o_ctrl->ois_fw_flag) {
				/* xiaomi add begin */
				CAM_GET_TIMESTAMP(ts1);
				CAM_DBG(MI_PERF, "%s start firmware download", o_ctrl->ois_name);
				/* xiaomi add end */
				rc = cam_ois_fw_download(o_ctrl);
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

			rc = cam_ois_apply_settings(o_ctrl, &o_ctrl->i2c_init_data);
			if ((rc == -EAGAIN) &&
				(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
				CAM_WARN(CAM_OIS,
					"CCI HW is restting: Reapplying INIT settings");
				usleep_range(1000, 1010);
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_init_data);
			}
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
					"Cannot apply Init settings: rc = %d",
					rc);
				goto pwr_dwn;
			} else {
				CAM_DBG(CAM_OIS, "apply Init settings success");
			}

			if (o_ctrl->is_ois_calib) {
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_calib_data);
				if ((rc == -EAGAIN) &&
					(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
					CAM_WARN(CAM_OIS,
						"CCI HW is restting: Reapplying calib settings");
					usleep_range(1000, 1010);
					rc = cam_ois_apply_settings(o_ctrl,
						&o_ctrl->i2c_calib_data);
				}
				if (rc) {
					CAM_ERR(CAM_OIS, "Cannot apply calib data");
					goto pwr_dwn;
				} else {
					CAM_DBG(CAM_OIS, "apply calib data settings success");
				}
			}

			if (o_ctrl->i2c_postinit_data.is_settings_valid == 1) {
				rc = cam_ois_apply_settings(o_ctrl,
					&o_ctrl->i2c_postinit_data);
				if ((rc == -EAGAIN) &&
					(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
					CAM_WARN(CAM_OIS,
						"CCI HW is restting: Reapplying postinit settings");
					usleep_range(1000, 1010);
					rc = cam_ois_apply_settings(o_ctrl,
						&o_ctrl->i2c_postinit_data);
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
		}

		rc = delete_request(&o_ctrl->i2c_fwinit_data);
		if (rc < 0) {
			CAM_WARN(CAM_OIS,
				"Fail deleting fwinit data: rc: %d", rc);
			rc = 0;
		}

		rc = delete_request(&o_ctrl->i2c_init_data);
		if (rc < 0) {
			CAM_WARN(CAM_OIS,
				"Fail deleting Init data: rc: %d", rc);
			rc = 0;
		}
		rc = delete_request(&o_ctrl->i2c_calib_data);
		if (rc < 0) {
			CAM_WARN(CAM_OIS,
				"Fail deleting Calibration data: rc: %d", rc);
			rc = 0;
		}
		rc = delete_request(&o_ctrl->i2c_postinit_data);
		if (rc < 0) {
			CAM_WARN(CAM_OIS,
				"Fail deleting postinit data: rc: %d", rc);
			rc = 0;
		}
		break;

	// xiaomi add
	case CAM_OIS_PACKET_OPCODE_OIS_PARKLENS:{
		int32_t exit_result = parklens_atomic_read(&(o_ctrl->parklens_ctrl.exit_result));
		CAM_DBG(CAM_OIS,"Received parklens buffer");
		parklens_state = parklens_atomic_read(
							&(o_ctrl->parklens_ctrl.parklens_state));
		CAM_DBG(CAM_OIS,"parklens_state %d", parklens_state);
		if ((parklens_state != PARKLENS_INVALID) ||
			(o_ctrl->cam_ois_state < CAM_OIS_CONFIG)) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
				"[OIS]Not in right state to do parklens: %d/%d, exit result %d release power control right",
				o_ctrl->cam_ois_state,
				parklens_state, exit_result);
			ois_parklens_thread_stop(o_ctrl, EXIT_PARKLENS_WITH_POWERDOWN);
			ois_deinit_parklens_info(o_ctrl);
			return rc;
		}

		CAM_DBG(CAM_OIS, "request_id %llu, i2c_data %p &i2c_data->per_frame %p",
			csl_packet->header.request_id,
			&(o_ctrl->i2c_data),
			&i2c_data->per_frame);


		i2c_reg_settings = &(o_ctrl->i2c_data.parklens_settings);

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
			CAM_ERR(CAM_OIS,
				"Failed:parse parklens settings: %d",
				rc);
			delete_request(i2c_reg_settings);
			return rc;
		}

		ois_parklens_thread_trigger(o_ctrl);
		break;
	}
	case CAM_OIS_PACKET_OPCODE_INIT_SECOND:
		offset = (uint32_t *)&csl_packet->payload;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);

		CAM_DBG(CAM_OIS, "num_cmd_buf %d",
			csl_packet->num_cmd_buf);

		/* Loop through multiple command buffers */
		for (i = 0; i < csl_packet->num_cmd_buf; i++) {
			total_cmd_buf_in_bytes = cmd_desc[i].length;
			if (!total_cmd_buf_in_bytes)
				continue;

			rc = cam_mem_get_cpu_buf(cmd_desc[i].mem_handle,
				&generic_ptr, &len_of_buff);
			if (rc < 0) {
				CAM_ERR(CAM_OIS, "Failed to get cpu buf : 0x%x",
					cmd_desc[i].mem_handle);
				return rc;
			}
			cmd_buf = (uint32_t *)generic_ptr;
			if (!cmd_buf) {
				CAM_ERR(CAM_OIS, "invalid cmd buf");
				return -EINVAL;
			}

			if ((len_of_buff < sizeof(struct common_header)) ||
				(cmd_desc[i].offset > (len_of_buff -
				sizeof(struct common_header)))) {
				CAM_ERR(CAM_OIS,
					"Invalid length for sensor cmd");
				return -EINVAL;
			}

			if ((o_ctrl->is_ois_calib != 0) &&
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
					return rc;
				}
			} else if (o_ctrl->i2c_postinit_data.is_settings_valid == 0) {
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
			}
		}

		o_ctrl->is_second_init=true;
		if ( o_ctrl->opcode.customized_ois_flag ) {
			for(j = 0; j < sizeof(pflash_ois) / sizeof(struct flash_ois_function) ; j++)
			{
				if(ps[j].flag == o_ctrl->opcode.customized_ois_flag){
					config_flag++;
					rc = ps[j].mi_ois_pkt_download(o_ctrl);
					if (rc) {
						CAM_ERR(CAM_OIS, "Failed OIS Customer Pkt Download");
						goto pwr_dwn;
					}
				}
			}
			if ( config_flag != 1 ) {
				CAM_ERR(CAM_OIS, "ERROR! need  pkt function or repeat flag , flag  %d", config_flag);
			}
		}

		if (o_ctrl->i2c_calib_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_calib_data);

		if (o_ctrl->i2c_postinit_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_postinit_data);

		break;

	case CAM_OIS_PACKET_OPCODE_OIS_MANUAL_MODE:

		CAM_DBG(CAM_OIS, "[CRM-OIS] Received aperture mode packets");

		if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
				"[CRM-OIS] Not in right state to control OIS: %d",
				o_ctrl->cam_ois_state);
			return rc;
		}

		CAM_DBG(CAM_OIS, "[CRM-OIS] request_id %llu, i2c_data %p &i2c_data->per_frame %p",
			csl_packet->header.request_id,
			&(o_ctrl->i2c_data),
			&i2c_data->per_frame);

		o_ctrl->setting_apply_state = OIS_APPLY_SETTINGS_LATER;
		i2c_data =  &(o_ctrl->i2c_data);
		i2c_reg_settings = &i2c_data->per_frame[
			csl_packet->header.request_id % MAX_PER_FRAME_ARRAY];

		i2c_reg_settings->request_id =
			csl_packet->header.request_id;
		i2c_reg_settings->is_settings_valid = 1;
		offset = (uint32_t *)&csl_packet->payload;
		offset += csl_packet->cmd_buf_offset / sizeof(uint32_t);
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);
		rc = cam_sensor_i2c_command_parser(
			&o_ctrl->io_master_info,
			i2c_reg_settings,
			cmd_desc, 1, NULL);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"[CRM-OIS] CRM OIS parsing failed: %d", rc);
			goto pwr_dwn;
		}

		rc = cam_ois_update_req_mgr(o_ctrl, csl_packet);
		if (rc) {
			CAM_ERR(CAM_OIS,
				"[CRM-OIS] Failed in adding request to request manager");
			goto pwr_dwn;
		}
		break;

	case CAM_PKT_NOP_OPCODE:
		if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
			CAM_DBG(CAM_OIS,
				"[CRM-OIS] Received NOP packets in invalid state: %d",
				o_ctrl->cam_ois_state);
			rc = -EINVAL;
			goto pwr_dwn;
		}
		rc = cam_ois_update_req_mgr(o_ctrl, csl_packet);
		if (rc) {
			CAM_ERR(CAM_OIS,
				"[CRM-OIS] Failed in adding request to request manager");
			goto pwr_dwn;
		}
		break;

	// xiaomi add
	case CAM_FRAME_SKIP_OPCODE: {
		if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
			CAM_ERR(CAM_OIS,
				"[SkipFrame] Recvied skip frame packet in invalid state: %d",
				o_ctrl->cam_ois_state);
			rc = -EINVAL;
			goto pwr_dwn;
		}

		offset = (uint32_t *)&csl_packet->payload;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);

		CAM_INFO(CAM_OIS, "[SkipFrame] num_cmd_buf %d",
			csl_packet->num_cmd_buf);

		if (0 == csl_packet->num_cmd_buf) {
			CAM_ERR(CAM_OIS,
				"[SkipFrame] Recvied skip frame packet is empty.");
			rc = -EINVAL;
			goto pwr_dwn;
		}

		total_cmd_buf_in_bytes = cmd_desc[0].length;

		CAM_INFO(CAM_OIS, "[SkipFrame] total_cmd_buf_in_bytes %d",
			total_cmd_buf_in_bytes);

		if (sizeof(struct skip_frame) > total_cmd_buf_in_bytes) {
			CAM_ERR(CAM_OIS,
				"[SkipFrame] Insufficient number of packet bytes received: %d.",
				total_cmd_buf_in_bytes);
			rc = -EINVAL;
			goto pwr_dwn;
		}

		rc = cam_mem_get_cpu_buf(cmd_desc[0].mem_handle,
			&generic_ptr, &len_of_buff);

		if (rc < 0) {
			CAM_ERR(CAM_OIS, "[SkipFrame] Failed to get cpu buf : 0x%x",
				cmd_desc[0].mem_handle);
			rc = -EINVAL;
			return rc;
		}

		cmd_buf = (uint32_t *)generic_ptr;

		if (!cmd_buf) {
			CAM_ERR(CAM_OIS, "[SkipFrame] invalid cmd buf");
			rc = -EINVAL;
			return rc;
		}

		if ((len_of_buff < sizeof(struct skip_frame)) ||
			(cmd_desc[0].offset > (len_of_buff -
			sizeof(struct skip_frame)))) {
			CAM_ERR(CAM_OIS,
				"[SkipFrame] Invalid length for skip frame packet");
			return -EINVAL;
		}

		cmd_buf += cmd_desc[0].offset / sizeof(uint32_t);
		skip_frame_msg = ((struct skip_frame*)cmd_buf);

		if (0 < skip_frame_msg->skip_num) {
			o_ctrl->skip_frame_queue[
				skip_frame_msg->req_id % MAX_PER_FRAME_ARRAY].req_id =
					skip_frame_msg->req_id;
			o_ctrl->skip_frame_queue[
				skip_frame_msg->req_id % MAX_PER_FRAME_ARRAY].skip_num =
					skip_frame_msg->skip_num - 1;
			CAM_INFO(CAM_OIS, "[SkipFrame] skip reqId: %llu, skip num: %llu queue index:%d",
					o_ctrl->skip_frame_queue[(skip_frame_msg->req_id) % MAX_PER_FRAME_ARRAY].req_id,
					o_ctrl->skip_frame_queue[(skip_frame_msg->req_id) % MAX_PER_FRAME_ARRAY].skip_num,
					(skip_frame_msg->req_id) % MAX_PER_FRAME_ARRAY);
		}

		break;
	}
	// xiaomi add

	case CAM_OIS_PACKET_OPCODE_OIS_CONTROL:
		if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
				"Not in right state to control OIS: %d",
				o_ctrl->cam_ois_state);
			return rc;
		}
		offset = (uint32_t *)&csl_packet->payload;
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
			return rc;
		}

		rc = cam_ois_apply_settings(o_ctrl, i2c_reg_settings);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "Cannot apply mode settings");
			return rc;
		}

		rc = delete_request(i2c_reg_settings);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"Fail deleting Mode data: rc: %d", rc);
			return rc;
		}
		break;
	case CAM_OIS_PACKET_OPCODE_READ: {
		uint64_t qtime_ns;
		struct cam_buf_io_cfg *io_cfg;
		struct i2c_settings_array i2c_read_settings;

		if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
				"Not in right state to read OIS: %d",
				o_ctrl->cam_ois_state);
			return rc;
		}
		CAM_DBG(CAM_OIS, "number of I/O configs: %d:",
			csl_packet->num_io_configs);
		if (csl_packet->num_io_configs == 0) {
			CAM_ERR(CAM_OIS, "No I/O configs to process");
			rc = -EINVAL;
			return rc;
		}

		INIT_LIST_HEAD(&(i2c_read_settings.list_head));

		io_cfg = (struct cam_buf_io_cfg *) ((uint8_t *)
			&csl_packet->payload +
			csl_packet->io_configs_offset);

		/* validate read data io config */
		if (io_cfg == NULL) {
			CAM_ERR(CAM_OIS, "I/O config is invalid(NULL)");
			rc = -EINVAL;
			return rc;
		}

		offset = (uint32_t *)&csl_packet->payload;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);
		i2c_read_settings.is_settings_valid = 1;
		i2c_read_settings.request_id = 0;
		rc = cam_sensor_i2c_command_parser(&o_ctrl->io_master_info,
			&i2c_read_settings,
			cmd_desc, 1, &io_cfg[0]);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "OIS read pkt parsing failed: %d", rc);
			return rc;
		}

		rc = cam_sensor_util_get_current_qtimer_ns(&qtime_ns);
		if (rc < 0) {
			CAM_ERR(CAM_SENSOR, "failed to get qtimer rc:%d");
			return rc;
		}

		// xiaomi change begin
		// rc = cam_sensor_i2c_read_data(
		//	&i2c_read_settings,
		//	&o_ctrl->io_master_info);
		rc = cam_sensor_i2c_read_write_ois_data(
			&i2c_read_settings,
			&o_ctrl->io_master_info);
		// xiaomi change end

		if (rc < 0) {
			CAM_ERR(CAM_OIS, "cannot read data rc: %d", rc);
			delete_request(&i2c_read_settings);
			return rc;
		}

		if (csl_packet->num_io_configs > 1) {
			rc = cam_sensor_util_write_qtimer_to_io_buffer(
				qtime_ns, &io_cfg[1]);
			if (rc < 0) {
				CAM_ERR(CAM_OIS,
					"write qtimer failed rc: %d", rc);
				delete_request(&i2c_read_settings);
				return rc;
			}
		}

		rc = delete_request(&i2c_read_settings);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"Failed in deleting the read settings");
			return rc;
		}
		break;
	}
	case CAM_OIS_PACKET_OPCODE_WRITE_TIME: {
		if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
			rc = -EINVAL;
			CAM_ERR(CAM_OIS,
				"Not in right state to write time to OIS: %d",
				o_ctrl->cam_ois_state);
			return rc;
		}
		offset = (uint32_t *)&csl_packet->payload;
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
			return rc;
		}

		rc = cam_ois_update_time(i2c_reg_settings);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "Cannot update time");
			return rc;
		}

		rc = cam_ois_apply_settings(o_ctrl, i2c_reg_settings);
		if (rc < 0) {
			CAM_ERR(CAM_OIS, "Cannot apply mode settings");
			return rc;
		}

		rc = delete_request(i2c_reg_settings);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"Fail deleting Mode data: rc: %d", rc);
			return rc;
		}
		break;
	}
	default:
		CAM_ERR(CAM_OIS, "Invalid Opcode: %d",
			(csl_packet->header.op_code & 0xFFFFFF));
		return -EINVAL;
	}

	if (!rc)
		return rc;
pwr_dwn:
	// modify by xiaomi for ois init fail will power down twice happen crash.
	// cam_ois_power_down(o_ctrl);
	CAM_ERR(CAM_OIS, "OIS init fail! rc=%d", rc);
	return rc;
}

void cam_ois_shutdown(struct cam_ois_ctrl_t *o_ctrl)
{
	int rc = 0;
	struct cam_ois_soc_private *soc_private =
		(struct cam_ois_soc_private *)o_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t *power_info = &soc_private->power_info;

	if (o_ctrl->cam_ois_state == CAM_OIS_INIT)
		return;

	if (o_ctrl->cam_ois_state >= CAM_OIS_CONFIG) {
		rc = cam_ois_power_down(o_ctrl);
		if (rc < 0)
			CAM_ERR(CAM_OIS, "OIS Power down failed");
		o_ctrl->cam_ois_state = CAM_OIS_ACQUIRE;
	}

	if (o_ctrl->cam_ois_state >= CAM_OIS_ACQUIRE) {
		rc = cam_destroy_device_hdl(o_ctrl->bridge_intf.device_hdl);
		if (rc < 0)
			CAM_ERR(CAM_OIS, "destroying the device hdl");
		o_ctrl->bridge_intf.device_hdl = -1;
		o_ctrl->bridge_intf.link_hdl = -1;
		o_ctrl->bridge_intf.session_hdl = -1;
	}

	if (o_ctrl->i2c_fwinit_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_fwinit_data);

	if (o_ctrl->i2c_mode_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_mode_data);

	if (o_ctrl->i2c_calib_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_calib_data);

	if (o_ctrl->i2c_init_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_init_data);

	if (o_ctrl->i2c_postinit_data.is_settings_valid == 1)
		delete_request(&o_ctrl->i2c_postinit_data);

	/* xiaomi add begin */
	if (PARKLENS_INVALID != parklens_atomic_read(&(o_ctrl->parklens_ctrl.parklens_state))) {
		ois_deinit_parklens_info(o_ctrl);
		CAM_DBG(CAM_OIS,
			"parklens is not valid, deinit parklens info");
	}
	/* xiaomi add end */

	kfree(power_info->power_setting);
	kfree(power_info->power_down_setting);
	power_info->power_setting = NULL;
	power_info->power_down_setting = NULL;
	power_info->power_down_setting_size = 0;
	power_info->power_setting_size = 0;

	o_ctrl->last_flush_req = 0; // xiaomi add
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
	int                              rc = 0;
	struct cam_ois_query_cap_t       ois_cap = {0};
	struct cam_control              *cmd = (struct cam_control *)arg;
	struct cam_ois_soc_private      *soc_private = NULL;
	struct cam_sensor_power_ctrl_t  *power_info = NULL;
	bool    parklens_power_down = true;   //xiaomi add

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
		o_ctrl->last_flush_req = 0;  // xiaomi add
		break;
	case CAM_CONFIG_DEV:
		rc = cam_ois_pkt_parse(o_ctrl, arg);
		if (rc) {
			CAM_ERR(CAM_OIS, "Failed in ois pkt Parsing");
			goto release_mutex;
		}
		break;
	case CAM_RELEASE_DEV: {
		struct i2c_settings_array *i2c_set = NULL;
		int i;

		if (o_ctrl->cam_ois_state == CAM_OIS_START) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
				"Cant release ois: in start state");
			goto release_mutex;
		}

		if (o_ctrl->cam_ois_state == CAM_OIS_CONFIG) {
			parklens_power_down = ois_parklens_power_down(o_ctrl);
			if(parklens_power_down == false){
				CAM_DBG(CAM_OIS, "cam_ois_power_down");
				rc = cam_ois_power_down(o_ctrl);
				if (rc < 0) {
					CAM_ERR(CAM_OIS, "OIS Power down failed");
					goto release_mutex;
				}
			}
		}

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
		o_ctrl->last_flush_req = 0; // xiaomi add
		// xiaomi add
		memset(o_ctrl->skip_frame_queue,
			0,
			sizeof(struct skip_frame) * MAX_PER_FRAME_ARRAY);
		// xiaomi add
		if (parklens_power_down == false) {
			kfree(power_info->power_setting);
			kfree(power_info->power_down_setting);
			power_info->power_setting = NULL;
			power_info->power_down_setting = NULL;
			power_info->power_down_setting_size = 0;
			power_info->power_setting_size = 0;
		}

		if (o_ctrl->i2c_mode_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_mode_data);

		if (o_ctrl->i2c_calib_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_calib_data);

		if (o_ctrl->i2c_init_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_init_data);

		if (o_ctrl->i2c_fwinit_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_fwinit_data);

		if (o_ctrl->i2c_postinit_data.is_settings_valid == 1)
			delete_request(&o_ctrl->i2c_postinit_data);

		// xiaomi add
		for (i = 0; i < MAX_PER_FRAME_ARRAY; i++) {
			i2c_set = &(o_ctrl->i2c_data.per_frame[i]);

			if (i2c_set->is_settings_valid == 1) {
				rc = delete_request(i2c_set);
				if (rc < 0)
					CAM_ERR(CAM_SENSOR,
						"[CRM-OIS] delete request: %lld rc: %d",
						i2c_set->request_id, rc);
			}
		}
	}

		break;
	case CAM_STOP_DEV:
		if (o_ctrl->cam_ois_state != CAM_OIS_START) {
			rc = -EINVAL;
			CAM_WARN(CAM_OIS,
			"Not in right state for stop : %d",
			o_ctrl->cam_ois_state);
		}
		o_ctrl->last_flush_req = 0; // xiaomi add
		o_ctrl->cam_ois_state = CAM_OIS_CONFIG;
		// xiaomi add
		memset(o_ctrl->skip_frame_queue,
			0,
			sizeof(struct skip_frame) * MAX_PER_FRAME_ARRAY);
		// xiaomi add
		break;
	case CAM_FLUSH_REQ:
		// ignore the flush cmd
		CAM_DBG(CAM_OIS, "CAM_FLUSH_REQ");
		break;
	default:
		CAM_ERR(CAM_OIS, "invalid opcode");
		goto release_mutex;
	}
release_mutex:
	mutex_unlock(&(o_ctrl->ois_mutex));
	return rc;
}

// xiaomi add

/**
 * cam_ois_publish_dev_info - publish device information
 * @info:        camera req manager device information
 *
 * Returns success or failure
 */
int cam_ois_publish_dev_info(struct cam_req_mgr_device_info *info)
{
	if (!info) {
		CAM_ERR(CAM_OIS, "[CRM-OIS] Invalid Args");
		return -EINVAL;
	}

	info->dev_id = CAM_REQ_MGR_DEVICE_OIS;
	strlcpy(info->name, CAM_OIS_NAME, sizeof(info->name));
	info->p_delay = 1;
	info->trigger = CAM_TRIGGER_POINT_SOF;

	return 0;
}

/**
 * cam_ois_establish_link - camera ois establish link
 * @link:        camera request manager core device link setup
 *
 * Returns success or failure
 */
int32_t cam_ois_establish_link(
	struct cam_req_mgr_core_dev_link_setup *link)
{
	struct cam_ois_ctrl_t *o_ctrl = NULL;

	if (!link) {
		CAM_ERR(CAM_OIS, "[CRM-OIS] Invalid Args");
		return -EINVAL;
	}

	o_ctrl = (struct cam_ois_ctrl_t *)
		cam_get_device_priv(link->dev_hdl);
	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "[CRM-OIS] Device data is NULL");
		return -EINVAL;
	}

	mutex_lock(&(o_ctrl->ois_mutex));
	if (link->link_enable) {
		o_ctrl->bridge_intf.link_hdl = link->link_hdl;
		o_ctrl->bridge_intf.crm_cb = link->crm_cb;
	} else {
		o_ctrl->bridge_intf.link_hdl = -1;
		o_ctrl->bridge_intf.crm_cb = NULL;
	}
	mutex_unlock(&(o_ctrl->ois_mutex));

	return 0;
}

/**
 * cam_ois_update_req_mgr - camera ois update reg manager
 * @o_ctrl:        camera ois controller
 * @csl_packet:    camera packet
 *
 * Returns success or failure
 */
int cam_ois_update_req_mgr(
	struct cam_ois_ctrl_t *o_ctrl,
	struct cam_packet *csl_packet)
{
	int rc = 0;
	struct cam_req_mgr_add_request add_req;

	memset(&add_req, 0, sizeof(add_req));
	add_req.link_hdl = o_ctrl->bridge_intf.link_hdl;
	add_req.req_id = csl_packet->header.request_id;
	add_req.dev_hdl = o_ctrl->bridge_intf.device_hdl;
	if ((csl_packet->header.op_code & 0xFFFFFF) ==
		CAM_OIS_PACKET_OPCODE_OIS_MANUAL_MODE) {
		add_req.trigger_eof = true;
		add_req.skip_at_eof = 0;
	}

	if (o_ctrl->bridge_intf.crm_cb &&
		o_ctrl->bridge_intf.crm_cb->add_req) {
		rc = o_ctrl->bridge_intf.crm_cb->add_req(&add_req);
		if (rc) {
			if (rc == -EBADR)
				CAM_INFO(CAM_OIS,
					"[CRM-OIS] Adding request: %llu failed: rc: %d, it has been flushed",
					csl_packet->header.request_id, rc);
			else
				CAM_ERR(CAM_OIS,
					"[CRM-OIS] Adding request: %llu failed: rc: %d",
					csl_packet->header.request_id, rc);
			return rc;
		}
		CAM_DBG(CAM_OIS, "[CRM-OIS] Request Id: %lld added to CRM",
			add_req.req_id);
	} else {
		CAM_ERR(CAM_OIS, "[CRM-OIS] Can't add Request ID: %lld to CRM",
			csl_packet->header.request_id);
		rc = -EINVAL;
	}

	return rc;
}

int32_t cam_ois_flush_request(struct cam_req_mgr_flush_request *flush_req)
{
	int32_t rc = 0, i;
	uint32_t cancel_req_id_found = 0;
	struct cam_ois_ctrl_t *o_ctrl = NULL;
	struct i2c_settings_array *i2c_set = NULL;

	if (!flush_req)
		return -EINVAL;

	o_ctrl = (struct cam_ois_ctrl_t *)
		cam_get_device_priv(flush_req->dev_hdl);
	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "[CRM-OIS] Device data is NULL");
		return -EINVAL;
	}

	if (o_ctrl->i2c_data.per_frame == NULL) {
		CAM_ERR(CAM_OIS, "[CRM-OIS] I2C frame data is NULL");
		return -EINVAL;
	}

	mutex_lock(&(o_ctrl->ois_mutex));

	if (flush_req->type == CAM_REQ_MGR_FLUSH_TYPE_ALL) {
		o_ctrl->last_flush_req = flush_req->req_id;
		CAM_DBG(CAM_OIS, "[CRM-OIS] Last reqest to flush is %lld",
			flush_req->req_id);
	}

	for (i = 0; i < MAX_PER_FRAME_ARRAY; i++) {
		i2c_set = &(o_ctrl->i2c_data.per_frame[i]);

		if ((flush_req->type == CAM_REQ_MGR_FLUSH_TYPE_CANCEL_REQ)
				&& (i2c_set->request_id != flush_req->req_id))
			continue;

		if (i2c_set->is_settings_valid == 1) {
			rc = delete_request(i2c_set);
			if (rc < 0)
				CAM_ERR(CAM_OIS,
					"[CRM-OIS] Delete request: %lld rc: %d",
					i2c_set->request_id, rc);

			if (flush_req->type ==
				CAM_REQ_MGR_FLUSH_TYPE_CANCEL_REQ) {
				cancel_req_id_found = 1;
				break;
			}
		}
	}

	// xiaomi add
	memset(o_ctrl->skip_frame_queue,
		0,
		sizeof(struct skip_frame) * MAX_PER_FRAME_ARRAY);
	// xiaomi add

	if (flush_req->type == CAM_REQ_MGR_FLUSH_TYPE_CANCEL_REQ &&
		!cancel_req_id_found)
		CAM_DBG(CAM_OIS,
			"[CRM-OIS] Flush request id:%lld not found in the pending list",
			flush_req->req_id);

	mutex_unlock(&(o_ctrl->ois_mutex));
	return rc;
}

/**
 * cam_ois_apply_request - apply ois request
 * @apply:        camera request manager apply request
 *
 * Returns success or failure
 */
int32_t cam_ois_apply_request(struct cam_req_mgr_apply_request *apply)
{
	int32_t rc = 0, request_id, del_req_id;
	struct cam_ois_ctrl_t *o_ctrl = NULL;

	if (!apply) {
		CAM_ERR(CAM_OIS, "[CRM-OIS] Invalid Input Args");
		return -EINVAL;
	}

	CAM_DBG(CAM_OIS, "[CRM-OIS] OIS apply request:%llu", apply->request_id);

	o_ctrl = (struct cam_ois_ctrl_t *)
		cam_get_device_priv(apply->dev_hdl);

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "[CRM-OIS] Device data is NULL");
		return -EINVAL;
	}

	request_id = apply->request_id % MAX_PER_FRAME_ARRAY;

	trace_cam_apply_req("OIS", o_ctrl->soc_info.index, apply->request_id, apply->link_hdl);

	mutex_lock(&(o_ctrl->ois_mutex));
	if ((apply->request_id ==
		o_ctrl->i2c_data.per_frame[request_id].request_id) &&
		(o_ctrl->i2c_data.per_frame[request_id].is_settings_valid)
		== 1) {
		rc = cam_ois_apply_settings(o_ctrl,
			&o_ctrl->i2c_data.per_frame[request_id]);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"[CRM-OIS] Failed in applying the request: %llu\n",
				apply->request_id);
			goto release_mutex;
		} else {
			o_ctrl->i2c_data.per_frame[request_id].is_settings_valid = 0;
		}
		CAM_DBG(CAM_OIS, "Apply setting at req %llu, rc = %d", apply->request_id, rc);
	}

	if (o_ctrl->skip_frame_queue[apply->request_id % MAX_PER_FRAME_ARRAY].req_id == apply->request_id &&
		o_ctrl->skip_frame_queue[apply->request_id % MAX_PER_FRAME_ARRAY].skip_num >= 1) {
		rc = -EAGAIN;
		CAM_INFO(CAM_OIS, "[CRM-OIS] Skipframe at request:%llu", apply->request_id);
	}
		
	if (o_ctrl->skip_frame_queue[apply->request_id % MAX_PER_FRAME_ARRAY].req_id == apply->request_id &&
		o_ctrl->skip_frame_queue[apply->request_id % MAX_PER_FRAME_ARRAY].skip_num > 0) {
		CAM_INFO(CAM_OIS, "[CRM-OIS] Skipframe number from %llu to %llu at request:%llu",
				 o_ctrl->skip_frame_queue[apply->request_id % MAX_PER_FRAME_ARRAY].skip_num,
				 o_ctrl->skip_frame_queue[apply->request_id % MAX_PER_FRAME_ARRAY].skip_num - 1,
				 apply->request_id);
		o_ctrl->skip_frame_queue[apply->request_id % MAX_PER_FRAME_ARRAY].skip_num--;
		if (0 == o_ctrl->skip_frame_queue[apply->request_id % MAX_PER_FRAME_ARRAY].skip_num)
		{
			o_ctrl->skip_frame_queue[apply->request_id % MAX_PER_FRAME_ARRAY].req_id = 0;
		}
	}

	if (rc < 0)
		goto release_mutex;
	del_req_id = (request_id +
		MAX_PER_FRAME_ARRAY - MAX_SYSTEM_PIPELINE_DELAY) %
		MAX_PER_FRAME_ARRAY;

	if (apply->request_id >
		o_ctrl->i2c_data.per_frame[del_req_id].request_id) {
		o_ctrl->i2c_data.per_frame[del_req_id].request_id = 0;
		rc = delete_request(&o_ctrl->i2c_data.per_frame[del_req_id]);
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"[CRM-OIS] Fail deleting the req: %d err: %d\n",
				del_req_id, rc);
			goto release_mutex;
		}
	} else {
		CAM_DBG(CAM_OIS, "[CRM-OIS] No Valid Req to clean Up");
	}

release_mutex:
	mutex_unlock(&(o_ctrl->ois_mutex));
	return rc;
}
// xiaomi add

/**
 * cam_ois_do_frame_skip - cam ois frame skip
 */

bool cam_ois_do_frame_skip(
	int64_t req_id,
	int32_t dev_hdl)
{
	struct cam_ois_ctrl_t *o_ctrl = NULL;
	bool result = false;

	o_ctrl = (struct cam_ois_ctrl_t *)cam_get_device_priv(dev_hdl);


	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "[SkipFrame] Device data is NULL");
		return result;
	}

	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_INFO(CAM_OIS,
			"[SkipFrame] Recvied skip frame packet in invalid state: %d",
			o_ctrl->cam_ois_state);
		return result;
	}

	mutex_lock(&(o_ctrl->ois_mutex));

	if (req_id > 0 && o_ctrl->skip_frame_queue[req_id % MAX_PER_FRAME_ARRAY].req_id == req_id &&
		o_ctrl->skip_frame_queue[req_id % MAX_PER_FRAME_ARRAY].skip_num > 0) {
		-- (o_ctrl->skip_frame_queue[req_id % MAX_PER_FRAME_ARRAY].skip_num);
		if (0 == o_ctrl->skip_frame_queue[req_id % MAX_PER_FRAME_ARRAY].skip_num) {
			o_ctrl->skip_frame_queue[req_id % MAX_PER_FRAME_ARRAY].req_id = 0;
		}
		CAM_INFO(CAM_OIS, "[SkipFrame] req_id:%lld trigger skip frame", req_id);
		result = true;
	}

	mutex_unlock(&(o_ctrl->ois_mutex));

	return result;
}

// xiaomi add
