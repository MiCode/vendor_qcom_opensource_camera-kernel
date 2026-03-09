// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/module.h>
#include <cam_sensor_cmn_header.h>
#include "cam_zoom_core.h"
#include "cam_sensor_util.h"
#include "cam_trace.h"
#include "cam_common_util.h"
#include "cam_packet_util.h"
#include "cam_mem_mgr_api.h"
#include <linux/vmalloc.h>
#include "Sem1218s_zoom.h"

/* xiaomi dev protection add*/
#include "xm_cam_dev_protection.h"
/* xiaomi dev protection add*/

static int zoomfwctrl;
module_param(zoomfwctrl, int, 0644);
#define WRITE_BUFFER_MAXSIZE  4

int32_t cam_zoom_construct_default_power_setting(
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

static int32_t cam_zoom_power_up(struct cam_zoom_ctrl_t *z_ctrl)
{
	int rc = 0;
	struct cam_hw_soc_info                 *soc_info = &z_ctrl->soc_info;
	struct cam_zoom_soc_private        *soc_private;
	struct cam_sensor_power_ctrl_t         *power_info;
	struct completion                      *i3c_probe_completion = NULL;
	struct timespec64                       ts1, ts2; // xiaomi add
	long                                    microsec = 0; // xiaomi add

	/* xiaomi add begin */
	CAM_GET_TIMESTAMP(ts1);
	CAM_DBG(MI_DEBUG, "%s start power_up", z_ctrl->device_name);
	/* xiaomi add end */

	soc_private =
		(struct cam_zoom_soc_private *)z_ctrl->soc_info.soc_private;
	power_info = &soc_private->power_info;

	if ((power_info->power_setting == NULL) &&
		(power_info->power_down_setting == NULL)) {
		CAM_INFO(CAM_ZOOM,
			"Using default power settings");
		rc = cam_zoom_construct_default_power_setting(power_info);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"Construct default zoom power setting failed.");
			return rc;
		}
	}

	/* Parse and fill vreg params for power up settings */
	rc = msm_camera_fill_vreg_params(
		&z_ctrl->soc_info,
		power_info->power_setting,
		power_info->power_setting_size);
	if (rc) {
		CAM_ERR(CAM_ZOOM,
			"failed to fill vreg params for power up rc:%d", rc);
		return rc;
	}

	/* Parse and fill vreg params for power down settings*/
	rc = msm_camera_fill_vreg_params(
		&z_ctrl->soc_info,
		power_info->power_down_setting,
		power_info->power_down_setting_size);
	if (rc) {
		CAM_ERR(CAM_ZOOM,
			"failed to fill vreg params power down rc:%d", rc);
		return rc;
	}

	power_info->dev = soc_info->dev;

	if (z_ctrl->io_master_info.master_type == I3C_MASTER)
		i3c_probe_completion = cam_zoom_get_i3c_completion(z_ctrl->soc_info.index);

	rc = cam_sensor_core_power_up(power_info, soc_info, i3c_probe_completion);
	if (rc) {
		CAM_ERR(CAM_ZOOM,
			"failed in zoom power up rc %d", rc);
		return rc;
	}

	rc = camera_io_init(&z_ctrl->io_master_info);
	if (rc < 0) {
		CAM_ERR(CAM_ZOOM, "cci init failed: rc: %d", rc);
		goto cci_failure;
	}
	/* xiaomi add begin */
	CAM_GET_TIMESTAMP(ts2);
	CAM_GET_TIMESTAMP_DIFF_IN_MICRO(ts1, ts2, microsec);
	CAM_DBG(MI_DEBUG, "%s end power_up, occupy time is: %ld ms",
		z_ctrl->device_name, microsec/1000);
	/* xiaomi add end */

	return rc;
cci_failure:
	if (cam_sensor_util_power_down(power_info, soc_info))
		CAM_ERR(CAM_ZOOM, "Power down failure");

	return rc;
}

static int32_t cam_zoom_power_down(struct cam_zoom_ctrl_t *z_ctrl)
{
	int32_t rc = 0;
	struct cam_sensor_power_ctrl_t *power_info;
	struct cam_hw_soc_info *soc_info = &z_ctrl->soc_info;
	struct cam_zoom_soc_private  *soc_private;
	struct timespec64 ts1, ts2; // xiaomi add
	long microsec = 0; // xiaomi add

	/* xiaomi add begin */
	CAM_GET_TIMESTAMP(ts1);
	CAM_DBG(MI_DEBUG, "%s start power_down", z_ctrl->device_name);
	/* xiaomi add end */

	if (!z_ctrl) {
		CAM_ERR(CAM_ZOOM, "failed: z_ctrl %pK", z_ctrl);
		return -EINVAL;
	}

	soc_private =
		(struct cam_zoom_soc_private *)z_ctrl->soc_info.soc_private;
	power_info = &soc_private->power_info;
	soc_info = &z_ctrl->soc_info;

	if (!power_info) {
		CAM_ERR(CAM_ZOOM, "failed: power_info %pK", power_info);
		return -EINVAL;
	}
	rc = cam_sensor_util_power_down(power_info, soc_info);
	if (rc) {
		CAM_ERR(CAM_ZOOM, "power down the core is failed:%d", rc);
		return rc;
	}

	camera_io_release(&z_ctrl->io_master_info);
	/* xiaomi add begin */
	CAM_GET_TIMESTAMP(ts2);
	CAM_GET_TIMESTAMP_DIFF_IN_MICRO(ts1, ts2, microsec);
	CAM_DBG(MI_DEBUG, "%s end power_down, occupy time is: %ld ms",
		z_ctrl->device_name, microsec/1000);
	/* xiaomi add end */

	return rc;
}

static int32_t cam_zoom_i2c_modes_util(
	struct camera_io_master *io_master_info,
	struct i2c_settings_list *i2c_list)
{
	int32_t rc = 0;
	uint32_t i, size;

	if (i2c_list->op_code == CAM_SENSOR_I2C_WRITE_RANDOM) {
		rc = camera_io_dev_write(io_master_info,
			&(i2c_list->i2c_settings));
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"Failed to random write I2C settings: %d",
				rc);
			return rc;
		}
	} else if (i2c_list->op_code == CAM_SENSOR_I2C_WRITE_SEQ) {
		rc = camera_io_dev_write_continuous(
			io_master_info,
			&(i2c_list->i2c_settings),
			CAM_SENSOR_I2C_WRITE_SEQ);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"Failed to seq write I2C settings: %d",
				rc);
			return rc;
			}
	} else if (i2c_list->op_code == CAM_SENSOR_I2C_WRITE_BURST) {
		rc = camera_io_dev_write_continuous(
			io_master_info,
			&(i2c_list->i2c_settings),
			CAM_SENSOR_I2C_WRITE_BURST);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"Failed to burst write I2C settings: %d",
				rc);
			return rc;
		}
	} else if (i2c_list->op_code == CAM_SENSOR_I2C_POLL) {
		size = i2c_list->i2c_settings.size;
		for (i = 0; i < size; i++) {
			rc = camera_io_dev_poll(
			io_master_info,
			i2c_list->i2c_settings.reg_setting[i].reg_addr,
			i2c_list->i2c_settings.reg_setting[i].reg_data,
			i2c_list->i2c_settings.reg_setting[i].data_mask,
			i2c_list->i2c_settings.addr_type,
			i2c_list->i2c_settings.data_type,
			i2c_list->i2c_settings.reg_setting[i].delay);
			if (rc < 0) {
				CAM_ERR(CAM_ZOOM,
					"i2c poll apply setting Fail: %d", rc);
				return rc;
			}
		}
	}

	return rc;
}

