#include <linux/module.h>
#include <linux/firmware.h>
#include <cam_sensor_cmn_header.h>
#include "cam_ois_core.h"
#include "cam_ois_soc.h"
#include "cam_sensor_util.h"
#include "cam_debug_util.h"
#include "cam_res_mgr_api.h"
#include "cam_common_util.h"
#include "cam_packet_util.h"
#include "rumbas4h.h"

int32_t rumbas4h_load_fw_buff(
	struct cam_ois_ctrl_t *o_ctrl,
	char* firmware_name,
	uint8_t *read_data,
	uint32_t read_length)
{

	uint8_t                             *ptr = NULL;
	int32_t                             rc = 0;
	int32_t                             i = 0;
	const struct firmware               *fw = NULL;
	const char                          *fw_name = firmware_name;
	struct device                       *dev = &(o_ctrl->pdev->dev);

	rc = request_firmware(&fw, fw_name, dev);

	if (rc) {

		CAM_ERR(CAM_OIS, "[RUMBAS4H] Failed to load %s", fw_name);

	} else {

		ptr = (uint8_t *)fw->data;

		if (read_data) {

			for (i = 0; i < read_length; i++) {
				read_data[i] = *(ptr + i);
			}

		}

	}

	release_firmware(fw);

	return rc;
}

int32_t rumbas4h_i2c_read_data(
	struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t length, uint8_t *data)
{
	int32_t rc = 0;
	int32_t i = 0;

	rc = camera_io_dev_read_seq(&o_ctrl->io_master_info,
		addr, data,
		CAMERA_SENSOR_I2C_TYPE_WORD,
		CAMERA_SENSOR_I2C_TYPE_BYTE,
		length);

	for (i = 0; i < length; i++) {
		CAM_DBG(CAM_OIS, "[RUMBAS4H] read addr 0x%04x[%d] = 0x%02x", addr, i, data[i]);
	}

	if (rc) {
		CAM_ERR(CAM_OIS, "[RUMBAS4H] Failed to read, rc: %d", rc);
	}

	return rc;
}