int32_t cam_zoom_slaveInfo_pkt_parser(struct cam_zoom_ctrl_t *z_ctrl,
	uint32_t *cmd_buf, size_t len)
{
	int32_t rc = 0;
	struct cam_cmd_zoom_info *i2c_info;

	if (!z_ctrl || !cmd_buf || (len < sizeof(struct cam_cmd_zoom_info))) {
		CAM_ERR(CAM_ZOOM, "Invalid Args");
		return -EINVAL;
	}

	i2c_info = (struct cam_cmd_zoom_info *)cmd_buf;
	if (z_ctrl->io_master_info.master_type == CCI_MASTER) {
		z_ctrl->io_master_info.cci_client->cci_i2c_master =
			z_ctrl->cci_i2c_master;
		z_ctrl->io_master_info.cci_client->i2c_freq_mode =
			i2c_info->i2c_freq_mode;
		z_ctrl->io_master_info.cci_client->sid =
			i2c_info->slave_addr >> 1;

		z_ctrl->zoom_3polefw_version = i2c_info->zoom_3polefw_version;
		z_ctrl->zoom_5polefw_version = i2c_info->zoom_5polefw_version;
		z_ctrl->zoom_hw_version = i2c_info->zoom_hw_version;
		z_ctrl->noupdate_fw_flash = i2c_info->noupdate_fw_flash;
		memcpy(z_ctrl->zoom_name, i2c_info->zoom_name, ZOOM_NAME_LEN);
		z_ctrl->zoom_name[ZOOM_NAME_LEN - 1] = '\0';
		CAM_DBG(CAM_ZOOM, "Slave addr: 0x%x Freq Mode: %d zoom_3polefw_version:%d zoom_5polefw_version:%d zoom_hw_version:%d zoom_name:%s",
			i2c_info->slave_addr, i2c_info->i2c_freq_mode, i2c_info->zoom_3polefw_version,
			i2c_info->zoom_5polefw_version, i2c_info->zoom_hw_version, z_ctrl->zoom_name);
	} else if (z_ctrl->io_master_info.master_type == I2C_MASTER &&
			(z_ctrl->io_master_info.qup_client != NULL)) {
		z_ctrl->io_master_info.qup_client->i2c_client->addr = i2c_info->slave_addr;
		CAM_DBG(CAM_ZOOM, "Slave addr: 0x%x", i2c_info->slave_addr);
	} else {
		CAM_ERR(CAM_ZOOM, "Invalid Master type: %d",
			z_ctrl->io_master_info.master_type);
		 rc = -EINVAL;
	}

	return rc;
}

/* xiaomi dev protection modified*/
int32_t cam_zoom_apply_settings(struct cam_zoom_ctrl_t *z_ctrl,
	struct i2c_settings_array *i2c_set,
	enum xm_cam_dev_i2c_cmd_type i2c_cmd_type)
/* xiaomi dev protection modified*/
{
	struct i2c_settings_list *i2c_list;
	int32_t rc = 0;
	int32_t j = 0; // xiaomi add

	if (z_ctrl == NULL || i2c_set == NULL) {
		CAM_ERR(CAM_ZOOM, "Invalid Args");
		return -EINVAL;
	}

	if (i2c_set->is_settings_valid != 1) {
		CAM_ERR(CAM_ZOOM, " Invalid settings");
		return -EINVAL;
	}

	/* xiaomi dev protection add*/
	if (XM_CAM_DEV_XXSKIP_SENSOR_OP_CODE == xm_cam_dev_protection_enable2(XM_CAM_DEV_TYPE_OIS, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
		if (XM_CAM_DEV_XXSKIP_SENSOR_OP_CODE == xm_cam_dev_need_skip_i2c_operation3(get_zoom_xm_cam_dev_info(z_ctrl), i2c_cmd_type)) {
			CAM_INFO(CAM_ZOOM, "skip i2c operation");
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
				trace_cam_i2c_write_log_event("[ZOOMSETTINGS]", z_ctrl->device_name,
					i2c_set->request_id, j, "WRITE", i2c_list->i2c_settings.reg_setting[j].reg_addr,
					i2c_list->i2c_settings.reg_setting[j].reg_data);
			}
			break;
		}
		case CAM_SENSOR_I2C_READ_RANDOM:
		case CAM_SENSOR_I2C_READ_SEQ: {
			for (j = 0;j < i2c_list->i2c_settings.size;j++) {
				trace_cam_i2c_write_log_event("[ZOOMSETTINGS]", z_ctrl->device_name,
					i2c_set->request_id, j, "READ", i2c_list->i2c_settings.reg_setting[j].reg_addr,
					i2c_list->i2c_settings.reg_setting[j].reg_data);
			}
			break;
		}
		case CAM_SENSOR_I2C_POLL: {
			for (j = 0;j < i2c_list->i2c_settings.size;j++) {
				trace_cam_i2c_write_log_event("[ZOOMSETTINGS]", z_ctrl->device_name,
					i2c_set->request_id, j, "POLL", i2c_list->i2c_settings.reg_setting[j].reg_addr,
					i2c_list->i2c_settings.reg_setting[j].reg_data);
			}
			break;
		}
		default:
			break;
		}
		/* xiaomi add I2C trace end */
		rc = cam_zoom_i2c_modes_util(
			&(z_ctrl->io_master_info),
			i2c_list);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM, "Failed to apply settings: %d", rc);
		} else {
			CAM_DBG(CAM_ZOOM, "Success:request ID: %d",
				i2c_set->request_id);
		}
		/* xiaomi dev protection add*/
		if (true == xm_cam_dev_protection_enable(XM_CAM_DEV_TYPE_ZOOM, XM_CAM_DEV_PROTECTION_TYPE_I2C)) {
			if (XM_CMD_DEV_I2C_ZOOM_CMD_TYPE_INIT == i2c_cmd_type &&
				true == xm_cam_dev_is_i2c_write_cmd(i2c_list)) {
				xm_cam_dev_set_init_result(get_zoom_xm_cam_dev_info(z_ctrl), XM_CAM_DEV_INIT_STATUS_SUCCESS);
				if (rc < 0) {
					xm_cam_dev_set_status_info(get_zoom_xm_cam_dev_info(z_ctrl),
												XM_CAM_DEV_OPERATION_INIT,
												XM_CAM_DEV_STATUS_CODE_I2C_WRITE_ERROR);
					xm_cam_dev_set_init_result(get_zoom_xm_cam_dev_info(z_ctrl), XM_CAM_DEV_INIT_STATUS_FAILURE);
					if (true == xm_cam_dev_need_change_i2c_rc(get_zoom_xm_cam_dev_info(z_ctrl), i2c_cmd_type)) {
						rc = 0;
					}
				} else {
					xm_cam_dev_set_status_info(get_zoom_xm_cam_dev_info(z_ctrl),
												XM_CAM_DEV_OPERATION_INIT,
												XM_CAM_DEV_STATUS_CODE_I2C_WRITE_SUCCESS);
				}
			}
		}
		/* xiaomi dev protection add*/
	}

	return rc;
}

int32_t cam_zoom_apply_request(struct cam_req_mgr_apply_request *apply)
{
	int32_t rc = 0, request_id, del_req_id;
	struct cam_zoom_ctrl_t *z_ctrl = NULL;

	if (!apply) {
		CAM_ERR(CAM_ZOOM, "Invalid Input Args");
		return -EINVAL;
	}

	z_ctrl = (struct cam_zoom_ctrl_t *)
		cam_get_device_priv(apply->dev_hdl);
	if (!z_ctrl) {
		CAM_ERR(CAM_ZOOM, "Device data is NULL");
		return -EINVAL;
	}
	request_id = apply->request_id % MAX_PER_FRAME_ARRAY;

	trace_cam_apply_req("Zoom", z_ctrl->soc_info.index, apply->request_id, apply->link_hdl);

	CAM_DBG(CAM_ZOOM, "Request Id: %lld", apply->request_id);
	mutex_lock(&(z_ctrl->zoom_mutex));
	if ((apply->request_id ==
		z_ctrl->i2c_data.per_frame[request_id].request_id) &&
		(z_ctrl->i2c_data.per_frame[request_id].is_settings_valid)
		== 1) {
		rc = cam_zoom_apply_settings(z_ctrl,
			&z_ctrl->i2c_data.per_frame[request_id],
			XM_CMD_DEV_I2C_ZOOM_CMD_TYPE_CRM_CONGIG);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"Failed in applying the request: %lld\n",
				apply->request_id);
			goto release_mutex;
		}
	}
	del_req_id = (request_id +
		MAX_PER_FRAME_ARRAY - MAX_SYSTEM_PIPELINE_DELAY) %
		MAX_PER_FRAME_ARRAY;

	if (apply->request_id >
		z_ctrl->i2c_data.per_frame[del_req_id].request_id) {
		z_ctrl->i2c_data.per_frame[del_req_id].request_id = 0;
		rc = delete_request(&z_ctrl->i2c_data.per_frame[del_req_id]);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"Fail deleting the req: %d err: %d\n",
				del_req_id, rc);
			goto release_mutex;
		}
	} else {
		CAM_DBG(CAM_ZOOM, "No Valid Req to clean Up");
	}

release_mutex:
	mutex_unlock(&(z_ctrl->zoom_mutex));
	return rc;
}

int32_t cam_zoom_establish_link(
	struct cam_req_mgr_core_dev_link_setup *link)
{
	struct cam_zoom_ctrl_t *z_ctrl = NULL;

	if (!link) {
		CAM_ERR(CAM_ZOOM, "Invalid Args");
		return -EINVAL;
	}

	z_ctrl = (struct cam_zoom_ctrl_t *)
		cam_get_device_priv(link->dev_hdl);
	if (!z_ctrl) {
		CAM_ERR(CAM_ZOOM, "Device data is NULL");
		return -EINVAL;
	}

	mutex_lock(&(z_ctrl->zoom_mutex));
	if (link->link_enable) {
		z_ctrl->bridge_intf.link_hdl = link->link_hdl;
		z_ctrl->bridge_intf.crm_cb = link->crm_cb;
	} else {
		z_ctrl->bridge_intf.link_hdl = -1;
		z_ctrl->bridge_intf.crm_cb = NULL;
	}
	mutex_unlock(&(z_ctrl->zoom_mutex));

	return 0;
}

static int cam_zoom_update_req_mgr(
	struct cam_zoom_ctrl_t *z_ctrl,
	struct cam_packet *csl_packet)
{
	int rc = 0;
	struct cam_req_mgr_add_request add_req;

	memset(&add_req, 0, sizeof(add_req));
	add_req.link_hdl = z_ctrl->bridge_intf.link_hdl;
	add_req.req_id = csl_packet->header.request_id;
	add_req.dev_hdl = z_ctrl->bridge_intf.device_hdl;

	if (z_ctrl->bridge_intf.crm_cb &&
		z_ctrl->bridge_intf.crm_cb->add_req) {
		rc = z_ctrl->bridge_intf.crm_cb->add_req(&add_req);
		if (rc) {
			if (rc == -EBADR)
				CAM_INFO(CAM_ZOOM,
					"Adding request: %llu failed: rc: %d, it has been flushed",
					csl_packet->header.request_id, rc);
			else
				CAM_ERR(CAM_ZOOM,
					"Adding request: %llu failed: rc: %d",
					csl_packet->header.request_id, rc);
			return rc;
		}
		CAM_DBG(CAM_ZOOM, "Request Id: %lld added to CRM",
			add_req.req_id);
	} else {
		CAM_ERR(CAM_ZOOM, "Can't add Request ID: %lld to CRM",
			csl_packet->header.request_id);
		rc = -EINVAL;
	}

	return rc;
}

int32_t cam_zoom_publish_dev_info(struct cam_req_mgr_device_info *info)
{
	struct cam_zoom_ctrl_t *z_ctrl;
	if (!info) {
		CAM_ERR(CAM_ZOOM, "Invalid Args");
		return -EINVAL;
	}

	z_ctrl = (struct cam_zoom_ctrl_t *)
		cam_get_device_priv(info->dev_hdl);
	if (!z_ctrl) {
		CAM_ERR(CAM_ZOOM, "Device data is NULL");
		return -EINVAL;
	}

	info->dev_id = CAM_REQ_MGR_DEVICE_ZOOM;
	snprintf(info->name, sizeof(info->name), "%s(camera-zoom%u)",
		CAM_ZOOM_NAME, z_ctrl->soc_info.index);
	info->p_delay = CAM_PIPELINE_DELAY_1;
	info->m_delay = CAM_MODESWITCH_DELAY_1;
	info->trigger = CAM_TRIGGER_POINT_SOF;

	return 0;
}