int rumbas4h_ois_apply_settings(struct cam_ois_ctrl_t *o_ctrl,
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

int rumbas4h_ois_pkt_download(struct cam_ois_ctrl_t *o_ctrl)
{
	uint8_t             txdata[TX_BUFFER_SIZE] = {0};
	uint8_t             rxdata[RX_BUFFER_SIZE] = {0};
	uint16_t            i = 0;
	uint16_t            j = 0;
	uint16_t            idx = 0;
	uint16_t            check_sum = 0;
	uint32_t            new_fw_ver = 0;
	uint32_t            current_fw_ver = 0;
	char                *fw_name_prog = NULL;
	char                name_prog[33] = {0};
	uint8_t             *chk_buffer = vmalloc(APP_RUMBAS4HFW_SIZE);
	uint8_t             *fw_data = vmalloc(APP_RUMBAS4HFW_SIZE);
	int32_t             rc = 0;
	uint8_t             i2c_freq_orig_mode = o_ctrl->io_master_info.cci_client->i2c_freq_mode;
	uint8_t             retrytime = 3;
	uint8_t             k = 0;

	if (NULL == chk_buffer || NULL == fw_data) {
		rc = -ENOMEM;
		goto memory_free;
	}

	/* Step1 Get FW Ver from Binary File */
	snprintf(name_prog, 33, "%s.prog", o_ctrl->ois_name);
	fw_name_prog = name_prog;
	rumbas4h_load_fw_buff(o_ctrl, fw_name_prog, fw_data, APP_RUMBAS4HFW_SIZE);
	new_fw_ver = *(uint32_t *)&fw_data[APP_RUMBAS4HFW_SIZE - 12];  /* 0x6FF4 ~ 0x6FF7 */

	/* Step2 Read OIS FW current version*/
	for(k=0; k<retrytime; k++)
	{
		rc = rumbas4h_i2c_read_data(o_ctrl, RUMBAS4H_REG_APP_VER, 4, rxdata);
		if (!rc) {
			CAM_ERR(CAM_OIS, "[RUMBAS4H] rumbas4h_i2c_read_data success retry time is %d",k);
			break;
		}
		msleep(1);
	}
	if (rc)
	{
		CAM_INFO(CAM_OIS, "[RUMBAS4H] rumbas4h_i2c_read_data fail retry time is %d",k);
		rc = -EINVAL;
		goto memory_free;
	}
	current_fw_ver = *(uint32_t *)rxdata;
	CAM_INFO(CAM_OIS, "[RUMBAS4H] current firmware version = %d, new firmware version = %d",
				current_fw_ver, new_fw_ver);

	/* Step3 Adjust whether to run download process */
	if (current_fw_ver == new_fw_ver) {
		CAM_INFO(CAM_OIS, "[RUMBAS4H] do not need do fw update");
	} else {
		/* Step4 If there is firmware that needs to be updated, check OIS now status */
		o_ctrl->io_master_info.cci_client->i2c_freq_mode = I2C_FAST_MODE;
		rumbas4h_i2c_read_data(o_ctrl, RUMBAS4H_REG_OIS_STS, 1, rxdata); /* Read RUMBAS4H_REG_OIS_STS */
		if (rxdata[0] != STATE_IDLE) {
			CAM_ERR(CAM_OIS, "[RUMBAS4H] not in idle status");
			goto memory_free;
		}
		CAM_DBG(CAM_OIS, "[RUMBAS4H] in idle status begin FW update");

		/* Step5 Set the firmware version update control register */
		cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_000C);
		CAM_DBG(CAM_OIS, "[RUMBAS4H] write firmware version update control register");

		/* Step6 Downloading */
		for (i = 0; i < (APP_RUMBAS4HFW_SIZE / 256); i++) {
			CAM_DBG(CAM_OIS, "[RUMBAS4H] Write RUMBAS4H_REG_DATA_BUF i = %d",i);
			memcpy(txdata, &fw_data[idx], 256);
			for (j = 0; j < 256; j++) {
				SETTING_0100[j].reg_addr = RUMBAS4H_REG_DATA_BUF;
				SETTING_0100[j].reg_data = txdata[j];
				SETTING_0100[j].delay = 0;
				SETTING_0100[j].data_mask = 0;
				CAM_DBG(CAM_OIS, "[RUMBAS4H] Write index: %d reg_addr: 0x%x reg_data: 0x%x",
							j ,RUMBAS4H_REG_DATA_BUF, txdata[j]);
			}
			camera_io_dev_write_continuous(&(o_ctrl->io_master_info), &write_0100, 1);
			idx += 256;
		}

		rumbas4h_i2c_read_data(o_ctrl, RUMBAS4H_REG_FWUP_ERR, 2, rxdata);
		if (RUMBAS4H_SUCCESS_STATE == *(uint16_t *)rxdata) {
			/* step7 Write checksum Reg value */
			check_sum = o_ctrl->opcode.fw_version;
			SETTING_0008[0].reg_data = check_sum & 0xFF;
			SETTING_0008[1].reg_data = (check_sum & 0xFF00) >> 8;
			camera_io_dev_write_continuous(&(o_ctrl->io_master_info), &write_0008, 1);

			/* step8 read checksum err status */
			rumbas4h_i2c_read_data(o_ctrl, RUMBAS4H_REG_FWUP_ERR, 2, rxdata);
			CAM_INFO(CAM_OIS, "[RUMBAS4H] CHKSUM status = 0x%x", *((uint16_t*)rxdata));
			if (*((uint16_t*)rxdata) == RUMBAS4H_SUCCESS_STATE) {
				/* Step9 software reset */
				cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_0036);
				cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_000D);
				i = 0;
				do
				{
					rumbas4h_i2c_read_data(o_ctrl, RUMBAS4H_REG_OIS_STS, 1, rxdata);
					i++;
					if (i > 10) {
						CAM_ERR(CAM_OIS, "[RUMBAS4H] software reset fail,addr %x rxdata %x",
									RUMBAS4H_REG_OIS_STS, rxdata);
						CAM_ERR(CAM_OIS, "[RUMBAS4H] FW update fail");
						rc = -EINVAL;
						goto memory_free;
					}
					msleep(1);
				} while (0x09 != rxdata[0]);
				CAM_DBG(CAM_OIS, "[RUMBAS4H] software reset success");
				cam_cci_i2c_write_table(&(o_ctrl->io_master_info), &write_000E);
				CAM_ERR(CAM_OIS, "[RUMBAS4H] FW update success");
			} else {
				CAM_ERR(CAM_OIS, "[RUMBAS4H] FW update fail");
				rc = -EINVAL;
				goto memory_free;
			}
		} else {
			CAM_ERR(CAM_OIS, "[RUMBAS4H] FW update fail");
			rc = -EINVAL;
			goto memory_free;
		}
	}
	o_ctrl->io_master_info.cci_client->i2c_freq_mode = i2c_freq_orig_mode;

	if (o_ctrl->is_ois_calib) {
		rc = rumbas4h_ois_apply_settings(o_ctrl,
			&o_ctrl->i2c_calib_data);
		if ((rc == -EAGAIN) &&
			(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
			CAM_WARN(CAM_OIS,
				"[RUMBAS4H] CCI HW is restting: Reapplying calib settings");
			usleep_range(1000, 1010);
			rc = rumbas4h_ois_apply_settings(o_ctrl,
				&o_ctrl->i2c_calib_data);
		}
		if (rc) {
			CAM_ERR(CAM_OIS, "[RUMBAS4H] Cannot apply calib data");
		} else {
			CAM_DBG(CAM_OIS, "[RUMBAS4H] apply calib data settings success");
		}
	}
	if (o_ctrl->i2c_init_data.is_settings_valid == 1){
		rc = rumbas4h_ois_apply_settings(o_ctrl, &o_ctrl->i2c_init_data);
		if ((rc == -EAGAIN) &&
			(o_ctrl->io_master_info.master_type == CCI_MASTER)) {
			CAM_WARN(CAM_OIS,
				"[RUMBAS4H] CCI HW is restting: Reapplying INIT settings");
			usleep_range(1000, 1010);
			rc = rumbas4h_ois_apply_settings(o_ctrl,
				&o_ctrl->i2c_init_data);
		}
		if (rc < 0) {
			CAM_ERR(CAM_OIS,
				"[RUMBAS4H] Cannot apply Init settings: rc = %d",
				rc);
		} else {
			CAM_DBG(CAM_OIS, "[RUMBAS4H] apply Init settings success");
		}
	}
memory_free:
	o_ctrl->io_master_info.cci_client->i2c_freq_mode = i2c_freq_orig_mode;
	if (NULL != chk_buffer)
	{
		vfree(chk_buffer);
	}
	if (NULL != fw_data)
	{
		vfree(fw_data);
	}
	return rc;
}