/*xiaomi modify start*/
static int32_t i2c_write_data_seq(struct camera_io_master *io_master_info, uint32_t addr, uint32_t length, uint8_t* data, uint32_t delay)
{
	struct cam_sensor_i2c_reg_array w_data[WRITE_BUFFER_MAXSIZE] = { {0} };
	struct cam_sensor_i2c_reg_setting write_setting;
	uint32_t i = 0;
	int32_t rc = 0;

	if (!data || (length < 1) || WRITE_BUFFER_MAXSIZE < length) {
		CAM_ERR(CAM_ZOOM, "[SEM1218S] Invalid Args");
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
		CAM_ERR(CAM_ZOOM, "[SEM1218S] ZOOM i2c_write_data write failed, rc: %d", rc);
	}

	for (i = 0; i < length; i++) {
		CAM_DBG(CAM_ZOOM, "[SEM1218S] write i %d is add 0x%04x data 0x%x delay %d",
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
		CAM_DBG(CAM_ZOOM, "[SEM1218S] read addr 0x%04x[%d] = 0x%02x", addr, i, data[i]);
	}

	if (rc) {
		CAM_ERR(CAM_ZOOM, "[SEM1218S] Failed to read, rc: %d", rc);
	}

	return rc;
}

static int cam_sem1218s_zoom_fw_download(struct cam_zoom_ctrl_t *z_ctrl)
{
	uint8_t txdata[TX_BUFFER_SIZE];
	uint8_t rxdata[RX_BUFFER_SIZE];
	uint16_t txBuffSize;
	uint16_t i = 0;
	uint32_t crc,updated_ver,new_3polefw_ver,new_5polefw_ver,cur_hw_ver,new_fw_ver,current_fw_ver;
	char	*fw_name_prog = NULL;
	char	name_prog[ZOOM_NAME_LEN] = {0};
	uint8_t *fw_data = NULL;
	int32_t rc = 0;
	int32_t cnt = 0;
	struct cam_sensor_i2c_reg_setting   setting;
	void                               *vaddr = NULL;
	uint8_t                            *ptr = NULL;

	i2c_read_data_seq(&(z_ctrl->io_master_info), REG_APP_VER, 4, rxdata);
	current_fw_ver = *(uint32_t *)rxdata;

	new_3polefw_ver = z_ctrl->zoom_3polefw_version;
	new_5polefw_ver = z_ctrl->zoom_5polefw_version;
	cur_hw_ver = z_ctrl->zoom_hw_version;

	if (cur_hw_ver == 1) {
		if(((current_fw_ver == new_5polefw_ver) ||(current_fw_ver == 0) || (new_5polefw_ver == 0)) && (zoomfwctrl == 0)){
			CAM_DBG(CAM_ZOOM, "[SEM1218S_ZOOM] current_fw_ver:%d new_5polefw_ver:%d, skip sem1218s_zoom_fw_download",
						current_fw_ver, new_5polefw_ver);
			return rc;
		}
	} else {
		if(((current_fw_ver == new_3polefw_ver) ||(current_fw_ver == 0) || (new_3polefw_ver == 0)) && (zoomfwctrl == 0)){
			CAM_DBG(CAM_ZOOM, "[SEM1218S_ZOOM] current_fw_ver:%d new_3polefw_ver:%d, skip sem1218s_zoom_fw_download",
						current_fw_ver, new_3polefw_ver);
			return rc;
		}
	}

	vaddr = vmalloc((sizeof(struct cam_sensor_i2c_reg_array) * TX_BUFFER_SIZE));
	if (!vaddr) {
		CAM_ERR(CAM_ZOOM,
			"[SEM1218S] Failed in allocating i2c_array: size: %u",
			(sizeof(struct cam_sensor_i2c_reg_array) * TX_BUFFER_SIZE));
		return -ENOMEM;
	}
	setting.reg_setting = (struct cam_sensor_i2c_reg_array *) (vaddr);
	setting.addr_type   = CAMERA_SENSOR_I2C_TYPE_WORD;
	setting.data_type   = CAMERA_SENSOR_I2C_TYPE_BYTE;
	setting.size        = TX_BUFFER_SIZE;
	setting.delay       = 0;//delay 0 ms

	fw_data = vmalloc(APP_FW_SIZE);

	if (cur_hw_ver == 1) {
		/* Get FW Ver from Binary File */
		snprintf(name_prog, ZOOM_NAME_LEN, "%s.5pole", z_ctrl->zoom_name);
		CAM_INFO(CAM_ZOOM, "[SEM1218S_ZOOM] start fw update current_fw_ver:%d, new_5polefw_ver:%d", current_fw_ver, new_5polefw_ver);
	} else {
		/* Get FW Ver from Binary File */
		snprintf(name_prog, ZOOM_NAME_LEN, "%s.3pole", z_ctrl->zoom_name);
		CAM_INFO(CAM_ZOOM, "[SEM1218S_ZOOM] start fw update current_fw_ver:%d, new_3polefw_ver:%d", current_fw_ver, new_3polefw_ver);
	}

	fw_name_prog = name_prog;
	rc = load_fw_buff(z_ctrl, fw_name_prog, fw_data, APP_FW_SIZE);
	if(rc) {
		CAM_ERR(CAM_ZOOM, "Failed to load firmware %s", fw_name_prog);
		return rc;
	}
	new_fw_ver = *(uint32_t *)&fw_data[APP_FW_SIZE - 12];  /* 0x7FF4 ~ 0x7FF7 */

	CAM_INFO(CAM_ZOOM, "[SEM1218S_ZOOM] current_fw_ver = %d, new_fw_ver = %d noupdate_fw_flash %d",
		current_fw_ver, new_fw_ver, z_ctrl->noupdate_fw_flash);
	if(((current_fw_ver == new_fw_ver) ||(current_fw_ver == 0) || (new_fw_ver == 0) || (z_ctrl->noupdate_fw_flash)) && 
		(zoomfwctrl == 0)){
		vfree(fw_data);
		vfree(vaddr);
		fw_data = NULL;
		vaddr = NULL;
		return rc;
	}else{
		/* If have FW app, Turnoff AFZM */
		if (current_fw_ver != 0)
		{
			i2c_read_data_seq(&(z_ctrl->io_master_info), REG_AFZM_STS, 1, rxdata);  /* Read REG_AFZM_STS */
			if (rxdata[0] != STATE_READY)
			{
				txdata[0] = AFZM_OFF;  /* Set AFZM_OFF */
				i2c_write_data_seq(&(z_ctrl->io_master_info), REG_AFZM_CTRL, 1, txdata, 0); /* Write 1 Byte to REG_AFZM_CTRL */
			}
		}
		/* PAYLOAD_LEN = Size Bytes, FW_UPEN = TRUE */
		txBuffSize = TX_SIZE_256_BYTE;
		switch (txBuffSize)
		{
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
				/* Does not setting Tx data size, Alert Message */
				break;
		}
		/* To update firmware using Secure Boot, please add the following statement. */
		txdata[0] |= SECURE_BOOT;
		i2c_write_data_seq(&(z_ctrl->io_master_info), REG_FWUP_CTRL, 1, txdata, 0);
		msleep(200);

		ptr = fw_data;
		for (i = 0; i < APP_FW_SIZE; i += TX_SIZE_256_BYTE) {
			CAM_DBG(CAM_ZOOM, "[SEM1218S] Write REG_DATA_BUF i = %d", i);
			for (cnt = 0; cnt < TX_SIZE_256_BYTE; cnt++, ptr++) {
				setting.reg_setting[cnt].reg_addr = REG_DATA_BUF + cnt;
				setting.reg_setting[cnt].reg_data = *((uint8_t *)ptr);
				setting.reg_setting[cnt].delay = 0;
				setting.reg_setting[cnt].data_mask = 0;
				CAM_DBG(CAM_ZOOM, "[SEM1218S] count %d address is: 0x%04x data is 0x%x",
					cnt, setting.reg_setting[cnt].reg_addr, setting.reg_setting[cnt].reg_data);
			}
			rc = camera_io_dev_write_continuous(&(z_ctrl->io_master_info),
					&setting, CAM_SENSOR_I2C_WRITE_SEQ);

			if (rc < 0) {
				CAM_ERR(CAM_ZOOM,
					"[SEM1218S] Failed in Applying i2c wrt settings");
				break;
			}
			msleep(20);
		}

		crc = CalculateCRC32(fw_data, APP_FW_SIZE);
		vfree(fw_data);
		fw_data = NULL;
		vfree(vaddr);
		vaddr = NULL;

		*(uint32_t *)txdata = crc;
		i2c_write_data_seq(&(z_ctrl->io_master_info), REG_FWUP_CRC, 4, txdata, 0);
		msleep(200);

		i2c_read_data_seq(&(z_ctrl->io_master_info), REG_FWUP_ERR, 1, rxdata);
		CAM_INFO(CAM_ZOOM, "[REG_FWUP_ERR] = 0x%x", rxdata[0]);
		if (rxdata[0] != NO_ERROR) {
			CAM_ERR(CAM_ZOOM, "[Error] : FW Update != NO_ERROR");
			return rc;
		}

		txdata[0] = RESET_REQ;
		i2c_write_data_seq(&(z_ctrl->io_master_info), REG_FWUP_CTRL, 1, txdata, 0);
		msleep(200);
		i2c_read_data_seq(&(z_ctrl->io_master_info), REG_APP_VER, 4, rxdata);
		updated_ver = *(uint32_t *)rxdata;
		CAM_INFO(CAM_ZOOM, "[updated_ver] = %d,[new_fw_ver] = %d",updated_ver,new_fw_ver);
		if (updated_ver != new_fw_ver) {
			CAM_ERR(CAM_ZOOM, "[Error]: updated_ver != new_fw_ver");
			return rc;
		}
		CAM_INFO(CAM_ZOOM, "FW Update Success.");
	}
	return rc;
}

int32_t cam_zoom_i2c_pkt_parse(struct cam_zoom_ctrl_t *z_ctrl,
	void *arg)
{
	int32_t  rc = 0;
	int32_t  i = 0;
	uint32_t total_cmd_buf_in_bytes = 0;
	size_t   len_of_buff = 0;
	size_t   remain_len = 0;
	uint32_t *offset = NULL;
	uint32_t *cmd_buf = NULL;
	uintptr_t generic_ptr;
	uintptr_t generic_pkt_ptr;
	struct common_header      *cmm_hdr = NULL;
	struct cam_control        *ioctl_ctrl = NULL;
	struct cam_packet         *csl_packet = NULL;
	struct cam_packet         *csl_packet_u = NULL;
	struct cam_config_dev_cmd config;
	struct i2c_data_settings  *i2c_data = NULL;
	struct i2c_settings_array *i2c_reg_settings = NULL;
	struct cam_cmd_buf_desc   *cmd_desc = NULL;
	struct cam_zoom_soc_private *soc_private = NULL;
	struct cam_sensor_power_ctrl_t  *power_info = NULL;

	if (!z_ctrl || !arg) {
		CAM_ERR(CAM_ZOOM, "Invalid Args");
		return -EINVAL;
	}

	soc_private =
		(struct cam_zoom_soc_private *)z_ctrl->soc_info.soc_private;

	power_info = &soc_private->power_info;

	ioctl_ctrl = (struct cam_control *)arg;
	if (copy_from_user(&config,
		u64_to_user_ptr(ioctl_ctrl->handle),
		sizeof(config)))
		return -EFAULT;
	rc = cam_mem_get_cpu_buf(config.packet_handle,
		&generic_pkt_ptr, &len_of_buff);
	if (rc < 0) {
		CAM_ERR(CAM_ZOOM, "Error in converting command Handle %d",
			rc);
		return rc;
	}

	remain_len = len_of_buff;
	if ((sizeof(struct cam_packet) > len_of_buff) ||
		((size_t)config.offset >= len_of_buff -
		sizeof(struct cam_packet))) {
		CAM_ERR(CAM_ZOOM,
			"Inval cam_packet strut size: %zu, len_of_buff: %zu",
			 sizeof(struct cam_packet), len_of_buff);
		rc = -EINVAL;
		goto put_buf;
	}

	remain_len -= (size_t)config.offset;
	csl_packet_u = (struct cam_packet *)
		(generic_pkt_ptr + (uint32_t)config.offset);
	rc = cam_packet_util_copy_pkt_to_kmd(csl_packet_u, &csl_packet, remain_len);
	if (rc) {
		CAM_ERR(CAM_ZOOM, "Copying packet to KMD failed");
		goto end;
	}

	CAM_DBG(CAM_ZOOM, "Pkt opcode: %d",	csl_packet->header.op_code);

	if ((csl_packet->header.op_code & 0xFFFFFF) !=
		CAM_ZOOM_PACKET_OPCODE_INIT &&
		csl_packet->header.request_id <= z_ctrl->last_flush_req
		&& z_ctrl->last_flush_req != 0) {
		CAM_DBG(CAM_ZOOM,
			"reject request %lld, last request to flush %lld",
			csl_packet->header.request_id, z_ctrl->last_flush_req);
		rc = -EBADR;
		goto end;
	}

	if (csl_packet->header.request_id > z_ctrl->last_flush_req)
		z_ctrl->last_flush_req = 0;

	switch (csl_packet->header.op_code & 0xFFFFFF) {
	case CAM_ZOOM_PACKET_OPCODE_INIT:
		offset = (uint32_t *)&csl_packet->payload_flex;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);

		/* Loop through multiple command buffers */
		for (i = 0; i < csl_packet->num_cmd_buf; i++) {
			rc = cam_packet_util_validate_cmd_desc(&cmd_desc[i]);
			if (rc)
				goto end;

			total_cmd_buf_in_bytes = cmd_desc[i].length;
			if (!total_cmd_buf_in_bytes)
				continue;
			rc = cam_mem_get_cpu_buf(cmd_desc[i].mem_handle,
					&generic_ptr, &len_of_buff);
			if (rc < 0) {
				CAM_ERR(CAM_ZOOM, "Failed to get cpu buf");
				goto end;
			}
			cmd_buf = (uint32_t *)generic_ptr;
			if (!cmd_buf) {
				CAM_ERR(CAM_ZOOM, "invalid cmd buf");
				cam_mem_put_cpu_buf(cmd_desc[i].mem_handle);
				rc = -EINVAL;
				goto end;
			}
			if ((len_of_buff < sizeof(struct common_header)) ||
				(cmd_desc[i].offset > (len_of_buff -
				sizeof(struct common_header)))) {
				CAM_ERR(CAM_ZOOM,
					"Invalid length for sensor cmd");
				cam_mem_put_cpu_buf(cmd_desc[i].mem_handle);
				rc = -EINVAL;
				goto end;
			}
			remain_len = len_of_buff - cmd_desc[i].offset;
			cmd_buf += cmd_desc[i].offset / sizeof(uint32_t);
			cmm_hdr = (struct common_header *)cmd_buf;

			switch (cmm_hdr->cmd_type) {
			case CAMERA_SENSOR_CMD_TYPE_I2C_INFO:
				CAM_DBG(CAM_ZOOM,
					"Received slave info buffer");
				rc = cam_zoom_slaveInfo_pkt_parser(
					z_ctrl, cmd_buf, remain_len);
				if (rc < 0) {
					CAM_ERR(CAM_ZOOM,
					"Failed to parse slave info: %d", rc);
					cam_mem_put_cpu_buf(cmd_desc[i].mem_handle);
					goto end;
				}
				break;
			case CAMERA_SENSOR_CMD_TYPE_PWR_UP:
			case CAMERA_SENSOR_CMD_TYPE_PWR_DOWN:
				CAM_DBG(CAM_ZOOM,
					"Received power settings buffer");

				rc = cam_sensor_update_power_settings(
					cmd_buf,
					total_cmd_buf_in_bytes,
					power_info, remain_len);
				if (rc) {
					CAM_ERR(CAM_ZOOM,
					"Failed:parse power settings: %d",
					rc);
					cam_mem_put_cpu_buf(cmd_desc[i].mem_handle);
					goto end;
				}

				break;
			default:
				CAM_DBG(CAM_ZOOM,
					"Received initSettings buffer");
				i2c_data = &(z_ctrl->i2c_data);
				i2c_reg_settings =
					&i2c_data->init_settings;

				i2c_reg_settings->request_id = 0;
				i2c_reg_settings->is_settings_valid = 1;
				rc = cam_sensor_i2c_command_parser(
					&z_ctrl->io_master_info,
					i2c_reg_settings,
					&cmd_desc[i], 1, NULL);
				if (rc < 0) {
					CAM_ERR(CAM_ZOOM,
					"Failed:parse init settings: %d",
					rc);
					cam_mem_put_cpu_buf(cmd_desc[i].mem_handle);
					goto end;
				}
				break;
			}
			cam_mem_put_cpu_buf(cmd_desc[i].mem_handle);
		}

		if (z_ctrl->cam_zoom_state == CAM_ZOOM_ACQUIRE) {
			rc = cam_zoom_power_up(z_ctrl);
			if (rc < 0) {
				CAM_ERR(CAM_ZOOM,
					"Zoom Power up failed");
				goto end;
			}
			z_ctrl->cam_zoom_state = CAM_ZOOM_CONFIG;
		}

		if (z_ctrl->zoom_3polefw_version != 0 || z_ctrl->zoom_5polefw_version != 0) {
			rc = cam_sem1218s_zoom_fw_download(z_ctrl);
			if (rc) {
				CAM_ERR(CAM_ZOOM, "Failed sem1218s zoom FW Download: %d", rc);
				goto end;
			}
		}

		rc = cam_zoom_apply_settings(z_ctrl,
			&z_ctrl->i2c_data.init_settings,
			XM_CMD_DEV_I2C_ZOOM_CMD_TYPE_INIT);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM, "Cannot apply Init settings");
			goto end;
		}

		/* Delete the request even if the apply is failed */
		rc = delete_request(&z_ctrl->i2c_data.init_settings);
		if (rc < 0) {
			CAM_WARN(CAM_ZOOM,
				"Fail in deleting the Init settings");
		}
		break;
	case CAM_ZOOM_PACKET_AUTO_MOVE_LENS:
		if (z_ctrl->cam_zoom_state < CAM_ZOOM_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_ZOOM,
				"Not in right state to move lens: %d",
				z_ctrl->cam_zoom_state);
			goto end;
		}
		z_ctrl->setting_apply_state = ZOOM_APPLY_SETTINGS_NOW;

		i2c_data = &(z_ctrl->i2c_data);
		i2c_reg_settings = &i2c_data->init_settings;

		i2c_data->init_settings.request_id =
			csl_packet->header.request_id;
		i2c_reg_settings->is_settings_valid = 1;
		offset = (uint32_t *)&csl_packet->payload_flex;
		offset += csl_packet->cmd_buf_offset / sizeof(uint32_t);
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);
		rc = cam_sensor_i2c_command_parser(
			&z_ctrl->io_master_info,
			i2c_reg_settings,
			cmd_desc, 1, NULL);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"Auto move lens parsing failed: %d", rc);
			goto end;
		}
		rc = cam_zoom_update_req_mgr(z_ctrl, csl_packet);
		if (rc) {
			CAM_ERR(CAM_ZOOM,
				"Failed in adding request to request manager");
			goto end;
		}
		break;
	case CAM_ZOOM_PACKET_MANUAL_MOVE_LENS:
		if (z_ctrl->cam_zoom_state < CAM_ZOOM_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_ZOOM,
				"Not in right state to move lens: %d",
				z_ctrl->cam_zoom_state);
			goto end;
		}

		z_ctrl->setting_apply_state = ZOOM_APPLY_SETTINGS_LATER;

		i2c_data = &(z_ctrl->i2c_data);
		i2c_reg_settings = &i2c_data->per_frame[
			csl_packet->header.request_id % MAX_PER_FRAME_ARRAY];

		i2c_reg_settings->request_id =
			csl_packet->header.request_id;
		i2c_reg_settings->is_settings_valid = 1;
		offset = (uint32_t *)&csl_packet->payload_flex;
		offset += csl_packet->cmd_buf_offset / sizeof(uint32_t);
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);
		rc = cam_sensor_i2c_command_parser(
			&z_ctrl->io_master_info,
			i2c_reg_settings,
			cmd_desc, 1, NULL);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"manual move lens parsing failed: %d", rc);
			goto end;
		}

		rc = cam_zoom_update_req_mgr(z_ctrl, csl_packet);
		if (rc) {
			CAM_ERR(CAM_ZOOM,
				"Failed in adding request to request manager");
			goto end;
		}
		break;
	case CAM_ZOOM_PACKET_NOP_OPCODE:
		if (z_ctrl->cam_zoom_state < CAM_ZOOM_CONFIG) {
			CAM_WARN(CAM_ZOOM,
				"Received NOP packets in invalid state: %d",
				z_ctrl->cam_zoom_state);
			rc = -EINVAL;
			goto end;
		}
		rc = cam_zoom_update_req_mgr(z_ctrl, csl_packet);
		if (rc) {
			CAM_ERR(CAM_ZOOM,
				"Failed in adding request to request manager");
			goto end;
		}
		break;
	case CAM_ZOOM_PACKET_OPCODE_READ: {
		uint64_t qtime_ns;
		struct cam_buf_io_cfg *io_cfg;
		struct i2c_settings_array i2c_read_settings;

		if (z_ctrl->cam_zoom_state < CAM_ZOOM_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_ZOOM,
				"Not in right state to read zoom: %d",
				z_ctrl->cam_zoom_state);
			goto end;
		}
		CAM_DBG(CAM_ZOOM, "number of I/O configs: %d:",
			csl_packet->num_io_configs);
		if (csl_packet->num_io_configs == 0) {
			CAM_ERR(CAM_ZOOM, "No I/O configs to process");
			rc = -EINVAL;
			goto end;
		}

		INIT_LIST_HEAD(&(i2c_read_settings.list_head));

		io_cfg = (struct cam_buf_io_cfg *) ((uint8_t *)
			&csl_packet->payload_flex +
			csl_packet->io_configs_offset);

		if (io_cfg == NULL) {
			CAM_ERR(CAM_ZOOM, "I/O config is invalid(NULL)");
			rc = -EINVAL;
			goto end;
		}

		offset = (uint32_t *)&csl_packet->payload_flex;
		offset += (csl_packet->cmd_buf_offset / sizeof(uint32_t));
		cmd_desc = (struct cam_cmd_buf_desc *)(offset);
		i2c_read_settings.is_settings_valid = 1;
		i2c_read_settings.request_id = 0;
		rc = cam_sensor_i2c_command_parser(&z_ctrl->io_master_info,
			&i2c_read_settings,
			cmd_desc, 1, io_cfg);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"zoom read pkt parsing failed: %d", rc);
			goto end;
		}

		rc = cam_sensor_i2c_read_data(
			&i2c_read_settings,
			&z_ctrl->io_master_info);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM, "cannot read data, rc:%d", rc);
			delete_request(&i2c_read_settings);
			goto end;
		}

		if (csl_packet->num_io_configs > 1) {
			rc = cam_sensor_util_get_current_qtimer_ns(&qtime_ns);
			if (rc < 0) {
				CAM_ERR(CAM_SENSOR, "failed to get qtimer rc:%d");
				delete_request(&i2c_read_settings);
				return rc;
			}

			rc = cam_sensor_util_write_qtimer_to_io_buffer(
				qtime_ns, &io_cfg[1]);
			if (rc < 0) {
				CAM_ERR(CAM_ZOOM,
					"write qtimer failed rc: %d", rc);
				delete_request(&i2c_read_settings);
				return rc;
			}
		}

		rc = delete_request(&i2c_read_settings);
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM,
				"Failed in deleting the read settings,rc=%d", rc);
			goto end;
		}
		break;
	}
	default:
		CAM_ERR(CAM_ZOOM, "Wrong Opcode: %d",
			csl_packet->header.op_code & 0xFFFFFF);
		rc = -EINVAL;
		goto end;
	}

end:
	cam_common_mem_free(csl_packet);
put_buf:
	cam_mem_put_cpu_buf(config.packet_handle);
	return rc;
}

void cam_zoom_shutdown(struct cam_zoom_ctrl_t *z_ctrl)
{
	int rc = 0;
	struct cam_zoom_soc_private  *soc_private =
		(struct cam_zoom_soc_private *)z_ctrl->soc_info.soc_private;
	struct cam_sensor_power_ctrl_t *power_info =
		&soc_private->power_info;

	if (z_ctrl->cam_zoom_state == CAM_ZOOM_INIT)
		return;

	if (z_ctrl->cam_zoom_state >= CAM_ZOOM_CONFIG) {
		rc = cam_zoom_power_down(z_ctrl);
		if (rc < 0)
			CAM_ERR(CAM_ZOOM, "Zoom Power down failed");
		z_ctrl->cam_zoom_state = CAM_ZOOM_ACQUIRE;
	}

	if (z_ctrl->cam_zoom_state >= CAM_ZOOM_ACQUIRE) {
		rc = cam_destroy_device_hdl(z_ctrl->bridge_intf.device_hdl);
		if (rc < 0)
			CAM_ERR(CAM_ZOOM, "destroying  dhdl failed");
		z_ctrl->bridge_intf.device_hdl = -1;
		z_ctrl->bridge_intf.link_hdl = -1;
		z_ctrl->bridge_intf.session_hdl = -1;
	}

	CAM_MEM_FREE(power_info->power_setting);
	CAM_MEM_FREE(power_info->power_down_setting);
	power_info->power_setting = NULL;
	power_info->power_down_setting = NULL;
	power_info->power_setting_size = 0;
	power_info->power_down_setting_size = 0;
	z_ctrl->last_flush_req = 0;

	z_ctrl->cam_zoom_state = CAM_ZOOM_INIT;
}

int32_t cam_zoom_driver_cmd(struct cam_zoom_ctrl_t *z_ctrl,
	void *arg)
{
	int rc = 0;
	struct cam_control *cmd = (struct cam_control *)arg;
	struct cam_zoom_soc_private *soc_private = NULL;
	struct cam_sensor_power_ctrl_t  *power_info = NULL;

	if (!z_ctrl || !cmd) {
		CAM_ERR(CAM_ZOOM, "Invalid Args");
		return -EINVAL;
	}

	soc_private =
		(struct cam_zoom_soc_private *)z_ctrl->soc_info.soc_private;

	power_info = &soc_private->power_info;

	if (cmd->handle_type != CAM_HANDLE_USER_POINTER) {
		CAM_ERR(CAM_ZOOM, "Invalid handle type: %d",
			cmd->handle_type);
		return -EINVAL;
	}

	CAM_DBG(CAM_ZOOM, "Opcode to Zoom: %d", cmd->op_code);

	mutex_lock(&(z_ctrl->zoom_mutex));
	switch (cmd->op_code) {
	case CAM_ACQUIRE_DEV: {
		struct cam_sensor_acquire_dev zoom_acq_dev;
		struct cam_create_dev_hdl bridge_params;

		if (z_ctrl->bridge_intf.device_hdl != -1) {
			CAM_ERR(CAM_ZOOM, "Device is already acquired");
			rc = -EINVAL;
			goto release_mutex;
		}
		rc = copy_from_user(&zoom_acq_dev,
			u64_to_user_ptr(cmd->handle),
			sizeof(zoom_acq_dev));
		if (rc < 0) {
			CAM_ERR(CAM_ZOOM, "Failed Copying from user\n");
			goto release_mutex;
		}

		bridge_params.session_hdl = zoom_acq_dev.session_handle;
		bridge_params.ops = &z_ctrl->bridge_intf.ops;
		bridge_params.v4l2_sub_dev_flag = 0;
		bridge_params.media_entity_flag = 0;
		bridge_params.priv = z_ctrl;
		bridge_params.dev_id = CAM_ZOOM;

		zoom_acq_dev.device_handle =
			cam_create_device_hdl(&bridge_params);
		if (zoom_acq_dev.device_handle <= 0) {
			rc = -EFAULT;
			CAM_ERR(CAM_ZOOM, "Can not create device handle");
			goto release_mutex;
		}
		z_ctrl->bridge_intf.device_hdl = zoom_acq_dev.device_handle;
		z_ctrl->bridge_intf.session_hdl =
			zoom_acq_dev.session_handle;

		CAM_DBG(CAM_ZOOM, "Device Handle: %d",
			zoom_acq_dev.device_handle);
		if (copy_to_user(u64_to_user_ptr(cmd->handle),
			&zoom_acq_dev,
			sizeof(struct cam_sensor_acquire_dev))) {
			CAM_ERR(CAM_ZOOM, "Failed Copy to User");
			rc = -EFAULT;
			goto release_mutex;
		}

		z_ctrl->cam_zoom_state = CAM_ZOOM_ACQUIRE;
		/* xiaomi add begin */
		if (NULL != z_ctrl) {
			xm_cam_dev_set_init_result(get_zoom_xm_cam_dev_info(z_ctrl), XM_CAM_DEV_INIT_STATUS_SUCCESS);
		}
		/* xiaomi add end */
	}
		break;
	case CAM_RELEASE_DEV: {
		/* xiaomi add begin */
		if (NULL != z_ctrl) {
			xm_cam_dev_set_init_result(get_zoom_xm_cam_dev_info(z_ctrl), XM_CAM_DEV_INIT_STATUS_SUCCESS);
		}
		/* xiaomi add end */
		if (z_ctrl->cam_zoom_state == CAM_ZOOM_START) {
			rc = -EINVAL;
			CAM_WARN(CAM_ZOOM,
				"Cant release zoom: in start state");
			goto release_mutex;
		}

		if (z_ctrl->bridge_intf.device_hdl == -1) {
			CAM_ERR(CAM_ZOOM, "link hdl: %d device hdl: %d",
				z_ctrl->bridge_intf.device_hdl,
				z_ctrl->bridge_intf.link_hdl);
			rc = -EINVAL;
			goto release_mutex;
		}

		if (z_ctrl->cam_zoom_state == CAM_ZOOM_CONFIG) {
			rc = cam_zoom_power_down(z_ctrl);
			if (rc < 0) {
				CAM_ERR(CAM_ZOOM,
					"Zoom Power Down Failed");
				goto release_mutex;
			}
		}

		if (z_ctrl->bridge_intf.link_hdl != -1) {
			CAM_ERR(CAM_ZOOM,
				"Device [%d] still zoomive on link 0x%x",
				z_ctrl->cam_zoom_state,
				z_ctrl->bridge_intf.link_hdl);
			rc = -EAGAIN;
			goto release_mutex;
		}

		rc = cam_destroy_device_hdl(z_ctrl->bridge_intf.device_hdl);
		if (rc < 0)
			CAM_ERR(CAM_ZOOM, "destroying the device hdl");
		z_ctrl->bridge_intf.device_hdl = -1;
		z_ctrl->bridge_intf.link_hdl = -1;
		z_ctrl->bridge_intf.session_hdl = -1;
		z_ctrl->cam_zoom_state = CAM_ZOOM_INIT;
		z_ctrl->last_flush_req = 0;
			CAM_MEM_FREE(power_info->power_setting);
			CAM_MEM_FREE(power_info->power_down_setting);
			power_info->power_setting = NULL;
			power_info->power_down_setting = NULL;
			power_info->power_down_setting_size = 0;
			power_info->power_setting_size = 0;
	}
		break;
	case CAM_QUERY_CAP: {
		struct cam_zoom_query_cap zoom_cap = {0};

		zoom_cap.slot_info = z_ctrl->soc_info.index;
		if (copy_to_user(u64_to_user_ptr(cmd->handle),
			&zoom_cap,
			sizeof(struct cam_zoom_query_cap))) {
			CAM_ERR(CAM_ZOOM, "Failed Copy to User");
			rc = -EFAULT;
			goto release_mutex;
		}
	}
		break;
	case CAM_START_DEV: {
		if (z_ctrl->cam_zoom_state != CAM_ZOOM_CONFIG) {
			rc = -EINVAL;
			CAM_WARN(CAM_ZOOM,
			"Not in right state to start : %d",
			z_ctrl->cam_zoom_state);
			goto release_mutex;
		}
		z_ctrl->cam_zoom_state = CAM_ZOOM_START;
		z_ctrl->last_flush_req = 0;
	}
		break;
	case CAM_STOP_DEV: {
		struct i2c_settings_array *i2c_set = NULL;
		int i;

		if (z_ctrl->cam_zoom_state != CAM_ZOOM_START) {
			rc = -EINVAL;
			CAM_WARN(CAM_ZOOM,
			"Not in right state to stop : %d",
			z_ctrl->cam_zoom_state);
			goto release_mutex;
		}

		for (i = 0; i < MAX_PER_FRAME_ARRAY; i++) {
			i2c_set = &(z_ctrl->i2c_data.per_frame[i]);

			if (i2c_set->is_settings_valid == 1) {
				rc = delete_request(i2c_set);
				if (rc < 0)
					CAM_ERR(CAM_SENSOR,
						"delete request: %lld rc: %d",
						i2c_set->request_id, rc);
			}
		}
		z_ctrl->last_flush_req = 0;
		z_ctrl->cam_zoom_state = CAM_ZOOM_CONFIG;
	}
		break;
	case CAM_CONFIG_DEV: {
		z_ctrl->setting_apply_state =
			ZOOM_APPLY_SETTINGS_LATER;
		rc = cam_zoom_i2c_pkt_parse(z_ctrl, arg);
		if (rc < 0) {
			if (rc == -EBADR)
				CAM_INFO(CAM_ZOOM,
					"Failed in zoom Parsing, it has been flushed");
			else
				CAM_ERR(CAM_ZOOM,
					"Failed in zoom Parsing");
			goto release_mutex;
		}

		if (z_ctrl->setting_apply_state ==
			ZOOM_APPLY_SETTINGS_NOW) {
			rc = cam_zoom_apply_settings(z_ctrl,
				&z_ctrl->i2c_data.init_settings,
				XM_CMD_DEV_I2C_ZOOM_CMD_TYPE_INIT);
			if ((rc == -EAGAIN) &&
			(z_ctrl->io_master_info.master_type == CCI_MASTER)) {
				CAM_WARN(CAM_ZOOM,
					"CCI HW is in resetting mode:: Reapplying Init settings");
				usleep_range(1000, 1010);
				rc = cam_zoom_apply_settings(z_ctrl,
					&z_ctrl->i2c_data.init_settings,
					XM_CMD_DEV_I2C_ZOOM_CMD_TYPE_INIT);
			}

			if (rc < 0) {
				CAM_ERR(CAM_ZOOM,
					"Failed to apply Init settings: rc = %d",
					rc);
			}

			/* Delete the request even if the apply is failed */
			rc = delete_request(&z_ctrl->i2c_data.init_settings);
			if (rc < 0) {
				CAM_ERR(CAM_ZOOM,
					"Failed in Deleting the Init Pkt: %d",
					rc);
				goto release_mutex;
			}
		}
	}
		break;
	default:
		CAM_ERR(CAM_ZOOM, "Invalid Opcode %d", cmd->op_code);
	}

release_mutex:
	mutex_unlock(&(z_ctrl->zoom_mutex));

	return rc;
}

int32_t cam_zoom_flush_request(struct cam_req_mgr_flush_request *flush_req)
{
	int32_t rc = 0, i;
	uint32_t cancel_req_id_found = 0;
	struct cam_zoom_ctrl_t *z_ctrl = NULL;
	struct i2c_settings_array *i2c_set = NULL;

	if (!flush_req)
		return -EINVAL;

	z_ctrl = (struct cam_zoom_ctrl_t *)
		cam_get_device_priv(flush_req->dev_hdl);
	if (!z_ctrl) {
		CAM_ERR(CAM_ZOOM, "Device data is NULL");
		return -EINVAL;
	}

	if (z_ctrl->i2c_data.per_frame == NULL) {
		CAM_ERR(CAM_ZOOM, "i2c frame data is NULL");
		return -EINVAL;
	}

	mutex_lock(&(z_ctrl->zoom_mutex));
	if (flush_req->type == CAM_REQ_MGR_FLUSH_TYPE_ALL) {
		z_ctrl->last_flush_req = flush_req->req_id;
		CAM_DBG(CAM_ZOOM, "last reqest to flush is %lld",
			flush_req->req_id);
	}

	for (i = 0; i < MAX_PER_FRAME_ARRAY; i++) {
		i2c_set = &(z_ctrl->i2c_data.per_frame[i]);

		if ((flush_req->type == CAM_REQ_MGR_FLUSH_TYPE_CANCEL_REQ)
				&& (i2c_set->request_id != flush_req->req_id))
			continue;

		if (i2c_set->is_settings_valid == 1) {
			rc = delete_request(i2c_set);
			if (rc < 0)
				CAM_ERR(CAM_ZOOM,
					"delete request: %lld rc: %d",
					i2c_set->request_id, rc);

			if (flush_req->type ==
				CAM_REQ_MGR_FLUSH_TYPE_CANCEL_REQ) {
				cancel_req_id_found = 1;
				break;
			}
		}
	}

	if (flush_req->type == CAM_REQ_MGR_FLUSH_TYPE_CANCEL_REQ &&
		!cancel_req_id_found)
		CAM_DBG(CAM_ZOOM,
			"Flush request id:%lld not found in the pending list",
			flush_req->req_id);
	mutex_unlock(&(z_ctrl->zoom_mutex));
	return rc;
}
